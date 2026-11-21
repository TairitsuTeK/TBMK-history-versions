#ifndef SCANCODE_H
#define SCANCODE_H

// 按下扫描码
#define SC_ESC          0x01
#define SC_1            0x02
#define SC_2            0x03
#define SC_3            0x04
#define SC_4            0x05
#define SC_5            0x06
#define SC_6            0x07
#define SC_7            0x08
#define SC_8            0x09
#define SC_9            0x0A
#define SC_0            0x0B
#define SC_MINUS        0x0C
#define SC_EQUAL        0x0D
#define SC_BACKSPACE    0x0E
#define SC_TAB          0x0F
#define SC_Q            0x10
#define SC_W            0x11
#define SC_E            0x12
#define SC_R            0x13
#define SC_T            0x14
#define SC_Y            0x15
#define SC_U            0x16
#define SC_I            0x17
#define SC_O            0x18
#define SC_P            0x19
#define SC_LBRACKET     0x1A
#define SC_RBRACKET     0x1B
#define SC_ENTER        0x1C
#define SC_LCTRL        0x1D
#define SC_A            0x1E
#define SC_S            0x1F
#define SC_D            0x20
#define SC_F            0x21
#define SC_G            0x22
#define SC_H            0x23
#define SC_J            0x24
#define SC_K            0x25
#define SC_L            0x26
#define SC_SEMICOLON    0x27
#define SC_QUOTE        0x28
#define SC_BACKTICK     0x29
#define SC_LSHIFT       0x2A
#define SC_BACKSLASH    0x2B
#define SC_Z            0x2C
#define SC_X            0x2D
#define SC_C            0x2E
#define SC_V            0x2F
#define SC_B            0x30
#define SC_N            0x31
#define SC_M            0x32
#define SC_COMMA        0x33
#define SC_DOT          0x34
#define SC_SLASH        0x35
#define SC_RSHIFT       0x36
#define SC_KP_STAR      0x37
#define SC_LALT         0x38
#define SC_SPACE        0x39
#define SC_CAPSLOCK     0x3A
#define SC_F1           0x3B
#define SC_F2           0x3C
#define SC_F3           0x3D
#define SC_F4           0x3E
#define SC_F5           0x3F
#define SC_F6           0x40
#define SC_F7           0x41
#define SC_F8           0x42
#define SC_F9           0x43
#define SC_F10          0x44
#define SC_NUMLOCK      0x45
#define SC_SCROLLLOCK   0x46
#define SC_KP_7         0x47
#define SC_KP_8         0x48
#define SC_KP_9         0x49
#define SC_KP_MINUS     0x4A
#define SC_KP_4         0x4B
#define SC_KP_5         0x4C
#define SC_KP_6         0x4D
#define SC_KP_PLUS      0x4E
#define SC_KP_1         0x4F
#define SC_KP_2         0x50
#define SC_KP_3         0x51
#define SC_KP_0         0x52
#define SC_KP_DOT       0x53
#define SC_F11          0x57
#define SC_F12          0x58
#define SC_LWIN         0x5B
#define SC_RWIN         0x5C
#define SC_EXTENDED     0xE0

// 释放扫描码（按下码 | 0x80）
#define SC_RELEASE_MASK 0x80

#define SC_RELEASE_CTRL    0x9D
#define SC_RELEASE_ALT     0xB8
#define SC_RELEASE_LSHIFT  0xAA
#define SC_RELEASE_RSHIFT  0xB6

// 扩展键释放码（需要 E0 前缀）
#define SC_EXT_UP     0x48
#define SC_EXT_DOWN   0x50
#define SC_EXT_LEFT   0x4B
#define SC_EXT_RIGHT  0x4D

#endif