#include "board.h"
#include <cstdlib>
#include <algorithm>

namespace othello {

void Board::reset() {
    for (int i = 0; i < CELLS; i++) cells[i] = Stone();
    lines.clear();
}

// Standard 4-stone start (T017): White d4,e5 / Black e4,d5.
// Row/col are 0-based: row 3 = rank 4, col 3 = file d.
void Board::standard_start() {
    reset();
    const int whites[2] = {3 * BOARD_SIZE + 3, 4 * BOARD_SIZE + 4};  // d4, e5
    const int blacks[2] = {3 * BOARD_SIZE + 4, 4 * BOARD_SIZE + 3};  // e4, d5
    for (int camp : {BLACK, WHITE}) {
        const int *set = (camp == BLACK) ? blacks : whites;
        for (int i = 0; i < 2; i++) {
            int p = set[i];
            Stone &s = cells[p];
            s.pos = p; s.camp = camp; s.state = NORMAL;
            s.origin_camp = camp; s.meter = 0; s.placed_by = camp;
        }
    }
}

int Board::chebyshev(int a, int b) const {
    int r1 = a / BOARD_SIZE, c1 = a % BOARD_SIZE;
    int r2 = b / BOARD_SIZE, c2 = b % BOARD_SIZE;
    return std::max(std::abs(r1 - r2), std::abs(c1 - c2));
}

int Board::nearest_own_distance(int pos, int camp) const {
    int best = 99;
    for (int p = 0; p < CELLS; p++) {
        if (cells[p].pos < 0) continue;
        if (p != pos && counts_as(p) == camp) best = std::min(best, chebyshev(pos, p));
    }
    return best;
}

// Place stone + create lines for every direction (lines exist only from placement time).
// Chain stones that are NORMAL become CONTESTED at meter 50. Returns newly contested cells.
std::vector<int> Board::place_and_create_lines(int camp, int cell) {
    Stone &s = cells[cell];
    s.pos = cell; s.camp = camp; s.state = NORMAL;
    s.origin_camp = camp; s.meter = 0; s.placed_by = camp;

    std::vector<int> newly_contested;
    static const int DIRS[8][2] = {
        {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}
    };
    for (auto &d : DIRS) {
        std::vector<int> chain; int far_end = -1;
        bool defend = false;
        int r = cell / BOARD_SIZE + d[0], c = cell % BOARD_SIZE + d[1];
        // First stone decides the line kind: opponent stone -> attack (sandwich) line;
        // own CONTESTED stone -> support line over a run of own contested stones.
        if (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
            int pos = r * BOARD_SIZE + c;
            defend = cells[pos].pos >= 0 && counts_as(pos) == camp && cells[pos].state == CONTESTED;
        }
        while (r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE) {
            int pos = r * BOARD_SIZE + c;
            if (cells[pos].pos < 0) break;                 // empty breaks the line
            bool own = counts_as(pos) == camp;
            if (defend) {
                if (!own) break;                           // support run must be all own stones
                if (cells[pos].state == CONTESTED) chain.push_back(pos);
                else { far_end = pos; break; }             // own settled stone closes the run
            } else if (!own) chain.push_back(pos);
            else { if (!chain.empty()) { far_end = pos; break; } else break; }
            r += d[0]; c += d[1];
        }
        if (chain.empty() || far_end < 0) continue;
        SandwichLine L; L.camp = camp; L.anchor = cell; L.chain = chain; L.far_end = far_end;
        L.active = true;                                    // active while far end holds our camp
        L.defend = defend;
        lines.push_back(L);
        if (defend) continue;                               // support lines never contest stones
        for (int pos : chain) {
            Stone &cs = cells[pos];
            if (cs.state == NORMAL) {                      // contest only at placement time
                cs.state = CONTESTED; cs.meter = 50;
                newly_contested.push_back(pos);
            }
        }
    }
    return newly_contested;
}

// Score per data-model.md: fixed/normal by camp, contested by meter direction,
// meter == 50 resolves to origin camp.
void Board::compute_score(int score[2]) const {
    score[0] = score[1] = 0;
    for (int p = 0; p < CELLS; p++) {
        const Stone &s = cells[p];
        if (s.pos < 0) continue;
        score[resolved_owner(s)]++;                        // 50-rule inside
    }
}

} // namespace othello
