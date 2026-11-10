// Engine unit tests per specs/001-new-othello/tasks.md (T010-T017, T022, T025).
// Plain asserts + pass/fail counter; exit code 0 only if all checks pass.
#include "../src/engine/game.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <algorithm>

using namespace othello;

static int g_checks = 0, g_fails = 0;
#define CHECK(cond) do { g_checks++; if (!(cond)) { g_fails++; \
    printf("FAIL [%s] line %d: %s\n", __func__, __LINE__, #cond); } } while (0)

static int cell(int row, int col) { return row * BOARD_SIZE + col; }

static bool has_event(const Game &g, int type, int cell_v) {
    for (const Event &e : g.events_since(0)) if (e.type == type && e.cell == cell_v) return true;
    return false;
}
static int count_contested(const Game &g) {
    int n = 0; for (int p = 0; p < CELLS; p++) if (g.board.at(p).state == CONTESTED) n++;
    return n;
}
static int count_events(const Game &g, int type) {
    int n = 0; for (const Event &e : g.events_since(0)) if (e.type == type) n++;
    return n;
}

// T010: sandwich detection — basic line, fixed-only chain legal, contest only at placement.
static void test_t010() {
    // Basic line: black places c4 (3,2), sandwiching white d4 (3,3) against black e4 (3,4).
    Game g; g.start_game();
    CHECK(g.place(BLACK, cell(3, 2)));
    g.tick_second();
    CHECK(g.board.at(cell(3, 3)).state == CONTESTED);
    CHECK(g.board.at(cell(3, 3)).meter == 50);
    CHECK(has_event(g, EV_PLACED, cell(3, 2)));
    CHECK(has_event(g, EV_CONTESTED, cell(3, 3)));

    // Fixed-only chain is still a legal placement but creates no contest.
    Game h; h.test_reset_board();
    h.test_place(WHITE, cell(3, 3)); h.test_set_state(cell(3, 3), FIXED);
    h.test_place(BLACK, cell(3, 4));
    CHECK(has_sandwich(h.board, BLACK, cell(3, 2)));          // fixed-only chain: legal
    h.start_game();
    CHECK(h.place(BLACK, cell(3, 2)));
    h.tick_second();
    CHECK(h.board.at(cell(3, 3)).state == FIXED);             // no contest on fixed stone
    CHECK(count_events(h, EV_CONTESTED) == 0);

    // Already-contested stone is never re-contested by a new line.
    Game k; k.test_reset_board();
    k.test_place(BLACK, cell(3, 0)); k.test_place(WHITE, cell(3, 1));
    k.test_place(WHITE, cell(3, 2)); k.test_place(BLACK, cell(3, 3));   // contests (3,1),(3,2)
    CHECK(k.board.at(cell(3, 1)).state == CONTESTED);
    k.test_place(BLACK, cell(2, 0)); k.test_place(BLACK, cell(4, 2));   // new line through (3,1)
    CHECK(k.board.at(cell(3, 1)).state == CONTESTED);
    CHECK(k.board.at(cell(3, 1)).meter == 50);                           // meter not reset
}

// T011: pressure p = ceil(N/k), recomputed every second, net pressure from multiple lines.
static void test_t011() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(3, 0));
    for (int c = 1; c <= 3; c++) g.test_place(WHITE, cell(3, c));
    g.test_place(BLACK, cell(3, 4));                   // L1 chain {(3,3),(3,2),(3,1)}, N=3
    CHECK(g.board.at(cell(3, 1)).meter == 50 && g.board.at(cell(3, 2)).meter == 50);

    g.tick_second();   // (3,1),(3,3): k=1 -> p=3 ; (3,2): k=2 -> p=ceil(3/2)=2
    CHECK(g.board.at(cell(3, 1)).meter == 53);
    CHECK(g.board.at(cell(3, 2)).meter == 52);
    CHECK(g.board.at(cell(3, 3)).meter == 53);

    g.test_place(BLACK, cell(2, 2));   // no line, but becomes nearest own stone for (3,2)
    g.tick_second();                   // (3,2): N=3,k=1 -> p=3 (recomputed)
    CHECK(g.board.at(cell(3, 2)).meter == 55);
    CHECK(g.board.at(cell(3, 1)).meter == 56);

    g.test_place(BLACK, cell(2, 1)); g.test_place(BLACK, cell(4, 1));   // L2 through (3,1): p=1
    g.tick_second();                   // net on (3,1) = 3 + 1 = 4 (same-camp lines summed)
    CHECK(g.board.at(cell(3, 1)).meter == 60);
}

// US1 acceptance 3: a contested stone pressed by both camps moves by the net difference.
// White re-encloses its own contested stone with support lines that push it back to white.
static void test_tug_of_war() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(3, 0)); g.test_place(WHITE, cell(3, 1));
    g.test_place(BLACK, cell(3, 2));                      // black line: (3,1) contested, +1/s
    CHECK(g.board.at(cell(3, 1)).state == CONTESTED);
    g.test_place(WHITE, cell(2, 1)); g.test_place(WHITE, cell(2, 0));
    g.test_place(WHITE, cell(4, 1));                      // white support (column): -1/s
    g.tick_second();
    CHECK(g.board.at(cell(3, 1)).meter == 50);            // +1 - 1 = 0
    CHECK(g.snapshot().predictions[cell(3, 1)].camp == -1);
    g.test_place(WHITE, cell(4, 2));                      // white support (diagonal): -1/s
    g.tick_second();
    CHECK(g.board.at(cell(3, 1)).meter == 49);            // +1 - 2 = -1 -> toward white
    g.tick_second();
    CHECK(g.board.at(cell(3, 1)).meter == 48);
    Snapshot s = g.snapshot();
    CHECK(s.predictions[cell(3, 1)].camp == WHITE);
    CHECK(std::abs(s.predictions[cell(3, 1)].seconds - 48.0) < 1e-9);
    CHECK(g.board.at(cell(3, 1)).state == CONTESTED);     // support never creates new contests
    CHECK(g.board.at(cell(2, 1)).state == NORMAL && g.board.at(cell(4, 1)).state == NORMAL);
}

// T012: meter reaches 100 exactly -> flip to line camp; fixed stones never re-contested,
// fixed-only chains get no pressure.
static void test_t012() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(3, 2)); g.test_place(WHITE, cell(3, 3));
    g.test_place(BLACK, cell(3, 4));                       // contests (3,3), p = ceil(1/1) = 1
    for (int i = 1; i <= 49; i++) {
        g.tick_second();
        CHECK(g.board.at(cell(3, 3)).meter == 50 + i);
        CHECK(g.board.at(cell(3, 3)).state == CONTESTED);
    }
    g.tick_second();                                       // exactly 100 -> flip
    CHECK(g.board.at(cell(3, 3)).state == FIXED);
    CHECK(g.board.at(cell(3, 3)).camp == BLACK);           // line camp = black end
    CHECK(has_event(g, EV_FLIPPED, cell(3, 3)));

    // Fixed stone in a new chain: never re-contested, no pressure.
    g.test_place(WHITE, cell(4, 3)); g.test_place(WHITE, cell(2, 3));   // line through fixed (3,3)
    CHECK(g.board.at(cell(3, 3)).state == FIXED && g.board.at(cell(3, 3)).meter == 100);
    g.tick_second();
    CHECK(g.board.at(cell(3, 3)).meter == 100);           // fixed stones take no pressure
}

// T013: frozen stones (all lines inactive) get no pressure; re-sandwiching resumes pressure.
static void test_t013() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(2, 0));
    for (int c = 1; c <= 3; c++) g.test_place(WHITE, cell(2, c));
    g.test_place(BLACK, cell(2, 4));                      // L1 chain {(2,3),(2,2),(2,1)}, N=3
    g.tick_second();
    CHECK(g.board.at(cell(2, 2)).meter == 52);            // N=3,k=2 -> p=2
    CHECK(g.board.at(cell(2, 1)).meter == 53);            // N=3,k=1 -> p=3

    g.test_set_state(cell(2, 0), FIXED); g.test_set_camp(cell(2, 0), WHITE);  // far end flips: L1 broken
    g.tick_second();
    CHECK(has_event(g, EV_RESCUED, cell(2, 2)));          // frozen transition
    CHECK(g.board.at(cell(2, 2)).state == CONTESTED);     // contest retained
    g.tick_second();
    CHECK(g.board.at(cell(2, 2)).meter == 52);            // frozen: no pressure

    g.test_place(WHITE, cell(3, 3)); g.test_place(BLACK, cell(1, 1));
    g.test_place(BLACK, cell(4, 4));                      // new line through (2,2): N=2,k=1 -> p=2
    g.tick_second();
    CHECK(g.board.at(cell(2, 2)).meter == 54);            // pressure resumed
}

// T014: independent 5s cooldowns, pass when expired with no legal move, same-second order black first.
static void test_t014() {
    // Pass scenario: white has no legal move anywhere.
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(WHITE, cell(0, 1)); g.test_place(BLACK, cell(0, 0));    // black legal via (0,2)
    CHECK(!has_legal_move(g.board, WHITE));
    CHECK(has_legal_move(g.board, BLACK));
    g.tick_second();                                      // white ready at t=0, no move -> pass
    CHECK(has_event(g, EV_PASSED, -1));
    CHECK(g.snapshot().cd_remaining[WHITE] == CD_SECONDS);

    // Independent cooldowns.
    Game h; h.start_game();
    CHECK(h.place(BLACK, cell(3, 2)));                    // legal sandwich line at standard start
    CHECK(h.snapshot().cd_remaining[BLACK] == CD_SECONDS);
    CHECK(h.snapshot().cd_remaining[WHITE] == 0);         // white's own CD untouched
    CHECK(!h.place(BLACK, cell(2, 3)));                   // CD not ready -> rejected
    const int exp_black[6] = {5, 4, 3, 2, 1, 0};
    for (int t = 1; t <= 6; t++) {
        h.tick_second();
        CHECK(h.snapshot().cd_remaining[BLACK] == exp_black[t - 1]);
        CHECK(h.snapshot().cd_remaining[WHITE] == 0);     // white never placed; stays ready
    }

    // Same-second resolution order: black first, then white.
    Game k; k.test_reset_board(); k.start_game();
    k.test_place(BLACK, cell(3, 0)); k.test_place(WHITE, cell(3, 1));
    k.test_place(BLACK, cell(3, 2)); k.test_place(WHITE, cell(3, 3));     // contests (3,2)
    k.test_place(WHITE, cell(4, 2));
    CHECK(has_legal_move(k.board, BLACK) && has_legal_move(k.board, WHITE));
    CHECK(k.place(BLACK, cell(3, 4)));                    // black queues first
    CHECK(k.place(WHITE, cell(2, 2)));                    // white queues same second
    k.tick_second();
    auto evs = k.events_since(0);
    int first_placed = -1, second_placed = -1;
    for (const Event &e : evs) if (e.type == EV_PLACED) {
        if (first_placed < 0) first_placed = e.id;
        else second_placed = e.id;
    }
    CHECK(first_placed >= 0 && second_placed > first_placed);
    for (const Event &e : evs) {
        if (e.type == EV_PLACED && e.id == first_placed) CHECK(e.camp == BLACK && e.cell == cell(3, 4));
        if (e.type == EV_PLACED && e.id == second_placed) CHECK(e.camp == WHITE && e.cell == cell(2, 2));
    }
}

// T015: end condition (board full), meter-direction scoring incl. meter==50 origin rule.
static void test_t015() {
    Game g; g.test_reset_board(); g.start_game();
    // Three contested stones at known meters.
    g.test_place(BLACK, cell(3, 0)); g.test_place(WHITE, cell(3, 1)); g.test_place(BLACK, cell(3, 2));
    g.test_place(BLACK, cell(4, 0)); g.test_place(WHITE, cell(4, 1)); g.test_place(BLACK, cell(4, 2));
    g.test_place(BLACK, cell(5, 0)); g.test_place(WHITE, cell(5, 1)); g.test_place(BLACK, cell(5, 2));
    CHECK(count_contested(g) == 3);                       // exactly the three chain stones
    g.test_set_meter(cell(3, 1), 60);                     // resolves black (>50)
    g.test_set_meter(cell(4, 1), 40);                     // resolves white (<50)
    g.test_set_state(cell(5, 0), FIXED); g.test_set_camp(cell(5, 0), WHITE);  // freeze the third:
    CHECK(g.snapshot().predictions[cell(5, 1)].camp == -1);                    // frozen, no pressure
    // Fill everything else black — only already-contested stones can be in lines, so no new contests.
    for (int p = 0; p < CELLS; p++) if (g.board.at(p).pos < 0) g.test_place(BLACK, p);
    CHECK(count_contested(g) == 3);                       // fills created no new contests
    g.tick_second();                                      // board full -> end
    CHECK(g.phase == ENDED);
    Snapshot s = g.snapshot();
    CHECK(s.score[BLACK] == 61 && s.score[WHITE] == 3);  // 58 fixed black + meter-60 stone;
    CHECK(s.result == RESULT_BLACK_WIN);                  // 1 flipped white + meter-40 + origin-50
    CHECK(has_event(g, EV_GAME_OVER, -1));
}

// T016: AI determinism, purity, tiebreak = lowest cell index among equal-value moves.
static void test_t016() {
    Game g; g.start_game();
    int first = g.ai_choose_move();                       // white has 4 legal moves at start
    auto start_moves = legal_moves(g.board, WHITE);
    CHECK(start_moves.size() == 4);
    CHECK(std::find(start_moves.begin(), start_moves.end(), first) != start_moves.end());
    Game empty; empty.test_reset_board();
    CHECK(empty.ai_choose_move() == -1);                  // no legal move -> -1

    Game k; k.test_reset_board(); k.start_game();
    k.test_place(BLACK, cell(3, 0)); k.test_place(WHITE, cell(3, 1));
    k.test_place(BLACK, cell(3, 2)); k.test_place(WHITE, cell(3, 3));     // contests (3,2)
    k.test_place(WHITE, cell(4, 2)); k.test_place(BLACK, cell(1, 0));
    auto moves = legal_moves(k.board, WHITE);
    CHECK(!moves.empty());
    int m1 = k.ai_choose_move(), m2 = k.ai_choose_move();
    CHECK(m1 >= 0 && m1 == m2);                           // deterministic across calls
    Snapshot before = k.snapshot();
    k.ai_choose_move();
    Snapshot after = k.snapshot();
    CHECK(before.tick == after.tick);
    CHECK(before.score[0] == after.score[0] && before.score[1] == after.score[1]); // pure: no mutation
    for (int p = 0; p < CELLS; p++) CHECK(before.predictions[p].camp == after.predictions[p].camp);

    long long best = LLONG_MIN; int argmax = -1;
    for (int m : moves) { long long v = k.ai_eval_move(m); if (v > best) { best = v; argmax = m; } }
    CHECK(m1 == argmax);                                  // tiebreak: lowest index among equals
}

// T017: standard start positions, both CDs ready at t=0; same init + same sequence ->
// identical snapshots (SC-003).
static void test_t017() {
    Game g; Snapshot s = g.init();
    int whites[2] = {cell(3, 3), cell(4, 4)}, blacks[2] = {cell(3, 4), cell(4, 3)};
    for (int p = 0; p < CELLS; p++) {
        bool in_w = p == whites[0] || p == whites[1], in_b = p == blacks[0] || p == blacks[1];
        if (!in_w && !in_b) { CHECK(s.board[p].camp == -1); continue; }
        int camp = in_w ? WHITE : BLACK;
        CHECK(s.board[p].camp == camp && s.board[p].state == NORMAL);
        CHECK(s.board[p].origin_camp == camp && s.board[p].meter == 0);
    }
    CHECK(s.cd_remaining[BLACK] == 0 && s.cd_remaining[WHITE] == 0);       // both ready at t=0

    Game a, b; a.init(); b.init(); a.start_game(); b.start_game();
    for (int t = 0; t < 12; t++) {
        int la = -1, lb = -1;
        auto ma = legal_moves(a.board, BLACK), mb = legal_moves(b.board, BLACK);
        if (!ma.empty() && a.snapshot().cd_remaining[BLACK] == 0) la = ma[0];
        if (!mb.empty() && b.snapshot().cd_remaining[BLACK] == 0) lb = mb[0];
        CHECK(la == lb);
        if (la >= 0) { a.place(BLACK, la); b.place(BLACK, lb); }
        a.tick_second(); b.tick_second();
        Snapshot sa = a.snapshot(), sb = b.snapshot();
        CHECK(sa.tick == sb.tick && sa.score[0] == sb.score[0] && sa.score[1] == sb.score[1]);
        for (int p = 0; p < CELLS; p++) {
            CHECK(sa.board[p].camp == sb.board[p].camp && sa.board[p].state == sb.board[p].state);
            CHECK(sa.board[p].meter == sb.board[p].meter);
        }
    }
}

// T022: full-game simulation — black plays lowest legal index, white plays the static AI;
// game must reach ENDED with consistent score. Deterministic across two runs.
static void test_t022() {
    auto run_once = [](Snapshot *final_snap) {
        Game g; g.init(); g.start_game();
        for (int t = 0; t < 10000 && g.phase != ENDED; t++) {
            Snapshot s = g.snapshot();
            if (s.cd_remaining[BLACK] == 0 && s.legal_move[BLACK]) {
                auto m = legal_moves(g.board, BLACK);
                g.place(BLACK, m[0]);
            }
            if (s.cd_remaining[WHITE] == 0) { int m = g.ai_choose_move(); if (m >= 0) g.place(WHITE, m); }
            g.tick_second();
            Snapshot ns = g.snapshot();
            for (int p = 0; p < CELLS; p++) CHECK(ns.board[p].meter >= 0 && ns.board[p].meter <= 100);
        }
        *final_snap = g.snapshot();
        return g.phase == ENDED;
    };
    Snapshot s1, s2;
    CHECK(run_once(&s1));
    CHECK(run_once(&s2));
    CHECK(s1.tick == s2.tick && s1.score[0] == s2.score[0] && s1.score[1] == s2.score[1]);
    CHECK(s1.score[0] + s1.score[1] == CELLS);           // every stone resolves to a camp
    int expected = s1.score[0] > s1.score[1] ? RESULT_BLACK_WIN : (s1.score[1] > s1.score[0] ? RESULT_WHITE_WIN : RESULT_DRAW);
    CHECK(s1.result == expected);
};

// T025: prediction fields — camp = dominant end, seconds = distance / net pressure;
// frozen stones have no prediction. SC-004 worked example.
static void test_t025() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(3, 0));
    for (int c = 1; c <= 4; c++) g.test_place(WHITE, cell(3, c));         // no line yet
    g.test_place(BLACK, cell(3, 5));                                       // line through 4 whites
    CHECK(g.board.at(cell(3, 4)).state == CONTESTED);
    g.test_set_meter(cell(3, 4), 62);
    Snapshot s = g.snapshot();
    CHECK(s.predictions[cell(3, 4)].camp == BLACK);                        // dominant end = black
    CHECK(std::abs(s.predictions[cell(3, 4)].seconds - 9.5) < 1e-9);      // (100-62)/ceil(4/1)

    Game k; k.test_reset_board(); k.start_game();
    k.test_place(BLACK, cell(2, 0)); k.test_place(WHITE, cell(2, 1)); k.test_place(BLACK, cell(2, 2));
    CHECK(k.snapshot().predictions[cell(2, 1)].camp == BLACK);             // active line: prediction
    k.test_set_state(cell(2, 0), FIXED); k.test_set_camp(cell(2, 0), WHITE);   // line broken
    Snapshot fs = k.snapshot();
    CHECK(fs.predictions[cell(2, 1)].camp == -1);                         // frozen: no prediction
}

// Quickstart 5: neither camp has a legal move (board not full) -> game ends; draw possible.
static void test_end_no_moves_draw() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(0, 0)); g.test_place(WHITE, cell(7, 7));   // isolated stones
    CHECK(!has_legal_move(g.board, BLACK) && !has_legal_move(g.board, WHITE));
    g.tick_second();
    CHECK(g.phase == ENDED);
    Snapshot s = g.snapshot();
    CHECK(s.score[BLACK] == 1 && s.score[WHITE] == 1);
    CHECK(s.result == RESULT_DRAW);
}

// Quickstart 6 / US4: restart mid-game -> fresh 4-stone start, both CDs ready, no events.
static void test_restart_mid_game() {
    Game g; g.start_game();
    CHECK(g.place(BLACK, cell(3, 2)));
    for (int t = 0; t < 3; t++) g.tick_second();
    CHECK(g.board.at(cell(3, 3)).state == CONTESTED);
    g.init(); g.start_game();
    Game fresh; fresh.start_game();
    Snapshot a = g.snapshot(), b = fresh.snapshot();
    CHECK(a.tick == 0 && a.phase == PLAYING);
    CHECK(a.cd_remaining[BLACK] == 0 && a.cd_remaining[WHITE] == 0);
    CHECK(a.pending[BLACK] == -1 && a.pending[WHITE] == -1);
    for (int p = 0; p < CELLS; p++)
        CHECK(a.board[p].camp == b.board[p].camp && a.board[p].state == b.board[p].state);
    CHECK(g.events_since(0).empty());
    CHECK(g.board.lines.empty());
}

// Same-cell collision: black resolves first, white's queued move is cancelled and its CD refunded.
static void test_same_cell_collision() {
    Game g; g.test_reset_board(); g.start_game();
    g.test_place(BLACK, cell(3, 0)); g.test_place(WHITE, cell(3, 1));
    g.test_place(BLACK, cell(3, 3)); g.test_place(WHITE, cell(3, 4));
    // (3,2): black sandwiches (3,1) with (3,0); white sandwiches (3,3) with (3,4).
    CHECK(g.place(BLACK, cell(3, 2)));
    CHECK(g.place(WHITE, cell(3, 2)));
    g.tick_second();
    CHECK(g.board.at(cell(3, 2)).camp == BLACK);
    CHECK(g.snapshot().cd_remaining[WHITE] == 0);
    CHECK(count_events(g, EV_PLACED) == 1);
}

int main() {
    test_t010(); test_t011(); test_tug_of_war(); test_t012(); test_t013(); test_t014();
    test_t015(); test_t016(); test_t017(); test_t022(); test_t025();
    test_end_no_moves_draw(); test_restart_mid_game(); test_same_cell_collision();
    printf("engine tests: %d checks, %d failures\n", g_checks, g_fails);
    return g_fails == 0 ? 0 : 1;
}
