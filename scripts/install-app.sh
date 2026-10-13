#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT/Genesis Agent Micro Controller.app"
[[ -d "$APP" ]] || { echo 'Extract the complete release ZIP first; the app bundle is missing.' >&2; exit 1; }
DEST="$HOME/Applications/Genesis Agent Micro Controller.app"
[[ ! -e "$DEST" ]] || { echo 'An app already exists at the destination. Quit it and back it up before replacing it.' >&2; exit 1; }
mkdir -p "$HOME/Applications"
/usr/bin/ditto "$APP" "$DEST"
/usr/bin/codesign --verify --deep --strict "$DEST"
echo "Installed: $DEST"
echo 'Open the app from Finder; approve first launch in Privacy & Security if macOS asks. Then enable Accessibility for it.'
