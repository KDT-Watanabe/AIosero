#pragma once
#include "types.h"

namespace othello {

class Board {
public:
    Stone cells[CELLS];
    std::vector<SandwichLine> lines;

    Board() { reset(); }
    void reset();

    // Standard 4-stone start (T017): White d4/e5, Black e4/d5.
    void standard_start();

    const Stone &at(int pos) const { return cells[pos]; }

    int counts_as(int pos) const { return othello::counts_as(cells[pos]); }

    // Place a stone on an empty cell and create sandwich lines for every direction
    // (lines are created only at placement time). Newly sandwiched NORMAL stones
    // become CONTESTED with meter 50. Returns the newly contested cells.
    std::vector<int> place_and_create_lines(int camp, int cell);

    int chebyshev(int a, int b) const;

    // Nearest own-camp stone distance for a contested stone (k in p = ceil(N/k)).
    int nearest_own_distance(int pos, int camp) const;

    // Score per data-model.md: fixed/normal by camp + contested by meter direction
    // (50 -> origin camp).
    void compute_score(int score[2]) const;
};

} // namespace othello
