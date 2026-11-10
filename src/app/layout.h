#pragma once
#include "../engine/types.h"

namespace app {

// Fixed logical drawing surface. The window is resizable (T030): DxLib scales this
// surface to the window size and converts mouse coordinates back into it.
constexpr int SCREEN_W = 1040;
constexpr int SCREEN_H = 760;

struct Rect {
    int x1, y1, x2, y2;
    bool contains(int x, int y) const { return x >= x1 && x <= x2 && y >= y1 && y <= y2; }
};

struct Layout {
    int cell_px = 80;
    int ox = 40, oy = 60;            // grid origin (top-left of cell 0)
    int panel_x = 720;               // side panel left edge
};

inline Layout compute_layout() { return Layout{}; }

// Click -> board cell, or -1 when outside the grid.
inline int cell_at(const Layout &L, int mx, int my) {
    if (mx < L.ox || my < L.oy) return -1;
    int col = (mx - L.ox) / L.cell_px, row = (my - L.oy) / L.cell_px;
    if (col < 0 || col >= othello::BOARD_SIZE || row < 0 || row >= othello::BOARD_SIZE) return -1;
    return row * othello::BOARD_SIZE + col;
}

// Buttons shared by render and input.
inline Rect title_start_button() { return {SCREEN_W / 2 - 120, 620, SCREEN_W / 2 + 120, 680}; }
inline Rect restart_button(const Layout &L) { return {L.panel_x, 640, L.panel_x + 260, 690}; }

} // namespace app
