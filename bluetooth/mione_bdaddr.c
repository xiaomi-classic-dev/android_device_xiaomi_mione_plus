/* Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "mione_bdaddr"
#include <cutils/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BDADDR_FILE "/data/misc/bluetooth/bdaddr"
#define NV_READ_F 0
#define NV_BD_ADDR_I 447

static void timed_out(int signal_number)
{
    static const char message[] = "mione_bdaddr: NV read timed out\n";
    (void)signal_number;
    (void)write(STDERR_FILENO, message, sizeof(message) - 1);
    _exit(124);
}

int main(void)
{
    union {
        uint32_t alignment;
        uint8_t address[6];
    } item = {0};
    static const uint8_t zero[6] = {0};
    static const uint8_t broadcast[6] = {255, 255, 255, 255, 255, 255};
    char address[18];
    const char temporary[] = BDADDR_FILE ".tmp";

    signal(SIGALRM, timed_out);
    alarm(20);
    void *rpc = dlopen("liboncrpc.so", RTLD_NOW | RTLD_GLOBAL);
    void *nv = rpc ? dlopen("libnv.so", RTLD_NOW | RTLD_GLOBAL) : NULL;
    if (!rpc || !nv) {
        ALOGE("Cannot load NV RPC libraries: %s", dlerror());
        return 1;
    }

    void (*rpc_init)(void) = dlsym(rpc, "oncrpc_init");
    void (*rpc_start)(void) = dlsym(rpc, "oncrpc_task_start");
    int (*nv_available)(void) = dlsym(nv, "nv_null");
    int (*nv_read)(int, int, void *) = dlsym(nv, "nv_cmd_remote");
    if (!rpc_init || !rpc_start || !nv_available || !nv_read) {
        ALOGE("Missing NV RPC entry point");
        return 1;
    }

    rpc_init();
    rpc_start();
    if (!nv_available()) {
        ALOGE("NV RPC service unavailable");
        return 1;
    }
    int result = nv_read(NV_READ_F, NV_BD_ADDR_I, &item);
    alarm(0);
    if (result != 0) {
        ALOGE("NV_BD_ADDR_I read failed: %d", result);
        return 1;
    }
    if (!memcmp(item.address, zero, sizeof(zero)) ||
        !memcmp(item.address, broadcast, sizeof(broadcast)) ||
        (item.address[5] & 1)) {
        ALOGE("Invalid Bluetooth address in NV_BD_ADDR_I");
        return 1;
    }

    /* NV and the stock patchram HCI command store the least significant byte first. */
    snprintf(address, sizeof(address), "%02X:%02X:%02X:%02X:%02X:%02X",
             item.address[5], item.address[4], item.address[3],
             item.address[2], item.address[1], item.address[0]);
    int fd = open(temporary, O_WRONLY | O_CREAT | O_TRUNC | O_NOFOLLOW | O_CLOEXEC, 0640);
    if (fd < 0) {
        ALOGE("Cannot open address file: %s", strerror(errno));
        return 1;
    }
    ssize_t count = write(fd, address, sizeof(address) - 1);
    int failed = count != (ssize_t)(sizeof(address) - 1);
    if (!failed && fsync(fd) != 0) failed = 1;
    if (close(fd) != 0) failed = 1;
    if (!failed && rename(temporary, BDADDR_FILE) != 0) failed = 1;
    if (failed) {
        ALOGE("Cannot save address file: %s", strerror(errno));
        unlink(temporary);
        return 1;
    }
    ALOGI("Factory Bluetooth address from NV_BD_ADDR_I: %s", address);
    return 0;
}
