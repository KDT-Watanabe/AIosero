# Engine Contract (headless, testable)

The engine is a pure library (no DxLib). Public surface:

```cpp
namespace othello {

struct Snapshot {
    int tick;
    struct Cell { int camp; int state; int meter; int origin_camp; };
    Cell board[64];
    int cd_remaining[2];          // [BLACK=0, WHITE=1]
    bool legal_move[2];           // has legal move at this moment
    struct Pred { int stone; int camp; double seconds; }; // prediction per contested stone
    Pred predictions[64];
    int score[2];                 // with meter-direction resolution incl. 50-rule
    int phase;                    // TITLE/PLAYING/ENDED
    int result;                   // -1 ongoing, 0 draw, 1 black, 2 white
};

// Deterministic operations
Snapshot init();                          // standard 4-stone start, both CDs ready
bool place(int camp, int cell);           // legal only if sandwich line exists; starts that camp's CD
void tick_second();                       // one second: resolve due placements (black first),
                                          // recompute N,k, apply net pressure, resolve flips,
                                          // check end condition
int ai_choose_move();                     // deterministic, 2-move lookahead, pure function of board
Snapshot snapshot() const;                // for UI + tests
std::vector<Event> events_since(int n);   // observability (constitution V)
}
```

Invariants (tested):
- Same init + same sequence of (place, tick_second) → identical snapshot (SC-003).
- predictions match remaining_distance / net_pressure per contested stone (SC-004).
- Meter never leaves [0,100]; flip occurs exactly when reaching an end.
- Contested stone counts as own-camp (origin) for line formation.
- Frozen meter when all lines broken; resumes on re-sandwich.
