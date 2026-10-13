#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CLI="${ARDUINO_CLI:-arduino-cli}"
extra=(--build-property "compiler.cpp.extra_flags=-ffile-prefix-map=$HOME=/build/home -fmacro-prefix-map=$HOME=/build/home" --build-property "compiler.c.extra_flags=-ffile-prefix-map=$HOME=/build/home -fmacro-prefix-map=$HOME=/build/home")
if [[ -n "${ARDUINO_CTAGS_PATH:-}" ]]; then extra+=(--build-property "runtime.tools.ctags.path=$ARDUINO_CTAGS_PATH"); fi
"$CLI" compile --clean --build-path "$ROOT/build/compile" --fqbn esp32:esp32:axiometa_genesis_mini:USBMode=default,CDCOnBoot=default "${extra[@]}" --output-dir "$ROOT/build/firmware" "$ROOT/firmware/GenesisMicro"
