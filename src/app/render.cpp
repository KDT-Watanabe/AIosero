#include "render.h"
#include <DxLib.h>
#include <cwchar>
#include <cstring>

using namespace othello;

namespace app {

static int f_small = -1, f_mid = -1, f_big = -1, f_title = -1, f_meter = -1;

static unsigned int C_BG, C_BOARD, C_GRID, C_TEXT, C_SUB, C_STONE_B, C_STONE_W,
                    C_ACC_B, C_ACC_W, C_BTN, C_HINT, C_FIXED;

void render_init() {
    const wchar_t *face = L"Meiryo UI";
    f_small = CreateFontToHandle(face, 15, 2, DX_FONTTYPE_ANTIALIASING_4X4);
    f_mid   = CreateFontToHandle(face, 20, 3, DX_FONTTYPE_ANTIALIASING_4X4);
    f_big   = CreateFontToHandle(face, 28, 5, DX_FONTTYPE_ANTIALIASING_4X4);
    f_title = CreateFontToHandle(face, 48, 7, DX_FONTTYPE_ANTIALIASING_4X4);
    f_meter = CreateFontToHandle(face, 22, 6, DX_FONTTYPE_ANTIALIASING_4X4);

    C_BG      = GetColor(24, 28, 34);
    C_BOARD   = GetColor(32, 96, 64);
    C_GRID    = GetColor(18, 60, 40);
    C_TEXT    = GetColor(232, 236, 240);
    C_SUB     = GetColor(150, 160, 170);
    C_STONE_B = GetColor(16, 16, 20);
    C_STONE_W = GetColor(240, 240, 236);
    C_ACC_B   = GetColor(90, 150, 255);   // black-side accent (meter toward 100)
    C_ACC_W   = GetColor(255, 196, 80);   // white-side accent (meter toward 0)
    C_BTN     = GetColor(60, 100, 160);
    C_HINT    = GetColor(120, 200, 150);
    C_FIXED   = GetColor(200, 60, 60);
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

static void button(const Rect &r, const wchar_t *label, bool hot) {
    DrawBox(r.x1, r.y1, r.x2, r.y2, hot ? GetColor(80, 130, 200) : C_BTN, TRUE);
    DrawBox(r.x1, r.y1, r.x2, r.y2, C_TEXT, FALSE);
    text_center((r.x1 + r.x2) / 2, (r.y1 + r.y2) / 2 - 15, C_TEXT, f_big, label);
}

static bool mouse_in(const Rect &r) {
    int mx, my; GetMousePoint(&mx, &my);
    return r.contains(mx, my);
}

void render_title(const Layout &) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    text_center(SCREEN_W / 2, 50, C_TEXT, f_title, L"テンション・オセロ");
    text_center(SCREEN_W / 2, 112, C_SUB, f_mid, L"〜 挟んだ石は、すぐには裏返らない 〜");

    const wchar_t *rules[] = {
        L"■ 置き方は普通のオセロと同じ。相手の石を挟めるマスにだけ置けます。",
        L"■ ただし手番はありません。黒(あなた)も白(AI)も、置くと5秒のクールダウン。",
        L"   クールダウンが明けたら、いつでもクリックで置けます(置ける所が無ければパス)。",
        L"■ 挟まれた石は「張力メーター」を持ちます(白 0 〜 50 〜 100 黒)。開始は50。",
        L"■ 毎秒、挟んでいる線の圧力 = 切り上げ(挟んだ石数 ÷ 最寄りの自石までの距離)",
        L"   だけメーターが動き、0 か 100 に振り切れた瞬間に裏返って「確定」します。",
        L"■ 綱引き: 挟まれた自分の石を、さらに自分の石で挟み直すと押し返せます(援護線)。",
        L"■ 救出: 挟んでいる線の端の石が裏返ると線は切れ、メーターは停止します。",
        L"■ 盤が埋まるか両者とも置けなくなったら終局。メーターが傾いている側の色で数えます",
        L"   (ちょうど50なら元の色)。石の多い方が勝ち。",
    };
    int y = 170;
    for (const wchar_t *line : rules) { text(90, y, C_TEXT, f_mid, line); y += 40; }

    Rect b = title_start_button();
    button(b, L"スタート", mouse_in(b));
    text_center(SCREEN_W / 2, 692, C_SUB, f_small, L"クリック または Enter で開始 / ゲーム中 R でリスタート");
}

static void draw_board(const Snapshot &s, const Layout &L, int hover, bool show_hints) {
    int bw = L.cell_px * BOARD_SIZE;
    DrawBox(L.ox - 6, L.oy - 6, L.ox + bw + 6, L.oy + bw + 6, GetColor(60, 40, 24), TRUE);
    DrawBox(L.ox, L.oy, L.ox + bw, L.oy + bw, C_BOARD, TRUE);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        DrawLine(L.ox + i * L.cell_px, L.oy, L.ox + i * L.cell_px, L.oy + bw, C_GRID, 2);
        DrawLine(L.ox, L.oy + i * L.cell_px, L.ox + bw, L.oy + i * L.cell_px, C_GRID, 2);
    }
    for (int i = 0; i < BOARD_SIZE; i++) {           // coordinates a-h / 1-8
        wchar_t b[4];
        swprintf(b, 4, L"%c", L'a' + i);
        text_center(L.ox + i * L.cell_px + L.cell_px / 2, L.oy - 26, C_SUB, f_small, b);
        swprintf(b, 4, L"%d", i + 1);
        text_center(L.ox - 20, L.oy + i * L.cell_px + L.cell_px / 2 - 8, C_SUB, f_small, b);
    }

    for (int p = 0; p < CELLS; p++) {
        int row = p / BOARD_SIZE, col = p % BOARD_SIZE;
        int x0 = L.ox + col * L.cell_px, y0 = L.oy + row * L.cell_px;
        float cx = x0 + L.cell_px / 2.0f, cy = y0 + L.cell_px / 2.0f;
        float r = L.cell_px * 0.40f;
        const auto &c = s.board[p];

        if (c.camp < 0) {
            bool pend_b = s.pending[BLACK] == p, pend_w = s.pending[WHITE] == p;
            if (pend_b || pend_w) {                      // queued: resolves at the next second
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
                DrawCircleAA(cx, cy, r, 40, pend_b ? C_STONE_B : C_STONE_W, TRUE);
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
            } else if (show_hints && s.legal[BLACK][p]) {
                bool hot = hover == p;
                DrawCircleAA(cx, cy, hot ? r : 6.0f, 32, C_HINT, hot ? FALSE : TRUE, 2.0f);
            }
            continue;
        }

        int base = c.state == CONTESTED ? c.origin_camp : c.camp;
        unsigned int fill = base == BLACK ? C_STONE_B : C_STONE_W;
        unsigned int ink  = base == BLACK ? C_STONE_W : C_STONE_B;

        if (c.state == CONTESTED) {
            const auto &pr = s.predictions[p];
            unsigned int ring = pr.camp < 0 ? C_SUB : (pr.camp == BLACK ? C_ACC_B : C_ACC_W);
            DrawCircleAA(cx, cy, r + 3, 40, ring, TRUE);
            DrawCircleAA(cx, cy, r - 1, 40, fill, TRUE);

            wchar_t buf[32];
            swprintf(buf, 32, L"%d", c.meter);       // equals engine meter (SC-004)
            text_center((int)cx, (int)cy - 18, ink, f_meter, buf);

            // Tension bar: right part (blue) = meter toward black(100), left (amber) toward white(0).
            int bx1 = x0 + 10, bx2 = x0 + L.cell_px - 10, by = y0 + L.cell_px / 2 + 8;
            int split = bx1 + (bx2 - bx1) * c.meter / 100;
            DrawBox(bx1, by, split, by + 6, C_ACC_B, TRUE);
            DrawBox(split, by, bx2, by + 6, C_ACC_W, TRUE);
            DrawBox(bx1 - 1, by - 1, bx2 + 1, by + 7, ink, FALSE);

            if (pr.camp >= 0) {
                int secs = (int)(pr.seconds + 0.999);    // round up: "about N seconds"
                swprintf(buf, 32, L"%s 約%d秒", camp_name(pr.camp), secs);
            } else {
                swprintf(buf, 32, L"停止");
            }
            int tw = text_w(f_small, buf);
            int ty = y0 + L.cell_px - 19;
            DrawBox((int)cx - tw / 2 - 3, ty, (int)cx + tw / 2 + 3, ty + 17, GetColor(0, 0, 0), TRUE);
            text_center((int)cx, ty, ring, f_small, buf);
        } else {
            DrawCircleAA(cx + 2, cy + 3, r, 40, GetColor(10, 40, 25), TRUE);   // shadow
            DrawCircleAA(cx, cy, r, 40, fill, TRUE);
            if (c.state == othello::FIXED) DrawCircleAA(cx, cy, 5.0f, 16, C_FIXED, TRUE);  // fixed forever
        }
    }
}

static void cd_bar(int x, int y, int camp, int remaining, bool passed, const wchar_t *who) {
    int bw = 260, bh = 18;
    unsigned int acc = camp == BLACK ? C_ACC_B : C_ACC_W;
    wchar_t buf[64];
    swprintf(buf, 64, L"%s %s", camp_name(camp), who);
    text(x, y, C_TEXT, f_mid, buf);
    DrawBox(x, y + 30, x + bw, y + 30 + bh, GetColor(50, 56, 64), TRUE);
    int fill = remaining * bw / CD_SECONDS;
    if (fill > 0) DrawBox(x, y + 30, x + fill, y + 30 + bh, acc, TRUE);
    DrawBox(x, y + 30, x + bw, y + 30 + bh, C_SUB, FALSE);
    if (remaining > 0) swprintf(buf, 64, L"クールダウン 残り %d 秒", remaining);
    else swprintf(buf, 64, L"準備OK");
    text(x, y + 54, remaining > 0 ? C_SUB : acc, f_small, buf);
    if (passed) text(x + bw - 40, y + 54, C_FIXED, f_small, L"パス");
}

void render_game(const Snapshot &s, const Layout &L, const UiState &ui) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    bool ready = s.cd_remaining[BLACK] == 0 && s.pending[BLACK] < 0;
    draw_board(s, L, ui.hover_cell, ready);

    int x = L.panel_x;
    text(x, 20, C_TEXT, f_big, L"テンション・オセロ");
    wchar_t buf[64];
    swprintf(buf, 64, L"経過 %d 秒", s.tick);
    text(x, 58, C_SUB, f_small, buf);

    swprintf(buf, 64, L"黒 %d  -  %d 白", s.score[BLACK], s.score[WHITE]);
    text(x, 90, C_TEXT, f_big, buf);
    text(x, 124, C_SUB, f_small, L"(メーターの傾きで数えた現在の石数)");

    cd_bar(x, 160, BLACK, s.cd_remaining[BLACK], ui.pass[BLACK], L"あなた");
    cd_bar(x, 250, WHITE, s.cd_remaining[WHITE], ui.pass[WHITE], L"AI");

    const wchar_t *msg;
    if (s.pending[BLACK] >= 0) msg = L"配置中…";
    else if (ready && s.legal_move[BLACK]) msg = L"緑の点のマスをクリック!";
    else if (ready) msg = L"置けるマスがありません";
    else msg = L"クールダウン中…";
    text(x, 345, C_TEXT, f_mid, msg);

    int ly = 395;
    text(x, ly, C_SUB, f_small, L"数字 = 張力メーター (白0〜黒100)"); ly += 22;
    text(x, ly, C_SUB, f_small, L"枠の色 = 裏返りそうな側"); ly += 22;
    text(x, ly, C_ACC_B, f_small, L"  青 = 黒へ"); text(x + 110, ly, C_ACC_W, f_small, L"橙 = 白へ"); ly += 22;
    text(x, ly, C_SUB, f_small, L"「停止」= 線が切れて凍結中"); ly += 22;
    text(x, ly, C_SUB, f_small, L"赤い点 = 確定した石(もう裏返らない)"); ly += 22;
    text(x, ly, C_SUB, f_small, L"圧力 = 切上げ(挟んだ数 ÷ 自石までの距離)"); ly += 22;

    Rect rb = restart_button(L);
    button(rb, L"リスタート (R)", mouse_in(rb));
}

void render_end(const Snapshot &s, const Layout &L) {
    ClearDrawScreen();
    DrawBox(0, 0, SCREEN_W, SCREEN_H, C_BG, TRUE);
    draw_board(s, L, -1, false);

    int x = L.panel_x;
    const wchar_t *w = s.result == RESULT_BLACK_WIN ? L"黒(あなた)の勝ち!"
                     : s.result == RESULT_WHITE_WIN ? L"白(AI)の勝ち" : L"引き分け";
    text(x, 20, C_TEXT, f_big, L"終局");
    text(x, 60, s.result == RESULT_BLACK_WIN ? C_ACC_B : C_ACC_W, f_big, w);
    wchar_t buf[96];
    swprintf(buf, 96, L"黒 %d  -  %d 白", s.score[BLACK], s.score[WHITE]);
    text(x, 100, C_TEXT, f_big, buf);

    text(x, 150, C_TEXT, f_mid, L"未確定の石(メーター内訳)");
    int y = 180, n = 0;
    for (int p = 0; p < CELLS; p++) {
        const auto &c = s.board[p];
        if (c.state != CONTESTED) continue;
        if (y > 600) { n++; continue; }
        int row = p / BOARD_SIZE, col = p % BOARD_SIZE;
        swprintf(buf, 96, L"%c%d  元:%s  メーター %3d → %s", L'a' + col, row + 1,
                 camp_name(c.origin_camp), c.meter, camp_name(c.owner));
        text(x, y, C_SUB, f_small, buf);
        y += 20;
    }
    if (y == 180) text(x, y, C_SUB, f_small, L"なし(すべて確定)");
    if (n > 0) { swprintf(buf, 96, L"…ほか %d 個", n); text(x, y, C_SUB, f_small, buf); }

    Rect rb = restart_button(L);
    button(rb, L"もう一度 (Enter)", mouse_in(rb));
}

} // namespace app
