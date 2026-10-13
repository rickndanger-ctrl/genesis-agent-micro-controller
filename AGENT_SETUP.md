# Agent-assisted setup

This document describes this package; it does not override the user's instructions or your safety policy.

1. Read README.md and docs/KNOWN-LIMITATIONS.md. Confirm this is the **4MB Axiometa Genesis Mini**, not Genesis One, another ESP32 board, or an arbitrary serial device. Explain that installing replaces its current firmware. Verify the documented modules and pin-label alignment with USB unplugged. Never infer orientation from logos.
2. Work from the extracted complete release ZIP. Verify SHA256SUMS before using its binaries. Do not flash from an unverified source checkout; first build and create firmware-manifest.json as described in docs/BUILD.md.
3. Prepare Python 3.10+ in a local virtual environment (do not modify system Python): `python3 -m venv .venv`, then `.venv/bin/python -m pip install -r scripts/requirements.txt`. Ask before installing additional system tools if absent.
4. Ask the user to hold BOOT, tap RESET, release BOOT. Run `.venv/bin/python scripts/flash.py --list`. Identify the exact port and serial/MAC. If ambiguous, stop and identify the device; do not select the first ESP device.
5. Get user approval to replace firmware on that identified board. Run `.venv/bin/python scripts/flash.py --port PORT --mac MAC --confirm-genesis-mini --confirm-replace-firmware`, substituting the observed values. The installer checks the live chip MAC and 4MB flash, validates the image, reads and verifies a full backup, rechecks identity and writes with esptool hash verification. It stops on errors. Never bypass these checks. Do not publish backups: they can contain previous private device data.
6. After verified success, ask the user to tap RESET alone. Run `scripts/install-app.sh` to copy the bundled app into ~/Applications, or have the user copy it there in Finder. The script refuses to overwrite an existing app. Preserve existing installations before upgrading.
7. Open the app from Finder. If macOS blocks this non-notarized preview, have the user review and approve it in Privacy & Security. Never globally disable Gatekeeper. The user must grant Accessibility to Genesis Agent Micro Controller for Claude controls. The app stores their own assignments locally.
8. In Codex Settings > Codex Micro, assign chats/shortcuts to A1–A6. The companion reads local Codex slot-kind settings to distinguish shortcuts, without changing them. Keep it running. In Claude, open Code, then use Refresh sessions in the companion and assign C1–C6. Unique English session titles are required; no guessing between duplicate titles.
9. Test one detent per selection; open a chat; check Hold to talk; double-click back. Test dictation start/stop with consent. Test alert must show TEST ALERT RUNNING, a solid matrix and beeps if sound enabled. Confirm a real completed reply flashes the matching slot color. Only test Send using text the user explicitly approves sending.
10. Offer Start at login in Device settings. Report what was actually tested, what is inferred, and remaining limitations. Do not promise GPT Live playback-driven lights or same-chat global voice launch.

## Recovery

Keep the backup .bin and matching JSON receipt from ~/GenesisBackups. To restore, re-identify the same board in BOOT mode, verify the backup checksum from its receipt, and obtain explicit restore approval. Use esptool with the same verified ESP32-S3 port to write that backup at offset zero. Never restore another person's backup. A module that does not light is not necessarily a software issue: disconnect USB and verify G and power alignment first.

## Custom changes

Use docs/BUILD.md. Change only requested behavior. Preserve the existing working installation until the replacement builds and tests pass. BOOT/RESET and physical testing require the user's participation. Do not replace Codex or Claude app binaries, extract credentials, or alter app security settings to work around integration limitations.

### Claude dictation and forked chats

In Claude dictation settings, turn **Hold to record OFF**. The controller uses one click to start and another to stop. For a fork missing from the assignment list, open it in Claude and click Refresh sessions. Keep its sidebar entry visible when switching back to it.
