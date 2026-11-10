#pragma once
#include "layout.h"

namespace app {

struct Mouse {
    int x = 0, y = 0;        // in logical screen coordinates
    bool clicked = false;    // left button went down this frame
};

// Poll once per frame; click is edge-detected (one placement per press).
Mouse poll_mouse();

// Edge-detected key press (true once per press). Uses DxLib KEY_INPUT_* codes.
bool key_pressed(int key);

} // namespace app
