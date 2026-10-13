# Phase 0 Research: Tension Othello

## Decision: C++17 with MSVC (Visual Studio 18 Community, MSVC 14.51)

- **Rationale**: User-specified C++/DxLib on Windows. MSVC toolchain confirmed present on this machine (`C:\Program Files\Microsoft Visual Studio\18\Community`).
- **Alternatives**: MinGW — not installed; clang — not installed.

## Decision: DxLib must be downloaded separately

- **Rationale**: DxLib is not present on this machine (searched common locations). It is distributed as a zip from the official site (http://lib.dxlib.com/), containing headers, .lib, and DLL. The app target links it; the engine target does not.
- **Alternatives considered**: none — user explicitly required DxLib.
- **Risk/mitigation**: If download is blocked, engine + tests can still be built and run headless; the app build waits for DxLib.

## Decision: Engine is DxLib-free and deterministic

- **Rationale**: Constitution I/II; SC-003. Determinism requires: fixed processing order (black → white per second), integer meters, no wall-clock inside engine logic (tick counter instead), deterministic AI (no RNG).
- **Design consequence**: engine exposes `tick()` driven by an external clock source; the app calls it once per second. Tests call `tick()` directly.

## Decision: Meter advancement order within a second

- **Rationale**: To be deterministic and testable, each second is processed as: (1) resolve placements due (black first, then white), (2) recompute N,k for all active lines, (3) apply net pressure to every contested stone, (4) resolve flips (meters reaching 0/100), (5) check end condition.
- **Alternatives**: per-stone immediate flip during pressure — rejected (order-dependent ambiguity).

## Decision: Pressure formula

- `p = ceil(N / k)` per active line, N = chain length (sandwiched stones in that line), k = distance from the nearest own stone at that moment. Same-camp lines on the same stone are summed; net movement = black_sum − white_sum. p ≥ 1 per active line.
- **Note**: "distance from the nearest own stone at that moment" is interpreted as the distance from the contested stone to the far-end own stone of that line (the line's own end), recomputed each second — consistent with the interview answer "その時最も近い自石からの距離で計算し直す".

## Decision: Line activity and rescue

- A line is active while: the placing stone of that line is still present, the far end holds a stone of the line's camp, and the contested stone is still contested. If the far end flips to the other camp, the line breaks. If all lines on a contested stone break, the meter freezes (contest retained).
- **Edge**: a line created by a placement whose chain contains only fixed stones produces no pressure (no-op move).

## Decision: AI (white), deterministic, 2-move lookahead

- **Rationale**: spec FR-008. Evaluation heuristic over: expected meter movement, contested-stone count, corner/edge safety, rescue risk. Deterministic tie-break by square index (lowest index wins).
- **Alternatives**: stronger search — rejected by interview (middle strength) and YAGNI.

## Decision: Test framework

- Plain C++ assert-based test runner (no external framework) to keep the toolchain minimal on Windows. Tests cover: sandwich detection, pressure computation, meter advance, flip, rescue/freeze, no-op legality, end detection, scoring incl. meter-direction resolution and 50-rule, same-second order black→white, AI determinism.

## Decision: Sound effects

- Simple DxLib sounds (LoadSoundVolume/PlaySoundVolume) for flip and CD unlock; small embedded .wav assets under `assets/`.

## Decision: Japanese UI text

- DxLib font rendering with Japanese text requires Shift-JIS or UTF-8 handling; use UTF-8 source with DxLib's font settings, or Shift-JIS literals. Decision: UTF-8 + `ChangeFont` with a Japanese-capable font (Meiryo/Yu Gothic) to avoid mojibake.
