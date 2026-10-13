# 0.4.2 Preview

- Track working and idle states for the open Claude fork using unique session-header and Stop/Send controls. Ambiguous controls remain unknown.
- Play a brief two-tone, bell-like completion chime alongside the slot-colored ripple. Existing Sound ON/OFF controls the chime; attention alerts retain priority.
- Includes regression checks for activity-state ambiguity and bell timing.

Verified on the personal Genesis: working animation, then a controlled working-to-idle transition produced one completion, a blue ripple, and an audible chime confirmed by the user. Natural Claude completion remains to be observed end-to-end.

Keep forked chats open for this activity fallback; background fork status may remain unavailable. GPT Live speaking-only effects remain unsupported.

Upgrade both the Mac app and firmware for this release. Existing assignments remain stored under the same public app identifier. App remains ad-hoc signed, not notarized. Public firmware uses per-device identification; the exact public bundle has not been flashed to a second board.
