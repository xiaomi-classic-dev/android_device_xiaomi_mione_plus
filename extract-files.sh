#!/bin/sh

BASE=../../../vendor/xiaomi/mione_plus/proprietary
rm -rf $BASE/*

for FILE in `egrep -v '(^#|^$)' proprietary-files.txt`; do
  DIR=`dirname $FILE`
  if [ ! -d $BASE/$DIR ]; then
    mkdir -p $BASE/$DIR
  fi
  adb pull /system/$FILE $BASE/$FILE
done

python3 "$BASE/../tools/patch-isp-poll.py" "$BASE/lib/liboemcamera.so" || exit 1

./setup-makefiles.sh
