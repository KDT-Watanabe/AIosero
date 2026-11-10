#pragma once
#include "board.h"
#include "rules.h"

namespace othello {

class Game {
public:
    Board board;
    Cooldown cd[2];
    int tick = 0;
    int phase = TITLE;
    int result = RESULT_ONGOING;

    Game() { init(); }

    // Standard start, both CDs ready at t=0 (T017). Phase starts as TITLE;
    // the UI's start button calls start_game() to enter PLAYING.
    Snapshot init() {
        board.standard_start();
        cd[BLACK] = Cooldown{BLACK, 0};
        cd[WHITE] = Cooldown{WHITE, 0};
        tick = 0; phase = TITLE; result = RESULT_ONGOING;
        pending[BLACK] = pending[WHITE] = -1;
        events.clear(); event_seq = 0;
        for (int i = 0; i < CELLS; i++) was_frozen[i] = false;
        return snapshot();
    }

    void start_game() { phase = PLAYING; }

    // Legal only if a sandwich line exists for an empty cell; starts that camp's CD.
    // The stone is queued and resolved at the next tick_second (black first).
    bool place(int camp, int cell) {
        if (phase != PLAYING) return false;
        if (board.at(cell).pos >= 0) return false;              // occupied
        if (cd[camp].remaining != 0) return false;              // CD not ready
        if (!has_sandwich(board, camp, cell)) return false;     // must create a line now
        pending[camp] = cell;
        cd[camp].remaining = CD_SECONDS;                        // starts on placement (T014)
        return true;
    }

    // One simulated second: resolve due placements (black first), recompute N,k,
    // apply net pressure, resolve flips, check end condition (T015).
    void tick_second() {
        if (phase != PLAYING) return;
        tick++;

        // Stones contested this second start at 50 and receive pressure from the next second.
        bool fresh[CELLS] = {};

        for (int camp : {BLACK, WHITE}) {
            if (pending[camp] >= 0) {
                int cell = pending[camp];
                pending[camp] = -1;
                // Re-validate: black resolves first, so white's queued cell may now be
                // occupied or no longer a sandwich. Cancel it and give the CD back.
                if (!has_sandwich(board, camp, cell)) {
                    cd[camp].remaining = 0;
                    continue;
                }
                auto newly = board.place_and_create_lines(camp, cell);
                add_event(EV_PLACED, camp, cell);
                for (int c : newly) { add_event(EV_CONTESTED, camp, c); was_frozen[c] = false; fresh[c] = true; }
            } else if (cd[camp].remaining > 0) {
                cd[camp].remaining--;
                if (cd[camp].remaining == 0 && !has_legal_move(board, camp)) {
                    add_event(EV_PASSED, camp, -1);             // expired with no legal move
                    cd[camp].remaining = CD_SECONDS;            // pass restarts the CD
                }
            } else {
                if (!has_legal_move(board, camp)) {
                    add_event(EV_PASSED, camp, -1);
                    cd[camp].remaining = CD_SECONDS;
                }
            }
        }

        // Board filled (or both camps stuck) by this second's placements: end now,
        // before any further meter movement.
        if (check_end()) return;

        int net[CELLS]; bool active[CELLS];
        compute_net_pressure(board, net, active);
        for (int p = 0; p < CELLS; p++) if (fresh[p]) { active[p] = false; net[p] = 0; }

        // Frozen transitions: RESCUED fires when a contested stone goes from
        // active to frozen (all its lines inactive).
        for (int p = 0; p < CELLS; p++) {
            if (board.at(p).state != CONTESTED || fresh[p]) continue;
            if (!was_frozen[p] && !active[p]) { add_event(EV_RESCUED, -1, p); was_frozen[p] = true; }
            else if (was_frozen[p] && active[p]) { was_frozen[p] = false; }
        }

        for (auto &fl : apply_second(board, net, active)) add_event(EV_FLIPPED, fl.second, fl.first);

        check_end();
    }

    // End when the board is full or neither camp has a legal move (FR-009).
    bool check_end() {
        if (!board_full(board) && (has_legal_move(board, BLACK) || has_legal_move(board, WHITE)))
            return false;
        phase = ENDED;
        pending[BLACK] = pending[WHITE] = -1;
        int score[2]; board.compute_score(score);
        result = score[0] > score[1] ? RESULT_BLACK_WIN : (score[1] > score[0] ? RESULT_WHITE_WIN : RESULT_DRAW);
        add_event(EV_GAME_OVER, -1, -1);
        return true;
    }

    // Cell queued by `camp` for the next tick, or -1 (UI shows it as a ghost stone).
    int pending_cell(int camp) const { return pending[camp]; }

    // Pure function of board state; deterministic; tiebreak = lowest cell index
    // among equal-value moves. Implemented in ai.cpp.
    int ai_choose_move() const;

    // Test hook: depth-2 value of playing `cell` as WHITE (T016 tiebreak check).
    long long ai_eval_move(int cell) const;

    Snapshot snapshot() const {
        Snapshot s;
        s.tick = tick; s.phase = phase; s.result = result;
        for (int p = 0; p < CELLS; p++) {
            const Stone &st = board.at(p);
            s.board[p] = {st.camp, st.state, st.meter, st.origin_camp, resolved_owner(st)};
            s.predictions[p] = {-1, -1, 0.0};
        }
        s.cd_remaining[BLACK] = cd[BLACK].remaining;
        s.cd_remaining[WHITE] = cd[WHITE].remaining;
        s.pending[BLACK] = pending[BLACK]; s.pending[WHITE] = pending[WHITE];
        s.legal_move[BLACK] = has_legal_move(board, BLACK);
        s.legal_move[WHITE] = has_legal_move(board, WHITE);
        for (int c = 0; c < 2; c++) for (int p = 0; p < CELLS; p++) s.legal[c][p] = has_sandwich(board, c, p);
        board.compute_score(s.score);

        int net[CELLS]; bool active[CELLS];
        compute_net_pressure(board, net, active);
        for (int p = 0; p < CELLS; p++) {
            if (board.at(p).state != CONTESTED) continue;
            if (!active[p] || net[p] == 0) continue;            // frozen: no prediction
            int camp = net[p] > 0 ? BLACK : WHITE;
            double secs = (net[p] > 0) ? (100.0 - board.at(p).meter) / net[p]
                                       : (double)board.at(p).meter / -net[p];
            s.predictions[p] = {p, camp, secs};
        }
        return s;
    }

    std::vector<Event> events_since(int n) const {
        std::vector<Event> out;
        for (const Event &e : events) if (e.id >= n) out.push_back(e);
        return out;
    }

    // Test-only helpers (tests/engine_tests.cpp).
    void test_place(int camp, int cell) { board.place_and_create_lines(camp, cell); }
    void test_set_meter(int cell, int m) { board.cells[cell].meter = m; }
    void test_set_state(int cell, int st) { board.cells[cell].state = st; }
    void test_set_camp(int cell, int c) { board.cells[cell].camp = c; }
    void test_reset_board() {
        board.reset();
        for (int i = 0; i < CELLS; i++) was_frozen[i] = false;
    }

private:
    int pending[2] = {-1, -1};
    bool was_frozen[CELLS] = {};
    std::vector<Event> events;
    int event_seq = 0;

    void add_event(int type, int camp, int cell) {
        Event e; e.id = event_seq++; e.type = type; e.tick = tick; e.cell = cell; e.camp = camp;
        events.push_back(e);
    }
};

} // namespace othello
