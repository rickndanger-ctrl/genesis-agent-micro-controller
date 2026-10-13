# Genesis Agent Micro Controller

Turn an **Axiometa Genesis Mini** into a physical chat controller for Codex and Claude Code on a Mac: six assignable slots, dictation, send, app switching, and chat-colored status animations.

**Community DIY project. Not an official Axiometa, OpenAI, Anthropic, or Work Louder product.**

## Download, give it to your agent, plug it in

Download the complete prebuilt ZIP from [Releases](https://github.com/rickndanger-ctrl/genesis-agent-micro-controller/releases). Extract it and tell your coding agent:

> Read AGENT_SETUP.md in this folder and help me install Genesis Agent Micro Controller on my Genesis Mini. Verify my hardware and save its original firmware before flashing. Help me assign my own chats afterward.

You do **not** need to compile the app or firmware. The agent guide handles setup using the included verified-image installer. Initial setup still needs physical module alignment, a BOOT/RESET press, macOS launch/Accessibility approval, and your chat assignments. This is agent-assisted setup, not a driver-free one-click promise.

## What you need

- Axiometa Genesis Mini, ESP32-S3, **4MB flash**, with a data-capable USB cable.
- AX22 0.96-inch display, rotary encoder, 5×5 addressable matrix, and buzzer.
- Apple Silicon Mac, macOS 14 or later. Prebuilt app is arm64; Intel and Linux are not validated.
- Codex with Codex Micro enabled for native Codex operation; Claude desktop's **Code** interface for Claude sessions.
- A local coding agent able to run Python and follow the installation guide. The firmware itself requires no API key.

## Wiring: USB at the top

| Position | Socket | Module | Signals used |
|---|---|---|---|
| Top left | P3 | Display | CS 9, DC 15, reset 16; shared SPI 14/13/12 |
| Top right | P2 | Buzzer | GPIO 6 |
| Bottom left | P4 | 5×5 matrix | DIN GPIO 17; DOUT is not driven |
| Bottom right | P1 | Rotary encoder | button 4, A 3, B 2 |

**Unplug USB before moving modules. Align G with G and Vin/3V3 with the socket's 3V3; do not use the logo or apparent upright orientation as a guide.** Opposite sockets can have opposite orientations. No shifted rows or pins. Incorrect alignment can prevent operation or damage hardware. The display rotation and BGR color order match this arrangement.

## Controls

- Turn to choose a slot; press to open its chat or run its shortcut.
- Chat slots jump to Hold to talk. Shortcut slots stay on the slot menu.
- Send prompt is one position left of Hold to talk; press to send the current draft.
- Double-click in Actions to return to slots.
- Middle USER button switches Codex and Claude. BOOT and RESET are for firmware setup.
- Codex dictation follows the native microphone setting (hold/release with Push to talk). Claude uses click once to start and once to stop.
- App: independently assign C1–C6 to Chat, Hold to talk, or Send prompt. Configure A1–A6 in Codex's own Micro settings.
- Keep the companion app running for Claude and Codex shortcut-aware navigation. Start at login is optional in Device settings.

## Color means identity

| Slot | Color |
|---|---|
| A1 / C1 | Blue |
| A2 / C2 | Purple |
| A3 / C3 | Pink |
| A4 / C4 | Orange |
| A5 / C5 | Green |
| A6 / C6 | Cyan |

Working uses an orbit; completion uses ripples and a screen accent; attention flashes the relevant marker and optionally beeps. Idle animates gently for about one minute, then the matrix sleeps. Physical controls wake it. Attention takes priority. Screen, matrix and bottom LED use the same identity palette, though different hardware can render colors slightly differently.

## Honest limits

GPT Live speaking-only animation is **not implemented**. Its global Voice Chat shortcut may create a separate voice chat; start through the on-screen in-chat voice button for an existing conversation. Claude uses Accessibility and English control labels, so app updates can break integration. Written completion is inferred from host working/idle status; it does not prove an overall project is finished. See [known limitations](docs/KNOWN-LIMITATIONS.md).

The release app is ad-hoc signed, **not notarized**. macOS may require explicit approval to open it. Do not disable system-wide security protections. No personal chat mappings, credentials, or original device backups are included.

## Customize with your agent

Read [AGENT_SETUP.md](AGENT_SETUP.md), [build instructions](docs/BUILD.md), and [architecture](docs/ARCHITECTURE.md). Source and tests are included. Keep navigation, voice recording and sending tests separate; never send a user's draft merely to test a button.

## Verification

This project was tested on one Genesis Mini and one Apple Silicon Mac. The personal build's physical navigation, display, matrix, and Claude dictation were confirmed interactively. Public packaging changes (portable identity, app bundle identity, installer) are build/test verified; a fresh second-device install has not been physically validated. Release is a preview.
