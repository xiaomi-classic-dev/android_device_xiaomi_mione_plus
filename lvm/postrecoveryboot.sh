#!/sbin/sh
# SPDX-License-Identifier: Apache-2.0
# TWRP runs this after fstab processing and before loading its settings.
# Cache survives the device's System/Data LV recreation.

if ! grep -q ' /cache ' /proc/mounts; then
    mount /cache || exit 1
fi
# The bootstrap used a temporary /dev that stock init has now replaced.
# Recreate only verified active LV aliases in the final tmpfs, before the
# existing-preferences early return. This does not activate or resize volumes.
/sbin/mione-lvm link-active || exit 1
mkdir -p /cache/TWRP || exit 1

if [ -e /cache/TWRP/.twrps ]; then
    [ -f /cache/TWRP/.twrps ] && [ ! -L /cache/TWRP/.twrps ]
    exit $?
fi

# Migrate an existing choice once. Fresh preferences select internal storage,
# while TWRP's native settings loader/writer keeps its backing file in Cache.
settings=/etc/mione-twrp-defaults
for previous in /data/media/0/TWRP/.twrps /data/media/TWRP/.twrps /external_sd/TWRP/.twrps; do
    if [ -f "$previous" ] && [ ! -L "$previous" ]; then
        settings=$previous
        break
    fi
done
cp "$settings" /cache/TWRP/.twrps.tmp || exit 1
chmod 0600 /cache/TWRP/.twrps.tmp || exit 1
mv /cache/TWRP/.twrps.tmp /cache/TWRP/.twrps || exit 1
sync
echo 'MiOne TWRP: preferences stored in /cache/TWRP/.twrps'
