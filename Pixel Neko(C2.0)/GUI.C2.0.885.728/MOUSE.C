#include "port.h"

static int mx = 160, my = 100;
static int btn = 0;
static int cycle = 0;
static unsigned char buf[3];
static int init_done = 0;

static unsigned char saved[9][9];
static int saved_x = -1, saved_y = -1;

static void wait_kbc(void)
{
    for (volatile int i = 0; i < 10000; i++) {
        if (!(inb(0x64) & 0x02)) return;
    }
}

static void wait_read(void)
{
    for (volatile int i = 0; i < 10000; i++) {
        if (inb(0x64) & 0x01) return;
    }
}

void mouse_init(void)
{
    while (inb(0x64) & 0x01) inb(0x60);
    wait_kbc(); outb(0x64, 0xA8);
    wait_kbc(); outb(0x64, 0x20);
    wait_read();
    unsigned char cfg = inb(0x60);
    cfg |= 0x02;
    wait_kbc(); outb(0x64, 0x60);
    wait_kbc(); outb(0x60, cfg);
    wait_kbc(); outb(0x64, 0xD4);
    wait_kbc(); outb(0x60, 0xF4);
    wait_read();
    if (inb(0x60) == 0xFA) init_done = 1;
    wait_kbc(); outb(0x64, 0xD4); wait_kbc(); outb(0x60, 0xF3);
    wait_read(); inb(0x60);
    wait_kbc(); outb(0x64, 0xD4); wait_kbc(); outb(0x60, 60);
    wait_read(); inb(0x60);
}

void mouse_handle_byte(unsigned char d)
{
    if (!init_done) return;
    if (cycle == 0 && !(d & 0x08)) return;
    buf[cycle] = d;
    cycle++;
    if (cycle == 3) {
        cycle = 0;
        btn = buf[0] & 0x07;
        int dx = buf[1];
        int dy = buf[2];
        if (buf[0] & 0x10) dx = dx - 256;
        if (buf[0] & 0x20) dy = dy - 256;
        mx += dx / 2;
        my -= dy / 2;
        if (mx < 0) mx = 0;
        if (mx > 319) mx = 319;
        if (my < 0) my = 0;
        if (my > 199) my = 199;
    }
}

void mouse_get(int* x, int* y, int* b)
{
    if (x) *x = mx;
    if (y) *y = my;
    if (b) *b = btn;
}

void mouse_draw(unsigned char* vga)
{
    if (!init_done) return;
    if (saved_x >= 0) {
        for (int i = 0; i < 9; i++)
            for (int j = 0; j < 9; j++)
                if (saved_y+i < 200 && saved_x+j < 320)
                    vga[(saved_y+i)*320+(saved_x+j)] = saved[i][j];
    }
    int x = mx, y = my;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            saved[i][j] = (y+i < 200 && x+j < 320) ? vga[(y+i)*320+(x+j)] : 0;
    saved_x = x;
    saved_y = y;
    for (int j = 0; j < 9; j++) {
        int px = x+j, py = y+4;
        if (px < 320 && py < 200) vga[py*320+px] = 0;
    }
    for (int i = 0; i < 9; i++) {
        int px = x+4, py = y+i;
        if (px < 320 && py < 200) vga[py*320+px] = 0;
    }
    if (x+4 < 320 && y+4 < 200) vga[(y+4)*320+(x+4)] = 15;
}

void mouse_reset(void)
{
    saved_x = -1;
    saved_y = -1;
}