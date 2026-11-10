#pragma once
#include "board.h"

namespace othello {

// Legal placement = empty cell + at least one sandwich line in 8 directions (T010).
bool has_sandwich(const Board &b, int camp, int cell);
std::vector<int> legal_moves(const Board &b, int camp);

// Recompute N,k for every line every second (T011):
//   N = chain stones currently counting as the line's opponent,
//   k = Chebyshev distance from a contested stone to its nearest own-camp stone,
//   p = ceil(N/k) toward the line's camp end.
// Outputs net pressure per contested stone (net>0 = toward black end / meter 100)
// and has_active per stone (false = frozen, no prediction). Pure computation.
void compute_net_pressure(const Board &b, int net[CELLS], bool has_active[CELLS]);

bool has_legal_move(const Board &b, int camp);

// Apply one second of pressure to active contested stones and resolve flips
// (meter reaching 0/100 exactly). Returns (cell, new_camp) for each flip so the
// Game layer can emit FLIPPED events.
std::vector<std::pair<int,int>> apply_second(Board &b, const int net[CELLS],
                                             const bool has_active[CELLS]);

bool board_full(const Board &b);

} // namespace othello
