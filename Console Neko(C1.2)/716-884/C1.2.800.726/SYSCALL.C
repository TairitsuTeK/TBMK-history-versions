#include "syscall.h"
#include "port.h"
#include "vga.h"

void sys_print(const char* str)
{
    int row = 6, col = 2;
    while (*str) {
        if (*str == '\n') { row++; col = 2; str++; continue; }
        if (row > 20) break;
        vga_put_char(col, row, *str, COLOR_WHITE_ON_BLUE);
        col++; str++;
        if (col > 70) { col = 2; row++; }
    }
}

void sys_clear(void)
{
    vga_fill(' ', COLOR_WHITE_ON_BLUE);
}

void sys_reboot(void)
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

__attribute__((naked)) void sys_exec(void)
{
    __asm__ volatile(
        "movl $0x90000, %%esp\n"
        "movl $0x200000, %%eax\n"
        "jmp *%%eax\n"
        : : : "eax"
    );
}

__attribute__((naked)) void sys_exit(void)
{
    __asm__ volatile("call _sys_reboot\n");
}

__attribute__((naked)) void syscall_handler(void)
{
    __asm__ volatile(
        "pushal\n"
        "cmpl $1, %%eax\n je 1f\n"
        "cmpl $2, %%eax\n je 2f\n"
        "cmpl $3, %%eax\n je 3f\n"
        "cmpl $4, %%eax\n je 4f\n"
        "cmpl $5, %%eax\n je 5f\n"
        "jmp 9f\n"
        "1:\n pushl %%ebx\n call _sys_print\n addl $4, %%esp\n jmp 9f\n"
        "2:\n call _sys_clear\n jmp 9f\n"
        "3:\n call _sys_reboot\n jmp 9f\n"
        "4:\n call _sys_exec\n jmp 9f\n"
        "5:\n call _sys_exit\n jmp 9f\n"
        "9:\n popal\n iret\n"
        : : : "eax", "ebx"
    );
}