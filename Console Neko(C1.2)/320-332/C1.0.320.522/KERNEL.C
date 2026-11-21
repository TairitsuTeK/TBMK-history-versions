#define VGA_BUFFER 0xB8000
#define COLOR 0x1F

// VGA端口
#define VGA_CRTC_ADDR 0x3D4
#define VGA_CRTC_DATA 0x3D5

void kernel_main(void) __attribute__((noreturn));

// 端口输出字节（内联汇编）—— 必须放在最前面
static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

// 关闭光标
static void disable_cursor(void)
{
    // 选择光标开始寄存器(0x0A)
    outb(VGA_CRTC_ADDR, 0x0A);
    // 设置第5位，禁用光标
    outb(VGA_CRTC_DATA, 0x20);
}

void kernel_main(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    
    // 关闭光标
    disable_cursor();
    
    // 清屏
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = (COLOR << 8) | ' ';
    }
    
    // 从左上角开始输出第一行
    int pos = 0;
    const char *str1 = "[!]Micro Kernel Started Successfully!";
    while (*str1) {
        vga[pos++] = (COLOR << 8) | *str1++;
    }
    
    // 第二行（位置80，即第二行开头）
    pos = 80;
    const char *str2 = "[!]Running at x86 Bare-metal Protected Mode......(OK)";
    while (*str2) {
        vga[pos++] = (COLOR << 8) | *str2++;
    }

    pos = 160;
    const char *str3 = "[!]T.B.M.K. (C) Tairitsu_tty 2026 [Version C1.0.320.522 Beta]";
    while (*str3) {
        vga[pos++] = (COLOR << 8) | *str3++;
    }

    pos = 240;
    const char *str4 = "";
    while (*str4) {
        vga[pos++] = (COLOR << 8) | *str4++;
    }

    pos = 320;
    const char *str5 = "[!]Welcome text:";
    while (*str5) {
        vga[pos++] = (COLOR << 8) | *str5++;
    }

    pos = 400;
    const char *str6 = "   +------------------------+";
    while (*str6) {
        vga[pos++] = (COLOR << 8) | *str6++;
    }

    pos = 480;
    const char *str7 = "   | print(\"Hello, World!\") |";
    while (*str7) {
        vga[pos++] = (COLOR << 8) | *str7++;
    }

    pos = 560;
    const char *str8 = "   +------------------------+";
    while (*str8) {
        vga[pos++] = (COLOR << 8) | *str8++;
    }
    
    // 死循环
    while (1) {
        __asm__ volatile("hlt");
    }
}