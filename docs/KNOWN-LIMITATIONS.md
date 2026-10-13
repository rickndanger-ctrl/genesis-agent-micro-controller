# Known limitations

- Mac-first prototype; current prebuilt app targets Apple Silicon and macOS 14+. Linux and Intel Mac operation are not verified.
- Codex support depends on its native Codex Micro feature and undocumented host status messages. App updates may change these.
- Claude Code desktop control uses Accessibility and English UI labels; duplicate chat titles are rejected. Sidebar discovery and status polling can miss short transitions.
- Written-reply completion is inferred from working-to-idle/unread status, not an overall task goal.
- GPT Live speaking-only animation is not implemented. The global Voice Chat shortcut can start a separate chat; use the in-chat on-screen voice button to start voice in an existing chat.
- Codex dictation follows the app's push-to-talk setting; Claude dictation uses click-to-toggle. These differ intentionally.
- Automatic Codex chat-to-microphone navigation requires the companion app running and readable local Codex configuration. Unknown or stale mappings disable that automatic jump.
- The binary is ad-hoc signed, not Apple notarized. First launch may need explicit approval in System Settings > Privacy & Security. Do not disable Gatekeeper system-wide.
- USB supports one attached host at a time. Only the documented board/module arrangement has been tested.
