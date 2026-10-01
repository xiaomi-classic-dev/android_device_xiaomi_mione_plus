MiOne MSM8660 Power HAL (CM11 / Android 4.4)
==========================================

Build: lunch lineage_mione_plus-userdebug; mka power.msm8660
Install: /system/lib/hw/power.msm8660.so
Log tag: MionePowerHAL

The HAL loader selects this module using ro.board.platform=msm8660.
Adding the module to PRODUCT_PACKAGES also includes it in future ROM builds.
PowerManagerService loads it once; restart zygote after replacing the module.

Policy:
  * Non-interactive: cap each online CPU at 918000 kHz, as in the original
    MiOne MIUI Power HAL. Preserve any existing lower cap.
  * Interactive: restore the pre-sleep cap, bounded by cpuinfo_max_freq.
    Restore only if the current cap still matches the cap this HAL applied.
    Preserve a changed cap from thermald or init. Combine the screen cap with
    sys.mione.battery_low=1 (CPU0 972000, CPU1 594000 kHz).
  * libhealthd.mione uses healthd's board callback to publish this boolean.
    Present battery at <=10% enters low policy; >=15% releases it. Invalid or
    absent-battery readings keep the previous state. Connecting USB does not
    release the policy until the battery level recovers. SOC is never changed.
    The Power HAL waits for property notifications using a futex; a battery
    policy change applies to an already awake/asleep phone immediately.
  * Interaction / positive CM CPU_BOOST hint: write an interactive governor
    boostpulse, at most once per 100 ms, only while interactive. This kernel
    has no boostpulse_duration; a duration hint becomes a one-shot pulse.
  * Leave governor, CPU hotplug, GPU, suspend and thermal services alone.
    Other power hints are ignored. There is no polling timer or wake lock.

Integration:
  BOARD_HAL_STATIC_LIBRARIES selects libhealthd.mione through the standard
  healthd board API; no generic healthd/framework source modification is needed.
  /sbin/healthd is in the ramdisk. A live replacement lasts only until reboot;
  include the newly linked healthd in the boot ramdisk for persistence.
  Do not publish the old sys.mione.battlevel property: old ramdisks attach
  unsafe thermald-stop and hard-coded frequency actions to it. Those actions
  have been removed from this device's init.mione.rc.

Required access for system_server (uid system):
  /sys/devices/system/cpu/cpu{0,1}/cpufreq/scaling_max_freq: writable
  /sys/devices/system/cpu/cpufreq/interactive/boostpulse: writable
The current kernel provides the CPU max permissions; init.qcom.post_boot.sh
grants boostpulse to system. An offline CPU is skipped, and pending restores
are retried on the next interactive notification or interaction hint.

Limitations:
  thermald and init share scaling_max_freq with the HAL. Ownership checking
  prevents a routine wake transition from overwriting a different cap, but
  userspace cannot atomically arbitrate concurrent writes, detect an external
  write of the identical value, or enforce the screen-off cap after a later
  thermald write. Complete arbitration needs separate kernel limit votes.
  This policy does not calibrate MAX17043 or establish the cause of spontaneous
  resets. The watcher uses Bionic's CM11 private property-wait interface; adapt
  this device-local implementation when moving to a newer Android release.

Device test:
  Build mione_power_probe, run as root with the phone awake/cool and zygote and
  healthd temporarily stopped. Keep thermald and watchdog running. The probe
  links the real board callback, loads the real installed Power HAL, exercises
  hysteresis/screen transitions/external lower caps, and restores original caps.
  It changes only the policy boolean, not the BatteryService or fuel gauge SOC.
