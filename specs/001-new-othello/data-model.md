# Data Model: Tension Othello

All entities are in-memory (no persistence). Engine-only model; the UI reads it.

## Stone

| Field | Type | Notes |
|---|---|---|
| pos | (row, col) 0..7 | fixed cell |
| camp | enum {BLACK, WHITE} | current camp |
| state | enum {NORMAL, CONTESTED, FIXED} | NORMAL = placed, not contested; CONTESTED = meter active; FIXED = flipped, permanent |
| origin_camp | enum {BLACK, WHITE} | camp at placement time (used for the 50-rule at end) |
| meter | int 0..100 | only meaningful when CONTESTED; start 50 |
| placed_by | camp | the camp that placed this stone |

State transitions:
- placed → CONTESTED (if sandwiched at placement time; meter = 50)
- placed → NORMAL (if not sandwiched at placement time)
- CONTESTED → FIXED (meter reaches 0 or 100; camp set to that end's camp)
- CONTESTED stays CONTESTED when all lines break (meter frozen)
- FIXED is terminal (never re-contested)

## SandwichLine

| Field | Type | Notes |
|---|---|---|
| camp | enum | the line's camp |
| anchor | Stone | the stone that created the line (placed stone) |
| chain | Stone[] | contested stones in the line (N = chain length) |
| far_end | Stone | own-camp stone at the far end |
| active | bool | false if far_end flipped to other camp, or anchor removed |

Recomputed every second: N = chain length, k = distance from contested stone to far_end (nearest own stone at that moment). p = ceil(N / k).

## Cooldown

| Field | Type | Notes |
|---|---|---|
| camp | enum | per-camp instance |
| remaining | seconds | 5 at start; starts on placement; passes when expired with no legal move |

## Game

| Field | Type | Notes |
|---|---|---|
| board | 8x8 Stone grid | standard 4-stone start |
| cd[BLACK], cd[WHITE] | Cooldown | independent |
| tick | int | engine tick counter (1 per second) |
| phase | enum {TITLE, PLAYING, ENDED} | |
| events | Event[] | log: PLACED, CONTESTED, FLIPPED, RESCUED(frozen), PASSED, GAME_OVER |

End condition (checked each tick): board full OR neither camp has a legal move.
Score: fixed/normal stones by camp + contested stones by meter direction (50 → origin camp). Tie = draw.

## Validation rules

- Legal placement: empty cell + at least one sandwich line (8 directions).
- No-op move: legal, produces no pressure, counts for end detection.
- Same-second CD expiries: process black first, then white.
- AI decision must be a pure function of board state (deterministic).
