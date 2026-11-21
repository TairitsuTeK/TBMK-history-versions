// font.c - 恢复8像素宽
#include "font.h"

void font_draw_char(unsigned char* vga, int x, int y, char c, unsigned char fg)
{
    unsigned char* glyph = font[(unsigned char)c];
    for (int row = 0; row < 8; row++) {
        unsigned char line = glyph[row];
        for (int col = 0; col < 8; col++) {
            if (line & (1 << (7 - col))) {
                int px = x + col, py = y + row;
                if (px >= 0 && px < 320 && py >= 0 && py < 200)
                    vga[py*320+px] = fg;
            }
        }
    }
}

void font_draw_text(unsigned char* vga, int x, int y, const char* text, unsigned char fg)
{
    while (*text) {
        font_draw_char(vga, x, y, *text, fg);
        x += 8;
        text++;
    }
}