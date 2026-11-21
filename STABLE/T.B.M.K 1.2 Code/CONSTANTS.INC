STACK_BASE      equ 0x90000
VGA_BASE        equ 0xB8000
SCREEN_COLS     equ 80
SCREEN_ROWS     equ 25
TEXT_ROWS       equ 15
TOTAL_CHARS     equ 2000
LINE_BYTES      equ 160         ; 80 * 2

; VGA 端口
CRTC_ADDR       equ 0x3D4
CRTC_DATA       equ 0x3D5
KEYB_STATUS     equ 0x64
KEYB_DATA       equ 0x60

; 键盘扫描码
SC_CTRL_DOWN    equ 0x1D
SC_CTRL_UP      equ 0x9D
SC_C            equ 0x2E
SC_R            equ 0x13
SC_F1           equ 0x3B
SC_F2           equ 0x3C
SC_F3           equ 0x3D
SC_F4           equ 0x3E
SC_F5           equ 0x3F
SC_F6           equ 0x40
SC_F7           equ 0x41
SC_F8           equ 0x42
SC_F9           equ 0x43
SC_F10          equ 0x44
SC_F11          equ 0x57
SC_F12          equ 0x58

; 颜色常量
COLOR_BLACK         equ 0x00
COLOR_BLUE          equ 0x01
COLOR_GREEN         equ 0x02
COLOR_CYAN          equ 0x03
COLOR_RED           equ 0x04
COLOR_MAGENTA       equ 0x05
COLOR_BROWN         equ 0x06
COLOR_LIGHT_GRAY    equ 0x07
COLOR_BRIGHT_BLUE   equ 0x09
COLOR_BRIGHT_GREEN  equ 0x0A
COLOR_BRIGHT_CYAN   equ 0x0B
COLOR_BRIGHT_RED    equ 0x0C
COLOR_BRIGHT_MAGENTA equ 0x0D
COLOR_YELLOW        equ 0x0E
COLOR_WHITE         equ 0x0F

; 背景颜色（高4位）
BG_BLACK            equ 0x00
BG_BLUE             equ 0x10
BG_GREEN            equ 0x20
BG_CYAN             equ 0x30
BG_RED              equ 0x40
BG_MAGENTA          equ 0x50
BG_BROWN            equ 0x60
BG_LIGHT_GRAY       equ 0x70