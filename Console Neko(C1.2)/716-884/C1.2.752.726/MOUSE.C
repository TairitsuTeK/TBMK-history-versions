#include "mouse.h"

#define VGA_BUFFER 0xB8000

int mouse_x = 40;
int mouse_y = 10;
unsigned short mouse_backup = 0x1F00 | ' ';
int mouse_moved = 0;

static void clamp_mouse(void)
{
    if (mouse_x < 1) mouse_x = 1;
    if (mouse_x > 78) mouse_x = 78;
    if (mouse_y < 2) mouse_y = 2;
    if (mouse_y > 22) mouse_y = 22;
}

void mouse_redraw(void)
{
    if (!mouse_moved) return;
    mouse_moved = 0;
    clamp_mouse();

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int pos = mouse_y * 80 + mouse_x;
    vga[pos] = mouse_backup;
    mouse_backup = vga[pos];
    unsigned short ch = vga[pos] & 0x00FF;
    vga[pos] = 0x7000 | ch;
}

void mouse_handler(void)
{
}

void mouse_init(void)
{
    clamp_mouse();
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int pos = mouse_y * 80 + mouse_x;
    mouse_backup = vga[pos];
    unsigned short ch = vga[pos] & 0x00FF;
    vga[pos] = 0x7000 | ch;
}