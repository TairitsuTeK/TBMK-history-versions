#include "except.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x4F

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
    if (x >= 0 && x < 80 && y >= 0 && y < 25) {
        vga[y * 80 + x] = (attr << 8) | c;
    }
}

static void print_str(int x, int y, const char* str, unsigned char attr)
{
    while (*str && x < 80) {
        put_char(x++, y, *str++, attr);
    }
}

void show_exception(int num)
{
    // 停止 PIT
    outb(0x43, 0x30);
    outb(0x40, 0);
    outb(0x40, 0);

    const char* names[] = {
        "Division by Zero", "", "", "", "", "",
        "Invalid Opcode", "", "", "", "", "", "",
        "General Protection Fault", "Page Fault"
    };
    const char* name = (num <= 14) ? names[num] : "Unknown";

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = (COLOR << 8) | ' ';
    }

    print_str(2, 3, "==============================================", COLOR);
    print_str(2, 4, "           SYSTEM EXCEPTION", COLOR);
    print_str(2, 5, "==============================================", COLOR);
    print_str(2, 7, "Exception: ", COLOR);
    print_str(14, 7, name, COLOR);

    char num_str[5];
    num_str[0] = '0';
    num_str[1] = 'x';
    num_str[2] = "0123456789ABCDEF"[(num >> 4) & 0x0F];
    num_str[3] = "0123456789ABCDEF"[num & 0x0F];
    num_str[4] = '\0';
    print_str(2, 8, "Number: ", COLOR);
    print_str(11, 8, num_str, COLOR);

    print_str(2, 10, "The system has encountered a fatal error.", COLOR);
    print_str(2, 12, "Please force reboot your computer.", COLOR);

    while (1) {
        __asm__ volatile("hlt");
    }
}

__attribute__((naked)) void except_handler_0(void)
{
    __asm__ volatile(
        "pushal\n"
        "push $0\n"
        "call _show_exception\n"
        "add $4, %esp\n"
        "popal\n"
        "iret\n"
    );
}

__attribute__((naked)) void except_handler_6(void)
{
    __asm__ volatile(
        "pushal\n"
        "push $6\n"
        "call _show_exception\n"
        "add $4, %esp\n"
        "popal\n"
        "iret\n"
    );
}

__attribute__((naked)) void except_handler_13(void)
{
    __asm__ volatile(
        "pushal\n"
        "push $13\n"
        "call _show_exception\n"
        "add $4, %esp\n"
        "popal\n"
        "iret\n"
    );
}

__attribute__((naked)) void except_handler_14(void)
{
    __asm__ volatile(
        "pushal\n"
        "push $14\n"
        "call _show_exception\n"
        "add $4, %esp\n"
        "popal\n"
        "iret\n"
    );
}