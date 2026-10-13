#include <EGL/egl.h>
#include <android/log.h>
#include <errno.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <system/window.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <vndk/hardware_buffer.h>

#define TAG "mione-egl-compat"
#define MAX_WINDOWS 64
#define DEQUEUE_TIMEOUT_NS 50000000LL

typedef void (*EglProc)(void);
typedef EGLSurface (*CreateWindow)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *);
typedef EGLBoolean (*DestroySurface)(EGLDisplay, EGLSurface);
typedef EGLBoolean (*MakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
typedef int (*LegacyDequeue)(ANativeWindow *, ANativeWindowBuffer **);
typedef int (*ModernDequeue)(ANativeWindow *, ANativeWindowBuffer **, int *);
typedef int (*LegacyReturn)(ANativeWindow *, ANativeWindowBuffer *);
typedef int (*ModernReturn)(ANativeWindow *, ANativeWindowBuffer *, int);

struct ScratchBuffer {
    ANativeWindowBuffer native;
    AHardwareBuffer *hardware;
    atomic_int refs;
};

struct WindowState {
    ANativeWindow *window;
    EGLSurface surface;
    LegacyDequeue legacy;
    ModernDequeue modern;
    LegacyReturn legacy_queue, legacy_cancel;
    ModernReturn queue, cancel;
    struct ScratchBuffer *scratch;
    ANativeWindowBuffer *fallback;
    pid_t owner;
    bool pending_destroy;
};

static pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
static struct WindowState windows[MAX_WINDOWS];
static CreateWindow create_next;
static DestroySurface destroy_next;
static MakeCurrent make_next;
static EGLSurface (*current_surface)(EGLint);

static void scratch_acquire(android_native_base_t *base) {
    struct ScratchBuffer *buffer = (struct ScratchBuffer *)base;
    atomic_fetch_add_explicit(&buffer->refs, 1, memory_order_relaxed);
}

static void scratch_release(android_native_base_t *base) {
    struct ScratchBuffer *buffer = (struct ScratchBuffer *)base;
    if (atomic_fetch_sub_explicit(&buffer->refs, 1, memory_order_acq_rel) == 1) {
        AHardwareBuffer_release(buffer->hardware);
        free(buffer);
    }
}

static int wait_fence(int fd) {
    if (fd < 0) return 0;
    struct pollfd fence = {.fd = fd, .events = POLLIN};
    int result;
    do {
        result = poll(&fence, 1, -1);
    } while (result < 0 && errno == EINTR);
    const int error = result < 0 ? errno :
            ((fence.revents & (POLLERR | POLLNVAL | POLLHUP)) ? EINVAL : 0);
    close(fd);
    return -error;
}

static int return_buffer(ANativeWindow *window, ANativeWindowBuffer *buffer,
                         int fence_fd, bool cancel) {
    ModernReturn operation = NULL;
    ANativeWindowBuffer *original = buffer;
    pthread_mutex_lock(&state_mutex);
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        struct WindowState *state = &windows[i];
        if (state->window != window) continue;
        operation = cancel ? state->cancel : state->queue;
        if (state->scratch && buffer == &state->scratch->native) original = state->fallback;
        break;
    }
    pthread_mutex_unlock(&state_mutex);
    if (!operation) {
        if (fence_fd >= 0) close(fence_fd);
        return -EINVAL;
    }
    if (original != buffer) {
        /* HWUI drops its fallback buffer. Finish our GPU writes before reuse. */
        int result = wait_fence(fence_fd);
        int returned = operation(window, original, -1);
        return result != 0 ? result : returned;
    }
    return operation(window, buffer, fence_fd);
}

static int queue_buffer(ANativeWindow *w, ANativeWindowBuffer *b, int fd) {
    return return_buffer(w, b, fd, false);
}
static int cancel_buffer(ANativeWindow *w, ANativeWindowBuffer *b, int fd) {
    return return_buffer(w, b, fd, true);
}
static int legacy_queue(ANativeWindow *w, ANativeWindowBuffer *b) {
    return queue_buffer(w, b, -1);
}
static int legacy_cancel(ANativeWindow *w, ANativeWindowBuffer *b) {
    return cancel_buffer(w, b, -1);
}

static int resize_fallback(ANativeWindow *window, ANativeWindowBuffer **buffer) {
    if ((*buffer)->width != 1 || (*buffer)->height != 1) return 0;
    int width = 0, height = 0;
    if (window->query(window, NATIVE_WINDOW_WIDTH, &width) != 0 ||
            window->query(window, NATIVE_WINDOW_HEIGHT, &height) != 0 ||
            width <= 0 || height <= 0) return -EINVAL;
    if (width == 1 && height == 1) return 0;

    pthread_mutex_lock(&state_mutex);
    struct WindowState *state = NULL;
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        if (windows[i].window == window) { state = &windows[i]; break; }
    }
    if (!state) { pthread_mutex_unlock(&state_mutex); return -EINVAL; }
    if (!state->scratch || state->scratch->native.width != width ||
            state->scratch->native.height != height ||
            state->scratch->native.format != (*buffer)->format) {
        const AHardwareBuffer_Desc desc = {
            .width = width, .height = height, .layers = 1, .format = (*buffer)->format,
            .usage = AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE,
        };
        struct ScratchBuffer *scratch = calloc(1, sizeof(*scratch));
        int result = scratch ? AHardwareBuffer_allocate(&desc, &scratch->hardware) : -ENOMEM;
        if (result != 0) {
            free(scratch);
            pthread_mutex_unlock(&state_mutex);
            return result < 0 ? result : -result;
        }
        AHardwareBuffer_Desc actual;
        AHardwareBuffer_describe(scratch->hardware, &actual);
        scratch->native = (ANativeWindowBuffer){
            .common = {.magic = ANDROID_NATIVE_BUFFER_MAGIC, .version = sizeof(ANativeWindowBuffer),
                       .incRef = scratch_acquire, .decRef = scratch_release},
            .width = actual.width, .height = actual.height, .stride = actual.stride,
            .format = actual.format, .usage_deprecated = actual.usage, .usage = actual.usage,
            .layerCount = 1, .handle = AHardwareBuffer_getNativeHandle(scratch->hardware),
        };
        atomic_init(&scratch->refs, 1);
        if (state->scratch) scratch_release(&state->scratch->native.common);
        state->scratch = scratch;
        __android_log_print(ANDROID_LOG_INFO, TAG, "fallback target %dx%d format=%d",
                            width, height, scratch->native.format);
    }
    /* Old A220 rendering assumes window dimensions even for HWUI's 1x1
     * fallback. Give it a full-size target, then return the original marker
     * to HWUI's queue/cancel interceptors so this frame remains undisplayed. */
    state->fallback = *buffer;
    *buffer = &state->scratch->native;
    pthread_mutex_unlock(&state_mutex);
    return 0;
}

static int legacy_dequeue(ANativeWindow *window, ANativeWindowBuffer **buffer) {
    ModernDequeue dequeue = NULL;
    pthread_mutex_lock(&state_mutex);
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        if (windows[i].window == window) {
            dequeue = windows[i].modern;
            break;
        }
    }
    pthread_mutex_unlock(&state_mutex);
    if (!dequeue) return -EINVAL;

    int fence_fd = -1;
    int result = dequeue(window, buffer, &fence_fd);
    if (result != 0) return result;
    /* Preserve the synchronous fence contract of the deprecated API. */
    result = wait_fence(fence_fd);
    if (result == 0) result = resize_fallback(window, buffer);
    if (result != 0) {
        window->cancelBuffer(window, *buffer, -1);
        *buffer = NULL;
    }
    return result;
}

static void retire_windows(EGLSurface draw, EGLSurface read) {
    ANativeWindow *released[MAX_WINDOWS];
    struct ScratchBuffer *scratch[MAX_WINDOWS];
    unsigned count = 0;
    const pid_t owner = gettid();
    pthread_mutex_lock(&state_mutex);
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        struct WindowState *state = &windows[i];
        if (!state->window || !state->pending_destroy || state->owner != owner ||
                state->surface == draw || state->surface == read) continue;
        if (state->window->dequeueBuffer_DEPRECATED == legacy_dequeue) {
            state->window->dequeueBuffer_DEPRECATED = state->legacy;
        }
        if (state->window->queueBuffer == queue_buffer) state->window->queueBuffer = state->queue;
        if (state->window->cancelBuffer == cancel_buffer) state->window->cancelBuffer = state->cancel;
        if (state->window->queueBuffer_DEPRECATED == legacy_queue)
            state->window->queueBuffer_DEPRECATED = state->legacy_queue;
        if (state->window->cancelBuffer_DEPRECATED == legacy_cancel)
            state->window->cancelBuffer_DEPRECATED = state->legacy_cancel;
        released[count] = state->window;
        scratch[count++] = state->scratch;
        memset(state, 0, sizeof(*state));
    }
    pthread_mutex_unlock(&state_mutex);
    for (unsigned i = 0; i < count; ++i) {
        if (scratch[i]) scratch_release(&scratch[i]->native.common);
        released[i]->common.decRef(&released[i]->common);
    }
}

static EGLSurface create_window(EGLDisplay display, EGLConfig config,
                                 EGLNativeWindowType native_window, const EGLint *attributes) {
    EGLSurface surface = create_next(display, config, native_window, attributes);
    if (surface == EGL_NO_SURFACE) return surface;

    /* Only HWUI windows need its ReliableSurface recovery and bounded waits. */
    char thread_name[16];
    if (prctl(PR_GET_NAME, thread_name, 0, 0, 0) != 0 ||
            strcmp(thread_name, "RenderThread") != 0) return surface;
    ANativeWindow *window = native_window;
    if (!window || window->common.magic != ANDROID_NATIVE_WINDOW_MAGIC ||
            !window->dequeueBuffer_DEPRECATED || !window->dequeueBuffer || !window->query ||
            !window->queueBuffer || !window->cancelBuffer) return surface;

    pthread_mutex_lock(&state_mutex);
    struct WindowState *free_state = NULL;
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        if (windows[i].window == window) {
            pthread_mutex_unlock(&state_mutex);
            return surface;
        }
        if (!windows[i].window && !free_state) free_state = &windows[i];
    }
    if (free_state) {
        window->common.incRef(&window->common);
        *free_state = (struct WindowState){
            .window = window, .surface = surface,
            .legacy = window->dequeueBuffer_DEPRECATED, .modern = window->dequeueBuffer,
            .legacy_queue = window->queueBuffer_DEPRECATED,
            .legacy_cancel = window->cancelBuffer_DEPRECATED,
            .queue = window->queueBuffer, .cancel = window->cancelBuffer,
            .owner = gettid(), .pending_destroy = false,
        };
        window->dequeueBuffer_DEPRECATED = legacy_dequeue;
        window->queueBuffer = queue_buffer;
        window->cancelBuffer = cancel_buffer;
        window->queueBuffer_DEPRECATED = legacy_queue;
        window->cancelBuffer_DEPRECATED = legacy_cancel;
    }
    pthread_mutex_unlock(&state_mutex);
    if (free_state) {
        /* The old driver maps a dequeue timeout to GL_OUT_OF_MEMORY. Routing
         * through HWUI's modern interceptor allows scratch-buffer recovery.
         * Avoid holding the UI thread for its normal four-second timeout. */
        int result = window->perform(window, NATIVE_WINDOW_SET_DEQUEUE_TIMEOUT,
                                     DEQUEUE_TIMEOUT_NS);
        if (result != 0) {
            __android_log_print(ANDROID_LOG_WARN, TAG, "set dequeue timeout failed: %d", result);
        }
    }
    return surface;
}

static EGLBoolean destroy_surface(EGLDisplay display, EGLSurface surface) {
    EGLBoolean result = destroy_next(display, surface);
    if (result == EGL_FALSE) return result;
    pthread_mutex_lock(&state_mutex);
    for (unsigned i = 0; i < MAX_WINDOWS; ++i) {
        if (windows[i].window && windows[i].surface == surface) {
            windows[i].pending_destroy = true;
        }
    }
    pthread_mutex_unlock(&state_mutex);
    /* EGL permits deferred destruction of a current surface. */
    retire_windows(current_surface(EGL_DRAW), current_surface(EGL_READ));
    return result;
}

static EGLBoolean make_current(EGLDisplay display, EGLSurface draw, EGLSurface read,
                               EGLContext context) {
    EGLBoolean result = make_next(display, draw, read, context);
    if (result != EGL_FALSE) retire_windows(draw, read);
    return result;
}

__attribute__((visibility("default")))
void AndroidGLESLayer_Initialize(void *layer, EglProc (*resolve)(void *, const char *)) {
    current_surface = (EGLSurface (*)(EGLint))resolve(layer, "eglGetCurrentSurface");
}

__attribute__((visibility("default")))
EglProc AndroidGLESLayer_GetProcAddress(const char *name, EglProc next) {
    if (!strcmp(name, "eglCreateWindowSurface")) {
        create_next = (CreateWindow)next;
        return (EglProc)create_window;
    }
    if (!strcmp(name, "eglDestroySurface")) {
        destroy_next = (DestroySurface)next;
        return (EglProc)destroy_surface;
    }
    if (!strcmp(name, "eglMakeCurrent")) {
        make_next = (MakeCurrent)next;
        return (EglProc)make_current;
    }
    return next;
}
