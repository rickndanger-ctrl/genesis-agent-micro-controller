# 0.4.3 Preview

Both Claude and Codex now animate working chats for 30 seconds, then show three dim, slowly pulsing dots in the same slot color. Repeated working updates do not restart the timer. A new job starts a fresh timer. Completion still plays the chat-colored ripple and bell; Sound OFF silences the bell.

User verified on the personal Genesis: spin, three pulsing dots after 30 seconds, then completion ripple and audible bell during a controlled working-to-idle test. Normal Claude monitoring was restored afterward. Regression checks cover timing, independent slots, timer wraparound, and exactly three dim dots.

This is a firmware update. The Mac app functionality is unchanged from 0.4.2. Public app remains ad-hoc signed and not notarized. Public firmware retains per-device identification; the exact public bundle has not been flashed to another board. Open-chat limitations for Claude fork activity and lack of GPT Live speaking-only detection remain.
