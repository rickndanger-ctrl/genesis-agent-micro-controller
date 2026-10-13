#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
APP="$ROOT/Genesis Agent Micro Controller.app"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
cp "$ROOT/Assets/Genesis.png" "$APP/Contents/Resources/Genesis.png"
if [ -f "$ROOT/Assets/Genesis.icns" ]; then cp "$ROOT/Assets/Genesis.icns" "$APP/Contents/Resources/Genesis.icns"; fi
xcrun swiftc -swift-version 5 -parse-as-library -O "$ROOT"/Sources/*.swift -o "$APP/Contents/MacOS/GenesisMapper" -framework SwiftUI -framework AppKit -framework ApplicationServices -framework IOKit
cat > "$APP/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><dict>
<key>CFBundleIconFile</key><string>Genesis</string><key>CFBundleExecutable</key><string>GenesisMapper</string><key>CFBundleIdentifier</key><string>io.github.rickndanger-ctrl.genesis-agent-controller</string><key>CFBundleName</key><string>Genesis Agent Micro Controller</string><key>CFBundlePackageType</key><string>APPL</string><key>CFBundleShortVersionString</key><string>0.4.0</string><key>LSMinimumSystemVersion</key><string>14.0</string><key>NSHighResolutionCapable</key><true/>
</dict></plist>
PLIST
codesign --force --sign "${GENESIS_SIGN_ID:--}" "$APP"
codesign --verify --deep --strict "$APP"
