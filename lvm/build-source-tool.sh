#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Invoke from a systemd build unit with the Android tree's memory limits.
set -euo pipefail
src=$(cd "$(dirname "$0")" && pwd)
tree=${ANDROID_TREE:-$(cd "$src/../../../.." && pwd)}
work=${1:?supply an empty work directory}
mkdir -p "$work"
work=$(cd "$work" && pwd)
tar xf "$src/musl-1.2.5.tar.gz" -C "$work"
tar xf "$src/lvm2-ea6a8078-src.tar.gz" -C "$work"
patch -d "$work/lvm2-src" -p1 < "$src/musl-port.patch"
prefix=$work/toolchain
cross=$tree/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin/arm-linux-androideabi-
export LC_ALL=C
cd "$work/musl-1.2.5"
CC=${cross}gcc AR=${cross}ar RANLIB=${cross}ranlib ./configure --prefix="$prefix" --target=arm-linux-musleabi --disable-shared --enable-wrapper=gcc
make -j16
make install
# Use exported ARM Linux 3.4 headers; do not overwrite libc headers.
kernel=${MIONE_KERNEL_SOURCE:?set path to MiOne's Linux 3.4 source}
make -C "$kernel" ARCH=arm INSTALL_HDR_PATH="$work/kernel-headers" headers_install
cp -a "$work/kernel-headers/include/asm" "$work/kernel-headers/include/asm-generic" "$work/kernel-headers/include/linux" "$prefix/include/"
cd "$work/lvm2-src"
ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes CC="$prefix/bin/musl-gcc" AR=${cross}ar RANLIB=${cross}ranlib ./configure --host=arm-linux-gnueabi --prefix=/lvm --enable-static_link --disable-readline --disable-selinux --disable-udev_sync --disable-udev_rules --disable-nls --disable-shared --with-pool=none --with-cluster=none --with-confdir=/lvm/etc --with-default-dm-run-dir=/dev/mione-lvm/run --with-default-run-dir=/dev/mione-lvm/run --with-default-system-dir=/lvm/etc --with-default-locking-dir=/dev/mione-lvm/lock --with-optimisation='-Os -march=armv7-a -mfloat-abi=soft' LDFLAGS=-static CFLAGS='-DMIONE_MUSL -include sys/sysmacros.h'
make -C include CC="$prefix/bin/musl-gcc" MKDIR_P="mkdir -p"
make -C libdm -j16 ioctl/libdevmapper.a CC="$prefix/bin/musl-gcc" MKDIR_P="mkdir -p"
make -C libdaemon/client -j16 libdaemonclient.a CC="$prefix/bin/musl-gcc" MKDIR_P="mkdir -p"
make -C lib -j16 liblvm-internal.a CC="$prefix/bin/musl-gcc" MKDIR_P="mkdir -p"
make -C tools -j16 lvm.static CC="$prefix/bin/musl-gcc" MKDIR_P="mkdir -p"
${cross}strip tools/lvm.static
printf 'Built: %s\n' "$work/lvm2-src/tools/lvm.static"
