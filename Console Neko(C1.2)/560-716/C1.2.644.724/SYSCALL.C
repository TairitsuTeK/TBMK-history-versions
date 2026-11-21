#include "syscall.h"

#define VGA_BUFFER 0xB8000

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

static void __attribute__((used)) sys_print(const char* str)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    static int row = 2, col = 1;
    while (*str) {
        if (*str == '\n') { row++; col = 1; str++; continue; }
        if (row >= 22) row = 2;
        vga[row * 80 + col] = 0x1F00 | *str;
        col++; str++;
        if (col >= 79) { col = 1; row++; }
        if (row >= 22) row = 2;
    }
    row = 2;
    col = 1;
}

static void __attribute__((used)) sys_clear(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0x1F00 | ' ';
    }
}

static void __attribute__((used)) sys_reboot(void)
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

__attribute__((naked)) void syscall_handler(void)
{
    __asm__ volatile(
        "pushal\n"
        "cmpl $1, %%eax\n"
        "je 1f\n"
        "cmpl $2, %%eax\n"
        "je 2f\n"
        "cmpl $3, %%eax\n"
        "je 3f\n"
        "jmp 9f\n"
        "1:\n"
        "pushl %%ebx\n"
        "call _sys_print\n"
        "addl $4, %%esp\n"
        "jmp 9f\n"
        "2:\n"
        "call _sys_clear\n"
        "jmp 9f\n"
        "3:\n"
        "call _sys_reboot\n"
        "9:\n"
        "popal\n"
        "iret\n"
        :
        :
        : "eax", "ebx", "esp"
    );
}