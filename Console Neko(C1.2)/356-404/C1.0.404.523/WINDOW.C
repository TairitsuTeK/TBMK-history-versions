#include "window.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define WIDTH  80
#define HEIGHT 25
#define MAX_INPUT_POS (WIDTH - 5)

static int window_open = 0;

static int input_pos = 0;
static int cursor_y = 1;
static int shift_pressed = 0;
static unsigned short screen_backup[WIDTH * 24];
static int caps_lock = 0;

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

static char scancode_to_ascii(unsigned char scancode)
{
    int upper = shift_pressed ^ caps_lock;
    switch (scancode)
    {
        case 0x02: return upper ? '!' : '1';
        case 0x03: return upper ? '@' : '2';
        case 0x04: return upper ? '#' : '3';
        case 0x05: return upper ? '$' : '4';
        case 0x06: return upper ? '%' : '5';
        case 0x07: return upper ? '^' : '6';
        case 0x08: return upper ? '&' : '7';
        case 0x09: return upper ? '*' : '8';
        case 0x0A: return upper ? '(' : '9';
        case 0x0B: return upper ? ')' : '0';
        case 0x0C: return upper ? '_' : '-';
        case 0x0D: return upper ? '+' : '=';
        case 0x10: return upper ? 'Q' : 'q';
        case 0x11: return upper ? 'W' : 'w';
        case 0x12: return upper ? 'E' : 'e';
        case 0x13: return upper ? 'R' : 'r';
        case 0x14: return upper ? 'T' : 't';
        case 0x15: return upper ? 'Y' : 'y';
        case 0x16: return upper ? 'U' : 'u';
        case 0x17: return upper ? 'I' : 'i';
        case 0x18: return upper ? 'O' : 'o';
        case 0x19: return upper ? 'P' : 'p';
        case 0x1A: return upper ? '{' : '[';
        case 0x1B: return upper ? '}' : ']';
        case 0x1E: return upper ? 'A' : 'a';
        case 0x1F: return upper ? 'S' : 's';
        case 0x20: return upper ? 'D' : 'd';
        case 0x21: return upper ? 'F' : 'f';
        case 0x22: return upper ? 'G' : 'g';
        case 0x23: return upper ? 'H' : 'h';
        case 0x24: return upper ? 'J' : 'j';
        case 0x25: return upper ? 'K' : 'k';
        case 0x26: return upper ? 'L' : 'l';
        case 0x27: return upper ? ':' : ';';
        case 0x28: return upper ? '"' : '\'';
        case 0x29: return upper ? '~' : '`';
        case 0x2B: return upper ? '|' : '\\';
        case 0x2C: return upper ? 'Z' : 'z';
        case 0x2D: return upper ? 'X' : 'x';
        case 0x2E: return upper ? 'C' : 'c';
        case 0x2F: return upper ? 'V' : 'v';
        case 0x30: return upper ? 'B' : 'b';
        case 0x31: return upper ? 'N' : 'n';
        case 0x32: return upper ? 'M' : 'm';
        case 0x33: return upper ? '<' : ',';
        case 0x34: return upper ? '>' : '.';
        case 0x35: return upper ? '?' : '/';
        case 0x39: return ' ';
        default: return 0;
    }
}

void window_input(unsigned char scancode)
{
    if (!window_open) return;

    if(scancode == 0x2A || scancode == 0x36) shift_pressed = 1;
    if(scancode == 0xAA || scancode == 0xB6) shift_pressed = 0;
    if(scancode == 0x3A) caps_lock = !caps_lock;

    if (scancode & 0x80) return;
    
    if (scancode == 0x0E)
    {
        if (input_pos > 0)
        {
            input_pos--;
            put_char(2 + input_pos, cursor_y, ' ', COLOR);
            put_char(2 + input_pos + 1, cursor_y, ' ', COLOR);
            put_char(2 + input_pos, cursor_y, '_', COLOR);
        }
        else if (cursor_y > 1)
        {
            cursor_y--;
            volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
            input_pos = 0;
            for (int x = WIDTH - 3; x >= 2; x--)
            {
                unsigned short ch = vga[cursor_y * WIDTH + x];
                if ((ch & 0xFF) != ' ' && (ch & 0xFF) != '_')
                {
                    input_pos = x - 1;
                    break;
                }
            }
            put_char(2, cursor_y + 1, ' ', COLOR);
            put_char(2 + input_pos, cursor_y, '_', COLOR);
        }
        return;
    }
    
    if (scancode == 0x1C)
    {
        put_char(2 + input_pos, cursor_y, ' ', COLOR);
        cursor_y++;
        input_pos = 0;
        
        if (cursor_y >= 22)
        {
            cursor_y = 22;
        }
        put_char(2, cursor_y, '_', COLOR);
        return;
    }
    
    char c = scancode_to_ascii(scancode);
    if (c)
    {
        if (input_pos >= MAX_INPUT_POS)
        {
            cursor_y++;
            input_pos = 0;
            if (cursor_y >= 22)
            {
                cursor_y = 22;
            }
        }

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
    shift_pressed = 0;
    caps_lock = 0;
}

void window_show(void)
{
    if (window_open) return;
    window_open = 1;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    // 保存 0-23 行
    for (int i = 0; i < WIDTH * 24; i++) {
        screen_backup[i] = vga[i];
    }
    
    for (int y = 1; y < 23; y++)
        for (int x = 1; x < WIDTH - 1; x++)
            put_char(x, y, ' ', COLOR);

    draw_h_line_top(0, WIDTH - 1, 0, COLOR);
    draw_h_line_bottom(0, WIDTH - 1, 23, COLOR);
    for (int y = 1; y < 23; y++) {
        put_char(0, y, 0xBA, COLOR);
        put_char(WIDTH - 1, y, 0xBA, COLOR);
    }

    draw_string(3, 0, " Input windows ", COLOR);
    draw_string(3, 23, " [!][ESC] to back ", COLOR);

    input_pos = 0;
    cursor_y = 1;
    put_char(2, cursor_y, '_', COLOR);
}

void window_close(void)
{
    if (!window_open) return;
    window_open = 0;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    // 恢复 0-23 行
    for (int i = 0; i < WIDTH * 24; i++) {
        vga[i] = screen_backup[i];
    }
}

int window_is_open(void)
{
    return window_open;
}