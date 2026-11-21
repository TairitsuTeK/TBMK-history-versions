#include "keyboard.h"
#include "window.h"
#include "shell.h"

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

static void reboot(void)
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

static const char* get_key_name_raw(unsigned char scancode)
{
    unsigned char code = scancode & 0x7F;

    switch (code) {
        case 0x01: return "Esc";
        case 0x02: return "1";
        case 0x03: return "2";
        case 0x04: return "3";
        case 0x05: return "4";
        case 0x06: return "5";
        case 0x07: return "6";
        case 0x08: return "7";
        case 0x09: return "8";
        case 0x0A: return "9";
        case 0x0B: return "0";
        case 0x0C: return "-";
        case 0x0D: return "=";
        case 0x0E: return "Backspace";
        case 0x0F: return "Tab";
        case 0x10: return "Q";
        case 0x11: return "W";
        case 0x12: return "E";
        case 0x13: return "R";
        case 0x14: return "T";
        case 0x15: return "Y";
        case 0x16: return "U";
        case 0x17: return "I";
        case 0x18: return "O";
        case 0x19: return "P";
        case 0x1A: return "[";
        case 0x1B: return "]";
        case 0x1C: return "Enter";
        case 0x1D: return "Ctrl";
        case 0x1E: return "A";
        case 0x1F: return "S";
        case 0x20: return "D";
        case 0x21: return "F";
        case 0x22: return "G";
        case 0x23: return "H";
        case 0x24: return "J";
        case 0x25: return "K";
        case 0x26: return "L";
        case 0x27: return ";";
        case 0x28: return "'";
        case 0x29: return "`";
        case 0x2A: return "LShift";
        case 0x2B: return "\\";
        case 0x2C: return "Z";
        case 0x2D: return "X";
        case 0x2E: return "C";
        case 0x2F: return "V";
        case 0x30: return "B";
        case 0x31: return "N";
        case 0x32: return "M";
        case 0x33: return ",";
        case 0x34: return ".";
        case 0x35: return "/";
        case 0x36: return "RShift";
        case 0x37: return "Keypad *";
        case 0x38: return "Alt";
        case 0x39: return "Space";
        case 0x3A: return "CapsLock";
        case 0x3B: return "F1";
        case 0x3C: return "F2";
        case 0x3D: return "F3";
        case 0x3E: return "F4";
        case 0x3F: return "F5";
        case 0x40: return "F6";
        case 0x41: return "F7";
        case 0x42: return "F8";
        case 0x43: return "F9";
        case 0x44: return "F10";
        case 0x45: return "NumLock";
        case 0x46: return "ScrollLock";
        case 0x47: return "Keypad 7";
        case 0x48: return "Keypad 8";
        case 0x49: return "Keypad 9";
        case 0x4A: return "Keypad -";
        case 0x4B: return "Keypad 4";
        case 0x4C: return "Keypad 5";
        case 0x4D: return "Keypad 6";
        case 0x4E: return "Keypad +";
        case 0x4F: return "Keypad 1";
        case 0x50: return "Keypad 2";
        case 0x51: return "Keypad 3";
        case 0x52: return "Keypad 0";
        case 0x53: return "Keypad .";
        case 0x57: return "F11";
        case 0x58: return "F12";
        case 0x5B: return "LWin";
        case 0x5C: return "RWin";
        default: return "Unknown";
    }
}

static void show_string(const char* str, int row, int col)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    int pos = row * 80 + col;
    for (int i = 0; i < 30; i++) {
        vga[pos + i] = 0x1F00 | ' ';
    }
    while (*str) {
        vga[pos++] = 0x1F00 | *str++;
    }
}

__attribute__((naked)) void keyboard_handler(void)
{
    __asm__ volatile(
        "pushal\n"
        "cld\n"
        "call _keyboard_callback\n"
        "popal\n"
        "iret\n"
    );
}

void keyboard_callback(void)
{
    unsigned char scancode = inb(0x60);
    static unsigned char ctrl_pressed = 0;
    static unsigned char alt_pressed = 0;
    static char key_status[40];

    const char* name = get_key_name_raw(scancode);
    if (scancode & 0x80) {
        int i = 0;
        while (name[i] && i < 20) { key_status[i] = name[i]; i++; }
        const char* rel = " Released";
        int j = 0;
        while (rel[j] && (i + j) < 39) { key_status[i + j] = rel[j]; j++; }
        key_status[i + j] = '\0';
    } else {
        int i = 0;
        while (name[i] && i < 39) { key_status[i] = name[i]; i++; }
        key_status[i] = '\0';
    }

    int len = 0;
    while (key_status[len]) len++;

    show_string("Key Status:[", 24, 0);
    show_string(key_status, 24, 13);
    show_string("]", 24, 13 + len + 1);

    if (scancode == 0x1D) {
        ctrl_pressed = 1;
    } else if (scancode == 0x9D) {
        ctrl_pressed = 0;
    }

    if (scancode == 0x38) {
        alt_pressed = 1;
    } else if (scancode == 0xB8) {
        alt_pressed = 0;
    }

    // Ctrl+Alt+Delete 重启（全局有效）
    if (scancode == 0x53 && ctrl_pressed && alt_pressed && !(scancode & 0x80)) {
        reboot();
    }

    // Shell 打开时
    if (shell_is_open()) {
        if (scancode == 0x01 && !(scancode & 0x80)) {
            shell_close();
        } else if (scancode == 0x26 && ctrl_pressed && !(scancode & 0x80)) {
            shell_clear();
        } else {
            shell_input(scancode);
        }
        outb(0x20, 0x20);
        return;
    }

    if (window_is_open()) {
        if (scancode == 0x01 && !(scancode & 0x80)) {
            window_close();
        } else {
            window_input(scancode);
        }
        outb(0x20, 0x20);
        return;
    }

    if (scancode == 0x17 && ctrl_pressed) {
        window_show();
    } else if (scancode == 0x1F && ctrl_pressed) {
        shell_show();
    }

    outb(0x20, 0x20);
}

void keyboard_init(void)
{
    static unsigned long long idt[256] __attribute__((aligned(8)));
    
    unsigned long long handler = (unsigned long long)keyboard_handler;
    unsigned short low = handler & 0xFFFF;
    unsigned short high = (handler >> 16) & 0xFFFF;
    
    idt[0x21] = ((unsigned long long)low)
              | (0x08ULL << 16)
              | (0x8EULL << 40)
              | ((unsigned long long)high << 48);
    
    struct {
        unsigned short limit;
        unsigned int base;
    } __attribute__((packed)) idt_ptr;
    
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (unsigned int)&idt;
    
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
    
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFD);
    outb(0xA1, 0xFF);
    
    for (int i = 0; i < 10; i++) {
        if (inb(0x64) & 0x01) {
            inb(0x60);
        }
    }
    
    __asm__ volatile("sti");
}