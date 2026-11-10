#include "input.h"
#include <DxLib.h>

namespace app {

Mouse poll_mouse() {
    Mouse m;
    GetMousePoint(&m.x, &m.y);
    // Click log catches presses shorter than a frame; keep the last left-button press.
    int button, cx, cy, type;
    while (GetMouseInputLog2(&button, &cx, &cy, &type, TRUE) == 0) {
        if (button == MOUSE_INPUT_LEFT && type == MOUSE_INPUT_LOG_DOWN) {
            m.clicked = true; m.x = cx; m.y = cy;
        }
    }
    return m;
}

bool key_pressed(int key) {
    static bool prev[256] = {};
    bool down = CheckHitKey(key) != 0;
    bool edge = down && !prev[key & 0xFF];
    prev[key & 0xFF] = down;
    return edge;
}

} // namespace app
