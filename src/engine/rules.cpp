#include "rules.h"
#include <algorithm>

namespace othello {

static const int DIRS[8][2] = {
    {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
};

// Walk from `cell` in direction (dr,dc) through contiguous stones: collect those
// counting as the line's opponent; a sandwich line exists when >=1 chain stone is
// followed by a stone counting as camp.
static bool walk_line(const Board &b, int camp, int cell, int dr, int dc,
                     std::vector<int> &chain, int &far_end) {
    chain.clear(); far_end = -1;
    int r = cell / BOARD_SIZE + dr, c = cell % BOARD_SIZE + dc;
    while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
        int pos = r * BOARD_SIZE + c;
        if (b.at(pos).pos < 0) return false;                    // empty breaks the line
        if (b.counts_as(pos) != camp) chain.push_back(pos);
        else { if (!chain.empty()) { far_end = pos; return true; } return false; }
        r += dr; c += dc;
    }
    return false;
}

bool has_sandwich(const Board &b, int camp, int cell) {
    if (b.at(cell).pos >= 0) return false;
    std::vector<int> chain; int far_end;
    for (auto &d : DIRS) if (walk_line(b, camp, cell, d[0], d[1], chain, far_end)) return true;
    return false;
}

std::vector<int> legal_moves(const Board &b, int camp) {
    std::vector<int> out;
    for (int p = 0; p < CELLS; p++) if (has_sandwich(b, camp, p)) out.push_back(p);
    return out;   // ascending order -> deterministic tiebreak uses lowest index
}

bool has_legal_move(const Board &b, int camp) {
    std::vector<int> chain; int far_end;
    for (int p = 0; p < CELLS; p++) {
        if (b.at(p).pos >= 0) continue;
        for (auto &d : DIRS) if (walk_line(b, camp, p, d[0], d[1], chain, far_end)) return true;
    }
    return false;
}

// Recompute N,k for every line every second (T011). Pure computation.
void compute_net_pressure(const Board &b, int net[CELLS], bool has_active[CELLS]) {
    for (int i = 0; i < CELLS; i++) { net[i] = 0; has_active[i] = false; }
    for (const SandwichLine &L : b.lines) {
        // Active only while the far end still counts as the line's camp.
        if (b.at(L.far_end).pos < 0 || b.counts_as(L.far_end) != L.camp) continue;
        int N = 0;
        for (int pos : L.chain) {
            if (L.defend) { if (b.at(pos).state == CONTESTED) N++; }   // support: own stones still contested
            else if (b.counts_as(pos) != L.camp) N++;
        }
        if (N == 0) continue;
        for (int pos : L.chain) {
            if (b.at(pos).state != CONTESTED) continue;         // fixed stones: no pressure
            int k = b.nearest_own_distance(pos, L.camp);
            if (k <= 0 || k >= 99) continue;
            int p = (N + k - 1) / k;                            // ceil(N/k)
            net[pos] += (L.camp == BLACK) ? p : -p;
            has_active[pos] = true;
        }
    }
}

// Apply pressure, then resolve flips after all pressure is applied (flip effects on
// line activity take effect the following second). Meter clamped to [0,100].
std::vector<std::pair<int,int>> apply_second(Board &b, const int net[CELLS],
                                             const bool has_active[CELLS]) {
    for (int p = 0; p < CELLS; p++) {
        if (b.at(p).state != CONTESTED || !has_active[p]) continue;
        b.cells[p].meter += net[p];
        if (b.cells[p].meter > 100) b.cells[p].meter = 100;
        if (b.cells[p].meter < 0) b.cells[p].meter = 0;
    }
    std::vector<std::pair<int,int>> flips;
    for (int p = 0; p < CELLS; p++) {
        Stone &s = b.cells[p];
        if (s.state != CONTESTED) continue;
        if (s.meter == 100 || s.meter == 0) {
            s.state = FIXED;
            s.camp = (s.meter == 100) ? BLACK : WHITE;
            flips.push_back({p, s.camp});
        }
    }
    return flips;
}

bool board_full(const Board &b) {
    for (int p = 0; p < CELLS; p++) if (b.at(p).pos < 0) return false;
    return true;
}

} // namespace othello
