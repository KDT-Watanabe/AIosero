#include "render.h"
#include <DxLib.h>
#include <cwchar>
#include <cstring>
#include <cmath>

using namespace othello;

namespace app {

static int f_small = -1, f_mid = -1, f_big = -1, f_title = -1, f_meter = -1, f_label = -1, f_huge = -1;

static unsigned int C_BG, C_BOARD, C_BOARD_HI, C_GRID, C_TEXT, C_SUB, C_STONE_B, C_STONE_W,
                    C_ACC_B, C_ACC_W, C_BTN, C_HINT, C_FIXED, C_TRACK, C_DARK;

static const float PI = 3.14159265f;

void render_init() {
    const wchar_t *face = L"Meiryo UI";
    f_small = CreateFontToHandle(face, 15, 2, DX_FONTTYPE_ANTIALIASING_4X4);
    f_label = CreateFontToHandle(face, 15, 6, DX_FONTTYPE_ANTIALIASING_4X4);
    f_mid   = CreateFontToHandle(face, 20, 3, DX_FONTTYPE_ANTIALIASING_4X4);
    f_big   = CreateFontToHandle(face, 28, 5, DX_FONTTYPE_ANTIALIASING_4X4);
    f_huge  = CreateFontToHandle(face, 34, 8, DX_FONTTYPE_ANTIALIASING_4X4);
    f_title = CreateFontToHandle(face, 48, 7, DX_FONTTYPE_ANTIALIASING_4X4);
    f_meter = CreateFontToHandle(face, 22, 7, DX_FONTTYPE_ANTIALIASING_4X4);

    C_BG       = GetColor(24, 28, 34);
    C_BOARD    = GetColor(32, 96, 64);
    C_BOARD_HI = GetColor(38, 110, 74);
    C_GRID     = GetColor(18, 60, 40);
    C_TEXT     = GetColor(232, 236, 240);
    C_SUB      = GetColor(150, 160, 170);
    C_STONE_B  = GetColor(16, 16, 20);
    C_STONE_W  = GetColor(240, 240, 236);
    C_ACC_B    = GetColor(70, 140, 255);   // black side (meter toward 100)
    C_ACC_W    = GetColor(255, 170, 40);   // white side (meter toward 0)
    C_BTN      = GetColor(60, 100, 160);
    C_HINT     = GetColor(140, 230, 170);
    C_FIXED    = GetColor(230, 60, 60);
    C_TRACK    = GetColor(20, 50, 34);
    C_DARK     = GetColor(10, 12, 16);
}

static void text(int x, int y, unsigned int c, int f, const wchar_t *s) {
    DrawStringToHandle(x, y, s, c, f);
}
static int text_w(int f, const wchar_t *s) {
    return GetDrawStringWidthToHandle(s, (int)wcslen(s), f);
}
static void text_center(int cx, int y, unsigned int c, int f, const wchar_t *s) {
    text(cx - text_w(f, s) / 2, y, c, f, s);
}
static const wchar_t *camp_name(int camp) { return camp == BLACK ? L"黒" : L"白"; }
static unsigned int camp_accent(int camp) { return camp == BLACK ? C_ACC_B : C_ACC_W; }

static void button(const Rect &r, const wchar_t *label, bool hot) {
    DrawBox(r.x1, r.y1, r.x2, r.y2, hot ? GetColor(80, 130, 200) : C_BTN, TRUE);
    DrawBox(r.x1, r.y1, r.x2, r.y2, C_TEXT, FALSE);
    text_center((r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 - 15, C_TEXT, f_big, label);
}

static bool mouse_in(const Rect &r) {
    int mx, my; GetMousePoint(&mx, &my);
    return r.contains(mx, my);
}

// Arc from fraction a to b of a full turn, clockwise starting at 12 o'clock.
static void arc(float cx, float cy, float r, float a, float b, unsigned int col, float thick) {
    if (b <= a) return;
    float ri = r - thick / 2, ro = r + thick / 2;
    int steps = (int)((b - a) * 96) + 1;
    for (int i = 0; i < steps; i++) {
        float t0 = a + (b - a) * i / steps, t1 = a + (b - a) * (i + 1) / steps;
        float g0 = -PI / 2 + 2 * PI * t0, g1 = -PI / 2 + 2 * PI * t1;
        float c0 = cosf(g0), s0 = sinf(g0), c1 = cosf(g1), s1 = sinf(g1);
        DrawQuadrangleAA(cx + ri * c0, cy + ri * s0, cx + ro * c0, cy + ro * s0,
                         cx + ro * c1, cy + ro * s1, cx + ri * c1, cy + ri * s1, col, TRUE);
    }
}

static void cell_center(const Layout &L, int p, float &cx, float &cy) {
    cx = L.ox + (p % BOARD_SIZE) * L.cell_px + L.cell_px / 2.0f;
    cy = L.oy + (p / BOARD_SIZE) * L.cell_px + L.cell_px / 2.0f;
}

void render_title(const Layout &) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    text_center(SCREEN_W / 2, 40, C_TEXT, f_title, L"テンション・オセロ");
    text_center(SCREEN_W / 2, 102, C_SUB, f_mid, L"〜 挟んだ石は、すぐには裏返らない 〜");

    wchar_t cd_line[96];
    swprintf(cd_line, 96, L"■ 手番はありません。黒(あなた)も白(AI)も、置くと %d 秒のクールダウン。", CD_SECONDS);
    const wchar_t *rules[] = {
        L"■ 置き方は普通のオセロと同じ。相手の石を挟めるマスにだけ置けます。",
        cd_line,
        L"   クールダウンが明けたら、いつでもクリックで置けます(置ける所が無ければパス)。",
        L"■ 挟まれた石は裏返らず「綱引き」が始まります。石の周りのリングが張力メーター。",
        L"   青が一周すると黒に、橙が一周すると白に裏返り、それ以降は二度と裏返りません。",
        L"■ 押す力は、挟んだ石が多いほど・自分の石が近いほど強くなります。",
        L"   盤上の青/橙の線が「いま押している線」、太いほど強い力です。",
        L"■ 押し返し: 挟まれた自分の石を、さらに自分の石で挟み直すと逆向きに押せます。",
        L"■ 救出: 挟んでいる線の端の石が裏返ると線が切れ、綱引きは止まります。",
        L"■ 盤が埋まるか両者とも置けなくなったら終局。リングが傾いている側の色で数えます。",
    };
    int y = 160;
    for (const wchar_t *line : rules) { text(80, y, C_TEXT, f_mid, line); y += 40; }

    Rect b = title_start_button();
    button(b, L"スタート", mouse_in(b));
    text_center(SCREEN_W / 2, 692, C_SUB, f_small, L"クリック または Enter で開始 / ゲーム中 R でリスタート");
}

static void draw_stone(float cx, float cy, float r, int camp, int alpha = 255) {
    if (alpha < 255) SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    else DrawCircleAA(cx + 2, cy + 3, r, 40, GetColor(10, 40, 25), TRUE);   // shadow
    DrawCircleAA(cx, cy, r, 40, camp == BLACK ? C_STONE_B : C_STONE_W, TRUE);
    if (alpha < 255) SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

static void draw_board(const Snapshot &s, const Layout &L, const UiState &ui, bool black_ready) {
    int bw = L.cell_px * BOARD_SIZE;
    // Frame glows blue while the player can place.
    unsigned int frame = black_ready && s.legal_move[BLACK] ? C_ACC_B : GetColor(60, 40, 24);
    DrawBox(L.ox - 7, L.oy - 7, L.ox + bw + 7, L.oy + bw + 7, frame, TRUE);
    DrawBox(L.ox, L.oy, L.ox + bw, L.oy + bw, C_BOARD, TRUE);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        DrawLine(L.ox + i * L.cell_px, L.oy, L.ox + i * L.cell_px, L.oy + bw, C_GRID, 2);
        DrawLine(L.ox, L.oy + i * L.cell_px, L.ox + bw, L.oy + i * L.cell_px, C_GRID, 2);
    }
    for (int i = 0; i < BOARD_SIZE; i++) {           // coordinates a-h / 1-8
        wchar_t b[4];
        swprintf(b, 4, L"%c", L'a' + i);
        text_center(L.ox + i * L.cell_px + L.cell_px / 2, L.oy - 28, C_SUB, f_small, b);
        swprintf(b, 4, L"%d", i + 1);
        text_center(L.ox - 22, L.oy + i * L.cell_px + L.cell_px / 2 - 8, C_SUB, f_small, b);
    }

    // Hover cell highlight.
    bool hover_legal = black_ready && ui.hover_cell >= 0 && s.legal[BLACK][ui.hover_cell];
    if (hover_legal) {
        int x0 = L.ox + (ui.hover_cell % BOARD_SIZE) * L.cell_px, y0 = L.oy + (ui.hover_cell / BOARD_SIZE) * L.cell_px;
        DrawBox(x0 + 1, y0 + 1, x0 + L.cell_px - 1, y0 + L.cell_px - 1, C_BOARD_HI, TRUE);
    }

    // Pushing lines (under the stones): colored by camp, thicker = stronger.
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
    for (const LineView &v : s.lines) {
        float x1, y1, x2, y2;
        cell_center(L, v.anchor, x1, y1); cell_center(L, v.far_end, x2, y2);
        float th = 5.0f + 3.0f * (float)v.pressure / PRESSURE_SCALE;
        if (th > 18.0f) th = 18.0f;
        DrawLineAA(x1, y1, x2, y2, camp_accent(v.camp), th);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

    float r = L.cell_px * 0.40f;
    for (int p = 0; p < CELLS; p++) {
        int x0 = L.ox + (p % BOARD_SIZE) * L.cell_px, y0 = L.oy + (p / BOARD_SIZE) * L.cell_px;
        float cx, cy; cell_center(L, p, cx, cy);
        const auto &c = s.board[p];

        if (c.camp < 0) {
            bool pend_b = s.pending[BLACK] == p, pend_w = s.pending[WHITE] == p;
            if (pend_b || pend_w) draw_stone(cx, cy, r, pend_b ? BLACK : WHITE, 140);   // resolves next second
            else if (hover_legal && ui.hover_cell == p) draw_stone(cx, cy, r, BLACK, 110);
            else if (black_ready && s.legal[BLACK][p]) DrawCircleAA(cx, cy, 7.0f, 24, C_HINT, TRUE);
            continue;
        }

        if (c.state == CONTESTED) {
            const auto &pr = s.predictions[p];
            // Ring = tension meter: blue share = meter (toward black 100), amber = the rest.
            float ring_r = r + 1.0f, thick = 8.0f;
            DrawCircleAA(cx, cy, ring_r, 48, C_TRACK, FALSE, thick + 2.0f);
            float m = c.meter / 100.0f;
            arc(cx, cy, ring_r, 0.0f, m, C_ACC_B, thick);
            arc(cx, cy, ring_r, m, 1.0f, C_ACC_W, thick);
            DrawCircleAA(cx, cy, r - 5, 40, c.origin_camp == BLACK ? C_STONE_B : C_STONE_W, TRUE);

            // About to flip (<= 3 s): pulse the ring.
            if (pr.camp >= 0 && pr.seconds <= 3.0) {
                int a = (int)(110 + 110 * sinf(ui.now_ms / 90.0f));
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
                DrawCircleAA(cx, cy, ring_r + 5, 48, camp_accent(pr.camp), FALSE, 3.0f);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            }

            wchar_t buf[32];
            swprintf(buf, 32, L"%d", c.meter);       // equals engine meter (SC-004)
            text_center((int)cx, (int)cy - 14, c.origin_camp == BLACK ? C_STONE_W : C_STONE_B, f_meter, buf);

            // Label pill: where it is heading and when.
            unsigned int pill = pr.camp < 0 ? GetColor(90, 96, 104) : camp_accent(pr.camp);
            if (pr.camp >= 0) swprintf(buf, 32, L"→%s %d秒", camp_name(pr.camp), (int)std::ceil(pr.seconds - 1e-9));
            else swprintf(buf, 32, L"停止中");
            int tw = text_w(f_label, buf);
            int ty = y0 + L.cell_px - 20;
            DrawBox((int)cx - tw / 2 - 5, ty, (int)cx + tw / 2 + 5, ty + 18, pill, TRUE);
            text_center((int)cx, ty + 1, C_DARK, f_label, buf);
        } else {
            draw_stone(cx, cy, r, c.camp);
            if (c.state == othello::FIXED) {             // locked forever
                DrawCircleAA(cx, cy, 7.0f, 20, C_FIXED, TRUE);
                DrawCircleAA(cx, cy, 7.0f, 20, C_STONE_W, FALSE, 1.5f);
            }
        }
    }

    // Hover preview: stones this move would put into a tug-of-war.
    if (hover_legal) {
        for (int p : ui.hover_captures) {
            float cx, cy; cell_center(L, p, cx, cy);
            DrawCircleAA(cx, cy, r + 4, 40, C_ACC_B, FALSE, 4.0f);
        }
    }

    // Event effects: placement ripple, flip burst.
    for (const Effect &e : ui.effects) {
        float t = (ui.now_ms - e.start_ms) / (e.kind == Effect::FLIP ? 900.0f : 500.0f);
        if (t < 0 || t >= 1) continue;
        float cx, cy; cell_center(L, e.cell, cx, cy);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * (1 - t)));
        if (e.kind == Effect::FLIP) {
            DrawCircleAA(cx, cy, r * (1.0f + 0.9f * t), 48, camp_accent(e.camp), FALSE, 6.0f);
            text_center((int)cx, (int)(cy - r - 18 - 20 * t), camp_accent(e.camp), f_mid, L"反転!");
        } else {
            DrawCircleAA(cx, cy, r * (1.0f + 0.5f * t), 48, C_TEXT, FALSE, 3.0f);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

static void cd_bar(int x, int y, int camp, int remaining, bool pending, double frac, bool passed,
                   const wchar_t *who) {
    int bw = 280, bh = 22;
    unsigned int acc = camp_accent(camp);
    wchar_t buf[64];
    swprintf(buf, 64, L"%s  %s", camp_name(camp), who);
    text(x, y, C_TEXT, f_mid, buf);
    // Smooth fill between engine seconds (display only; the number below is the engine value).
    double left = pending ? CD_SECONDS : (remaining > 0 ? remaining - frac : 0.0);
    if (left < 0) left = 0;
    DrawBox(x, y + 30, x + bw, y + 30 + bh, GetColor(50, 56, 64), TRUE);
    int fill = (int)(left * bw / CD_SECONDS);
    if (fill > 0) DrawBox(x, y + 30, x + fill, y + 30 + bh, acc, TRUE);
    DrawBox(x, y + 30, x + bw, y + 30 + bh, C_SUB, FALSE);
    if (remaining > 0 || pending) {
        swprintf(buf, 64, L"クールダウン 残り %d 秒", pending ? CD_SECONDS : remaining);
        text(x, y + 58, C_SUB, f_small, buf);
    } else {
        text(x, y + 56, acc, f_mid, L"置けます!");
    }
    if (passed) text(x + bw - 40, y + 58, C_FIXED, f_small, L"パス");
}

static void legend(int x, int y) {
    text(x, y, C_TEXT, f_mid, L"見かた"); y += 30;
    // ring sample
    float cx = x + 18.0f, cy = y + 16.0f;
    DrawCircleAA(cx, cy, 14, 32, C_TRACK, FALSE, 6);
    arc(cx, cy, 14, 0, 0.65f, C_ACC_B, 5); arc(cx, cy, 14, 0.65f, 1, C_ACC_W, 5);
    DrawCircleAA(cx, cy, 9, 24, C_STONE_W, TRUE);
    text(x + 42, y, C_SUB, f_small, L"リング = 張力メーター(数字は0〜100)");
    text(x + 42, y + 18, C_SUB, f_small, L"青が一周→黒に / 橙が一周→白に"); y += 46;
    DrawLineAA((float)x + 4, (float)y + 9, (float)x + 34, (float)y + 9, C_ACC_B, 6);
    text(x + 42, y, C_SUB, f_small, L"線 = いま押している線(太いほど強い)"); y += 28;
    DrawBox(x + 2, y, x + 36, y + 18, C_ACC_W, TRUE);
    text(x + 42, y, C_SUB, f_small, L"→白 6秒 = あと何秒でどちらに裏返るか"); y += 28;
    DrawCircleAA(x + 18.0f, y + 9.0f, 7, 20, C_FIXED, TRUE);
    text(x + 42, y, C_SUB, f_small, L"赤い点 = 確定(もう裏返らない)"); y += 28;
    DrawCircleAA(x + 18.0f, y + 9.0f, 6, 20, C_HINT, TRUE);
    text(x + 42, y, C_SUB, f_small, L"緑の点 = 置けるマス");
}

void render_game(const Snapshot &s, const Layout &L, const UiState &ui) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    bool ready = s.cd_remaining[BLACK] == 0 && s.pending[BLACK] < 0;
    draw_board(s, L, ui, ready);

    int x = L.panel_x;
    wchar_t buf[64];
    swprintf(buf, 64, L"経過 %d 秒", s.tick);
    text(x, 14, C_SUB, f_small, buf);

    // Score: big, with a proportion bar.
    swprintf(buf, 64, L"黒 %d", s.score[BLACK]);
    text(x, 34, C_ACC_B, f_huge, buf);
    swprintf(buf, 64, L"%d 白", s.score[WHITE]);
    text(x + 280 - text_w(f_huge, buf), 34, C_ACC_W, f_huge, buf);
    int tot = s.score[BLACK] + s.score[WHITE];
    int split = tot > 0 ? 280 * s.score[BLACK] / tot : 140;
    DrawBox(x, 80, x + split, 88, C_ACC_B, TRUE);
    DrawBox(x + split, 80, x + 280, 88, C_ACC_W, TRUE);

    cd_bar(x, 110, BLACK, s.cd_remaining[BLACK], s.pending[BLACK] >= 0, ui.tick_frac, ui.pass[BLACK], L"あなた");
    cd_bar(x, 200, WHITE, s.cd_remaining[WHITE], s.pending[WHITE] >= 0, ui.tick_frac, ui.pass[WHITE], L"AI");

    const wchar_t *msg; unsigned int mc = C_TEXT;
    if (s.pending[BLACK] >= 0) msg = L"置きました!";
    else if (ready && s.legal_move[BLACK]) { msg = L"緑の点のマスをクリック!"; mc = C_HINT; }
    else if (ready) msg = L"置けるマスがありません";
    else msg = L"クールダウン中…";
    text(x, 292, mc, f_mid, msg);

    legend(x, 340);

    Rect rb = restart_button(L);
    button(rb, L"リスタート (R)", mouse_in(rb));
}

void render_end(const Snapshot &s, const Layout &L, const UiState &ui) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    draw_board(s, L, ui, false);

    int x = L.panel_x;
    const wchar_t *w = s.result == RESULT_BLACK_WIN ? L"あなた(黒)の勝ち!"
                     : s.result == RESULT_WHITE_WIN ? L"AI(白)の勝ち" : L"引き分け";
    text(x, 14, C_SUB, f_mid, L"終局");
    text(x, 44, s.result == RESULT_BLACK_WIN ? C_ACC_B : (s.result == RESULT_WHITE_WIN ? C_ACC_W : C_TEXT), f_huge, w);
    wchar_t buf[96];
    swprintf(buf, 96, L"黒 %d  -  %d 白", s.score[BLACK], s.score[WHITE]);
    text(x, 96, C_TEXT, f_big, buf);

    text(x, 150, C_TEXT, f_mid, L"綱引き中だった石の内訳");
    text(x, 176, C_SUB, f_small, L"(メーターが傾いている側に数える。50なら元の色)");
    int y = 204, n = 0;
    for (int p = 0; p < CELLS; p++) {
        const auto &c = s.board[p];
        if (c.state != CONTESTED) continue;
        if (y > 600) { n++; continue; }
        int row = p / BOARD_SIZE, col = p % BOARD_SIZE;
        swprintf(buf, 96, L"%c%d  元:%s  メーター %3d → %s", L'a' + col, row + 1,
                 camp_name(c.origin_camp), c.meter, camp_name(c.owner));
        text(x, y, camp_accent(c.owner), f_small, buf);
        y += 20;
    }
    if (y == 204) text(x, y, C_SUB, f_small, L"なし(すべて確定)");
    if (n > 0) { swprintf(buf, 96, L"…ほか %d 個", n); text(x, y, C_SUB, f_small, buf); }

    Rect rb = restart_button(L);
    button(rb, L"もう一度 (Enter)", mouse_in(rb));
}

} // namespace app
