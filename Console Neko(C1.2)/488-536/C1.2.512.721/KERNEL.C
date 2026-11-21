#include "keyboard.h"
#include "window.h"
#include "shell.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define MEM_COLOR 0x1E
#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5

#define STACK_TOP       0x90000

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

static int num_to_str_three_decimal(unsigned int value_1000ths, char *buf)
{
    unsigned int integer_part = value_1000ths / 1000;
    unsigned int decimal_part = value_1000ths % 1000;

    int len = num_to_str(integer_part, buf);
    buf[len++] = '.';
    buf[len++] = '0' + (decimal_part / 100);
    buf[len++] = '0' + ((decimal_part / 10) % 10);
    buf[len++] = '0' + (decimal_part % 10);
    buf[len] = '\0';
    return len;
}

static unsigned int detect_used_memory_1000ths(void)
{
    unsigned int current_esp;
    __asm__ volatile("mov %%esp, %0" : "=r"(current_esp));

    unsigned int stack_used = STACK_TOP - current_esp;
    unsigned int kernel_estimate = 0xA000;

    unsigned int total_bytes = kernel_estimate + stack_used;
    unsigned int value_1000ths = (total_bytes * 1000) / 1048576;
    if (value_1000ths == 0) value_1000ths = 1;

    return value_1000ths;
}

static void update_memory_display(volatile unsigned short *vga, unsigned int total_mb)
{
    unsigned int used_1000ths = detect_used_memory_1000ths();
    unsigned int total_1000ths = total_mb * 1000;

    char mem_str[50];
    int pos = 0;

    mem_str[pos++] = 'M';
    mem_str[pos++] = 'e';
    mem_str[pos++] = 'm';
    mem_str[pos++] = 'o';
    mem_str[pos++] = 'r';
    mem_str[pos++] = 'y';
    mem_str[pos++] = ' ';
    mem_str[pos++] = 'U';
    mem_str[pos++] = 's';
    mem_str[pos++] = 'a';
    mem_str[pos++] = 'g';
    mem_str[pos++] = 'e';
    mem_str[pos++] = ':';
    mem_str[pos++] = ' ';

    pos += num_to_str_three_decimal(used_1000ths, mem_str + pos);
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos++] = '/';

    pos += num_to_str_three_decimal(total_1000ths, mem_str + pos);
    mem_str[pos++] = 'M';
    mem_str[pos++] = 'B';
    mem_str[pos] = '\0';

    int i = 0;
    while (mem_str[i]) {
        vga[i] = (MEM_COLOR << 8) | mem_str[i];
        i++;
    }
    while (i < 40) {
        vga[i] = (MEM_COLOR << 8) | ' ';
        i++;
    }
}

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;

    disable_cursor();

    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (COLOR << 8) | ' ';

    unsigned short *mem_ptr = (unsigned short *)0x7E00;
    unsigned int total_mb = *mem_ptr;
    if (total_mb == 0 || total_mb > 4096) {
        total_mb = 128;
    }

    print_string(vga, 2, "[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version C1.2.512.721 Beta 4]");
    print_string(vga, 3, "[!]Welcome text:");
    print_string(vga, 4, "   +------------------------+");
    print_string(vga, 5, "   | print(\"Hello, World!\") |");
    print_string(vga, 6, "   +------------------------+");
    print_string(vga, 23, "[!][Ctrl+I] Input | [Ctrl+S] Shell | [Ctrl+R] Reboot");

    keyboard_init();
    window_init();
    shell_init();

    while (1) {
        update_memory_display(vga, total_mb);
        __asm__ volatile("hlt");
    }
}