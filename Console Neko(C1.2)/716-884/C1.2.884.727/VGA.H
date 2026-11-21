#ifndef VGA_H
#define VGA_H

#define VGA_BUFFER   0xB8000
#define VGA_WIDTH    80
#define VGA_HEIGHT   25

#define COLOR_WHITE_ON_BLUE   0x1F
#define COLOR_WHITE_ON_BLACK  0x0F
#define COLOR_GREEN_ON_BLACK  0x0A
#define COLOR_RED_ON_BLACK    0x0C
#define COLOR_WHITE_ON_RED    0x4F
#define COLOR_BLACK_ON_WHITE  0x70

// 画框字符
#define BOX_TL  0xC9
#define BOX_TR  0xBB
#define BOX_BL  0xC8
#define BOX_BR  0xBC
#define BOX_HZ  0xCD
#define BOX_VT  0xBA

static inline void vga_put_char(int x, int y, unsigned char c, unsigned char attr)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga[y * VGA_WIDTH + x] = (attr << 8) | c;
    }
}

static inline unsigned short vga_get_char(int x, int y)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        return vga[y * VGA_WIDTH + x];
    }
    return 0;
}

static inline void vga_fill(unsigned char c, unsigned char attr)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    unsigned short val = (attr << 8) | c;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = val;
    }
}

static void vga_print(int x, int y, const char *str, unsigned char attr)
{
    while (*str && x < VGA_WIDTH) {
        vga_put_char(x++, y, *str++, attr);
    }
}

#endif