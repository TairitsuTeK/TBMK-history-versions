#include "window.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define WIDTH  80
#define HEIGHT 25

static int window_open = 0;
static unsigned short screen_backup[WIDTH * 24];

static int input_pos = 0;
static int cursor_y = 4;

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void put_char(int x, int y, unsigned char c, unsigned char attr)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT - 1) {
        vga[y * WIDTH + x] = (attr << 8) | c;
    }
}

static void draw_string(int x, int y, const char* str, unsigned char attr)
{
    while (*str && x < WIDTH) {
        put_char(x++, y, *str++, attr);
    }
}

static void draw_h_line_top(int x1, int x2, int y, unsigned char attr)
{
    put_char(x1, y, 0xC9, attr);
    for (int x = x1 + 1; x < x2; x++) put_char(x, y, 0xCD, attr);
    put_char(x2, y, 0xBB, attr);
}

static void draw_h_line_bottom(int x1, int x2, int y, unsigned char attr)
{
    put_char(x1, y, 0xC8, attr);
    for (int x = x1 + 1; x < x2; x++) put_char(x, y, 0xCD, attr);
    put_char(x2, y, 0xBC, attr);
}

// 扫描码转 ASCII（非Shift）
static char scancode_to_ascii(unsigned char scancode)
{
    switch (scancode) {
        case 0x02: return '1';
        case 0x03: return '2';
        case 0x04: return '3';
        case 0x05: return '4';
        case 0x06: return '5';
        case 0x07: return '6';
        case 0x08: return '7';
        case 0x09: return '8';
        case 0x0A: return '9';
        case 0x0B: return '0';
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1A: return '[';
        case 0x1B: return ']';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x29: return '`';
        case 0x2B: return '\\';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        case 0x39: return ' ';
        default: return 0;
    }
}

void window_input(unsigned char scancode)
{
    if (!window_open) return;
    if (scancode & 0x80) return;
    
    // Backspace
    if (scancode == 0x0E) {
        if (input_pos > 0) {
            // 删除当前行光标前一个字符
            input_pos--;
            put_char(2 + input_pos, cursor_y, ' ', COLOR);
            put_char(2 + input_pos + 1, cursor_y, ' ', COLOR);
            put_char(2 + input_pos, cursor_y, '_', COLOR);
        } else if (cursor_y > 1) {
            // 跳到上一行末尾
            cursor_y--;
            // 找上一行最后一个非空字符位置
            volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
            input_pos = 0;
            for (int x = WIDTH - 3; x >= 2; x--) {
                unsigned short ch = vga[cursor_y * WIDTH + x];
                if ((ch & 0xFF) != ' ' && (ch & 0xFF) != '_') {
                    input_pos = x - 1;  // 停在最后一个字符之后
                    break;
                }
            }
            // 清除旧光标
            put_char(2, cursor_y + 1, ' ', COLOR);
            // 在新位置画光标
            put_char(2 + input_pos, cursor_y, '_', COLOR);
        }
        return;
    }
    
    // Enter：换行
    if (scancode == 0x1C) {
        put_char(2 + input_pos, cursor_y, ' ', COLOR);
        cursor_y++;
        input_pos = 0;
        
        if (cursor_y >= 22) {
            cursor_y = 22;
            volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
            for (int y = 3; y < 22; y++) {
                for (int x = 1; x < WIDTH - 1; x++) {
                    vga[y * WIDTH + x] = vga[(y + 1) * WIDTH + x];
                }
            }
            for (int x = 1; x < WIDTH - 1; x++) {
                put_char(x, 22, ' ', COLOR);
            }
        }
        
        put_char(2, cursor_y, '_', COLOR);
        return;
    }
    
    // 可打印字符
    char c = scancode_to_ascii(scancode);
    if (c && input_pos < WIDTH - 5) {
        put_char(2 + input_pos, cursor_y, c, COLOR);
        input_pos++;
        put_char(2 + input_pos, cursor_y, '_', COLOR);
    }
}

void window_init(void)
{
    window_open = 0;
    input_pos = 0;
    cursor_y = 1;  
}

void window_show(void)
{
    if (window_open) return;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    for (int i = 0; i < WIDTH * 24; i++) {
        screen_backup[i] = vga[i];
    }
    
    for (int y = 1; y < 23; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            put_char(x, y, ' ', COLOR);
        }
    }
    
    draw_h_line_top(0, WIDTH - 1, 0, COLOR);
    for (int y = 1; y < 23; y++) {
        put_char(0, y, 0xBA, COLOR);
        put_char(WIDTH - 1, y, 0xBA, COLOR);
    }
    draw_h_line_bottom(0, WIDTH - 1, 23, COLOR);
    
    draw_string(3, 0, " Input windows ", COLOR);
    draw_string(3, 23, " [!][ESC] to back ", COLOR);
    
    input_pos = 0;
    cursor_y = 1;
    put_char(2, cursor_y, '_', COLOR);
    
    window_open = 1;
}

void window_close(void)
{
    if (!window_open) return;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    for (int i = 0; i < WIDTH * 24; i++) {
        vga[i] = screen_backup[i];
    }
    
    window_open = 0;
}

int window_is_open(void)
{
    return window_open;
}