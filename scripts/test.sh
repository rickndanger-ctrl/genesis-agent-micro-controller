#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p "$ROOT/build/tests"
for file in "$ROOT"/tests/firmware/*.cpp; do
 name="$(basename "$file" .cpp)"
 clang++ -std=c++17 -Wall -Wextra -Werror "$file" -o "$ROOT/build/tests/$name"
 "$ROOT/build/tests/$name"
done
xcrun swiftc "$ROOT/macos/Sources/CodexSlotTypes.swift" "$ROOT/macos/tests/main.swift" -o "$ROOT/build/tests/slot-types"
"$ROOT/build/tests/slot-types"
python3 -m unittest discover -s "$ROOT/tests" -p 'test_*.py'
