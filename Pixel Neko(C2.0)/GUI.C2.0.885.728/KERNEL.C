#include "port.h"
#include "mouse.h"
#include "font.h"

static void draw_win(unsigned char* v, int wx, int wy)
{
    int ww = 240, wh = 160;
    for (int y = wy+2; y < wy+wh+2 && y < 200; y++)
        for (int x = wx+2; x < wx+ww+2 && x < 320; x++)
            v[y*320+x] = 0;
    for (int y = wy; y < wy+wh && y < 200; y++)
        for (int x = wx; x < wx+ww && x < 320; x++)
            v[y*320+x] = (y==wy||y==wy+wh-1||x==wx||x==wx+ww-1)?0:(y<wy+15)?1:15;
    for (int y = wy+3; y < wy+12; y++)
        for (int x = wx+ww-20; x < wx+ww-2; x++)
            v[y*320+x] = (y==wy+3||x==wx+ww-20)?15:(y==wy+11||x==wx+ww-3)?0:7;
    font_draw_text(v, wx+8, wy+4, "Program Manager", 15);
    int lx = wx+70, ly = wy+30;
    for (int y = ly+4; y < ly+24; y++)
        for (int x = lx-10; x < lx+70; x++) v[y*320+x] = 1;
    for (int x = lx-6; x < lx+66; x++) v[(ly+4)*320+x] = 9;
    for (int y = ly; y < ly+18; y++)
        for (int x = lx; x < lx+18; x++)
            v[y*320+x] = (y==ly||x==lx)?12:4;
    for (int y = ly; y < ly+18; y++)
        for (int x = lx+14; x < lx+32; x++)
            v[y*320+x] = (y==ly||x==lx+14)?10:2;
    for (int y = ly+10; y < ly+28; y++)
        for (int x = lx+28; x < lx+46; x++)
            v[y*320+x] = (y==ly+10||x==lx+28)?11:3;
    for (int y = ly+10; y < ly+28; y++)
        for (int x = lx+42; x < lx+60; x++)
            v[y*320+x] = (y==ly+10||x==lx+42)?13:5;
    font_draw_char(v, lx+6, ly+6, 'T', 15);
    font_draw_char(v, lx+20, ly+6, 'B', 15);
    font_draw_char(v, lx+34, ly+16, 'M', 15);
    font_draw_char(v, lx+48, ly+16, 'K', 15);
    font_draw_text(v, wx+30, wy+75, "T.B.M.K. Micro Kernel", 0);
    font_draw_text(v, wx+45, wy+89, "GUI Version  C2.0", 0);
    font_draw_text(v, wx+55, wy+103, "by Tairitsu_tty", 0);
    font_draw_text(v, wx+30, wy+125, "(C) 2026  Tairitsu_tty", 8);
}

static void draw_box(unsigned char* v, int x, int y, int w, int h)
{
    for (int i = x; i < x+w; i+=2) {
        if (i < 320 && y < 200) v[y*320+i] = 0;
        if (i < 320 && y+h-1 < 200) v[(y+h-1)*320+i] = 0;
    }
    for (int i = y; i < y+h; i+=2) {
        if (x < 320 && i < 200) v[i*320+x] = 0;
        if (x+w-1 < 320 && i < 200) v[i*320+x+w-1] = 0;
    }
}

static void draw_icon(unsigned char* v, int x, int y)
{
    for (int i = y; i < y+24 && i < 200; i++)
        for (int j = x; j < x+24 && j < 320; j++)
            v[i*320+j] = 1;
    for (int j = x; j < x+24; j++) v[y*320+j] = 9;
    for (int i = y; i < y+24; i++) v[i*320+x] = 9;
    for (int j = x; j < x+24; j++) v[(y+23)*320+j] = 0;
    for (int i = y; i < y+24; i++) v[i*320+x+23] = 0;
    for (int i = y+6; i < y+18; i++)
        for (int j = x+8; j < x+16; j++)
            v[i*320+j] = 15;
    font_draw_text(v, x, y+28, "Program", 0);
}

static void animate_close(unsigned char* v, int wx, int wy)
{
    int ww = 240, wh = 160;
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
        draw_icon(v, 12, 156);
        for (int yi = ny; yi < ny+nh && yi < 200; yi++)
            for (int xi = nx; xi < nx+nw && xi < 320; xi++)
                v[yi*320+xi] = (yi==ny||yi==ny+nh-1||xi==nx||xi==nx+nw-1)?0:(yi<ny+15)?1:15;
        for (volatile int i = 0; i < 20000; i++);
    }
}

void kernel_main(void)
{
    unsigned char* v = (unsigned char*)0xA0000;
    int mx, my, mb;
    int last_mx = -1, last_my = -1;

    int wx = 40, wy = 20, ww = 240, wh = 160;
    int win_visible = 1;
    int dragging = 0, drag_ox = 0, drag_oy = 0;
    int old_wx = wx, old_wy = wy;
    int last_mb = 0;

    for (int i = 0; i < 320*200; i++) v[i] = 7;
    draw_icon(v, 12, 156);
    draw_win(v, wx, wy);
    mouse_init();

    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char status = inb(0x64);
            if (status & 0x20) {
                mouse_handle_byte(inb(0x60));
            } else {
                inb(0x60);
            }
        }

        mouse_get(&mx, &my, &mb);

        int mb_press = (mb & 1) && !(last_mb & 1);
        last_mb = mb;

        // 单击图标打开窗口
        if (mb_press && !win_visible) {
            if (mx >= 12 && mx < 36 && my >= 156 && my < 180) {
                win_visible = 1;
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_icon(v, 12, 156);
                draw_win(v, wx, wy);
                mouse_reset();
                last_mx = -1;
                last_my = -1;
            }
        }

        // 关闭按钮
        if (mb & 1 && win_visible && !dragging) {
            if (mx >= wx+ww-20 && mx < wx+ww-2 && my >= wy+1 && my < wy+14) {
                win_visible = 0;
                animate_close(v, wx, wy);
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_icon(v, 12, 156);
                mouse_reset();
                last_mx = -1;
                last_my = -1;
                mouse_draw(v);
                last_mx = mx;
                last_my = my;
                while (mb & 1) {
                    if (inb(0x64) & 0x01) {
                        unsigned char st = inb(0x64);
                        if (st & 0x20) mouse_handle_byte(inb(0x60));
                    }
                    mouse_get(&mx, &my, &mb);
                }
                continue;
            }
        }

        // 拖动
        if (win_visible && mb & 1) {
            if (!dragging) {
                if (mx >= wx && mx < wx+ww && my >= wy && my < wy+15) {
                    dragging = 1;
                    drag_ox = mx - wx;
                    drag_oy = my - wy;
                    old_wx = wx;
                    old_wy = wy;
                }
            }
            if (dragging) {
                int nwx = mx - drag_ox;
                int nwy = my - drag_oy;
                if (nwx < 0) nwx = 0;
                if (nwy < 0) nwy = 0;
                if (nwx+ww > 320) nwx = 320-ww;
                if (nwy+wh > 200) nwy = 200-wh;
                if (nwx != wx || nwy != wy) {
                    for (int i = 0; i < 320*200; i++) v[i] = 7;
                    draw_icon(v, 12, 156);
                    draw_box(v, nwx, nwy, ww, wh);
                    wx = nwx; wy = nwy;
                }
            }
        } else {
            if (dragging) {
                dragging = 0;
                for (int i = 0; i < 320*200; i++) v[i] = 7;
                draw_icon(v, 12, 156);
                draw_win(v, wx, wy);
                mouse_reset();
                last_mx = -1;
                last_my = -1;
            }
        }

        if (!dragging && (mx != last_mx || my != last_my)) {
            mouse_draw(v);
            last_mx = mx;
            last_my = my;
        }
    }
}