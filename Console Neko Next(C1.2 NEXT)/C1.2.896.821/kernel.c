#include <stdint.h>
#include "keyboard.h"
#include "shell.h"
#include "version.h"
#include "except.h"
#include "syscall.h"
#include "disk.h"
#include "port.h"
#include "vga.h"
#include "scancode.h"

#define PAGE_SIZE 4096
#define MAX_PAGES 65536

#define CMOS_ADDR_PORT  0x70
#define CMOS_DATA_PORT  0x71
#define CMOS_EXTMEM_LOW  0x17
#define CMOS_EXTMEM_HIGH 0x18

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} multiboot_info_t;

void kernel_main(void) __attribute__((noreturn));

static unsigned char mem_bitmap[MAX_PAGES / 8];
unsigned int total_pages = 0;
unsigned int allocated_pages = 0;

volatile unsigned int timer_ticks = 0;

int cursor_x = 40;
int cursor_y = 10;
unsigned short cursor_backup = COLOR_WHITE_ON_BLUE << 8 | ' ';

static unsigned long long idt[256] __attribute__((aligned(8)));

// CPU检测函数
static void detect_cpu(void)
{
    unsigned int eax, ebx, ecx, edx;
    
    // 获取厂商信息
    eax = 0;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
    *(unsigned int*)0x7F00 = ebx;
    *(unsigned int*)0x7F04 = edx;
    *(unsigned int*)0x7F08 = ecx;
    *(unsigned char*)0x7F0C = 0;
    
    // 获取CPU名称
    eax = 0x80000000;
    __asm__ volatile("cpuid" : "=a"(eax) : "a"(eax));
    
    if (eax >= 0x80000004) {
        eax = 0x80000002;
        __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        *(unsigned int*)0x7F10 = eax;
        *(unsigned int*)0x7F14 = ebx;
        *(unsigned int*)0x7F18 = ecx;
        *(unsigned int*)0x7F1C = edx;
        
        eax = 0x80000003;
        __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        *(unsigned int*)0x7F20 = eax;
        *(unsigned int*)0x7F24 = ebx;
        *(unsigned int*)0x7F28 = ecx;
        *(unsigned int*)0x7F2C = edx;
        
        eax = 0x80000004;
        __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
        *(unsigned int*)0x7F30 = eax;
        *(unsigned int*)0x7F34 = ebx;
        *(unsigned int*)0x7F38 = ecx;
        *(unsigned int*)0x7F3C = edx;
        
        *(unsigned char*)0x7F40 = 0;
    } else {
        *(unsigned char*)0x7F10 = 0;
    }
}

static unsigned int cmos_get_memory(void)
{
    outb(CMOS_ADDR_PORT, CMOS_EXTMEM_LOW);
    unsigned char low = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDR_PORT, CMOS_EXTMEM_HIGH);
    unsigned char high = inb(CMOS_DATA_PORT);
    return ((unsigned int)high << 8) | low;
}

static unsigned int detect_total_memory(void)
{
    multiboot_info_t* mbi = (multiboot_info_t*)0x7F64;
    if (mbi && (mbi->flags & 0x01)) {
        unsigned int total_kb = mbi->mem_upper + 1024;
        if (total_kb > 0 && total_kb < 1048576) {
            unsigned int pages = total_kb * 1024 / PAGE_SIZE;
            return (pages > MAX_PAGES) ? MAX_PAGES : pages;
        }
    }
    
    unsigned int ext_kb = cmos_get_memory();
    if (ext_kb == 0 || ext_kb > 1048576) {
        ext_kb = 130048;
    }
    unsigned int pages = (1024 + ext_kb) * 1024 / PAGE_SIZE;
    return (pages > MAX_PAGES) ? MAX_PAGES : pages;
}

static void bitmap_set(unsigned int page, int used)
{
    if (page < total_pages) {
        if (used)
            mem_bitmap[page / 8] |=  (1 << (page % 8));
        else
            mem_bitmap[page / 8] &= ~(1 << (page % 8));
    }
}

static int bitmap_test(unsigned int page)
{
    if (page >= total_pages) return 1;
    return (mem_bitmap[page / 8] & (1 << (page % 8))) != 0;
}

static void mm_init(void)
{
    total_pages = detect_total_memory();
    for (unsigned int i = 0; i < (total_pages + 7) / 8; i++)
        mem_bitmap[i] = 0;

    unsigned int esp_val;
    __asm__ volatile("mov %%esp, %0" : "=r"(esp_val));

    unsigned int kernel_end = esp_val + 0x40000;
    if (kernel_end > 0x90000) kernel_end = 0x90000;

    unsigned int start_page = 0x1000 / PAGE_SIZE;
    unsigned int end_page = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned int i = start_page; i < end_page && i < total_pages; i++)
        bitmap_set(i, 1);

    unsigned int vga_start = 0xA0000 / PAGE_SIZE;
    unsigned int vga_end   = (0xC0000 + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned int i = vga_start; i < vga_end && i < total_pages; i++)
        bitmap_set(i, 1);

    for (unsigned int i = 0; i < start_page && i < total_pages; i++)
        bitmap_set(i, 1);

    allocated_pages = 0;
    for (unsigned int i = 0; i < total_pages; i++)
        if (bitmap_test(i)) allocated_pages++;
}

unsigned int alloc_page(void)
{
    for (unsigned int i = 1; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i, 1);
            allocated_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0;
}

void free_page(unsigned int addr)
{
    unsigned int page = addr / PAGE_SIZE;
    if (page > 0 && page < total_pages && bitmap_test(page)) {
        bitmap_set(page, 0);
        allocated_pages--;
    }
}

unsigned int alloc_pages(unsigned int count)
{
    if (count == 0) return 0;
    for (unsigned int i = 1; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            unsigned int j;
            for (j = 1; j < count; j++)
                if (bitmap_test(i + j)) break;
            if (j == count) {
                for (unsigned int k = 0; k < count; k++) {
                    bitmap_set(i + k, 1);
                    allocated_pages++;
                }
                return i * PAGE_SIZE;
            }
            i += j;
        }
    }
    return 0;
}

static void set_idt_entry(int num, void* handler)
{
    unsigned int addr = (unsigned int)handler;
    idt[num] = ((unsigned long long)(addr & 0xFFFF))
             | (0x08ULL << 16)
             | (0x8EULL << 40)
             | ((unsigned long long)(addr >> 16) << 48);
}

static void load_idt(void)
{
    struct { unsigned short limit; unsigned int base; }
    __attribute__((packed)) idt_ptr;
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (unsigned int)&idt;
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

static void cursor_draw(void)
{
    cursor_backup = vga_get_char(cursor_x, cursor_y);
    unsigned short ch = cursor_backup & 0x00FF;
    vga_put_char(cursor_x, cursor_y, ch, COLOR_BLACK_ON_WHITE);
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
        "incl timer_ticks\n"
        "movb $0x20, %al\n"
        "outb %al, $0x20\n"
        "popal\n"
        "iret\n"
    );
}

static void disable_cursor(void)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
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

static void print_string(int row, const char *str)
{
    int pos = row * VGA_WIDTH + 2;
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    while (*str) {
        if ((pos % VGA_WIDTH) >= 78) break;
        vga[pos++] = (COLOR_WHITE_ON_BLUE << 8) | *str++;
    }
}

static void draw_box(void)
{
    vga_put_char(0, 1, BOX_TL, COLOR_WHITE_ON_BLUE);
    for (int i = 1; i < 79; i++) vga_put_char(i, 1, BOX_HZ, COLOR_WHITE_ON_BLUE);
    vga_put_char(79, 1, BOX_TR, COLOR_WHITE_ON_BLUE);

    for (int y = 2; y < 23; y++) {
        vga_put_char(0, y, BOX_VT, COLOR_WHITE_ON_BLUE);
        vga_put_char(79, y, BOX_VT, COLOR_WHITE_ON_BLUE);
    }

    vga_put_char(0, 23, BOX_BL, COLOR_WHITE_ON_BLUE);
    for (int i = 1; i < 79; i++) vga_put_char(i, 23, BOX_HZ, COLOR_WHITE_ON_BLUE);
    vga_put_char(79, 23, BOX_BR, COLOR_WHITE_ON_BLUE);
}

static void wait_for_reboot(void)
{
    unsigned char ctrl = 0, alt = 0;
    while (1) {
        if (inb(0x64) & 0x01) {
            unsigned char sc = inb(0x60);
            if (sc == SC_LCTRL) ctrl = 1;
            if (sc == SC_RELEASE_CTRL) ctrl = 0;
            if (sc == SC_LALT) alt = 1;
            if (sc == SC_RELEASE_ALT) alt = 0;
            if (sc == SC_KP_DOT && ctrl && alt && !(sc & SC_RELEASE_MASK)) {
                while (inb(0x64) & 0x02);
                outb(0x64, 0xFE);
            }
        }
    }
}

void kernel_main(void)
{
    detect_cpu();  // 检测CPU信息
    
    disable_cursor();
    beep();
    mm_init();

    unsigned char boot_drive = *(unsigned char*)0x7F60;

    if (boot_drive == 0x00) {
        vga_fill(' ', COLOR_WHITE_ON_BLUE);
        draw_box();

        print_string(5,  "  T.B.M.K. " KERNEL_VERSION " Installer");
        print_string(6,  "  ==============================");
        print_string(8,  "  [1] Install to hard disk");
        print_string(9,  "  [2] Boot from hard disk");
        print_string(10, "  [3] Try live system");
        print_string(12, "  Press 1, 2 or 3 to select...");
        print_string(14, "  Ctrl+Alt+Del to reboot at any time.");

        unsigned char ctrl = 0, alt = 0;
        int mode = 0;

        while (mode == 0) {
            if (inb(0x64) & 0x01) {
                unsigned char sc = inb(0x60);
                if (sc == SC_LCTRL) ctrl = 1;
                if (sc == SC_RELEASE_CTRL) ctrl = 0;
                if (sc == SC_LALT) alt = 1;
                if (sc == SC_RELEASE_ALT) alt = 0;

                if (sc == SC_KP_DOT && ctrl && alt && !(sc & SC_RELEASE_MASK)) {
                    while (inb(0x64) & 0x02);
                    outb(0x64, 0xFE);
                }
                if (sc == SC_1) mode = 1;
                if (sc == SC_2) mode = 2;
                if (sc == SC_3) mode = 3;
            }
        }

        if (mode == 1) {
            if (!disk_detect()) {
                print_string(16, "  No hard disk detected!");
                print_string(17, "  Install aborted.");
                print_string(18, "  Press Ctrl+Alt+Del to reboot.");
                wait_for_reboot();
            }

            print_string(16, "  Installing...");

            unsigned char* kernel = (unsigned char*)0x1000;
            int write_failed = 0;
            for (int sec = 0; sec < 50; sec++) {
                if (disk_write(1 + sec, kernel + sec * 512) != 0) {
                    write_failed = 1;
                    break;
                }
            }

            if (write_failed) {
                print_string(18, "  Write kernel failed!");
                wait_for_reboot();
            }

            unsigned char orig_sector[512];
            disk_read(0, orig_sector);

            unsigned char buf[512];
            unsigned char* boot_sector = (unsigned char*)0x7C00;
            for (int i = 0; i < 512; i++) buf[i] = boot_sector[i];

            if (orig_sector[11] == 0x00 && orig_sector[12] == 0x02) {
                for (int i = 3; i < 62; i++) buf[i] = orig_sector[i];
            }

            buf[500] = 65 & 0xFF;
            buf[501] = (65 >> 8) & 0xFF;
            buf[502] = (65 >> 16) & 0xFF;
            buf[503] = (65 >> 24) & 0xFF;
            buf[510] = 0x55;
            buf[511] = 0xAA;

            if (disk_write(0, buf) != 0) {
                print_string(18, "  Write boot sector failed!");
                wait_for_reboot();
            }

            print_string(18, "  Installation complete!");
            print_string(19, "  Remove floppy and reboot.");
            print_string(20, "  Ctrl+Alt+Del to reboot.");
            wait_for_reboot();
        }

        if (mode == 2) {
            print_string(16, "  Remove floppy disk, then");
            print_string(17, "  press Ctrl+Alt+Del to reboot.");
            print_string(18, "  System will boot from hard disk.");
            wait_for_reboot();
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

    vga_fill(' ', COLOR_WHITE_ON_BLUE);

    for (int i = 0; i < VGA_WIDTH; i++)
        vga_put_char(i, 0, ' ', COLOR_BLACK_ON_WHITE);

    char *title = " T.B.M.K. Micro Kernel - Main Page ";
    int title_len = 0;
    while (title[title_len]) title_len++;
    int title_start = (VGA_WIDTH - title_len) / 2;
    for (int i = 0; i < title_len; i++)
        vga_put_char(title_start + i, 0, title[i], COLOR_BLACK_ON_WHITE);

    print_string(2,  KERNEL_FULL_STRING);
    print_string(4,  "  _______   ______   .___  ___.  __  ___  ");
    print_string(5,  " |       | |   _  |  |   ||   | |  |/  /  ");
    print_string(6,  " |_     _| |  |_) |  |   ||   | |  '  /   ");
    print_string(7,  "   |   |   |   _ <   |   ||   | |    <    ");
    print_string(8,  "   |   |   |  |_) |  |   ||   | |  .  \\  ");
    print_string(9,  "   |___|   |______/  |___||___| |__|\\__\\ ");
    print_string(11, "   Tiny Bare-Metal Micro Kernel");
    print_string(21, "  [!][Arrow keys] Move cursor");
    print_string(22, "  [!][Ctrl+S] Shell | [Ctrl+Alt+Del] Reboot");

    draw_box();

    shell_init();
    cursor_draw();

    while (1) {
        __asm__ volatile("hlt");
    }
}