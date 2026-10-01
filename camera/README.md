# MiOne Plus camera HAL1

This is the MiOne-specific camera implementation. It is deliberately built from
`device/xiaomi/mione_plus/camera`, selected by `BOARD_MIONE_SOURCE_CAMERA` and
`USE_DEVICE_SPECIFIC_CAMERA` in the device's `BoardConfig.mk`.
The shared MSM8660 device tree skips its original wrapper while this is enabled.
The generic `hardware/qcom/camera` tree needs no changes.

## Source baseline

The CAF camera release `M8260AAABQNLZA313065` matches the Qualcomm release used
by the public MiCode `mi1` kernel. Camera commit:
`b0d4d5aa4c3cbe94f7c43eb4c6d4a57f7d02e7c7`; public kernel commit:
`d6b55280bc8c1667f5d12538a148bb1edf441ecf`.
This identifies the public release baseline, not Xiaomi's unpublished changes.
See `source-provenance.json` for the donor mirror and missing SDK declarations.

The later CM 10.1 camera protocol was rejected. Only missing SDK declarations
were taken from it; the stock command numbers and wire layouts are retained.
Existing proprietary camera, sensor and JPEG/OMX libraries remain dependencies.

## MiOne integration

- Build `camera.msm8660.so` and the isolated `libmmcamera_mione.so`. Do not replace
  the proprietary `libmmcamera_interface2.so` or `camera.vendor.msm8660.so`.
- Use local SDK headers and the current MiOne kernel's generated UAPI headers;
  no camera headers are exported to other devices.
- Preserve the stock 32-bit ABI: `mm_camera_t` 68 bytes, `cam_prop_t` 100,
  `cam_ctrl_dimension_t` 420, focus distances 12 and FPS range 8. Camera position
  and mount-angle offsets are 24 and 28. `tests/abi.c` checks these at build time.
- The current kernel's `msm_frame` is 112 bytes. The historical MIUI binary used
  108; the source HAL must use the current kernel layout, not that binary layout.
- Adapt ION allocation to `heap_mask`, use the CP multimedia heap with the
  camera-heap fallback, and load `libmmstillomx` dynamically.
- Focal-length and view-angle queries transfer 12 bytes on this protocol. Use a
  12-byte temporary buffer before copying the scalar float into the HAL member;
  submitting the 4-byte member directly corrupts adjacent memory.
- Advertise the real 3264x2448 maximum photo size. Full-resolution live snapshot
  during video remains disabled because that path timed out on this device.
- Export the HAL entry point and the required interface functions only.

### Media framework integration

The KitKat media framework needs an independent lifecycle fix in
`frameworks/av/media/libstagefright/omx/OMXNodeInstance.cpp`.
KitKat `freeNode()` deletes its own instance.
The existing security backport kept `Mutex::Autolock` alive across `delete this`,
then unlocked a mutex in freed memory. End the lock scope before deletion while
retaining the lock around component destruction and node invalidation.
Do not enable `BOARD_SKIP_CVE_2017_13154` as a workaround.

The camera build does not inject, patch or replace framework sources. There is
no MiOne-specific OMX hook, duplicate framework implementation or private OMX
API. The lifecycle fix belongs in the framework that owns the object and mutex;
it also applies to independent MediaCodec use and other devices with the same
backport. It is maintained as a single framework change, separately from this
HAL. When upgrading Android, check the new framework's lifetime rules and keep
this change only if the defect is still present. If upstream already fixes it,
drop the old change without modifying the camera or device configuration.

Upstream comparison (checked against pinned sources on 2026-10-01):

- CM 12.1 and the `lineageos-lollipop` CM 12.1 fork still use raw node pointers
  and `delete this`; their inspected `freeNode()` does not contain this later
  teardown lock backport.
- CM 13.0 and CM 14.1 branch tips contain the teardown lock across `delete this`.
  CM 13.0 also offers `SKIP_CVE_2017_13154`; disabling it is an opt-out, not a
  repair of the lock's lifetime.
- Lineage 15.1 uses `BnOMXNode` and `sp<OMXNodeInstance>` ownership, and no longer
  deletes the node inside `freeNode()`. The inspected Lineage 15.1 and 18.1 tips
  also guard repeated teardown with an atomic `mDying` transition.
- The AOSP ownership refactor is `d59b97223424a3974d2ac31cff998d02eecf2eed`
  (18 files, including IOMX, ACodec and OMXClient). Its interface changes are
  broader than a compatible KitKat fix.

Keep stock camera/daemon/kernel wire layouts local to this HAL. Rebuild and
adapt the Android-facing HAL API when moving to another Android release; do
not carry the old framework patch automatically or inject source during builds.
The current change repairs the confirmed freed-mutex access. Repeated encode
and camera tests are not a complete audit of concurrent node destruction.
Pinned source files, commit diffs and comparison results are archived under
`out/diagnostics/mione-camera-port-20261001/research-omx`; see that diagnostic
directory's `analysis.md` for source links and the migration decision.

## Build

Use this checkout's Android build environment (Python 2.7 and JDK 6):

```sh
source build/envsetup.sh
lunch lineage_mione_plus-userdebug
make -j8 ONE_SHOT_MAKEFILE=device/xiaomi/mione_plus/camera/Android.mk \
    camera.msm8660 libmmcamera_mione mione_camera_abi
make -j8 ONE_SHOT_MAKEFILE=frameworks/av/media/libstagefright/omx/Android.mk \
    libstagefright_omx
```

## Device tests and deployment

`tests/CameraProbe.java` exercises preview, autofocus, JPEG, preview restart and
video (optionally with audio) through the Android Camera API.
`tests/CodecProbe.java` exercises repeated H.264 encode/release cycles separately.
Run `tests/build-probes.sh` with `JAVA_HOME` pointing to JDK 6 to compile with the
checkout's framework classes and package the classes with `dx`.
`tests/run-probe.sh` reads the installed BOOTCLASSPATH and puts only the test
process in the application CPU group before exec. Use this runner for root
app_process tests: otherwise MediaMetadataRetriever inherits SP_SYSTEM and
CM11's scheduling workaround logs policy=2 errors unrelated to a normal app.
`MetadataProbe` checks recorded-video metadata; `SensorProbe` holds a bounded
display wake lock while sampling the five sensors (HMC5883L early-suspends
with the display). These probes do not measure physical calibration accuracy.
Example after pushing the probe JARs and runner:

```sh
adb -s 1374137e shell 'su -c "sh /data/local/tmp/run-probe.sh /data/local/tmp/mione-camera-probe.jar CameraProbe /data/local/tmp/mione-photo photo 8mp"'
adb -s 1374137e shell 'su -c "sh /data/local/tmp/run-probe.sh /data/local/tmp/mione-camera-probe.jar CameraProbe /data/local/tmp/mione-video video 720p-audio"'
adb -s 1374137e shell 'su -c "sh /data/local/tmp/run-probe.sh /data/local/tmp/mione-codec-probe.jar CodecProbe 5"'
```

Before deployment, close camera clients and back up the installed libraries.
Install only the two camera libraries and the framework fix, restore root
ownership, mode 0644 and `system_file` SELinux context, then restart the media
service. Read the installed binaries back and compare SHA-256. Restore `/system`
read-only. No phone reboot or kernel flash is required.

Hot replacement of `libstagefright_omx.so` leaves the old inode mapped in Zygote
and existing apps. On this kernel, unlinked open inodes can make a whole-filesystem
read-only remount return `EBUSY`. The deployment helpers fall back to BusyBox
`mount -o remount,bind,ro /system`: this makes the existing `/system` mount
read-only without stopping those apps. Mountinfo still shows the backing ext4
superblock as writable; do not confuse these two flags. Verify `/proc/mounts`
and `/proc/self/mountinfo` after deployment rather than trusting the shell exit.

The 2026-10-01 deployment scripts, original binaries, readback hashes and test
artifacts are in `out/diagnostics/mione-camera-port-20261001`. The deployment
scripts restrict ADB to serial `1374137e` and verify the expected boot ID.
Rollback scripts restore the original HAL wrapper and media framework library;
the isolated interface library is unused by the original wrapper.

## Verified and remaining coverage

After the OMX fix, one unchanged media-service PID passed five encode/release
cycles (300 input frames), 8 MP autofocus/JPEG/preview restart, ZSL JPEG,
VGA video, and two 720p H.264 + AAC audio recordings. All saved media decoded
successfully on the host. The illuminated scene produced real image detail and
autofocus reported success.

Observed video rate was approximately 25 fps even when 30 was requested; 30 fps
has not been verified. HDR, RAW, face detection, all lighting/focus conditions and
long-duration recordings are not covered. The regular camera application's
foreground UI test was blocked by the lock screen. These tests do not establish
that the independent random kernel reboot problem has been eliminated.
