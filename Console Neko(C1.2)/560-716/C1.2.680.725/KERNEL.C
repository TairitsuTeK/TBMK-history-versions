#include "keyboard.h"
#include "shell.h"
#include "version.h"
#include "except.h"
#include "syscall.h"
#include "disk.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define BAR_COLOR 0x0F
#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5

#define PAGE_SIZE 4096
#define MAX_PAGES 4096

void kernel_main(void) __attribute__((noreturn));

static unsigned char mem_bitmap[MAX_PAGES / 8];
unsigned int allocated_pages = 0;
volatile unsigned int timer_ticks = 0;

int cursor_x = 40;
int cursor_y = 10;
unsigned short cursor_backup = 0x1F00 | ' ';

static unsigned long long idt[256] __attribute__((aligned(8)));

static void set_idt_entry(int num, void* handler)
{
    unsigned long long addr = (unsigned long long)handler;
    unsigned short low = addr & 0xFFFF;
    unsigned short high = (addr >> 16) & 0xFFFF;

    idt[num] = ((unsigned long long)low)
             | (0x08ULL << 16)
             | (0x8EULL << 40)
             | ((unsigned long long)high << 48);
}

static void load_idt(void)
{
    struct {
        unsigned short limit;
        unsigned int base;
    } __attribute__((packed)) idt_ptr;

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (unsigned int)&idt;

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

static void mm_init(void)
{
    unsigned int esp_val;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp_val));

    unsigned int kernel_end_estimate = 0x90000 - esp_val + 0x20000;
    unsigned int kernel_pages = kernel_end_estimate / PAGE_SIZE + 1;
    if (kernel_pages < 1) kernel_pages = 1;
    if (kernel_pages > MAX_PAGES) kernel_pages = MAX_PAGES;

    for (int i = 0; i < MAX_PAGES / 8; i++) {
        mem_bitmap[i] = 0;
    }
    for (unsigned int i = 0; i < kernel_pages; i++) {
        mem_bitmap[i / 8] |= (1 << (i % 8));
    }
    allocated_pages = kernel_pages;
}

static void cursor_draw(void)
{
    volatile unsigned short *vga_ptr = (volatile unsigned short *)VGA_BUFFER;
    int pos = cursor_y * 80 + cursor_x;
    cursor_backup = vga_ptr[pos];
    unsigned short ch = vga_ptr[pos] & 0x00FF;
    vga_ptr[pos] = 0x7000 | ch;
}

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

static void pit_init(unsigned int hz)
{
    unsigned int divisor = 1193180 / hz;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

__attribute__((naked)) void pit_handler(void)
{
    __asm__ volatile(
        "pushal\n"
        "incl _timer_ticks\n"
        "movb $0x20, %al\n"
        "outb %al, $0x20\n"
        "popal\n"
        "iret\n"
    );
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
    for (int i = 0; i < 80; i++) {
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

    unsigned int used_kb = (allocated_pages * 4096) / 1024;
    unsigned int total_mb_for_bar = detect_total_memory_tenths() / 10;
    unsigned int percent_int = (allocated_pages * 10000) / (total_mb_for_bar * 256);

    char mem_str[50];
    int mp = 0;
    mem_str[mp++] = 'M';
    mem_str[mp++] = 'e';
    mem_str[mp++] = 'm';
    mem_str[mp++] = ' ';
    mem_str[mp++] = 'U';
    mem_str[mp++] = 's';
    mem_str[mp++] = 'a';
    mem_str[mp++] = 'g';
    mem_str[mp++] = 'e';
    mem_str[mp++] = ':';
    mem_str[mp++] = ' ';
    char num[10]; int np = 0;
    unsigned int n = used_kb;
    if (n == 0) num[np++] = '0';
    while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
    while (np > 0) mem_str[mp++] = num[--np];
    mem_str[mp++] = 'K';
    mem_str[mp++] = 'B';
    mem_str[mp++] = '/';
    np = 0; n = total_mb_for_bar;
    if (n == 0) num[np++] = '0';
    while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
    while (np > 0) mem_str[mp++] = num[--np];
    mem_str[mp++] = 'M';
    mem_str[mp++] = 'B';
    mem_str[mp++] = ' ';
    mem_str[mp++] = '(';
    n = percent_int / 100;
    if (n == 0) { mem_str[mp++] = '0'; }
    else {
        np = 0;
        while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
        while (np > 0) mem_str[mp++] = num[--np];
    }
    mem_str[mp++] = '.';
    mem_str[mp++] = '0' + ((percent_int / 10) % 10);
    mem_str[mp++] = '0' + (percent_int % 10);
    mem_str[mp++] = '%';
    mem_str[mp++] = ')';
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
        vga[i] = (BAR_COLOR << 8) | bar_str[i];
    }
}

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;

    disable_cursor();
    beep();
    mm_init();

    unsigned char boot_drive = *(unsigned char*)0x7F60;

    if (boot_drive == 0x00) {
        for (int i = 0; i < 80 * 25; i++)
            vga[i] = (COLOR << 8) | ' ';

        draw_box(vga);

        print_string(vga, 5, "  T.B.M.K. " KERNEL_VERSION " Installer");
        print_string(vga, 6, "  ==============================");
        print_string(vga, 8, "  [1] Install to hard disk");
        print_string(vga, 9, "  [2] Boot from hard disk");
        print_string(vga, 10, "  [3] Try live system");
        print_string(vga, 12, "  Press 1, 2 or 3 to select...");
        print_string(vga, 14, "  Ctrl+Alt+Del to reboot at any time.");

        unsigned char ins_ctrl = 0;
        unsigned char ins_alt = 0;
        int mode = 0;

        while (mode == 0) {
            if (inb(0x64) & 0x01) {
                unsigned char sc = inb(0x60);

                if (sc == 0x1D) ins_ctrl = 1;
                if (sc == 0x9D) ins_ctrl = 0;
                if (sc == 0x38) ins_alt = 1;
                if (sc == 0xB8) ins_alt = 0;

                if (sc == 0x53 && ins_ctrl && ins_alt && !(sc & 0x80)) {
                    while (inb(0x64) & 0x02);
                    outb(0x64, 0xFE);
                }

                if (sc == 0x02) mode = 1;
                if (sc == 0x03) mode = 2;
                if (sc == 0x04) mode = 3;
            }
        }

        if (mode == 1) {
            print_string(vga, 16, "  Installing...");
            unsigned char buf[512];
            unsigned char* boot_sector = (unsigned char*)0x7C00;
            for (int i = 0; i < 512; i++) buf[i] = boot_sector[i];
            disk_write(0, buf);
            unsigned char* kernel = (unsigned char*)0x1000;
            for (int sec = 0; sec < 50; sec++) {
                disk_write(1 + sec, kernel + sec * 512);
            }
            print_string(vga, 18, "  Installation complete!");
            print_string(vga, 19, "  Remove floppy and reboot.");
            print_string(vga, 20, "  Ctrl+Alt+Del to reboot.");

            while (1) {
                if (inb(0x64) & 0x01) {
                    unsigned char sc2 = inb(0x60);
                    if (sc2 == 0x1D) ins_ctrl = 1;
                    if (sc2 == 0x9D) ins_ctrl = 0;
                    if (sc2 == 0x38) ins_alt = 1;
                    if (sc2 == 0xB8) ins_alt = 0;
                    if (sc2 == 0x53 && ins_ctrl && ins_alt && !(sc2 & 0x80)) {
                        while (inb(0x64) & 0x02);
                        outb(0x64, 0xFE);
                    }
                }
            }
        }

        if (mode == 2) {
            print_string(vga, 16, "  Remove floppy disk, then");
            print_string(vga, 17, "  press Ctrl+Alt+Del to reboot.");
            print_string(vga, 18, "  System will boot from hard disk.");

            while (1) {
                if (inb(0x64) & 0x01) {
                    unsigned char sc2 = inb(0x60);
                    if (sc2 == 0x1D) ins_ctrl = 1;
                    if (sc2 == 0x9D) ins_ctrl = 0;
                    if (sc2 == 0x38) ins_alt = 1;
                    if (sc2 == 0xB8) ins_alt = 0;
                    if (sc2 == 0x53 && ins_ctrl && ins_alt && !(sc2 & 0x80)) {
                        while (inb(0x64) & 0x02);
                        outb(0x64, 0xFE);
                    }
                }
            }
        }
    }

    set_idt_entry(0, except_handler_0);
    set_idt_entry(6, except_handler_6);
    set_idt_entry(13, except_handler_13);
    set_idt_entry(14, except_handler_14);
    set_idt_entry(0x20, pit_handler);
    set_idt_entry(0x80, syscall_handler);
    set_idt_entry(0x21, keyboard_handler);
    load_idt();

    keyboard_init();
    pit_init(100);
    outb(0x21, inb(0x21) & 0xFE);

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
    print_string(vga, 21, "  [!][Arrow keys] Move cursor");
    print_string(vga, 22, "  [!][Ctrl+S] Shell | [Ctrl+Alt+Del] Reboot");

    draw_box(vga);

    unsigned int* lfb = *(unsigned int**)0x7F70;
    char lfb_buf[20];
    lfb_buf[0] = 'L'; lfb_buf[1] = 'F'; lfb_buf[2] = 'B'; lfb_buf[3] = ':'; lfb_buf[4] = ' ';
    unsigned int addr = (unsigned int)lfb;
    for (int i = 7; i >= 0; i--) {
        lfb_buf[5 + i] = "0123456789ABCDEF"[addr & 0x0F];
        addr >>= 4;
    }
    lfb_buf[13] = '\0';
    int len = 13;
    int start_col = 78 - len;
    for (int i = 0; i < len; i++) {
        vga[23 * 80 + start_col + i] = (BAR_COLOR << 8) | lfb_buf[i];
    }

    shell_init();
    cursor_draw();

    while (1) {
        if (timer_ticks % 50 == 0) {
            update_status_bar(vga);
        }
        __asm__ volatile("hlt");
    }
}