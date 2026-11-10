#pragma once
#include <vector>

namespace othello {

constexpr int BOARD_SIZE = 8;
constexpr int CELLS = BOARD_SIZE * BOARD_SIZE;
constexpr int CD_SECONDS = 8;        // placement cooldown per camp (seconds)
constexpr int PRESSURE_SCALE = 3;    // meter points per unit of ceil(N/k) per second

enum Camp { BLACK = 0, WHITE = 1 };
enum StoneState { NORMAL = 0, CONTESTED = 1, FIXED = 2 };
enum Phase { TITLE = 0, PLAYING = 1, ENDED = 2 };
enum EventType { EV_PLACED = 0, EV_CONTESTED = 1, EV_FLIPPED = 2, EV_RESCUED = 3, EV_PASSED = 4, EV_GAME_OVER = 5 };
enum Result { RESULT_ONGOING = -1, RESULT_DRAW = 0, RESULT_BLACK_WIN = 1, RESULT_WHITE_WIN = 2 };

struct Stone {
    int pos = -1;            // row*8+col; -1 = empty cell
    int camp = -1;           // current camp
    int state = NORMAL;      // NORMAL / CONTESTED / FIXED
    int origin_camp = -1;    // camp at placement time (50-rule at end)
    int meter = 0;           // CONTESTED only: 0 = white end, 50 = start, 100 = black end
    int placed_by = -1;      // camp that placed this stone
};

struct SandwichLine {
    int camp = -1;
    int anchor = -1;                 // stone that created the line
    std::vector<int> chain;          // fixed positions of chain stones
    int far_end = -1;                // own-camp stone at the far end
    bool active = false;             // recomputed every second
    bool defend = false;             // support line: own contested stones re-enclosed by own camp,
                                     // pushing them back toward the camp's end (tug-of-war)
};

struct Cooldown {
    int camp = -1;
    int remaining = 0;               // seconds until ready; 0 = ready
};

struct Event {
    int id = -1;               // monotonic sequence for events_since(n) cursor
    int type = -1;
    int tick = -1;
    int cell = -1;
    int camp = -1;
};

// An active line currently pushing at least one contested stone (UI draws it).
struct LineView {
    int camp = -1;
    int anchor = -1;
    int far_end = -1;
    int pressure = 0;      // strongest per-stone push along this line (meter points / s)
    bool defend = false;   // support line (own contested stones pushed back)
};

// Engine public surface per contracts/engine-contract.md.
struct Snapshot {
    int tick = 0;
    struct Cell { int camp; int state; int meter; int origin_camp; int owner; };  // owner = camp it resolves to now
    Cell board[CELLS];
    int cd_remaining[2] = {0, 0};            // [BLACK=0, WHITE=1]
    int pending[2] = {-1, -1};               // cell queued for the next tick per camp
    bool legal_move[2] = {false, false};     // has legal move at this moment
    bool legal[2][CELLS] = {};               // per-cell legality per camp (UI hints)
    struct Pred { int stone; int camp; double seconds; };
    Pred predictions[CELLS];
    int score[2] = {0, 0};                   // meter-direction resolution incl. 50-rule
    int phase = TITLE;
    int result = RESULT_ONGOING;             // -1 ongoing, 0 draw, 1 black, 2 white
    std::vector<LineView> lines;             // active pushing lines
};

// Camp a stone resolves to for scoring: contested by meter direction, exactly 50 -> origin.
inline int resolved_owner(const Stone &s) {
    if (s.pos < 0) return -1;
    if (s.state != CONTESTED) return s.camp;
    if (s.meter > 50) return BLACK;
    if (s.meter < 50) return WHITE;
    return s.origin_camp;
}

// The camp a stone currently counts as for line formation / pressure.
inline int counts_as(const Stone &s) {
    if (s.pos < 0) return -1;
    if (s.state == CONTESTED) return s.origin_camp;   // contested counts as origin camp
    return s.camp;
}

} // namespace othello
