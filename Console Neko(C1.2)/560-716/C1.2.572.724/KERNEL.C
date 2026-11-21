#include "keyboard.h"
#include "window.h"
#include "shell.h"
#include "mouse.h"
#include "version.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define BAR_COLOR 0x0F
#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5

void kernel_main(void) __attribute__((noreturn));

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

static void disable_cursor(void)
{
    outb(VGA_CRTC_ADDR, 0x0A);
    outb(VGA_CRTC_DATA, 0x20);
}

static void beep(void)
{
    outb(0x43, 0xB6);
    outb(0x42, 0xA9);
    outb(0x42, 0x04);

    unsigned char tmp = inb(0x61);
    outb(0x61, tmp | 0x03);

    for (volatile int i = 0; i < 500000; i++) { }

    outb(0x61, tmp & 0xFC);
}

static void print_string(volatile unsigned short *vga, int row, const char *str)
{
    int pos = row * 80 + 2;
    while (*str) {
        if ((pos % 80) >= 78) break;
        vga[pos++] = (COLOR << 8) | *str++;
    }
}

static void draw_box(volatile unsigned short *vga)
{
    vga[80 + 0] = (COLOR << 8) | 0xC9;
    for (int i = 1; i < 79; i++) {
        vga[80 + i] = (COLOR << 8) | 0xCD;
    }
    vga[80 + 79] = (COLOR << 8) | 0xBB;

    for (int y = 2; y < 23; y++) {
        vga[y * 80 + 0] = (COLOR << 8) | 0xBA;
        vga[y * 80 + 79] = (COLOR << 8) | 0xBA;
    }

    vga[23 * 80 + 0] = (COLOR << 8) | 0xC8;
    for (int i = 1; i < 79; i++) {
        vga[23 * 80 + i] = (COLOR << 8) | 0xCD;
    }
    vga[23 * 80 + 79] = (COLOR << 8) | 0xBC;
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

static void update_status_bar(volatile unsigned short *vga)
{
    unsigned int total_tenths = detect_total_memory_tenths();

    for (int i = 0; i < 80; i++) {
        if (i == mouse_x && mouse_y == 0) continue;
        vga[i] = (BAR_COLOR << 8) | ' ';
    }

    char *cpu_name = (char *)0x7F10;
    while (*cpu_name == ' ') cpu_name++;
    if (cpu_name[0] == '\0') {
        cpu_name = (char *)0x7F00;
    }

    char bar_str[80];
    int pos = 0;

    bar_str[pos++] = ' ';
    bar_str[pos++] = 'C';
    bar_str[pos++] = 'P';
    bar_str[pos++] = 'U';
    bar_str[pos++] = ':';
    bar_str[pos++] = ' ';
    for (int i = 0; cpu_name[i] && pos < 48; i++) {
        bar_str[pos++] = cpu_name[i];
    }

    char mem_str[30];
    int mp = 0;
    mem_str[mp++] = 'I';
    mem_str[mp++] = 'n';
    mem_str[mp++] = 's';
    mem_str[mp++] = 't';
    mem_str[mp++] = 'a';
    mem_str[mp++] = 'l';
    mem_str[mp++] = 'l';
    mem_str[mp++] = 'e';
    mem_str[mp++] = 'd';
    mem_str[mp++] = ' ';
    mem_str[mp++] = 'M';
    mem_str[mp++] = 'e';
    mem_str[mp++] = 'm';
    mem_str[mp++] = 'o';
    mem_str[mp++] = 'r';
    mem_str[mp++] = 'y';
    mem_str[mp++] = ':';
    mem_str[mp++] = ' ';
    mp += num_to_str_one_decimal(total_tenths, mem_str + mp);
    mem_str[mp++] = 'M';
    mem_str[mp++] = 'B';
    mem_str[mp] = '\0';

    int mem_len = mp;
    int mem_start = 80 - mem_len - 1;

    while (pos < mem_start) {
        bar_str[pos++] = ' ';
    }
    for (int i = 0; i < mem_len; i++) {
        bar_str[pos++] = mem_str[i];
    }
    bar_str[pos++] = ' ';

    for (int i = 0; i < 80 && i < pos; i++) {
        if (i == mouse_x && mouse_y == 0) continue;
        vga[i] = (BAR_COLOR << 8) | bar_str[i];
    }
}

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;

    disable_cursor();
    beep();

    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (COLOR << 8) | ' ';

    print_string(vga, 2, KERNEL_FULL_STRING);
    print_string(vga, 4, "  _______   ______   .___  ___.  __  ___  ");
    print_string(vga, 5, " |       | |   _  |  |   ||   | |  |/  /  ");
    print_string(vga, 6, " |_     _| |  |_) |  |   ||   | |  '  /   ");
    print_string(vga, 7, "   |   |   |   _ <   |   ||   | |    <    ");
    print_string(vga, 8, "   |   |   |  |_) |  |   ||   | |  .  \\  ");
    print_string(vga, 9, "   |___|   |______/  |___||___| |__|\\__\\ ");
    print_string(vga, 11, "   Tiny Bare-Metal Micro Kernel");
    print_string(vga, 22, "  [!][Ctrl+I] Input | [Ctrl+S] Shell | [Ctrl+Alt+Del] Reboot");

    draw_box(vga);

    keyboard_init();
    mouse_init();

    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);

    window_init();
    shell_init();

    while (1) {
        update_status_bar(vga);
        mouse_redraw();
        __asm__ volatile("hlt");
    }
}