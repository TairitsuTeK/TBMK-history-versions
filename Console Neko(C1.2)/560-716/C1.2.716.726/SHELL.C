#include "shell.h"
#include "version.h"
#include "disk.h"
#include "fat.h"

#define VGA_BUFFER 0xB8000
#define COLOR_INFO  0x0F
#define COLOR_OK    0x0A
#define COLOR_ERR   0x0C
#define WIDTH  80
#define HEIGHT 25

extern unsigned int allocated_pages;
extern volatile unsigned int timer_ticks;

static int shell_open = 0;
static unsigned short screen_backup[WIDTH * 24];

static int input_pos = 0;
static int cursor_y = 2;
static int shift_pressed = 0;
static int caps_lock = 0;
static char input_buf[80];

static char history[5][80];
static int history_count = 0;
static int history_pos = -1;

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
    if (x >= 1 && x < WIDTH - 1 && y >= 2 && y < 23) {
        vga[y * WIDTH + x] = (attr << 8) | c;
    }
}

static void draw_shell_box(volatile unsigned short *vga)
{
    vga[80 + 0] = (COLOR_INFO << 8) | 0xC9;
    for (int i = 1; i < 79; i++) {
        vga[80 + i] = (COLOR_INFO << 8) | 0xCD;
    }
    vga[80 + 79] = (COLOR_INFO << 8) | 0xBB;

    for (int y = 2; y < 23; y++) {
        vga[y * 80 + 0] = (COLOR_INFO << 8) | 0xBA;
        vga[y * 80 + 79] = (COLOR_INFO << 8) | 0xBA;
    }

    vga[23 * 80 + 0] = (COLOR_INFO << 8) | 0xC8;
    for (int i = 1; i < 79; i++) {
        vga[23 * 80 + i] = (COLOR_INFO << 8) | 0xCD;
    }
    vga[23 * 80 + 79] = (COLOR_INFO << 8) | 0xBC;
}

static void scroll_up(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int y = 2; y < 22; y++) {
        for (int x = 1; x < WIDTH - 1; x++) {
            vga[y * WIDTH + x] = vga[(y + 1) * WIDTH + x];
        }
    }
    for (int x = 1; x < WIDTH - 1; x++) {
        put_char(x, 22, ' ', COLOR_INFO);
    }
}

static int str_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static int str_cmp(const char* a, const char* b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a - *b;
}

static void shell_print_line_color(const char* str, unsigned char attr)
{
    if (cursor_y > 22) {
        scroll_up();
        cursor_y = 22;
    }

    int x = 1;
    while (*str && x < WIDTH - 1) {
        put_char(x, cursor_y, *str, attr);
        x++;
        str++;
    }
    cursor_y++;

    if (cursor_y > 22) {
        scroll_up();
        cursor_y = 22;
    }
}

static void shell_print_line(const char* str)
{
    shell_print_line_color(str, COLOR_INFO);
}

// ============ DIR 命令回调 ============
static int dir_callback_count = 0;

static void dir_print_callback(const file_info_t* info)
{
    char buf[80];
    int p = 0;

    int name_len = str_len(info->name);
    for (int i = 0; i < 13; i++) {
        if (i < name_len)
            buf[p++] = info->name[i];
        else
            buf[p++] = ' ';
    }
    buf[p++] = ' ';

    if (info->attr & 0x10) {
        buf[p++] = '<'; buf[p++] = 'D'; buf[p++] = 'I'; buf[p++] = 'R'; buf[p++] = '>';
        buf[p++] = ' ';
    } else {
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
        buf[p++] = ' ';
    }

    unsigned int size = info->size;
    char num[10];
    int np = 0;
    if (size == 0) num[np++] = '0';
    while (size > 0) {
        num[np++] = '0' + (size % 10);
        size /= 10;
    }
    while (np > 0) buf[p++] = num[--np];

    buf[p++] = ' ';
    buf[p++] = 'b';
    buf[p++] = 'y';
    buf[p++] = 't';
    buf[p++] = 'e';
    buf[p++] = 's';
    buf[p] = '\0';

    shell_print_line(buf);
    dir_callback_count++;
}

// ============ 前向声明 ============
static void show_prompt(void);

// ============ execute_command ============
static void execute_command(const char* cmd)
{
    if (str_len(cmd) == 0) {
        return;
    }

    if (str_cmp(cmd, "help") == 0 || str_cmp(cmd, "HELP") == 0) {
        shell_print_line("[!]" KERNEL_NAME " Shell Commands:");
        shell_print_line("  ABT      -    About mirco kernel.");
        shell_print_line("  CLS      -    Clear screen. (or press [Ctrl+L])");
        shell_print_line("  CPU      -    Show CPU information.");
        shell_print_line("  CRASH    -    Crash this kernel.");
        shell_print_line("  DATE     -    Show build date.");
        shell_print_line("  DIR      -    List files on disk.");
        shell_print_line("  ECHO     -    Echo text.");
        shell_print_line("  HELP     -    Show this help.");
        shell_print_line("  LOAD     -    Load file to memory (0x200000).");
        shell_print_line("  MEM      -    Show memory info.");
        shell_print_line("  RBT      -    Reboot system.");
        shell_print_line("  SETUP    -    Install to hard disk.");
        shell_print_line("  SYSCALL  -    Test system call.");
        shell_print_line("  TYPE     -    Display file contents.");
        shell_print_line("  UPTIME   -    Show running time.");
        shell_print_line("  VER      -    Show mirco kernel version.");

    } else if (str_cmp(cmd, "cls") == 0 || str_cmp(cmd, "CLS") == 0) {
        for (int y = 2; y < 23; y++)
            for (int x = 1; x < WIDTH - 1; x++)
                put_char(x, y, ' ', COLOR_INFO);
        cursor_y = 2;
        input_pos = 3;

    } else if (str_cmp(cmd, "echo") == 0 || str_cmp(cmd, "ECHO") == 0) {
    } else if (str_len(cmd) >= 5 && (cmd[0] == 'e' || cmd[0] == 'E') &&
               (cmd[1] == 'c' || cmd[1] == 'C') &&
               (cmd[2] == 'h' || cmd[2] == 'H') &&
               (cmd[3] == 'o' || cmd[3] == 'O') && cmd[4] == ' ') {
        shell_print_line(cmd + 5);

    } else if (str_cmp(cmd, "abt") == 0 || str_cmp(cmd, "ABT") == 0) {
        shell_print_line_color("[!] " KERNEL_NAME " - A Micro kernel based on x86 structure.", COLOR_OK);
        shell_print_line_color("[!] " KERNEL_COPYRIGHT, COLOR_OK);
        shell_print_line_color("[!] Built on x86 Bare-metal Protected Mode", COLOR_OK);

    } else if (str_cmp(cmd, "cpu") == 0 || str_cmp(cmd, "CPU") == 0) {
        char *cpu_name = (char *)0x7F10;
        while (*cpu_name == ' ') cpu_name++;
        if (cpu_name[0] == '\0') cpu_name = (char *)0x7F00;
        char buf[60];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        for (int i = 0; cpu_name[i] && p < 55; i++) buf[p++] = cpu_name[i];
        buf[p] = '\0';
        shell_print_line(buf);

    } else if (str_cmp(cmd, "mem") == 0 || str_cmp(cmd, "MEM") == 0) {
        unsigned short *mem_ptr = (unsigned short *)0x7E00;
        unsigned int total = *mem_ptr;
        if (total == 0 || total > 4096) total = 128;
        unsigned int used_kb = (allocated_pages * 4096) / 1024;
        unsigned int total_kb = total * 1024;
        unsigned int percent_int = (allocated_pages * 10000) / (total * 256);

        char buf[80];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        buf[p++] = 'U'; buf[p++] = 's'; buf[p++] = 'e'; buf[p++] = 'd'; buf[p++] = ':'; buf[p++] = ' ';
        char num[10]; int np = 0;
        unsigned int n = used_kb;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
        while (np > 0) buf[p++] = num[--np];
        buf[p++] = 'K'; buf[p++] = 'B'; buf[p++] = ' '; buf[p++] = '/'; buf[p++] = ' ';
        buf[p++] = 'T'; buf[p++] = 'o'; buf[p++] = 't'; buf[p++] = 'a'; buf[p++] = 'l'; buf[p++] = ':'; buf[p++] = ' ';
        np = 0; n = total_kb;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
        while (np > 0) buf[p++] = num[--np];
        buf[p++] = 'K'; buf[p++] = 'B'; buf[p++] = ' '; buf[p++] = '(';
        n = percent_int / 100;
        if (n == 0) { buf[p++] = '0'; }
        else {
            np = 0;
            while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
            while (np > 0) buf[p++] = num[--np];
        }
        buf[p++] = '.';
        buf[p++] = '0' + ((percent_int / 10) % 10);
        buf[p++] = '0' + (percent_int % 10);
        buf[p++] = '%'; buf[p++] = ')';
        buf[p] = '\0';
        shell_print_line(buf);

    } else if (str_cmp(cmd, "ver") == 0 || str_cmp(cmd, "VER") == 0) {
        shell_print_line("[!] Ver. " KERNEL_VERSION);

    } else if (str_cmp(cmd, "date") == 0 || str_cmp(cmd, "DATE") == 0) {
        shell_print_line("[!] Built: " __DATE__ " " __TIME__);

    } else if (str_cmp(cmd, "syscall") == 0 || str_cmp(cmd, "SYSCALL") == 0) {
        for (int y = 2; y < 23; y++)
            for (int x = 1; x < WIDTH - 1; x++)
                put_char(x, y, ' ', COLOR_INFO);
        cursor_y = 2;
        input_pos = 3;
        const char* msg = "Hello from syscall!";
        __asm__ volatile("int $0x80" : : "a"(1), "b"(msg));
        cursor_y = 3;
        show_prompt();

    } else if (str_cmp(cmd, "rbt") == 0 || str_cmp(cmd, "RBT") == 0) {
        while (inb(0x64) & 0x02);
        outb(0x64, 0xFE);

    } else if (str_cmp(cmd, "uptime") == 0 || str_cmp(cmd, "UPTIME") == 0) {
        unsigned int seconds = timer_ticks / 100;
        char buf[30];
        char num[10];
        int np = 0;
        unsigned int n = seconds;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        buf[p++] = 'U'; buf[p++] = 'p'; buf[p++] = 't'; buf[p++] = 'i'; buf[p++] = 'm'; buf[p++] = 'e'; buf[p++] = ':'; buf[p++] = ' ';
        for (int i = np - 1; i >= 0; i--) buf[p++] = num[i];
        buf[p++] = 's';
        buf[p] = '\0';
        shell_print_line(buf);

    } else if (str_cmp(cmd, "crash") == 0 || str_cmp(cmd, "CRASH") == 0) {
        volatile int a = 1;
        volatile int b = 0;
        a = a / b;

    } else if (str_cmp(cmd, "setup") == 0 || str_cmp(cmd, "SETUP") == 0) {
        unsigned char* boot_drive = (unsigned char*)0x7F60;
        if (*boot_drive == 0x80) {
            shell_print_line_color("[X] Already running from hard disk! Install aborted.", COLOR_ERR);
        } else {
            if (!disk_detect()) {
                shell_print_line_color("[X] No hard disk detected! Install aborted.", COLOR_ERR);
                return;
            }

            shell_print_line("[!] Installing to hard disk...");
            unsigned char buf[512];
            unsigned char* boot_sector = (unsigned char*)0x7C00;
            for (int i = 0; i < 512; i++) buf[i] = boot_sector[i];

            if (disk_write(0, buf) != 0) {
                shell_print_line_color("[X] Write boot sector failed! Install aborted.", COLOR_ERR);
                return;
            }

            unsigned char* kernel = (unsigned char*)0x1000;
            int write_failed = 0;
            for (int sec = 0; sec < 50; sec++) {
                if (disk_write(1 + sec, kernel + sec * 512) != 0) {
                    write_failed = 1;
                    break;
                }
            }

            if (write_failed) {
                shell_print_line_color("[X] Write kernel failed! Install aborted.", COLOR_ERR);
            } else {
                shell_print_line_color("[!] Installation complete! Remove floppy and reboot.", COLOR_OK);
            }
        }

    // ============ FAT16 文件系统命令 ============

    } else if (str_cmp(cmd, "dir") == 0 || str_cmp(cmd, "DIR") == 0) {
        if (!disk_detect()) {
            shell_print_line_color("[X] No disk detected!", COLOR_ERR);
        } else {
            if (!g_fs.fat_loaded) {
                if (fat_init() != 0) {
                    shell_print_line_color("[X] Failed to initialize FAT filesystem!", COLOR_ERR);
                    return;
                }
            }
            shell_print_line("[!] Disk contents:");
            shell_print_line("  Name         Type   Size");
            shell_print_line("  ------------ ------ ------------");
            dir_callback_count = 0;
            fat_list_root(dir_print_callback);

            char buf[40];
            int p = 0;
            buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
            if (dir_callback_count >= 10) buf[p++] = '0' + (dir_callback_count / 10);
            buf[p++] = '0' + (dir_callback_count % 10);
            buf[p++] = ' ';
            buf[p++] = 'f';
            buf[p++] = 'i';
            buf[p++] = 'l';
            buf[p++] = 'e';
            if (dir_callback_count != 1) buf[p++] = 's';
            buf[p] = '\0';
            shell_print_line(buf);
        }

    } else if (str_cmp(cmd, "type") == 0 ||
               (str_len(cmd) >= 5 &&
                (cmd[0] == 'T' || cmd[0] == 't') &&
                (cmd[1] == 'Y' || cmd[1] == 'y') &&
                (cmd[2] == 'P' || cmd[2] == 'p') &&
                (cmd[3] == 'E' || cmd[3] == 'e') &&
                cmd[4] == ' ')) {

        const char* filename = cmd;
        if (str_len(cmd) >= 5 && cmd[4] == ' ') {
            filename = cmd + 5;
            while (*filename == ' ') filename++;
        } else {
            shell_print_line_color("[X] Usage: TYPE filename", COLOR_ERR);
            return;
        }

        if (!disk_detect()) {
            shell_print_line_color("[X] No disk detected!", COLOR_ERR);
        } else {
            if (!g_fs.fat_loaded) {
                if (fat_init() != 0) {
                    shell_print_line_color("[X] Failed to initialize FAT filesystem!", COLOR_ERR);
                    return;
                }
            }

            file_handle_t* f = fat_open(filename);
            if (!f) {
                char buf[60];
                int p = 0;
                buf[p++] = '['; buf[p++] = 'X'; buf[p++] = ']'; buf[p++] = ' ';
                char* s1 = "File not found: ";
                while (*s1) buf[p++] = *s1++;
                char* s2 = (char*)filename;
                while (*s2 && p < 55) buf[p++] = *s2++;
                buf[p] = '\0';
                shell_print_line_color(buf, COLOR_ERR);
            } else {
                unsigned char buf[80];

                while (1) {
                    int n = fat_read(f, buf, 78);
                    if (n <= 0) break;

                    buf[n] = '\0';
                    shell_print_line((char*)buf);
                }

                fat_close(f);
            }
        }

    } else if (str_cmp(cmd, "load") == 0 ||
               (str_len(cmd) >= 5 &&
                (cmd[0] == 'L' || cmd[0] == 'l') &&
                (cmd[1] == 'O' || cmd[1] == 'o') &&
                (cmd[2] == 'A' || cmd[2] == 'a') &&
                (cmd[3] == 'D' || cmd[3] == 'd') &&
                cmd[4] == ' ')) {

        const char* filename = cmd;
        if (str_len(cmd) >= 5 && cmd[4] == ' ') {
            filename = cmd + 5;
            while (*filename == ' ') filename++;
        } else {
            shell_print_line_color("[X] Usage: LOAD filename", COLOR_ERR);
            return;
        }

        if (!disk_detect()) {
            shell_print_line_color("[X] No disk detected!", COLOR_ERR);
        } else {
            if (!g_fs.fat_loaded) {
                if (fat_init() != 0) {
                    shell_print_line_color("[X] Failed to initialize FAT filesystem!", COLOR_ERR);
                    return;
                }
            }

            file_handle_t* f = fat_open(filename);
            if (!f) {
                char buf[60];
                int p = 0;
                buf[p++] = '['; buf[p++] = 'X'; buf[p++] = ']'; buf[p++] = ' ';
                char* s1 = "File not found: ";
                while (*s1) buf[p++] = *s1++;
                char* s2 = (char*)filename;
                while (*s2 && p < 55) buf[p++] = *s2++;
                buf[p] = '\0';
                shell_print_line_color(buf, COLOR_ERR);
            } else {
                unsigned char* load_addr = (unsigned char*)0x200000;
                int bytes_read = fat_read(f, load_addr, f->size);

                char buf[80];
                int p = 0;
                buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
                char* s1 = "Loaded ";
                while (*s1) buf[p++] = *s1++;

                char* s2 = (char*)filename;
                while (*s2 && p < 70) buf[p++] = *s2++;

                buf[p++] = ' ';
                buf[p++] = '(';

                char num[10];
                int np = 0;
                int n = bytes_read;
                if (n == 0) num[np++] = '0';
                while (n > 0) { num[np++] = '0' + (n % 10); n /= 10; }
                while (np > 0) buf[p++] = num[--np];

                buf[p++] = ' ';
                buf[p++] = 'b';
                buf[p++] = 'y';
                buf[p++] = 't';
                buf[p++] = 'e';
                buf[p++] = 's';
                buf[p++] = ')';
                buf[p] = '\0';

                shell_print_line_color(buf, COLOR_OK);
                fat_close(f);
            }
        }

    } else {
        shell_print_line_color("[X] Unknown command. Type HELP for list.", COLOR_ERR);
    }
}

static char scancode_to_ascii(unsigned char scancode)
{
    int upper = shift_pressed ^ caps_lock;
    switch (scancode)
    {
        case 0x02: return upper ? '!' : '1';
        case 0x03: return upper ? '@' : '2';
        case 0x04: return upper ? '#' : '3';
        case 0x05: return upper ? '$' : '4';
        case 0x06: return upper ? '%' : '5';
        case 0x07: return upper ? '^' : '6';
        case 0x08: return upper ? '&' : '7';
        case 0x09: return upper ? '*' : '8';
        case 0x0A: return upper ? '(' : '9';
        case 0x0B: return upper ? ')' : '0';
        case 0x0C: return upper ? '_' : '-';
        case 0x0D: return upper ? '+' : '=';
        case 0x10: return upper ? 'Q' : 'q';
        case 0x11: return upper ? 'W' : 'w';
        case 0x12: return upper ? 'E' : 'e';
        case 0x13: return upper ? 'R' : 'r';
        case 0x14: return upper ? 'T' : 't';
        case 0x15: return upper ? 'Y' : 'y';
        case 0x16: return upper ? 'U' : 'u';
        case 0x17: return upper ? 'I' : 'i';
        case 0x18: return upper ? 'O' : 'o';
        case 0x19: return upper ? 'P' : 'p';
        case 0x1A: return upper ? '{' : '[';
        case 0x1B: return upper ? '}' : ']';
        case 0x1E: return upper ? 'A' : 'a';
        case 0x1F: return upper ? 'S' : 's';
        case 0x20: return upper ? 'D' : 'd';
        case 0x21: return upper ? 'F' : 'f';
        case 0x22: return upper ? 'G' : 'g';
        case 0x23: return upper ? 'H' : 'h';
        case 0x24: return upper ? 'J' : 'j';
        case 0x25: return upper ? 'K' : 'k';
        case 0x26: return upper ? 'L' : 'l';
        case 0x27: return upper ? ':' : ';';
        case 0x28: return upper ? '"' : '\'';
        case 0x29: return upper ? '~' : '`';
        case 0x2B: return upper ? '|' : '\\';
        case 0x2C: return upper ? 'Z' : 'z';
        case 0x2D: return upper ? 'X' : 'x';
        case 0x2E: return upper ? 'C' : 'c';
        case 0x2F: return upper ? 'V' : 'v';
        case 0x30: return upper ? 'B' : 'b';
        case 0x31: return upper ? 'N' : 'n';
        case 0x32: return upper ? 'M' : 'm';
        case 0x33: return upper ? '<' : ',';
        case 0x34: return upper ? '>' : '.';
        case 0x35: return upper ? '?' : '/';
        case 0x39: return ' ';
        default: return 0;
    }
}

static void show_prompt(void)
{
    if (cursor_y > 22) {
        scroll_up();
        cursor_y = 22;
    }

    put_char(1, cursor_y, '>', COLOR_INFO);
    put_char(2, cursor_y, ' ', COLOR_INFO);
    input_pos = 3;
    put_char(input_pos, cursor_y, '_', COLOR_INFO);
}

void shell_input(unsigned char scancode)
{
    if (!shell_open) return;

    if(scancode == 0x2A || scancode == 0x36) { shift_pressed = 1; return; }
    if(scancode == 0xAA || scancode == 0xB6) { shift_pressed = 0; return; }
    if(scancode == 0x3A) { caps_lock = !caps_lock; return; }
    if (scancode & 0x80) return;

    if (scancode == 0x48) {
        if (history_count > 0 && history_pos < 4) {
            if (history_pos == -1) history_pos = (history_count - 1) % 5;
            else history_pos = (history_pos - 1 + 5) % 5;
            for (int i = 3; i < WIDTH - 1; i++) put_char(i, cursor_y, ' ', COLOR_INFO);
            put_char(1, cursor_y, '>', COLOR_INFO);
            put_char(2, cursor_y, ' ', COLOR_INFO);
            int h = history_pos;
            for (int i = 0; history[h][i] && i < 75; i++) put_char(3 + i, cursor_y, history[h][i], COLOR_INFO);
            input_pos = 3;
            for (int i = 0; history[h][i] && i < 75; i++) { input_buf[i] = history[h][i]; input_pos++; }
            input_buf[input_pos - 3] = '\0';
            put_char(input_pos, cursor_y, '_', COLOR_INFO);
        }
        return;
    }
    if (scancode == 0x50) {
        if (history_pos != -1) {
            history_pos = (history_pos + 1) % 5;
            if (history_pos >= history_count || history_count == 0) {
                history_pos = -1;
                for (int i = 3; i < WIDTH - 1; i++) put_char(i, cursor_y, ' ', COLOR_INFO);
                put_char(1, cursor_y, '>', COLOR_INFO);
                put_char(2, cursor_y, ' ', COLOR_INFO);
                input_buf[0] = '\0'; input_pos = 3;
                put_char(input_pos, cursor_y, '_', COLOR_INFO);
            } else {
                for (int i = 3; i < WIDTH - 1; i++) put_char(i, cursor_y, ' ', COLOR_INFO);
                put_char(1, cursor_y, '>', COLOR_INFO);
                put_char(2, cursor_y, ' ', COLOR_INFO);
                int h = history_pos;
                for (int i = 0; history[h][i] && i < 75; i++) put_char(3 + i, cursor_y, history[h][i], COLOR_INFO);
                input_pos = 3;
                for (int i = 0; history[h][i] && i < 75; i++) { input_buf[i] = history[h][i]; input_pos++; }
                input_buf[input_pos - 3] = '\0';
                put_char(input_pos, cursor_y, '_', COLOR_INFO);
            }
        }
        return;
    }

    if (scancode == 0x0F) {
        if (input_pos <= 3) return;
        input_buf[input_pos - 3] = '\0';

        const char* cmds[] = {"abt", "cls", "cpu", "crash", "date", "dir", "echo",
                              "help", "load", "mem", "rbt", "setup", "syscall",
                              "type", "uptime", "ver"};
        int match_count = 0;
        int match_idx = 0;
        int cmd_count = 16;

        for (int i = 0; i < cmd_count; i++) {
            int j;
            for (j = 0; input_buf[j] && cmds[i][j]; j++) {
                if (input_buf[j] != cmds[i][j] &&
                    input_buf[j] != cmds[i][j] - 'a' + 'A') break;
            }
            if (input_buf[j] == '\0') {
                match_count++;
                match_idx = i;
            }
        }

        if (match_count == 1) {
            for (int i = 3; i < WIDTH - 1; i++) put_char(i, cursor_y, ' ', COLOR_INFO);
            put_char(1, cursor_y, '>', COLOR_INFO);
            put_char(2, cursor_y, ' ', COLOR_INFO);
            int i;
            for (i = 0; cmds[match_idx][i]; i++) {
                put_char(3 + i, cursor_y, cmds[match_idx][i], COLOR_INFO);
                input_buf[i] = cmds[match_idx][i];
            }
            input_buf[i] = '\0';
            input_pos = 3 + i;
            put_char(input_pos, cursor_y, '_', COLOR_INFO);
        } else if (match_count > 1) {
            cursor_y++;
            if (cursor_y > 22) { scroll_up(); cursor_y = 22; }
            for (int i = 0; i < cmd_count; i++) {
                int j;
                for (j = 0; input_buf[j] && cmds[i][j]; j++) {
                    if (input_buf[j] != cmds[i][j] &&
                        input_buf[j] != cmds[i][j] - 'a' + 'A') break;
                }
                if (input_buf[j] == '\0') {
                    put_char(1, cursor_y, cmds[i][0], COLOR_INFO);
                    for (int k = 1; cmds[i][k]; k++) put_char(1 + k, cursor_y, cmds[i][k], COLOR_INFO);
                    cursor_y++;
                    if (cursor_y > 22) { scroll_up(); cursor_y = 22; }
                }
            }
            show_prompt();
        }
        return;
    }

    if (scancode == 0x0E)
    {
        if (input_pos > 3) {
            input_pos--;
            put_char(input_pos, cursor_y, ' ', COLOR_INFO);
            put_char(input_pos + 1, cursor_y, ' ', COLOR_INFO);
            put_char(input_pos, cursor_y, '_', COLOR_INFO);
            input_buf[input_pos - 3] = '\0';
        }
        return;
    }

    if (scancode == 0x1C)
    {
        put_char(input_pos, cursor_y, ' ', COLOR_INFO);
        input_buf[input_pos - 3] = '\0';

        if (input_buf[0] != '\0') {
            int h = history_count % 5;
            for (int i = 0; i < 80; i++) history[h][i] = input_buf[i];
            history_count++;
            history_pos = -1;
        }

        cursor_y++;
        if (cursor_y > 22) {
            scroll_up();
            cursor_y = 22;
        }
        input_pos = 3;

        execute_command(input_buf);

        input_buf[0] = '\0';
        input_pos = 3;
        show_prompt();
        return;
    }

    char c = scancode_to_ascii(scancode);
    if (c && input_pos < WIDTH - 3)
    {
        put_char(input_pos, cursor_y, c, COLOR_INFO);
        input_buf[input_pos - 3] = c;
        input_pos++;
        put_char(input_pos, cursor_y, '_', COLOR_INFO);
    }
}

void shell_init(void)
{
    shell_open = 0;
    input_pos = 3;
    cursor_y = 2;
    shift_pressed = 0;
    caps_lock = 0;
    input_buf[0] = '\0';
}

void shell_show(void)
{
    if (shell_open) return;
    shell_open = 1;

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 80; i < WIDTH * 24; i++) screen_backup[i - 80] = vga[i];

    for (int i = 0; i < 80; i++) {
        vga[i] = 0x7000 | ' ';
    }
    char *title = " T.B.M.K. Micro Kernel - Shell ";
    int title_len = 0;
    while (title[title_len]) title_len++;
    int title_start = (80 - title_len) / 2;
    for (int i = 0; i < title_len; i++) {
        vga[title_start + i] = 0x7000 | title[i];
    }

    draw_shell_box(vga);

    for (int y = 2; y < 23; y++)
        for (int x = 1; x < WIDTH - 1; x++)
            put_char(x, y, ' ', COLOR_INFO);

    cursor_y = 2;
    input_pos = 3;
    input_buf[0] = '\0';

    put_char(1, cursor_y, '>', COLOR_INFO);
    put_char(2, cursor_y, ' ', COLOR_INFO);
    input_pos = 3;
    put_char(input_pos, cursor_y, '_', COLOR_INFO);
}

void shell_close(void)
{
    if (!shell_open) return;

    put_char(1, cursor_y, ' ', COLOR_INFO);
    put_char(2, cursor_y, ' ', COLOR_INFO);
    put_char(3, cursor_y, ' ', COLOR_INFO);

    shell_open = 0;

    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = 80; i < WIDTH * 24; i++) vga[i] = screen_backup[i - 80];

    for (int i = 0; i < 80; i++) {
        vga[i] = 0x7000 | ' ';
    }
    char *title = " T.B.M.K. Micro Kernel - Main Page ";
    int title_len = 0;
    while (title[title_len]) title_len++;
    int title_start = (80 - title_len) / 2;
    for (int i = 0; i < title_len; i++) {
        vga[title_start + i] = 0x7000 | title[i];
    }
}

void shell_clear(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int y = 2; y < 23; y++)
        for (int x = 1; x < WIDTH - 1; x++)
            put_char(x, y, ' ', COLOR_INFO);
    cursor_y = 2;
    input_pos = 3;
    input_buf[0] = '\0';
    put_char(1, cursor_y, '>', COLOR_INFO);
    put_char(2, cursor_y, ' ', COLOR_INFO);
    put_char(3, cursor_y, '_', COLOR_INFO);
}

int shell_is_open(void)
{
    return shell_open;
}