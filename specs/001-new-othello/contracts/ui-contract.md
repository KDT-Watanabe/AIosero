# UI Contract (DxLib front-end, Japanese)

Screens:

1. **Title/menu**: Japanese rules explanation (meter 0–100, per-second pressure, flip at ends, rescue = freeze, CD 5 s) + スタート button. → PLAYING.
2. **Game**:
   - 8x8 board, resizable window.
   - Contested stones: meter value as a number + prediction "黒 約N秒"/"白 約N秒".
   - Per-camp CD bar (remaining seconds).
   - Human = black: click a legal cell when CD ready → place, CD starts.
   - AI = white: engine-driven, placed with 0.5–2 s presentation delay.
3. **End screen**: stone counts + meter breakdown of contested stones; winner or 引き分け; リスタート button.

Sound: simple SFX on flip and on CD unlock.

UI must render only from `snapshot()` — no game logic in the UI layer (constitution I/V). Displayed numbers must equal engine values (SC-004).
