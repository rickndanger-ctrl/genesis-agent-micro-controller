# Architecture and privacy

The ESP32-S3 firmware enumerates as a Micro-compatible USB HID interface. The compatibility VID/PID and descriptor are retained for native Codex recognition; this is not a USB-IF allocation or an official Work Louder device. Each board generates its own serial identity from its eFuse MAC. Review USB identity requirements before commercial distribution.

Codex sends native thread-status colors; the firmware maps their meaning to the user's fixed slot-identity palette. Claude mode sends knob events to the Mac companion, which targets uniquely resolved Claude Code controls through Accessibility and guarded pointer clicks. It may briefly move the pointer when an accessibility press is ineffective. It does not replace or inject code into Claude or Codex.

Assignments and preferences are stored locally with macOS UserDefaults. The public app has a separate bundle identity from the original development installation. Login startup uses a per-user LaunchAgent pointing at the installed app location. The app itself has no cloud backend or API-key requirement. The agent products it controls retain their own network/account behavior.

Codex chat-vs-shortcut classification reads local Codex configuration and assignment metadata every two seconds. A short-lived six-bit mask tells firmware which slots may jump to the microphone. Stale or unavailable metadata disables the automatic jump. The global voice shortcut is not silently converted to an accessibility workaround.

The public distribution disables diagnostic file logging and does not include users' chat titles, mappings, device backups, or chat content. Firmware backup files created during installation belong to the installing user and must remain private.

Dependencies and host protocols may change. This is an experimental single-device, single-host adapter, not a guaranteed cross-platform hardware driver.
