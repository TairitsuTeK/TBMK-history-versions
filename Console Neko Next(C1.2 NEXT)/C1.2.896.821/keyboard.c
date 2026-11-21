#include "keyboard.h"
#include "shell.h"
#include "port.h"
#include "vga.h"
#include "scancode.h"

extern int cursor_x;
extern int cursor_y;
extern unsigned short cursor_backup;

static void reboot(void)
{
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
}

static const char* get_key_name_raw(unsigned char scancode)
{
    unsigned char code = scancode & 0x7F;
    switch (code) {
        case SC_ESC:         return "Esc";
        case SC_1:           return "1";
        case SC_2:           return "2";
        case SC_3:           return "3";
        case SC_4:           return "4";
        case SC_5:           return "5";
        case SC_6:           return "6";
        case SC_7:           return "7";
        case SC_8:           return "8";
        case SC_9:           return "9";
        case SC_0:           return "0";
        case SC_MINUS:       return "-";
        case SC_EQUAL:       return "=";
        case SC_BACKSPACE:   return "Backspace";
        case SC_TAB:         return "Tab";
        case SC_Q:           return "Q";
        case SC_W:           return "W";
        case SC_E:           return "E";
        case SC_R:           return "R";
        case SC_T:           return "T";
        case SC_Y:           return "Y";
        case SC_U:           return "U";
        case SC_I:           return "I";
        case SC_O:           return "O";
        case SC_P:           return "P";
        case SC_LBRACKET:    return "[";
        case SC_RBRACKET:    return "]";
        case SC_ENTER:       return "Enter";
        case SC_LCTRL:       return "Ctrl";
        case SC_A:           return "A";
        case SC_S:           return "S";
        case SC_D:           return "D";
        case SC_F:           return "F";
        case SC_G:           return "G";
        case SC_H:           return "H";
        case SC_J:           return "J";
        case SC_K:           return "K";
        case SC_L:           return "L";
        case SC_SEMICOLON:   return ";";
        case SC_QUOTE:       return "'";
        case SC_BACKTICK:    return "`";
        case SC_LSHIFT:      return "LShift";
        case SC_BACKSLASH:   return "\\";
        case SC_Z:           return "Z";
        case SC_X:           return "X";
        case SC_C:           return "C";
        case SC_V:           return "V";
        case SC_B:           return "B";
        case SC_N:           return "N";
        case SC_M:           return "M";
        case SC_COMMA:       return ",";
        case SC_DOT:         return ".";
        case SC_SLASH:       return "/";
        case SC_RSHIFT:      return "RShift";
        case SC_KP_STAR:     return "Keypad *";
        case SC_LALT:        return "Alt";
        case SC_SPACE:       return "Space";
        case SC_CAPSLOCK:    return "CapsLock";
        case SC_F1:          return "F1";
        case SC_F2:          return "F2";
        case SC_F3:          return "F3";
        case SC_F4:          return "F4";
        case SC_F5:          return "F5";
        case SC_F6:          return "F6";
        case SC_F7:          return "F7";
        case SC_F8:          return "F8";
        case SC_F9:          return "F9";
        case SC_F10:         return "F10";
        case SC_NUMLOCK:     return "NumLock";
        case SC_SCROLLLOCK:  return "ScrollLock";
        case SC_KP_7:        return "Keypad 7";
        case SC_KP_8:        return "Keypad 8";
        case SC_KP_9:        return "Keypad 9";
        case SC_KP_MINUS:    return "Keypad -";
        case SC_KP_4:        return "Keypad 4";
        case SC_KP_5:        return "Keypad 5";
        case SC_KP_6:        return "Keypad 6";
        case SC_KP_PLUS:     return "Keypad +";
        case SC_KP_1:        return "Keypad 1";
        case SC_KP_2:        return "Keypad 2";
        case SC_KP_3:        return "Keypad 3";
        case SC_KP_0:        return "Keypad 0";
        case SC_KP_DOT:      return "Keypad .";
        case SC_F11:         return "F11";
        case SC_F12:         return "F12";
        case SC_LWIN:        return "LWin";
        case SC_RWIN:        return "RWin";
        default:             return "Unknown";
    }
}

static void show_status(const char* str)
{
    int pos = 24 * VGA_WIDTH;
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 0; i < 40; i++)
        vga[pos + i] = (COLOR_WHITE_ON_BLUE << 8) | ' ';
    vga_print(0, 24, "Key Status:[", COLOR_WHITE_ON_BLUE);
    vga_print(13, 24, str, COLOR_WHITE_ON_BLUE);
    int len = 0;
    while (str[len]) len++;
    vga_put_char(13 + len, 24, ']', COLOR_WHITE_ON_BLUE);
}

__attribute__((naked)) void keyboard_handler(void)
{
    __asm__ volatile(
        "pushal\n"
        "cld\n"
        "call keyboard_callback\n"
        "popal\n"
        "iret\n"
    );
}

void keyboard_callback(void)
{
    unsigned char scancode = inb(0x60);
    static unsigned char ctrl_pressed = 0;
    static unsigned char alt_pressed = 0;
    static unsigned char ext_key = 0;
    static char key_status[40];

    if (scancode == SC_EXTENDED) {
        ext_key = 1;
        outb(0x20, 0x20);
        return;
    }

    const char* name = get_key_name_raw(scancode);

    if (scancode & SC_RELEASE_MASK) {
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

    show_status(key_status);

    if (scancode == SC_LCTRL)       ctrl_pressed = 1;
    if (scancode == SC_RELEASE_CTRL) ctrl_pressed = 0;
    if (scancode == SC_LALT)         alt_pressed = 1;
    if (scancode == SC_RELEASE_ALT)  alt_pressed = 0;

    if (scancode == SC_KP_DOT && ctrl_pressed && alt_pressed)
        reboot();

    if (shell_is_open()) {
        if (scancode == SC_ESC)         shell_close();
        else if (scancode == SC_L && ctrl_pressed) shell_clear();
        else                            shell_input(scancode);
        outb(0x20, 0x20);
        return;
    }

    if (ext_key && !(scancode & SC_RELEASE_MASK)) {
        int old_pos = cursor_y * VGA_WIDTH + cursor_x;
        volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
        vga[old_pos] = cursor_backup;

        switch (scancode) {
            case SC_EXT_UP:    cursor_y--; break;
            case SC_EXT_DOWN:  cursor_y++; break;
            case SC_EXT_LEFT:  cursor_x--; break;
            case SC_EXT_RIGHT: cursor_x++; break;
        }
        if (cursor_x < 1)  cursor_x = 1;
        if (cursor_x > 78) cursor_x = 78;
        if (cursor_y < 2)  cursor_y = 2;
        if (cursor_y > 22) cursor_y = 22;

        int new_pos = cursor_y * VGA_WIDTH + cursor_x;
        cursor_backup = vga[new_pos];
        unsigned short ch = vga[new_pos] & 0x00FF;
        vga[new_pos] = (COLOR_BLACK_ON_WHITE << 8) | ch;
    }
    ext_key = 0;

    if (scancode == SC_S && ctrl_pressed)
        shell_show();

    outb(0x20, 0x20);
}

void keyboard_init(void)
{
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

    for (int i = 0; i < 10; i++)
        if (inb(0x64) & 0x01) inb(0x60);

    __asm__ volatile("sti");
}