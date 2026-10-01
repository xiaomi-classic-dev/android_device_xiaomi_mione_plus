#!/usr/bin/env bash
set -euo pipefail
src_dir="$(cd "$(dirname "$0")" && pwd)"
root_dir="$(cd "$src_dir/../../../../.." && pwd)"
probe_out="${1:-$root_dir/out/diagnostics/mione-camera-probes}"
framework_jar="$root_dir/out/target/common/obj/JAVA_LIBRARIES/framework_intermediates/classes.jar"
: "${JAVA_HOME:?Set JAVA_HOME to JDK 6}"
javac_bin="$JAVA_HOME/bin/javac"
test -f "$framework_jar"
mkdir -p "$probe_out/camera-classes" "$probe_out/codec-classes" "$probe_out/diagnostic-classes"
"$javac_bin" -cp "$framework_jar" -d "$probe_out/camera-classes" "$src_dir/CameraProbe.java"
"$javac_bin" -cp "$framework_jar" -d "$probe_out/codec-classes" "$src_dir/CodecProbe.java"
"$root_dir/out/host/linux-x86/bin/dx" --dex --output="$probe_out/camera-probe.jar" "$probe_out/camera-classes"
"$root_dir/out/host/linux-x86/bin/dx" --dex --output="$probe_out/codec-probe.jar" "$probe_out/codec-classes"
"$javac_bin" -cp "$framework_jar" -d "$probe_out/diagnostic-classes" \
    "$src_dir/MetadataProbe.java" "$src_dir/SensorProbe.java"
"$root_dir/out/host/linux-x86/bin/dx" --dex --output="$probe_out/diagnostic-probe.jar" "$probe_out/diagnostic-classes"
printf 'Camera probe: %s\nCodec probe: %s\n' "$probe_out/camera-probe.jar" "$probe_out/codec-probe.jar"
