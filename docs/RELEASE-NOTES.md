# v0.4.0-preview

First public, agent-installable Genesis Agent Micro Controller package. Includes an Apple Silicon Mac app, portable Genesis Mini firmware based on v0.2.15, full project source, installer, checksums, wiring and customization guides.

## Included

- Native Codex Micro-compatible USB controls and Claude Code desktop companion.
- Six assignable slots, middle-button app switching, dictation/send actions, double-click back.
- Chat-only automatic microphone navigation; shortcuts no longer jump to Hold to talk.
- Fixed slot colors, working orbit, completion ripple, attention markers and idle sleep.
- Board-specific firmware backup and image validation before replacement.

## Validation

- Firmware and ad-hoc signed arm64 app compile successfully.
- Eight C++ test programs, Swift slot-kind assertions, and six Python installer tests pass.
- Pinned esptool 5.1.0 environment installs; installer CLI loads.
- Release checksums and personal-reference scan pass.
- Prior development firmware physically tested on one Genesis Mini. New public serial identity and installer have not been flashed on a second device. The installer safety sequence is tested with mocks, not a claimed clean-install hardware test.

## Before installing

Use exactly the documented 4MB Genesis Mini and module arrangement. Align G and power labels while disconnected. Keep only one Genesis connected. Quit other controller companion instances before launching this one. Preserve backups privately. The app is not notarized; macOS may require explicit local approval.

GPT Live speaking-only effects and global-shortcut same-chat voice start are not supported. Linux, Intel Mac and other Genesis boards are not validated. Review KNOWN-LIMITATIONS.md.
