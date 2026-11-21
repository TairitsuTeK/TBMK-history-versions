#include "mouse.h"

#define VGA_BUFFER 0xB8000

int mouse_x = 40;
int mouse_y = 12;
unsigned short mouse_backup = 0x1F00 | ' ';
int mouse_moved = 0;

void mouse_redraw(void)
{
    // 主循环统一重绘，不需要额外操作
}

void mouse_handler(void)
{
}

void mouse_init(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int pos = mouse_y * 80 + mouse_x;
    mouse_backup = vga[pos];
    vga[pos] = 0x0F00 | 0xDB;
}