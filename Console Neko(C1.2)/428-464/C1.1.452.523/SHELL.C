#include "shell.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define WIDTH  80
#define HEIGHT 25

static int shell_open = 0;
static unsigned short screen_backup[WIDTH * 24];

static int input_pos = 0;
static int cursor_y = 1;
static int shift_pressed = 0;
static int caps_lock = 0;
static char input_buf[80];

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
    if (x >= 0 && x < WIDTH && y >= 0 && y < 24) {
        vga[y * WIDTH + x] = (attr << 8) | c;
    }
}

static void draw_string(int x, int y, const char* str, unsigned char attr)
{
    while (*str && x < WIDTH) put_char(x++, y, *str++, attr);
}

// 向上滚动一行
static void scroll_up(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int y = 1; y < 22; y++) {
        for (int x = 0; x < WIDTH; x++) {
            vga[y * WIDTH + x] = vga[(y + 1) * WIDTH + x];
        }
    }
    for (int x = 0; x < WIDTH; x++) {
        put_char(x, 22, ' ', COLOR);
    }
}

// 换行
static void new_line(void)
{
    if (cursor_y >= 22) {
        scroll_up();
    } else {
        cursor_y++;
    }
}

// 字符串长度
static int str_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

// 字符串比较
static int str_cmp(const char* a, const char* b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

// 输出一行文本
static void shell_print_line(const char* str)
{
    int x = 0;
    while (*str && x < WIDTH) {
        put_char(x, cursor_y, *str, COLOR);
        x++;
        str++;
    }
    new_line();
}

// 执行命令
static void execute_command(const char* cmd)
{
    if (str_len(cmd) == 0) {
        return;
    }
    
    if (str_cmp(cmd, "help") == 0 || str_cmp(cmd, "HELP") == 0) {
        shell_print_line("[!]T.M.B.K. Shell Commands:");
        shell_print_line("  HELP   -    Show this help.");
        shell_print_line("  CLS    -    Clear screen.");
        shell_print_line("  ABT    -    About mirco kernel.");
        shell_print_line("  RBT    -    Reboot system.");
        shell_print_line("  VER    -    Show mirco kernel version.");
    } else if (str_cmp(cmd, "cls") == 0 || str_cmp(cmd, "CLS") == 0) {
        for (int y = 1; y < 23; y++)
            for (int x = 0; x < WIDTH; x++)
                put_char(x, y, ' ', COLOR);
        cursor_y = 1;
    } else if (str_cmp(cmd, "abt") == 0 || str_cmp(cmd, "ABT") == 0) {
        shell_print_line("[!]T.M.B.K. - A Micro kernel based on x86 structure.");
        shell_print_line("[!](C) Tairitsu_tty 2026");
        shell_print_line("[!]Built on x86 Bare-metal Protected Mode");
    } else if (str_cmp(cmd, "ver") == 0 || str_cmp(cmd, "VER") == 0) {
        shell_print_line("[!]Ver. C1.1.452.523 Beta 3");
    } else if (str_cmp(cmd, "rbt") == 0 || str_cmp(cmd, "RBT") == 0) {
        while (inb(0x64) & 0x02);
        outb(0x64, 0xFE);
    } else {
        shell_print_line("[X]Unknown command. Type HELP for list.");
    }
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

// 显示提示符
static void show_prompt(void)
{
    cursor_y = (cursor_y > 22) ? 22 : cursor_y;
    put_char(0, cursor_y, '>', COLOR);
    put_char(1, cursor_y, ' ', COLOR);
    input_pos = 2;
    put_char(input_pos, cursor_y, '_', COLOR);
}

void shell_input(unsigned char scancode)
{
    if (!shell_open) return;

    if(scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if(scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; return; }
    if(scancode == 0x3A) { caps_lock = !caps_lock; return; }
    if (scancode & 0x80) return;

    if (scancode == 0x0E)
    {
        if (input_pos > 2) {
            input_pos--;
            put_char(input_pos, cursor_y, ' ', COLOR);
            put_char(input_pos + 1, cursor_y, ' ', COLOR);
            put_char(input_pos, cursor_y, '_', COLOR);
            input_buf[input_pos - 2] = '\0';
        }
        return;
    }

    if (scancode == 0x1C)
    {
        put_char(input_pos, cursor_y, ' ', COLOR);
        input_buf[input_pos - 2] = '\0';
        
        cursor_y++;
        input_pos = 0;
        
        execute_command(input_buf);
        
        input_buf[0] = '\0';
        input_pos = 0;
        show_prompt();
        return;
    }

    char c = scancode_to_ascii(scancode);
    if (c && input_pos < WIDTH - 2)
    {
        put_char(input_pos, cursor_y, c, COLOR);
        input_buf[input_pos - 2] = c;
        input_pos++;
        put_char(input_pos, cursor_y, '_', COLOR);
    }
}

void shell_init(void)
{
    shell_open = 0;
    input_pos = 0;
    cursor_y = 1;
    shift_pressed = 0;
    caps_lock = 0;
    input_buf[0] = '\0';
}

void shell_show(void)
{
    if (shell_open) return;
    shell_open = 1;

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < WIDTH * 24; i++) screen_backup[i] = vga[i];

    for (int y = 1; y < 23; y++)
        for (int x = 0; x < WIDTH; x++)
            put_char(x, y, ' ', COLOR);

    cursor_y = 1;
    input_pos = 0;
    input_buf[0] = '\0';
    
    put_char(0, cursor_y, '>', COLOR);
    put_char(1, cursor_y, ' ', COLOR);
    input_pos = 2;
    put_char(input_pos, cursor_y, '_', COLOR);
}

void shell_close(void)
{
    if (!shell_open) return;
    shell_open = 0;

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < WIDTH * 24; i++) vga[i] = screen_backup[i];
}

int shell_is_open(void)
{
    return shell_open;
}