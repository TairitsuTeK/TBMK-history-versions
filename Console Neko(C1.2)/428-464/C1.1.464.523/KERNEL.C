#include "keyboard.h"
#include "window.h"
#include "shell.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
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

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    disable_cursor();
    
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = (COLOR << 8) | ' ';
    
    print_string(vga, 0, "[!]Micro Kernel Started Successfully!");
    print_string(vga, 1, "[!]Running at x86 Bare-metal Protected Mode......(OK)");
    print_string(vga, 2, "[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version C1.1.464.523 Beta 3]");
    print_string(vga, 4, "[!]Welcome text:");
    print_string(vga, 5, "   +------------------------+");
    print_string(vga, 6, "   | print(\"Hello, World!\") |");
    print_string(vga, 7, "   +------------------------+");
    print_string(vga, 22, "[!][Ctrl+R] to reboot.");
    print_string(vga, 23, "[!][Ctrl+I] Input | [Ctrl+S] Shell | [Ctrl+R] Reboot");
    
    keyboard_init();
    window_init();
    shell_init();
    
    while (1) { __asm__ volatile("hlt"); }
}