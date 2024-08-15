#!/bin/sh
#
# applypatch.sh
# apply patches for xiaomi mione
#
# Author: Alex.wang
# Create: 2013-04-08 05:27

dir=`cd $(dirname $0) && pwd`
top=$dir/../../../..

for patch in `ls $dir/*.patch` ; do
	patch -p1 --no-backup-if-mismatch -B /tmp/ -l -N -i $patch -r - -d $top
done

rm -vf $(find /tmp -type f -name "*.orig")

