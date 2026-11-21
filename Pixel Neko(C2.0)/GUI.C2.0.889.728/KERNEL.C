#include "port.h"
#include "mouse.h"
#include "font.h"
#include "version.h"

static void draw_ok_button(unsigned char* v, int bx, int by, int pressed)
{
    for (int y = by; y < by+12; y++)
        for (int x = bx; x < bx+32; x++)
            v[y*320+x] = 7;
    if (pressed) {
        for (int x = bx; x < bx+32; x++) v[by*320+x] = 0;
        for (int y = by; y < by+12; y++) v[y*320+bx] = 0;
        for (int x = bx; x < bx+32; x++) v[(by+11)*320+x] = 15;
        for (int y = by; y < by+12; y++) v[y*320+bx+31] = 15;
        font_draw_text(v, bx+9, by+3, "OK", 0);
    } else {
        for (int x = bx; x < bx+32; x++) v[by*320+x] = 15;
        for (int y = by; y < by+12; y++) v[y*320+bx] = 15;
        for (int x = bx; x < bx+32; x++) v[(by+11)*320+x] = 0;
        for (int y = by; y < by+12; y++) v[y*320+bx+31] = 0;
        font_draw_text(v, bx+8, by+2, "OK", 0);
    }
}

static void draw_close_button(unsigned char* v, int cx, int cy, int pressed)
{
    for (int y = cy; y < cy+10; y++)
        for (int x = cx; x < cx+12; x++)
            v[y*320+x] = 7;
    if (pressed) {
        for (int x = cx; x < cx+12; x++) v[cy*320+x] = 0;
        for (int y = cy; y < cy+10; y++) v[y*320+cx] = 0;
        for (int x = cx; x < cx+12; x++) v[(cy+9)*320+x] = 15;
        for (int y = cy; y < cy+10; y++) v[y*320+cx+11] = 15;
        v[(cy+3)*320+cx+4] = 0; v[(cy+3)*320+cx+6] = 0;
        v[(cy+4)*320+cx+5] = 0;
        v[(cy+5)*320+cx+4] = 0; v[(cy+5)*320+cx+6] = 0;
        v[(cy+6)*320+cx+3] = 0; v[(cy+6)*320+cx+7] = 0;
    } else {
        for (int x = cx; x < cx+12; x++) v[cy*320+x] = 15;
        for (int y = cy; y < cy+10; y++) v[y*320+cx] = 15;
        for (int x = cx; x < cx+12; x++) v[(cy+9)*320+x] = 0;
        for (int y = cy; y < cy+10; y++) v[y*320+cx+11] = 0;
        v[(cy+2)*320+cx+3] = 0; v[(cy+2)*320+cx+7] = 0;
        v[(cy+3)*320+cx+4] = 0; v[(cy+3)*320+cx+6] = 0;
        v[(cy+4)*320+cx+5] = 0;
        v[(cy+5)*320+cx+4] = 0; v[(cy+5)*320+cx+6] = 0;
        v[(cy+6)*320+cx+3] = 0; v[(cy+6)*320+cx+7] = 0;
    }
}

static void draw_icon_highlight(unsigned char* v)
{
    int x = 20, y = 164;
    for (int i = y; i < y+16 && i < 200; i++)
        for (int j = x; j < x+16 && j < 320; j++)
            v[i*320+j] = 9;
    for (int i = y+2; i < y+12; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    for (int i = y+12; i < y+14; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    font_draw_text(v, x-2, y+18, "About", 0);
}

static int is_in_ok(int mx, int my, int bx, int by)
{
    return (mx >= bx && mx < bx+32 && my >= by && my < by+12);
}

static int is_in_close(int mx, int my, int cx, int cy)
{
    return (mx >= cx && mx < cx+12 && my >= cy && my < cy+10);
}

static int is_in_icon(int mx, int my)
{
    return (mx >= 20 && mx < 36 && my >= 164 && my < 180);
}

static void draw_taskbar(unsigned char* v)
{
    for (int y = 0; y < 18; y++)
        for (int x = 0; x < 320; x++)
            v[y*320+x] = 8;
    for (int x = 0; x < 320; x++) v[0*320+x] = 15;
    for (int y = 0; y < 18; y++) v[y*320+0] = 15;
    for (int x = 0; x < 320; x++) v[17*320+x] = 0;
    for (int y = 0; y < 18; y++) v[y*320+319] = 0;
    font_draw_text(v, 8, 5, "TBMK GUI Shell", 15);
}

static void draw_win(unsigned char* v, int wx, int wy)
{
    int ww = 160, wh = 112;
    for (int y = wy+1; y < wy+wh+1 && y < 200; y++)
        for (int x = wx+1; x < wx+ww+1 && x < 320; x++)
            v[y*320+x] = 0;
    for (int y = wy; y < wy+wh && y < 200; y++)
        for (int x = wx; x < wx+ww && x < 320; x++)
            v[y*320+x] = (y==wy||y==wy+wh-1||x==wx||x==wx+ww-1)?0:(y<wy+12)?1:15;

    int cx = wx + ww - 16, cy = wy + 3;
    draw_close_button(v, cx, cy, 0);
    font_draw_text(v, wx+5, wy+3, "About", 15);

    int lx = wx + (ww - 52) / 2;
    int ly = wy + 14;
    for (int y = ly+3; y < ly+18; y++)
        for (int x = lx-7; x < lx+48; x++) v[y*320+x] = 1;
    for (int x = lx-4; x < lx+45; x++) v[(ly+3)*320+x] = 9;
    for (int y = ly; y < ly+12; y++)
        for (int x = lx; x < lx+12; x++)
            v[y*320+x] = (y==ly||x==lx)?12:4;
    for (int y = ly; y < ly+12; y++)
        for (int x = lx+10; x < lx+22; x++)
            v[y*320+x] = (y==ly||x==lx+10)?10:2;
    for (int y = ly+7; y < ly+19; y++)
        for (int x = lx+19; x < lx+31; x++)
            v[y*320+x] = (y==ly+7||x==lx+19)?11:3;
    for (int y = ly+7; y < ly+19; y++)
        for (int x = lx+29; x < lx+41; x++)
            v[y*320+x] = (y==ly+7||x==lx+29)?13:5;
    font_draw_char(v, lx+4, ly+4, 'T', 15);
    font_draw_char(v, lx+14, ly+4, 'B', 15);
    font_draw_char(v, lx+23, ly+11, 'M', 15);
    font_draw_char(v, lx+33, ly+11, 'K', 15);

    int line_y = wy + 40;
    for (int x = wx+4; x < wx+ww-4; x++) v[line_y*320+x] = 8;

    font_draw_text(v, wx+8, wy+46, VER_STRING1, 0);
    font_draw_text(v, wx+8, wy+54, VER_STRING2, 0);
    font_draw_text(v, wx+8, wy+64, VER_STRING3, 8);

    int bx = wx + ww - 42, by = wy + 90;
    draw_ok_button(v, bx, by, 0);
}

static void draw_icon_normal(unsigned char* v, int x, int y)
{
    for (int i = y; i < y+16 && i < 200; i++)
        for (int j = x; j < x+16 && j < 320; j++)
            v[i*320+j] = 1;
    for (int j = x; j < x+16; j++) v[y*320+j] = 9;
    for (int i = y; i < y+16; i++) v[i*320+x] = 9;
    for (int j = x; j < x+16; j++) v[(y+15)*320+j] = 0;
    for (int i = y; i < y+16; i++) v[i*320+x+15] = 0;
    for (int i = y+2; i < y+12; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    for (int i = y+12; i < y+14; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    font_draw_text(v, x-2, y+18, "About", 0);
}

static void animate_close(unsigned char* v, int wx, int wy)
{
    int ww = 160, wh = 112;
    int cx = wx + ww/2;
    int cy = wy + wh/2;
    for (int step = 10; step >= 0; step--) {
        int nw = ww * step / 10;
        int nh = wh * step / 10;
        if (nw < 4) nw = 4;
        if (nh < 4) nh = 4;
        int nx = cx - nw/2;
        int ny = cy - nh/2;
        for (int i = 0; i < 320*200; i++) v[i] = 7;
        draw_taskbar(v);
        draw_icon_normal(v, 20, 164);
        for (int yi = ny; yi < ny+nh && yi < 200; yi++)
            for (int xi = nx; xi < nx+nw && xi < 320; xi++)
                v[yi*320+xi] = (yi==ny||yi==ny+nh-1||xi==nx||xi==nx+nw-1)?0:(yi<ny+12)?1:15;
        for (volatile int i = 0; i < 200000; i++);
    }
}

void kernel_main(void)
{
    unsigned char* v = (unsigned char*)0xA0000;
    int mx, my, mb;
    int last_mx = -1, last_my = -1;

    int wx = 80, wy = 52, ww = 160, wh = 112;
    int win_visible = 0;
    int dragging = 0, drag_ox = 0, drag_oy = 0;
    int old_wx = wx, old_wy = wy;
    int last_mb = 0;
    int ok_pressed = 0, ok_bx = 0, ok_by = 0;
    int close_pressed = 0, close_cx = 0, close_cy = 0;
    int icon_pressed = 0;

    for (int i = 0; i < 320*200; i++) v[i] = 7;
    draw_taskbar(v);
    draw_icon_normal(v, 20, 164);
    mouse_init();

    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char status = inb(0x64);
            if (status & 0x20) mouse_handle_byte(inb(0x60));
            else inb(0x60);
        }
        mouse_get(&mx, &my, &mb);
        int mb_press = (mb & 1) && !(last_mb & 1);
        int mb_release = !(mb & 1) && (last_mb & 1);
        last_mb = mb;

        int bx = wx + ww - 42, by = wy + 90;
        int cx = wx + ww - 16, cy = wy + 3;

        if (mb_press && !win_visible && is_in_icon(mx, my)) {
            icon_pressed = 1;
            draw_icon_highlight(v);
            mouse_reset(); last_mx = -1; last_my = -1;
        }
        if (mb_release && icon_pressed) {
            if (is_in_icon(mx, my)) {
                win_visible = 1;
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v);
                draw_icon_normal(v, 20, 164);
                draw_win(v, wx, wy);
                mouse_reset(); last_mx = -1; last_my = -1;
            } else {
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v);
                draw_icon_normal(v, 20, 164);
            }
            icon_pressed = 0;
        }
        if (icon_pressed && !is_in_icon(mx, my) && !(mb & 1)) {
            for (int i = 0; i < 320*200; i++) v[i] = 7;
            draw_taskbar(v);
            draw_icon_normal(v, 20, 164);
            icon_pressed = 0;
        }

        if (mb_press && win_visible && is_in_ok(mx, my, bx, by)) {
            ok_pressed = 1; ok_bx = bx; ok_by = by;
            draw_ok_button(v, bx, by, 1);
        }
        if (mb_release && ok_pressed) {
            draw_ok_button(v, ok_bx, ok_by, 0);
            if (is_in_ok(mx, my, ok_bx, ok_by)) {
                win_visible = 0;
                animate_close(v, wx, wy);
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v);
                draw_icon_normal(v, 20, 164);
                mouse_reset(); last_mx = -1; last_my = -1;
            }
            ok_pressed = 0;
        }
        if (ok_pressed && !is_in_ok(mx, my, ok_bx, ok_by) && !(mb & 1)) {
            draw_ok_button(v, ok_bx, ok_by, 0);
            ok_pressed = 0;
        }

        if (mb_press && win_visible && !dragging && !ok_pressed && is_in_close(mx, my, cx, cy)) {
            close_pressed = 1; close_cx = cx; close_cy = cy;
            draw_close_button(v, cx, cy, 1);
        }
        if (mb_release && close_pressed) {
            draw_close_button(v, close_cx, close_cy, 0);
            if (is_in_close(mx, my, close_cx, close_cy)) {
                win_visible = 0;
                animate_close(v, wx, wy);
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v);
                draw_icon_normal(v, 20, 164);
                mouse_reset(); last_mx = -1; last_my = -1;
            }
            close_pressed = 0;
        }
        if (close_pressed && !is_in_close(mx, my, close_cx, close_cy) && !(mb & 1)) {
            draw_close_button(v, close_cx, close_cy, 0);
            close_pressed = 0;
        }

        if (win_visible && mb & 1 && !ok_pressed && !close_pressed) {
            if (!dragging) {
                if (mx >= wx && mx < wx+ww && my >= wy && my < wy+12) {
                    dragging = 1; drag_ox = mx-wx; drag_oy = my-wy;
                    old_wx = wx; old_wy = wy;
                }
            }
            if (dragging) {
                int nwx = mx-drag_ox, nwy = my-drag_oy;
                if (nwx < 0) nwx = 0;
                if (nwy < 18) nwy = 18;
                if (nwx+ww > 320) nwx = 320-ww;
                if (nwy+wh > 200) nwy = 200-wh;
                if (nwx != wx || nwy != wy) {
                    // 擦旧框+阴影
                    for (int y = old_wy; y < old_wy+wh+2 && y < 200; y++)
                        for (int x = old_wx; x < old_wx+ww+2 && x < 320; x++)
                            if (y >= 18) v[y*320+x] = 7;
                    // 重绘图标
                    draw_icon_normal(v, 20, 164);
                    // 画新实线框
                    for (int i = nwx; i < nwx+ww; i++) {
                        v[nwy*320+i] = 0;
                        v[(nwy+wh-1)*320+i] = 0;
                    }
                    for (int i = nwy; i < nwy+wh; i++) {
                        v[i*320+nwx] = 0;
                        v[i*320+nwx+ww-1] = 0;
                    }
                    wx = nwx; wy = nwy;
                    old_wx = wx; old_wy = wy;
                }
            }
        } else {
            if (dragging) {
                dragging = 0;
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v);
                draw_icon_normal(v, 20, 164);
                draw_win(v, wx, wy);
                mouse_reset(); last_mx = -1; last_my = -1;
            }
        }

        if (!dragging && !ok_pressed && !close_pressed && !icon_pressed && (mx != last_mx || my != last_my)) {
            mouse_draw(v);
            last_mx = mx; last_my = my;
        }
    }
}