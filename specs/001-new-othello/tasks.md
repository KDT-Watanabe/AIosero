# Tasks: Tension Othello

**Input**: Design documents from `specs/001-new-othello/`

**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md

**Tests**: REQUIRED by constitution Principle II (test-first for game rules). Rule tests are written before rule implementation.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: parallelizable (different files, no dependency on incomplete tasks)
- **[Story]**: user story label (US1..US4)
- Paths are repository-root relative.

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project skeleton per plan.md

- [ ] T001 Create project structure per implementation plan (src/engine, src/app, tests, assets, third_party/dxlib placeholder)
- [ ] T002 Create CMakeLists.txt: `othello_engine` library (STL only, no DxLib), `engine_tests` executable, `othello_app` executable (links DxLib when present)
- [ ] T003 [P] Download DxLib from http://lib.dxlib.com/ into third_party/dxlib (headers, .lib, DLL); engine/tests must build without it

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core engine types + test harness; blocks all user stories

- [ ] T004 Define engine types in src/engine/types.h per data-model.md: Stone {pos, camp, state(NORMAL/CONTESTED/FIXED), origin_camp, meter 0..100, placed_by}, SandwichLine {camp, anchor, chain, far_end, active}, Cooldown {camp, remaining}, Game {board[64], cd[2], tick, phase, events}, Event {PLACED, CONTESTED, FLIPPED, FROZEN, PASSED, GAME_OVER}
- [ ] T005 Create assert-based test harness in tests/engine_tests.cpp (main() running named test cases, exit nonzero on failure)

**Checkpoint**: Foundation ready.

---

## Phase 3: User Story 1 - Play a full game against the AI (Priority: P1) — MVP

**Goal**: Engine rules + deterministic AI + minimal playable loop; meter tug-of-war, flips, end, scoring.

**Independent Test**: Headless simulation: scripted placements + tick_second() → contested at 50, meter advances per second, flip at 0/100, end condition, score incl. meter-direction resolution.

### Tests for User Story 1 (write FIRST, must fail before implementation)

- [ ] T010 [P] [US1] Sandwich detection tests in tests/engine_tests.cpp (8 directions, contested-at-placement-only, no-op legality: "sandwich lines made only of fixed stones are legal no-op moves (no pressure produced)")
- [ ] T011 [P] [US1] Pressure tests in tests/engine_tests.cpp: p = ceil(N/k), recomputed every second, same-camp lines summed, net = black_sum − white_sum, p ≥ 1
- [ ] T012 [P] [US1] Meter/flip tests in tests/engine_tests.cpp: start 50, advances every second when active lines exist even without placements, flip exactly at 0/100, flipped stones fixed forever
- [ ] T013 [P] [US1] Rescue/freeze tests in tests/engine_tests.cpp: all lines broken → meter frozen, contest retained, re-sandwich resumes; far-end flip breaks the line
- [ ] T014 [P] [US1] CD & order tests in tests/engine_tests.cpp: independent 5 s CDs, pass when no legal move, same-second expiries processed black first then white, no collisions
- [ ] T015 [P] [US1] End & scoring tests in tests/engine_tests.cpp: end when board full or neither camp has a legal move; contested resolved by meter direction, exactly 50 → origin camp; tie = draw
- [ ] T016 [P] [US1] AI determinism tests in tests/engine_tests.cpp: same board → same move; 2-move lookahead; deterministic tie-break lowest square index
- [ ] T017 [P] [US1] Standard-start test in tests/engine_tests.cpp: 4-stone start, both CDs ready at t=0

### Implementation for User Story 1

- [ ] T018 [US1] Implement board + sandwich detection in src/engine/board.h/.cpp (passes T010, T017)
- [ ] T019 [US1] Implement rules (pressure, meter advance, flip, rescue/freeze) in src/engine/rules.h/.cpp (passes T011–T013)
- [ ] T020 [US1] Implement game loop (tick_second order: placements black→white, recompute, pressure, flips, end check), CDs, pass, scoring in src/engine/game.h/.cpp (passes T014–T015)
- [ ] T021 [US1] Implement deterministic AI (2-move lookahead heuristic over meter movement, contested count, corner/edge safety, rescue risk; tie-break lowest index) in src/engine/ai.h/.cpp (passes T016)
- [ ] T022 [US1] Headless end-to-end simulation test in tests/engine_tests.cpp: full game AI-vs-scripted-black reaches end screen state without manual intervention (SC-005)
- [ ] T023 [US1] Minimal DxLib game screen in src/app/render.cpp + src/app/main.cpp: board, meter numbers, CD bars, click-to-place for black, AI presentation delay 0.5–2 s

**Checkpoint**: US1 fully functional and independently testable (headless tests + playable app).

---

## Phase 4: User Story 2 - Understand the novelty (Priority: P2)

**Goal**: Japanese title/menu screen with rules explanation.

**Independent Test**: Launch app → title screen shows Japanese rules (meter 0–100, per-second pressure, flip at ends, rescue = freeze, CD 5 s) + スタート button → game.

- [ ] T024 [US2] Title/menu screen with Japanese rules explanation and スタート button in src/app/main.cpp (UTF-8 + Japanese font via ChangeFont)

**Checkpoint**: US1 + US2 work independently.

---

## Phase 5: User Story 3 - Read the board state at a glance (Priority: P3)

**Goal**: Predictions (camp + seconds) per contested stone; CD bars verified against engine.

**Independent Test**: Contested stone at meter 62 with net black pressure 4/s → UI shows 62 and "黒 約10秒"; CD bar shows remaining seconds; displayed values equal snapshot() (SC-004).

- [ ] T025 [US3] Implement prediction computation (remaining distance / net pressure per second, current net pressure assumed constant) in src/engine/game.h/.cpp snapshot
- [ ] T026 [US3] Render predictions + CD bars in src/app/render.cpp from snapshot() only

**Checkpoint**: all stories independently functional.

---

## Phase 6: User Story 4 - Restart (Priority: P4)

**Goal**: Restart from any state; no undo.

**Independent Test**: Trigger restart mid-game → board, meters, CDs reset to initial state.

- [ ] T027 [US4] Implement restart (re-init engine, reset UI state) in src/app/main.cpp + engine re-init path

---

## Phase 7: Polish & Cross-Cutting Concerns

- [ ] T028 [P] Simple sound effects (flip, CD unlock) in src/app/sound.cpp with assets/ wav files
- [ ] T029 [P] Event log/observability hook (events_since) in src/engine/game.h/.cpp per constitution V
- [ ] T030 Resizable window handling in src/app/main.cpp
- [ ] T031 Run quickstart.md validation end-to-end (tests green, app playable, SC-001..SC-005 checked)

---

## Dependencies & Execution Order

- Phase 1 → Phase 2 (blocks everything) → Phase 3 (US1, MVP) → Phases 4–6 (independent after Phase 2; US3/US4 integrate with US1's engine) → Phase 7.
- Within US1: tests (T010–T017) before implementation (T018–T021); board before rules before game before AI.

## Parallel Opportunities

- T003 parallel with T001/T002; T010–T017 all parallel (separate test functions, same file conflict → run as separate test files if parallelized); T028/T029/T030 parallel.

## Implementation Strategy

MVP = Phase 1 + 2 + 3 (playable engine + minimal app). Then P2 title screen, P3 predictions, P4 restart, polish.

## Notes

- Engine must never include DxLib headers (constitution I).
- UI renders only from snapshot() (constitution V, SC-004).
- Commit messages reference `specs/001-new-othello`.
