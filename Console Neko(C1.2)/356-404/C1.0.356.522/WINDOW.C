#include "window.h"

#define VGA_BUFFER 0xB8000
#define COLOR 0x1F
#define WIDTH  80
#define HEIGHT 25

static int window_open = 0;

// 只保存窗口区域（0-23行），不碰第24行
static unsigned short screen_backup[WIDTH * 24];

// 端口操作
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

// 安全写像素
static void put_char(int x, int y, unsigned char c, unsigned char attr)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT - 1) {
        vga[y * WIDTH + x] = (attr << 8) | c;
    }
}

// 写字符串
static void draw_string(int x, int y, const char* str, unsigned char attr)
{
    while (*str && x < WIDTH) {
        put_char(x++, y, *str++, attr);
    }
}

// 画顶部边框
static void draw_h_line_top(int x1, int x2, int y, unsigned char attr)
{
    put_char(x1, y, 0xC9, attr);   // ╔
    for (int x = x1 + 1; x < x2; x++) {
        put_char(x, y, 0xCD, attr); // ═
    }
    put_char(x2, y, 0xBB, attr);    // ╗
}

// 画底部边框
static void draw_h_line_bottom(int x1, int x2, int y, unsigned char attr)
{
    put_char(x1, y, 0xC8, attr);   // ╚
    for (int x = x1 + 1; x < x2; x++) {
        put_char(x, y, 0xCD, attr); // ═
    }
    put_char(x2, y, 0xBC, attr);    // ╝
}

// 初始化窗口模块
void window_init(void)
{
    window_open = 0;
}

// 显示全屏窗口（不覆盖第24行）
void window_show(void)
{
    if (window_open) return;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    // 1. 只保存 0-23 行
    for (int i = 0; i < WIDTH * 24; i++) {
        screen_backup[i] = vga[i];
    }
    
    // 2. 清屏（0-23行）
    for (int y = 1; y < 23; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            put_char(x, y, ' ', COLOR);
        }
    }
    
    // 3. 画边框
    draw_h_line_top(0, WIDTH - 1, 0, COLOR);
    for (int y = 1; y < 23; y++) {
        put_char(0, y, 0xBA, COLOR);      // ║
        put_char(WIDTH - 1, y, 0xBA, COLOR); // ║
    }
    draw_h_line_bottom(0, WIDTH - 1, 23, COLOR);
    
    // 4. 标题栏
    draw_string(3, 0, " Default Window ", COLOR);
    
    // 5. 底部提示（第22行）
    draw_string(3, 23, " [!]Press ESC to back ", COLOR);
    
    window_open = 1;
}

// 关闭窗口（只恢复0-23行，第24行不受影响）
void window_close(void)
{
    if (!window_open) return;
    
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    // 只恢复 0-23 行
    for (int i = 0; i < WIDTH * 24; i++) {
        vga[i] = screen_backup[i];
    }
    
    window_open = 0;
}

// 窗口是否打开
int window_is_open(void)
{
    return window_open;
}