# SPDX-License-Identifier: Apache-2.0
# A physical System partition is an LVM PV after conversion. Never write it
# from an OTA, even if a legacy recovery has exposed its familiar by-name link.
import common


def FullOTA_Assertions(info):
    system = info.info_dict['fstab']['/system'].device
    if system != '/dev/block/mapper/mione-system':
        raise ValueError('MiOne OTA System target must be the mione-system LVM device')
    size = int(info.info_dict['system_size'])
    if size <= 0:
        raise ValueError('MiOne OTA System size must be positive')
    # Use the checker built with this ROM, including its target-identity guard.
    # A previous LVM recovery may contain an older check-install implementation.
    common.ZipWriteStr(info.output_zip, 'mione-lvm-check',
                       info.input_zip.read('BOOT/RAMDISK/sbin/mione-lvm'))
    info.script.AppendExtra('assert(getprop("ro.mione.lvm.recovery") == "1" || abort("Use MiOne LVM Recovery; legacy raw-partition installs are unsafe."));')
    info.script.AppendExtra('ui_print("Checking active MiOne LVM layout, System target and capacity...");')
    info.script.AppendExtra('assert(package_extract_file("mione-lvm-check", "/tmp/mione-lvm-check") || abort("Cannot extract MiOne LVM checker."));')
    info.script.AppendExtra('set_metadata("/tmp/mione-lvm-check", "uid", 0, "gid", 0, "mode", 0755);')
    info.script.AppendExtra('assert(run_program("/tmp/mione-lvm-check", "check-install", "%d", "%s") == 0 || abort("MiOne LVM layout, System target or capacity check failed. See the recovery log; convert/resize in MiOne LVM Recovery if needed."));' % (size, system))


def FullOTA_InstallBegin(info):
    pass  # Preflight runs in Assertions, before backup, partition or boot writes.


def IncrementalOTA_Assertions(info):
    raise ValueError('MiOne LVM layout changes require a full OTA package')


def IncrementalOTA_InstallBegin(info):
    raise ValueError('MiOne LVM layout changes require a full OTA package')
