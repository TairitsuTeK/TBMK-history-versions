#include "except.h"
#include "port.h"
#include "vga.h"

void show_exception(int num)
{
    outb(0x43, 0x30);
    outb(0x40, 0);
    outb(0x40, 0);

    const char* names[] = {
        "Division by Zero", "", "", "", "", "",
        "Invalid Opcode", "", "", "", "", "", "",
        "General Protection Fault", "Page Fault"
    };
    const char* name = (num <= 14) ? names[num] : "Unknown";

    vga_fill(' ', COLOR_WHITE_ON_RED);

    vga_print(2, 3,  "==============================================", COLOR_WHITE_ON_RED);
    vga_print(2, 4,  "           SYSTEM EXCEPTION", COLOR_WHITE_ON_RED);
    vga_print(2, 5,  "==============================================", COLOR_WHITE_ON_RED);
    vga_print(2, 7,  "Exception: ", COLOR_WHITE_ON_RED);
    vga_print(14, 7, name, COLOR_WHITE_ON_RED);

    char num_str[5];
    num_str[0] = '0'; num_str[1] = 'x';
    num_str[2] = "0123456789ABCDEF"[(num >> 4) & 0x0F];
    num_str[3] = "0123456789ABCDEF"[num & 0x0F];
    num_str[4] = '\0';
    vga_print(2, 8,  "Number: ", COLOR_WHITE_ON_RED);
    vga_print(11, 8, num_str, COLOR_WHITE_ON_RED);

    vga_print(2, 10, "The system has encountered a fatal error.", COLOR_WHITE_ON_RED);
    vga_print(2, 12, "Please force reboot your computer.", COLOR_WHITE_ON_RED);

    while (1) __asm__ volatile("hlt");
}

__attribute__((naked)) void except_handler_0(void)
{
    __asm__ volatile("pushal\npush $0\ncall show_exception\nadd $4, %esp\npopal\niret\n");
}
__attribute__((naked)) void except_handler_6(void)
{
    __asm__ volatile("pushal\npush $6\ncall show_exception\nadd $4, %esp\npopal\niret\n");
}
__attribute__((naked)) void except_handler_13(void)
{
    __asm__ volatile("pushal\npush $13\ncall show_exception\nadd $4, %esp\npopal\niret\n");
}
__attribute__((naked)) void except_handler_14(void)
{
    __asm__ volatile("pushal\npush $14\ncall show_exception\nadd $4, %esp\npopal\niret\n");
}