// Static AI per research.md #5: depth-2 minimax (white move + black best response),
// heuristic = resolved stone count + positional bonus + meter momentum.
// Deterministic; deterministic tiebreak = lowest cell index among equal-value moves.
#include "game.h"
#include <climits>
#include <algorithm>

namespace othello {

static const int CORNERS[4] = {0, 7, 56, 63};

// X-squares: diagonally adjacent to a corner (bad squares).
static bool is_x_square(int p) {
    for (int c : CORNERS) {
        int dr = (p / BOARD_SIZE) - (c / BOARD_SIZE), dc = (p % BOARD_SIZE) - (c % BOARD_SIZE);
        if (std::abs(dr) == 1 && std::abs(dc) == 1) return true;
    }
    return false;
}

// Evaluate board from `perspective`'s point of view. Uses only engine state
// (snapshot-equivalent data), so it is a pure function of the board.
static long long eval_for(const Board &b, int perspective) {
    int other = 1 - perspective;
    long long v = 0;

    int net[CELLS]; bool active[CELLS];
    compute_net_pressure(b, net, active);

    for (int p = 0; p < CELLS; p++) {
        const Stone &s = b.at(p);
        if (s.pos < 0) continue;

        int owner;   // camp the stone resolves to right now
        long long w;
        if (s.state == CONTESTED) {
            if (s.meter > 50) owner = BLACK;
            else if (s.meter < 50) owner = WHITE;
            else owner = s.origin_camp;                    // meter exactly 50: origin camp
            w = (owner == perspective) ? 3 : -3;           // contested stone worth less than fixed
            if (active[p] && net[p] != 0) {                // momentum bonus toward dominant end
                int dom = net[p] > 0 ? BLACK : WHITE;
                long long m = std::min(10, std::abs(net[p]));
                w += (dom == perspective) ? m : -m;
            }
        } else {
            owner = s.camp;
            w = (owner == perspective) ? 4 : -4;           // fixed / normal stone
        }

        if (std::find(CORNERS, CORNERS + 4, p) != CORNERS + 4) w += (owner == perspective) ? 5 : -5;
        else if (is_x_square(p)) w += (owner == perspective) ? -4 : 4;

        v += w;
    }

    long long my_moves = (long long)legal_moves(b, perspective).size();
    long long opp_moves = (long long)legal_moves(b, other).size();
    v += my_moves - opp_moves;   // mobility term
    return v;
}

// Depth-2 value of `camp` playing `cell`: own gain minus opponent's best reply.
static long long move_value(const Board &b, int camp, int cell) {
    Board sim = b;
    sim.place_and_create_lines(camp, cell);

    auto replies = legal_moves(sim, 1 - camp);
    if (replies.empty()) return eval_for(sim, camp);

    long long worst = LLONG_MAX;   // opponent maximizes their own eval = minimizes ours
    for (int r : replies) {
        Board sim2 = sim;
        sim2.place_and_create_lines(1 - camp, r);
        worst = std::min(worst, eval_for(sim2, camp));
    }
    return worst;
}

int Game::ai_choose_move() const {
    auto moves = legal_moves(board, WHITE);          // ascending order
    if (moves.empty()) return -1;

    long long best_val = LLONG_MIN;
    int best = -1;
    for (int m : moves) {                            // strict > keeps the lowest index on ties
        long long val = move_value(board, WHITE, m);
        if (val > best_val) { best_val = val; best = m; }
    }
    return best;
}

long long Game::ai_eval_move(int cell) const {
    return move_value(board, WHITE, cell);
}

} // namespace othello
