# AIosero Constitution

## Core Principles

### I. Deterministic Engine

The game engine (rules, pressure, meter, AI decisions) MUST be deterministic and separable from rendering. Same board + same inputs → same state. This makes the novel mechanics testable without a GUI.

### II. Test-First for Game Rules

Core rule logic (sandwich detection, pressure computation, meter advancement, flip, rescue, end detection, scoring) must have automated tests written before implementation. UI/rendering is exempt from test-first but must be verified against the engine.

### III. Spec-Driven Development

Implementation follows the feature spec strictly. Any deviation from spec'd rules (meter range, pressure formula, CD length, end conditions) requires a spec amendment first, not an ad-hoc code change.

### IV. Simplicity (YAGNI)

v1 scope is fixed by the spec: single human (black) vs deterministic AI (white), 8x8 board, Japanese UI, no online, no undo. New features require a new feature spec.

### V. Observability

Engine state (meters, lines, CDs, events like flip/rescue) must be inspectable — via a replay/log or test hooks — so displayed values can be verified against internal state (SC-004).

## Technology Constraints

- Language: C++ (user-specified).
- Graphics/audio: DxLib (user-specified), Windows .exe deliverable.
- UI language: Japanese.
- Engine logic must not depend on DxLib (so tests can run without it).

## Development Workflow

- Follow the Speckit cycle: specify → clarify → plan → tasks → implement.
- Every functional requirement (FR-xxx) in the active spec must map to at least one task and one test or verification step.
- Commits reference the feature directory (`specs/001-new-othello`).

## Governance

This constitution supersedes ad-hoc practices for this project. Amendments require documentation and approval by the project owner. Complexity in design must be justified against Principle IV.

**Version**: 1.0.0 | **Ratified**: 2026-10-13 | **Last Amended**: 2026-10-13
