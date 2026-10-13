# Attribution and third-party components

Genesis Agent Micro Controller is an independent community prototype for Axiometa hardware, compatible with selected Codex Micro host behavior and Claude Code desktop controls. No affiliation, sponsorship or approval by Axiometa, OpenAI, Anthropic, or Work Louder is claimed. Product names and the hybrid app artwork reference third-party marks; the MIT software license grants no trademark rights. Replace the artwork for your own branding or commercial use as appropriate.

The firmware uses externally installed open-source dependencies; their licenses continue to apply to binary distributions:

- Arduino ESP32 core 3.3.12: https://github.com/espressif/arduino-esp32 (LGPL-2.1 and bundled component licenses)
- ESP-IDF and TinyUSB as bundled by that core: respective component licenses
- Adafruit NeoPixel: https://github.com/adafruit/Adafruit_NeoPixel (LGPL-3.0)
- Adafruit GFX (BSD) and ST7735/ST7789 (MIT per bundled README): https://github.com/adafruit/Adafruit-GFX-Library and https://github.com/adafruit/Adafruit-ST7735-Library
- Adafruit BusIO: https://github.com/adafruit/Adafruit_BusIO (MIT)
- ArduinoJson: https://github.com/bblanchon/ArduinoJson (MIT)
- esptool is downloaded by the installer environment, not bundled: https://github.com/espressif/esptool (GPL-2.0-or-later)

Bundled license texts are in licenses/. Source for this project's firmware, build instructions and the exact dependency versions are provided to support modification and rebuilding. No proprietary Codex/Claude application code, extracted SDK, personal firmware backup, or third-party reverse-engineering repository is redistributed.
