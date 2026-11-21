#include "syscall.h"

#define VGA_BUFFER 0xB8000
#define USER_PROGRAM_ADDR 0x200000

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

void sys_print(const char* str)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int row = 6;
    int col = 2;

    while (*str) {
        if (*str == '\n') { row++; col = 2; str++; continue; }
        if (row > 20) break;
        vga[row * 80 + col] = 0x1F00 | *str;
        col++; str++;
        if (col > 70) { col = 2; row++; }
    }
}

void sys_clear(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < 80 * 25; i++)
        vga[i] = 0x1F00 | ' ';
}

void sys_reboot(void)
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

__attribute__((naked)) void sys_exec(void)
{
    __asm__ volatile(
        "movl $0x90000, %%esp\n"  // 重置栈
        "movl $0x200000, %%eax\n"
        "jmp *%%eax\n"
        :
        :
        : "eax"
    );
}

__attribute__((naked)) void sys_exit(void)
{
    __asm__ volatile(
        "call _sys_reboot\n"
    );
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
        "cmpl $4, %%eax\n"
        "je 4f\n"
        "cmpl $5, %%eax\n"
        "je 5f\n"
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
        "jmp 9f\n"
        "4:\n"
        "call _sys_exec\n"
        "jmp 9f\n"
        "5:\n"
        "call _sys_exit\n"
        "jmp 9f\n"
        "9:\n"
        "popal\n"
        "iret\n"
        :
        :
        : "eax", "ebx"
    );
}