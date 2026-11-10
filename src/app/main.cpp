// Tension Othello — DxLib front-end. UI derives display state only from engine
// snapshot()/events_since(); all game logic lives in the engine (constitution I/V).
#include <DxLib.h>
#include <windows.h>
#include <algorithm>
#include "../engine/game.h"
#include "layout.h"
#include "render.h"
#include "input.h"
#include "sound.h"

using namespace othello;
using namespace app;

// Deterministic 0.5–2.0 s presentation delay for the AI (display only; the move itself
// is chosen by the deterministic engine AI at fire time).
static unsigned long long ai_delay_ms(int tick) {
    return 500 + ((unsigned)tick * 7919u + 104729u) % 1501u;
}

// Run from the exe's folder so assets/ resolves regardless of how it was launched.
static void chdir_to_exe() {
    wchar_t path[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return;
    for (int i = (int)n - 1; i >= 0; i--) if (path[i] == L'\\' || path[i] == L'/') { path[i] = 0; break; }
    SetCurrentDirectoryW(path);
}

struct Session {
    Game game;
    int cursor = 0;                       // events_since cursor (constitution V)
    UiState ui;
    unsigned long long last_tick = 0;
    bool ai_scheduled = false;
    unsigned long long ai_fire = 0;

    void restart(bool play) {             // T027: re-init engine, reset UI state
        game.init();
        if (play) game.start_game();
        cursor = 0; ui = UiState{};
        ai_scheduled = false;
        last_tick = GetTickCount64();
    }
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    chdir_to_exe();
    SetOutApplicationLogValidFlag(FALSE);
    SetMainWindowText(L"テンション・オセロ");
    ChangeWindowMode(TRUE);
    SetGraphMode(SCREEN_W, SCREEN_H, 32);
    SetWindowSizeChangeEnableFlag(TRUE, TRUE);    // T030: resizable, surface scales to fit
    SetAlwaysRunFlag(TRUE);                       // real-time game keeps ticking when unfocused
    if (DxLib_Init() != 0) return 1;
    SetDrawScreen(DX_SCREEN_BACK);
    render_init();
    sound_init();

    Session S;
    S.restart(false);                             // TITLE phase
    Layout L = compute_layout();

    while (ProcessMessage() == 0) {
        Mouse m = poll_mouse();
        bool enter = key_pressed(KEY_INPUT_RETURN);
        bool rkey = key_pressed(KEY_INPUT_R);
        if (key_pressed(KEY_INPUT_ESCAPE)) break;
        unsigned long long now = GetTickCount64();

        int phase = S.game.snapshot().phase;
        if (phase == TITLE) {
            if ((m.clicked && title_start_button().contains(m.x, m.y)) || enter) S.restart(true);
        } else if (phase == PLAYING) {
            if ((m.clicked && restart_button(L).contains(m.x, m.y)) || rkey) S.restart(true);
            else if (m.clicked) {
                int cell = cell_at(L, m.x, m.y);
                if (cell >= 0) S.game.place(BLACK, cell);   // engine validates CD + legality
            }

            // AI (white): once its CD is ready, wait the presentation delay, then let the
            // deterministic AI pick a move on the board as it is at that moment.
            Snapshot s = S.game.snapshot();
            if (!S.ai_scheduled && s.cd_remaining[WHITE] == 0 && s.pending[WHITE] < 0) {
                S.ai_scheduled = true;
                S.ai_fire = now + ai_delay_ms(s.tick);
            }
            if (S.ai_scheduled && now >= S.ai_fire) {
                S.ai_scheduled = false;
                int mv = S.game.ai_choose_move();
                if (mv >= 0) S.game.place(WHITE, mv);
            }

            // Engine seconds.
            while (S.game.phase == PLAYING && now - S.last_tick >= 1000) {
                S.last_tick += 1000;
                int before = S.game.snapshot().cd_remaining[BLACK];
                S.game.tick_second();
                Snapshot after = S.game.snapshot();
                S.ui.pass[BLACK] = S.ui.pass[WHITE] = false;
                bool flipped = false;
                for (const Event &e : S.game.events_since(S.cursor)) {
                    S.cursor = e.id + 1;
                    if (e.type == EV_FLIPPED) {
                        flipped = true;
                        S.ui.effects.push_back({Effect::FLIP, e.cell, e.camp, now});
                    } else if (e.type == EV_PLACED) {
                        S.ui.effects.push_back({Effect::PLACE, e.cell, e.camp, now});
                    } else if (e.type == EV_PASSED) S.ui.pass[e.camp] = true;
                }
                if (flipped) play_flip();
                if (before > 0 && after.cd_remaining[BLACK] == 0) play_cd_unlock();
            }
        } else {   // ENDED
            if ((m.clicked && restart_button(L).contains(m.x, m.y)) || enter || rkey) S.restart(true);
        }

        Snapshot s = S.game.snapshot();
        int mx, my; GetMousePoint(&mx, &my);
        S.ui.hover_cell = cell_at(L, mx, my);
        S.ui.hover_captures = S.ui.hover_cell >= 0 ? preview_captures(S.game.board, BLACK, S.ui.hover_cell)
                                                   : std::vector<int>{};
        S.ui.now_ms = now;
        S.ui.tick_frac = s.phase == PLAYING ? (double)(now - S.last_tick) / 1000.0 : 0.0;
        if (S.ui.tick_frac > 1.0) S.ui.tick_frac = 1.0;
        S.ui.effects.erase(std::remove_if(S.ui.effects.begin(), S.ui.effects.end(),
                               [now](const Effect &e) { return now - e.start_ms > 1000; }),
                           S.ui.effects.end());
        if (s.phase == TITLE) render_title(L);
        else if (s.phase == PLAYING) render_game(s, L, S.ui);
        else render_end(s, L, S.ui);
        ScreenFlip();
    }
    DxLib_End();
    return 0;
}
