# Quickstart: Tension Othello

## Prerequisites

- Windows x64, MSVC (Visual Studio 18 Community confirmed on this machine).
- DxLib downloaded from http://lib.dxlib.com/ (not yet installed; engine/tests do not need it).
- CMake (or manual MSVC build).

## Build & run engine tests (headless, no DxLib)

```bash
cmake -S . -B build -G "Visual Studio 18 2026"   # or Ninja
cmake --build build --target engine_tests
build/engine_tests
```

Expected: all assertions pass (sandwich, pressure, meter, flip, rescue, no-op, end, scoring, same-second order, AI determinism).

## Build & run the app

1. Place DxLib headers/lib in `third_party/dxlib/`.
2. `cmake --build build --target othello_app`
3. Run `build/othello_app.exe` → title screen (Japanese rules) → スタート → play as black vs AI.

## Validation scenarios (maps to spec user stories)

1. Start game, place a sandwich stone → contested stone shows 50, meter advances each second.
2. Only black lines press a contested stone → meter reaches 100 → flips to black, becomes fixed.
3. Both camps press → net difference direction each second.
4. Break all lines → meter freezes; re-sandwich resumes.
5. Neither camp has a legal move → end screen with counts + meter breakdown (50 = origin camp), draw possible.
6. Restart mid-game → fresh 4-stone start, both CDs ready.
