#pragma once
#include <vector>
#include "../engine/types.h"
#include "layout.h"

namespace app {

// Short-lived visual effect triggered by an engine event (UI only).
struct Effect {
    enum Kind { PLACE = 0, FLIP = 1 };
    int kind = PLACE;
    int cell = -1;
    int camp = -1;
    unsigned long long start_ms = 0;
};

// Per-frame UI-only state (not game logic).
struct UiState {
    bool pass[2] = {false, false};   // a camp passed in the latest second
    int hover_cell = -1;             // board cell under the mouse
    std::vector<int> hover_captures; // engine preview of stones the hover move would contest
    std::vector<Effect> effects;
    unsigned long long now_ms = 0;
    double tick_frac = 0.0;          // 0..1 progress into the current engine second
};

// UI renders from snapshot() only — no game logic here (constitution I/V).
void render_init();                                   // creates font handles
void render_title(const Layout &L);                   // Japanese rules + start button
void render_game(const othello::Snapshot &s, const Layout &L, const UiState &ui);
void render_end(const othello::Snapshot &s, const Layout &L, const UiState &ui);

} // namespace app
