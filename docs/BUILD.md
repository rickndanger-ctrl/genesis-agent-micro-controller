# Build and customize

Prebuilt release users do not need these tools. Developers need Xcode Command Line Tools, Python 3.10+, Arduino CLI, ESP32 core 3.3.12 and these pinned Arduino libraries:

- ArduinoJson 7.4.3
- Adafruit NeoPixel 1.15.5
- Adafruit GFX Library 1.12.6
- Adafruit BusIO 1.17.4
- Adafruit ST7735 and ST7789 Library 1.11.0

Install the official Espressif core index `https://espressif.github.io/arduino-esp32/package_esp32_index.json` in Arduino CLI, install `esp32:esp32@3.3.12`, then install the library versions above using `arduino-cli lib install 'Name@version'`. The board target is `esp32:esp32:axiometa_genesis_mini:USBMode=default,CDCOnBoot=default`. Do not substitute a different board target or USB mode.

Run:

```sh
scripts/test.sh
scripts/build-firmware.sh
macos/build.sh
python3 scripts/package.py
```

`ARDUINO_CLI` can point at a custom Arduino CLI executable. `ARDUINO_CTAGS_PATH` is an optional ctags tool directory override. App builds default to ad-hoc signing; `GENESIS_SIGN_ID` can name a developer's own signing identity. No signing keys are included. The app build targets the local Mac architecture; official preview assets are arm64 for macOS 14+.

The packager creates `dist/Genesis-Agent-Micro-Controller-v0.4.0-preview.zip` with app, merged image, source, agent guide, installer, manifest and checksums. It requires the built image to be exactly 4MB. Do not include original-device backups, personal preferences, logs, private credentials or signing certificates.

## Main customization points

- `firmware/GenesisMicro/BoardConfig.h`: module pin layout. Changing it requires physical rewiring while USB is disconnected and a rebuild.
- `GenesisMicro.ino`: action menus, slot colors, rendering, native HID and companion messages.
- `MatrixAnimation.h`: effect geometry and brightness curves.
- `Completion.h`, `Attention.h`, `SlotNavigation.h`: status transitions and navigation policy.
- `macos/Sources/ControllerView.swift`: native configuration UI.
- `macos/Sources/Mapper.swift`: Claude Accessibility adapter and USB transport.
- `macos/Sources/CodexSlotTypes.swift`: read-only Codex assignment-kind import.

Run the tests and inspect the changed behavior before asking a user to flash. Physical verification is still required after a build succeeds. Never write to a board just to test packaging.
