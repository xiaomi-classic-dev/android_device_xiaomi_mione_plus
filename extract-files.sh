#!/bin/sh

set -eu
MY_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BASE="$MY_DIR/../../../vendor/xiaomi/mione_plus/proprietary"
SOURCE=${1:-adb}
STAGE=$(mktemp -d)
trap 'rm -rf "$STAGE"' EXIT HUP INT TERM

while IFS= read -r FILE; do
    case "$FILE" in ''|'#'*) continue ;; esac
    mkdir -p "$STAGE/$(dirname -- "$FILE")"
    if [ "$SOURCE" = adb ]; then
        adb -s "${ANDROID_SERIAL:-1374137e}" pull "/$FILE" "$STAGE/$FILE"
    else
        cp -p "$SOURCE/$FILE" "$STAGE/$FILE"
    fi
done < "$MY_DIR/proprietary-files.txt"

python3 "$BASE/../tools/patch-isp-poll.py" "$STAGE/vendor/lib/liboemcamera.so"
python3 "$BASE/../tools/patch-vendor-paths.py" "$STAGE"
cp -a "$STAGE/." "$BASE/"

cd "$MY_DIR"
./setup-makefiles.sh
