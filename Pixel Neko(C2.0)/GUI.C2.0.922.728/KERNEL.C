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

static void draw_min_button(unsigned char* v, int mx, int my, int pressed)
{
    for (int y = my; y < my+10; y++)
        for (int x = mx; x < mx+12; x++)
            v[y*320+x] = 7;
    if (pressed) {
        for (int x = mx; x < mx+12; x++) v[my*320+x] = 0;
        for (int y = my; y < my+10; y++) v[y*320+mx] = 0;
        for (int x = mx; x < mx+12; x++) v[(my+9)*320+x] = 15;
        for (int y = my; y < my+10; y++) v[y*320+mx+11] = 15;
        for (int x = mx+3; x < mx+9; x++) v[(my+7)*320+x] = 0;
    } else {
        for (int x = mx; x < mx+12; x++) v[my*320+x] = 15;
        for (int y = my; y < my+10; y++) v[y*320+mx] = 15;
        for (int x = mx; x < mx+12; x++) v[(my+9)*320+x] = 0;
        for (int y = my; y < my+10; y++) v[y*320+mx+11] = 0;
        for (int x = mx+3; x < mx+9; x++) v[(my+7)*320+x] = 0;
    }
}

static void draw_icon_highlight(unsigned char* v, int x, int y)
{
    for (int i = y; i < y+16 && i < 200; i++)
        for (int j = x; j < x+16 && j < 320; j++)
            v[i*320+j] = 9;
    for (int i = y+2; i < y+12; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    for (int i = y+12; i < y+14; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
}

static int is_in_ok(int mx, int my, int bx, int by) { return (mx>=bx && mx<bx+32 && my>=by && my<by+12); }
static int is_in_close(int mx, int my, int cx, int cy) { return (mx>=cx && mx<cx+12 && my>=cy && my<cy+10); }
static int is_in_min(int mx, int my, int wx, int wy) { return (mx>=wx && mx<wx+12 && my>=wy && my<wy+10); }
static int is_in_icon(int mx, int my, int x, int y) { return (mx>=x && mx<x+16 && my>=y && my<y+16); }
static int is_in_task_about(int mx, int my) { return (mx>=120 && mx<176 && my>=5 && my<13); }
static int is_in_task_clock(int mx, int my) { return (mx>=190 && mx<246 && my>=5 && my<13); }

static void draw_taskbar(unsigned char* v, int am, int cm)
{
    for (int y = 0; y < 18; y++)
        for (int x = 0; x < 320; x++)
            v[y*320+x] = 8;
    for (int x = 0; x < 320; x++) v[0*320+x] = 15;
    for (int y = 0; y < 18; y++) v[y*320+0] = 15;
    for (int x = 0; x < 320; x++) v[17*320+x] = 0;
    for (int y = 0; y < 18; y++) v[y*320+319] = 0;
    if (am) font_draw_text(v, 120, 5, "[About]", 15);
    if (cm) font_draw_text(v, 190, 5, "[Clock]", 15);
}

static void draw_win(unsigned char* v, int wx, int wy, const char* title, int ww, int wh)
{
    for (int y = wy+1; y < wy+wh+1 && y < 200; y++)
        for (int x = wx+1; x < wx+ww+1 && x < 320; x++)
            v[y*320+x] = 0;
    for (int y = wy; y < wy+wh && y < 200; y++)
        for (int x = wx; x < wx+ww && x < 320; x++)
            v[y*320+x] = (y==wy||y==wy+wh-1||x==wx||x==wx+ww-1)?0:(y<wy+12)?1:15;
    int cx = wx + ww - 16, cy = wy + 3;
    draw_close_button(v, cx, cy, 0);
    int mx = wx + ww - 30, my = wy + 3;
    draw_min_button(v, mx, my, 0);
    font_draw_text(v, wx+5, wy+3, title, 15);
}

static void draw_about_win(unsigned char* v, int wx, int wy)
{
    draw_win(v, wx, wy, "About", 160, 112);
    int lx = wx + (160 - 52) / 2;
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
    for (int x = wx+4; x < wx+160-4; x++) v[line_y*320+x] = 8;
    font_draw_text(v, wx+8, wy+46, VER_STRING1, 0);
    font_draw_text(v, wx+8, wy+54, VER_STRING2, 0);
    font_draw_text(v, wx+8, wy+64, VER_STRING3, 8);
    int bx = wx + 160 - 42, by = wy + 90;
    draw_ok_button(v, bx, by, 0);
}

static void draw_box(unsigned char* v, int x, int y, int w, int h)
{
    for (int i = x; i < x+w; i+=2) {
        if (i < 320 && y < 200 && y >= 0) v[y*320+i] = 0;
        if (i < 320 && y+h-1 < 200 && y+h-1 >= 0) v[(y+h-1)*320+i] = 0;
    }
    for (int i = y; i < y+h; i+=2) {
        if (x < 320 && i < 200 && i >= 0) v[i*320+x] = 0;
        if (x+w-1 < 320 && i < 200 && i >= 0) v[i*320+x+w-1] = 0;
    }
}

static void draw_icon_normal(unsigned char* v, int x, int y, unsigned char bg, const char* label)
{
    for (int i = y; i < y+16 && i < 200; i++)
        for (int j = x; j < x+16 && j < 320; j++)
            v[i*320+j] = bg;
    for (int j = x; j < x+16; j++) v[y*320+j] = bg == 1 ? 9 : 10;
    for (int i = y; i < y+16; i++) v[i*320+x] = bg == 1 ? 9 : 10;
    for (int j = x; j < x+16; j++) v[(y+15)*320+j] = 0;
    for (int i = y; i < y+16; i++) v[i*320+x+15] = 0;
    for (int i = y+2; i < y+12; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    for (int i = y+12; i < y+14; i++)
        for (int j = x+6; j < x+10; j++)
            v[i*320+j] = 15;
    font_draw_text(v, x-2, y+18, label, 0);
}

static void draw_all_windows(unsigned char* v, int wx, int wy, int wx2, int wy2, int ww2, int wh2,
                              int wv, int cv, int zo, int am, int cm)
{
    if (zo == 0) {
        if (cv && !cm) { draw_win(v, wx2, wy2, "Clock", ww2, wh2); char b[6]={'1','2',':','0','0',0}; font_draw_text(v, wx2+30, wy2+35, b, 0); }
        if (wv && !am) draw_about_win(v, wx, wy);
    } else {
        if (wv && !am) draw_about_win(v, wx, wy);
        if (cv && !cm) { draw_win(v, wx2, wy2, "Clock", ww2, wh2); char b[6]={'1','2',':','0','0',0}; font_draw_text(v, wx2+30, wy2+35, b, 0); }
    }
}

void kernel_main(void)
{
    unsigned char* v = (unsigned char*)0xA0000;
    int mx, my, mb, last_mx = -1, last_my = -1;
    int wx = 80, wy = 52, ww = 160, wh = 112;
    int wx2 = 100, wy2 = 70, ww2 = 100, wh2 = 70;
    int win_visible = 0, clock_visible = 0;
    int about_minimized = 0, clock_minimized = 0;
    int about_save_x = wx, about_save_y = wy;
    int clock_save_x = wx2, clock_save_y = wy2;
    int z_order = 0, dragging = 0, drag_ox = 0, drag_oy = 0, dragging_which = 0;
    int old_wx = wx, old_wy = wy, old_wx2 = wx2, old_wy2 = wy2;
    int last_mb = 0, ok_pressed = 0, ok_bx = 0, ok_by = 0;
    int close_pressed = 0, close_cx = 0, close_cy = 0, close_which = 0;
    int min_pressed = 0, min_which = 0;
    int about_icon_pressed = 0, clock_icon_pressed = 0, z_changed = 0;

    for (int i = 0; i < 320*200; i++) v[i] = 7;
    draw_taskbar(v, 0, 0);
    draw_icon_normal(v, 20, 164, 1, "About");
    draw_icon_normal(v, 60, 164, 2, "Clock");
    mouse_init();

    while (1) {
        if (inb(0x64) & 0x01) { unsigned char st = inb(0x64); if (st & 0x20) mouse_handle_byte(inb(0x60)); else inb(0x60); }
        mouse_get(&mx, &my, &mb);
        int mb_press = (mb & 1) && !(last_mb & 1);
        int mb_release = !(mb & 1) && (last_mb & 1);
        last_mb = mb;
        z_changed = 0;

        // 桌面图标
        if (mb_press && !win_visible && !about_minimized && is_in_icon(mx, my, 20, 164))
        { about_icon_pressed = 1; z_order = 0; draw_icon_highlight(v, 20, 164); font_draw_text(v, 18, 182, "About", 0); mouse_reset(); last_mx = -1; last_my = -1; }
        if (mb_release && about_icon_pressed)
        { about_icon_pressed = 0; if (is_in_icon(mx, my, 20, 164)) { win_visible = 1; about_minimized = 0; z_changed = 1; } }

        if (mb_press && !clock_visible && !clock_minimized && is_in_icon(mx, my, 60, 164))
        { clock_icon_pressed = 1; z_order = 1; draw_icon_highlight(v, 60, 164); font_draw_text(v, 58, 182, "Clock", 0); mouse_reset(); last_mx = -1; last_my = -1; }
        if (mb_release && clock_icon_pressed)
        { clock_icon_pressed = 0; if (is_in_icon(mx, my, 60, 164)) { clock_visible = 1; clock_minimized = 0; z_changed = 1; } }

        // 任务栏：最小化/恢复
        if (mb_press && is_in_task_about(mx, my))
        { if (about_minimized) { about_minimized = 0; win_visible = 1; wx = about_save_x; wy = about_save_y; }
          else if (win_visible) { about_minimized = 1; win_visible = 0; about_save_x = wx; about_save_y = wy; }
          z_order = 0; z_changed = 1; }
        if (mb_press && is_in_task_clock(mx, my))
        { if (clock_minimized) { clock_minimized = 0; clock_visible = 1; wx2 = clock_save_x; wy2 = clock_save_y; }
          else if (clock_visible) { clock_minimized = 1; clock_visible = 0; clock_save_x = wx2; clock_save_y = wy2; }
          z_order = 1; z_changed = 1; }

        // 点击窗口置顶（Clock优先）
        if (mb_press && !dragging && !ok_pressed && !close_pressed && !min_pressed) {
            if (clock_visible && !clock_minimized && mx >= wx2 && mx < wx2+ww2 && my >= wy2 && my < wy2+wh2)
            { if (z_order != 1) { z_order = 1; z_changed = 1; } }
            else if (win_visible && !about_minimized && mx >= wx && mx < wx+ww && my >= wy && my < wy+wh)
            { if (z_order != 0) { z_order = 0; z_changed = 1; } }
        }

        if (z_changed) {
            for (int i = 0; i < 320*200; i++) v[i] = 7;
            draw_taskbar(v, about_minimized || win_visible, clock_minimized || clock_visible);
            draw_icon_normal(v, 20, 164, 1, "About");
            draw_icon_normal(v, 60, 164, 2, "Clock");
            draw_all_windows(v, wx, wy, wx2, wy2, ww2, wh2, win_visible, clock_visible, z_order, about_minimized, clock_minimized);
            mouse_reset(); last_mx = -1; last_my = -1;
        }

        // OK按钮
        int bx = wx + ww - 42, by = wy + 90;
        if (mb_press && win_visible && !about_minimized && is_in_ok(mx, my, bx, by))
        { ok_pressed = 1; ok_bx = bx; ok_by = by; draw_ok_button(v, bx, by, 1); }
        if (mb_release && ok_pressed)
        { ok_pressed = 0; draw_ok_button(v, ok_bx, ok_by, 0);
          if (is_in_ok(mx, my, ok_bx, ok_by)) { win_visible = 0; about_minimized = 0; z_changed = 1; } }

        // ×关闭
        if (mb_press && !dragging && !ok_pressed && !close_pressed && !min_pressed) {
            if (win_visible && !about_minimized) { int cx = wx+ww-16, cy = wy+3; if (is_in_close(mx, my, cx, cy))
            { close_pressed = 1; close_which = 1; close_cx = cx; close_cy = cy; draw_close_button(v, cx, cy, 1); } }
            if (!close_pressed && clock_visible && !clock_minimized) { int cx2 = wx2+ww2-16, cy2 = wy2+3; if (is_in_close(mx, my, cx2, cy2))
            { close_pressed = 1; close_which = 2; close_cx = cx2; close_cy = cy2; draw_close_button(v, cx2, cy2, 1); } }
        }
        if (mb_release && close_pressed)
        { draw_close_button(v, close_cx, close_cy, 0);
          if (is_in_close(mx, my, close_cx, close_cy))
          { if (close_which == 1) { win_visible = 0; about_minimized = 0; }
            else { clock_visible = 0; clock_minimized = 0; }
            z_changed = 1; }
          close_pressed = 0; close_which = 0; }

        // 最小化按钮
        if (mb_press && !dragging && !ok_pressed && !close_pressed && !min_pressed) {
            if (win_visible && !about_minimized) { int mx2 = wx+ww-30, my2 = wy+3; if (is_in_min(mx, my, mx2, my2))
            { min_pressed = 1; min_which = 1; draw_min_button(v, mx2, my2, 1); } }
            if (!min_pressed && clock_visible && !clock_minimized) { int mx3 = wx2+ww2-30, my3 = wy2+3; if (is_in_min(mx, my, mx3, my3))
            { min_pressed = 1; min_which = 2; draw_min_button(v, mx3, my3, 1); } }
        }
        if (mb_release && min_pressed)
        { if (min_which == 1) { about_minimized = 1; win_visible = 0; about_save_x = wx; about_save_y = wy; }
          else { clock_minimized = 1; clock_visible = 0; clock_save_x = wx2; clock_save_y = wy2; }
          z_changed = 1; min_pressed = 0; }

        if (z_changed) {
            for (int i = 0; i < 320*200; i++) v[i] = 7;
            draw_taskbar(v, about_minimized || win_visible, clock_minimized || clock_visible);
            draw_icon_normal(v, 20, 164, 1, "About");
            draw_icon_normal(v, 60, 164, 2, "Clock");
            draw_all_windows(v, wx, wy, wx2, wy2, ww2, wh2, win_visible, clock_visible, z_order, about_minimized, clock_minimized);
            mouse_reset(); last_mx = -1; last_my = -1;
        }

        // 拖动
        if (mb & 1 && !ok_pressed && !close_pressed && !min_pressed) {
            if (!dragging) {
                if (win_visible && !about_minimized && mx >= wx && mx < wx+ww && my >= wy && my < wy+12)
                { z_order = 0; dragging = 1; dragging_which = 1; drag_ox = mx-wx; drag_oy = my-wy; old_wx = wx; old_wy = wy; }
                else if (clock_visible && !clock_minimized && mx >= wx2 && mx < wx2+ww2 && my >= wy2 && my < wy2+12)
                { z_order = 1; dragging = 1; dragging_which = 2; drag_ox = mx-wx2; drag_oy = my-wy2; old_wx2 = wx2; old_wy2 = wy2; }
            }
            if (dragging) {
                int nwx = mx-drag_ox, nwy = my-drag_oy;
                int nww = (dragging_which == 1) ? ww : ww2, nwh = (dragging_which == 1) ? wh : wh2;
                if (nwx < 0) nwx = 0; if (nwy < 0) nwy = 0;
                if (nwx+nww > 320) nwx = 320-nww; if (nwy+nwh > 200) nwy = 200-nwh;
                int* cwx = (dragging_which == 1) ? &wx : &wx2;
                int* cwy = (dragging_which == 1) ? &wy : &wy2;
                if (nwx != *cwx || nwy != *cwy) {
                    int oldx = (dragging_which == 1) ? old_wx : old_wx2;
                    int oldy = (dragging_which == 1) ? old_wy : old_wy2;
                    int oldw = (dragging_which == 1) ? ww : ww2, oldh = (dragging_which == 1) ? wh : wh2;
                    for (int y2 = oldy; y2 < oldy+oldh+2 && y2 < 200; y2++)
                        for (int x2 = oldx; x2 < oldx+oldw+2 && x2 < 320; x2++) v[y2*320+x2] = 7;
                    draw_taskbar(v, about_minimized || win_visible, clock_minimized || clock_visible);
                    draw_icon_normal(v, 20, 164, 1, "About");
                    draw_icon_normal(v, 60, 164, 2, "Clock");
                    if (dragging_which == 1 && clock_visible && !clock_minimized)
                    { draw_win(v, wx2, wy2, "Clock", ww2, wh2); char b[6]={'1','2',':','0','0',0}; font_draw_text(v, wx2+30, wy2+35, b, 0); }
                    if (dragging_which == 2 && win_visible && !about_minimized)
                    { draw_about_win(v, wx, wy); }
                    draw_box(v, nwx, nwy, nww, nwh);
                    if (dragging_which == 1) { old_wx = nwx; old_wy = nwy; } else { old_wx2 = nwx; old_wy2 = nwy; }
                    *cwx = nwx; *cwy = nwy;
                }
            }
        } else {
            if (dragging) {
                dragging = 0; dragging_which = 0;
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_taskbar(v, about_minimized || win_visible, clock_minimized || clock_visible);
                draw_icon_normal(v, 20, 164, 1, "About");
                draw_icon_normal(v, 60, 164, 2, "Clock");
                draw_all_windows(v, wx, wy, wx2, wy2, ww2, wh2, win_visible, clock_visible, z_order, about_minimized, clock_minimized);
                mouse_reset(); last_mx = -1; last_my = -1;
            }
        }

        if (!dragging && !ok_pressed && !close_pressed && !min_pressed && !about_icon_pressed && !clock_icon_pressed && (mx != last_mx || my != last_my))
        { mouse_draw(v); last_mx = mx; last_my = my; }
    }
}