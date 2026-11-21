#include "keyboard.h"
#include "window.h"
#include "shell.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define MEM_COLOR 0x1E
#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5

void kernel_main(void) __attribute__((noreturn));

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void disable_cursor(void)
{
    outb(VGA_CRTC_ADDR, 0x0A);
    outb(VGA_CRTC_DATA, 0x20);
}

static void print_string(volatile unsigned short *vga, int row, const char *str)
{
    int pos = row * 80;
    while (*str) { vga[pos++] = (COLOR << 8) | *str++; }
}

static int num_to_str(unsigned int num, char *buf)
{
    int len = 0;
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    char tmp[10];
    int i = 0;
    while (num > 0) {
        tmp[i++] = '0' + (num % 10);
        num /= 10;
    }
    while (i > 0) {
        buf[len++] = tmp[--i];
    }
    buf[len] = '\0';
    return len;
}

static int num_to_str_one_decimal(unsigned int value_tenths, char *buf)
{
    unsigned int integer_part = value_tenths / 10;
    unsigned int decimal_part = value_tenths % 10;

    int len = num_to_str(integer_part, buf);
    buf[len++] = '.';
    buf[len++] = '0' + decimal_part;
    buf[len] = '\0';
    return len;
}

static unsigned int detect_total_memory_tenths(void)
{
    unsigned short *mem_ptr = (unsigned short *)0x7E00;
    unsigned int total_mb = *mem_ptr;
    if (total_mb == 0 || total_mb > 4096) {
        total_mb = 128;
    }
    return total_mb * 10;
}

static void update_memory_display(volatile unsigned short *vga)
{
    unsigned int total_tenths = detect_total_memory_tenths();

    char mem_str[50];
    int pos = 0;

    mem_str[pos++] = 'I';
    mem_str[pos++] = 'n';
    mem_str[pos++] = 's';
    mem_str[pos++] = 't';
    mem_str[pos++] = 'a';
    mem_str[pos++] = 'l';
    mem_str[pos++] = 'l';
    mem_str[pos++] = 'e';
    mem_str[pos++] = 'd';
    mem_str[pos++] = ' ';
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'e';
    mem_str[pos++] = 'm';
    mem_str[pos++] = 'o';
    mem_str[pos++] = 'r';
    mem_str[pos++] = 'y';
    mem_str[pos++] = ':';
    mem_str[pos++] = ' ';

    pos += num_to_str_one_decimal(total_tenths, mem_str + pos);
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos] = '\0';

    int len = 0;
    while (mem_str[len]) len++;

    int start_col = 80 - len;

    int i = 0;
    while (mem_str[i]) {
        vga[start_col + i] = (MEM_COLOR << 8) | mem_str[i];
        i++;
    }
}

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;

    disable_cursor();

    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (COLOR << 8) | ' ';

    print_string(vga, 2, "[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version C1.2.524.721 Beta 4]");
    print_string(vga, 3, "[!]Welcome text:");
    print_string(vga, 4, "   +------------------------+");
    print_string(vga, 5, "   | print(\"Hello, World!\") |");
    print_string(vga, 6, "   +------------------------+");
    print_string(vga, 23, "[!][Ctrl+I] Input | [Ctrl+S] Shell | [Ctrl+R] Reboot");

    keyboard_init();
    window_init();
    shell_init();

    while (1) {
        update_memory_display(vga);
        __asm__ volatile("hlt");
    }
}