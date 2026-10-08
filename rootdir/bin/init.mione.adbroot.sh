#!/system/bin/sh
# Keep a saved user choice; seed the userdebug default before adb_root starts.
if [ ! -e /data/adbroot/enabled ]; then
    mkdir -p /data/adbroot || exit 1
    printf '1\n' > /data/adbroot/enabled || exit 1
    chown system:system /data/adbroot/enabled
    chmod 0600 /data/adbroot/enabled
fi

if [ "$(cat /data/adbroot/enabled)" = 1 ]; then
    setprop service.adb.root 1
else
    setprop service.adb.root 0
fi
