#pragma once
#include "../engine/types.h"
#include "layout.h"

namespace app {

// Per-frame UI-only state (not game logic).
struct UiState {
    bool pass[2] = {false, false};   // a camp passed in the latest second
    int hover_cell = -1;             // board cell under the mouse
};

// UI renders from snapshot() only — no game logic here (constitution I/V).
void render_init();                                   // creates font handles
void render_title(const Layout &L);                   // Japanese rules + start button
void render_game(const othello::Snapshot &s, const Layout &L, const UiState &ui);
void render_end(const othello::Snapshot &s, const Layout &L);

} // namespace app
