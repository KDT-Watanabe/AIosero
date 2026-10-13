# Implementation Plan: Tension Othello

**Branch**: `001-new-othello` | **Date**: 2026-10-13 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `specs/001-new-othello/spec.md`

## Summary

A real-time Othello variant ("Tension Othello") delivered as a Windows .exe in C++ with DxLib. Core novelty: sandwiched stones enter a contested state with a tension meter (0–100, start 50); every second, active sandwich lines push the meter by p = ceil(N/k) (same-camp lines summed, camps opposed as difference); the stone flips when the meter tips to an end and is then fixed forever. Each camp has an independent 5-second placement cooldown. Human (black) vs deterministic AI (white, 2-move lookahead). Engine logic is DxLib-free and deterministic so it is unit-testable; the DxLib layer only renders engine state and plays sounds.

## Technical Context

**Language/Version**: C++17 (MSVC, Visual Studio 18 Community available on this machine)
**Primary Dependencies**: DxLib (graphics/audio; NOT yet installed on this machine — see research.md), STL only for the engine
**Storage**: N/A (in-memory game state; no persistence in v1)
**Testing**: Engine unit tests in C++ (assert-based test runner, no external framework); engine tests run without DxLib
**Target Platform**: Windows x64 desktop app (.exe)
**Project Type**: desktop-app (engine library + DxLib front-end)
**Performance Goals**: UI 60 fps; per-second engine tick trivial (8x8 board)
**Constraints**: engine must not depend on DxLib; determinism required (SC-003); UI Japanese; resizable window
**Scale/Scope**: single game, 64 squares, ~15 functional requirements, ~1 engine module set + 1 UI module set

## Constitution Check

*Gates from `.specify/memory/constitution.md`:*

- I. Deterministic engine — PASS: engine separated from rendering; same inputs → same state.
- II. Test-first for game rules — PASS: rule tests written before rule implementation (see tasks ordering); UI verified against engine.
- III. Spec-driven — PASS: plan mirrors spec rules exactly.
- IV. Simplicity/YAGNI — PASS: no persistence, no network, no undo, no frameworks beyond DxLib + STL.
- V. Observability — PASS: engine exposes state snapshot + event log for tests and UI.

No violations → no Complexity Tracking entries.

## Project Structure

### Documentation (this feature)

```text
specs/001-new-othello/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   ├── engine-contract.md
│   └── ui-contract.md
└── tasks.md             # created by /speckit-tasks
```

### Source Code (repository root)

```text
src/
├── engine/            # DxLib-free, deterministic core
│   ├── types.h        # Stone, CellState, Meter, Line, GameEvent
│   ├── board.h/.cpp   # 8x8 board, sandwich detection, legal moves
│   ├── rules.h/.cpp   # pressure p=ceil(N/k), meter advance, flip, rescue
│   ├── game.h/.cpp    # per-second tick, per-camp CD, end detection, scoring
│   └── ai.h/.cpp      # deterministic white AI, 2-move lookahead
├── app/               # DxLib front-end (Japanese UI, sounds)
│   ├── main.cpp       # title screen → game → end screen
│   ├── render.cpp     # board, meter numbers, CD bars, predictions
│   └── sound.cpp      # flip / CD-unlock SFX
└── tests/
    └── engine_tests.cpp  # assert-based runner covering FR-001..FR-009, FR-014
CMakeLists.txt         # engine + tests buildable without DxLib; app target when DxLib present
```

**Structure Decision**: single project, engine/tests separated from the DxLib app so constitution Principle I/II holds (tests run headless).

## Complexity Tracking

No violations.
