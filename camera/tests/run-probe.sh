#!/system/bin/sh
# SPDX-License-Identifier: Apache-2.0
# Usage (root): run-probe.sh /data/local/tmp/probe.jar MainClass [arguments]
set -e
test "$#" -ge 2
export CLASSPATH="$1"
shift
# Use the installed framework's class list, including CM's extra jars.
export BOOTCLASSPATH="$(sed -n 's/^ *export BOOTCLASSPATH //p' /init.environ.rc)"
test -n "$BOOTCLASSPATH"
# Standalone root app_process otherwise inherits cpu:/ (SP_SYSTEM), which
# cannot be sent through CM11's MediaMetadataRetriever scheduling workaround.
# Change only this test process; exec and its threads inherit the app group.
if test -w /dev/cpuctl/apps/tasks; then
    echo $$ > /dev/cpuctl/apps/tasks
fi
exec app_process /system/bin "$@"
