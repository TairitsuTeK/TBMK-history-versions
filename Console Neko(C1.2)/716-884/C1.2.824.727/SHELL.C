#include "shell.h"
#include "version.h"
#include "disk.h"
#include "fat.h"
#include "syscall.h"
#include "port.h"
#include "vga.h"
#include "scancode.h"

#define SHELL_BOX_LEFT   1
#define SHELL_BOX_RIGHT  (VGA_WIDTH - 1)
#define SHELL_BOX_TOP    2
#define SHELL_BOX_BOTTOM 23
#define SHELL_INNER_W    (VGA_WIDTH - 2)

extern unsigned int allocated_pages;
extern unsigned int total_pages;
extern volatile unsigned int timer_ticks;

unsigned int alloc_page(void);
void free_page(unsigned int addr);
unsigned int alloc_pages(unsigned int count);

static int shell_open = 0;
static unsigned short screen_backup[VGA_WIDTH * 24];

static int input_pos = 0;
static int cmd_row = SHELL_BOX_TOP;
static int shift_pressed = 0;
static int caps_lock = 0;
static char input_buf[80];

static char history[5][80];
static int history_count = 0;
static int history_pos = -1;

static void draw_shell_box(void)
{
    vga_put_char(0, 1, BOX_TL, COLOR_WHITE_ON_BLACK);
    for (int i = 1; i < 79; i++) vga_put_char(i, 1, BOX_HZ, COLOR_WHITE_ON_BLACK);
    vga_put_char(79, 1, BOX_TR, COLOR_WHITE_ON_BLACK);
    for (int y = 2; y < 23; y++) {
        vga_put_char(0, y, BOX_VT, COLOR_WHITE_ON_BLACK);
        vga_put_char(79, y, BOX_VT, COLOR_WHITE_ON_BLACK);
    }
    vga_put_char(0, 23, BOX_BL, COLOR_WHITE_ON_BLACK);
    for (int i = 1; i < 79; i++) vga_put_char(i, 23, BOX_HZ, COLOR_WHITE_ON_BLACK);
    vga_put_char(79, 23, BOX_BR, COLOR_WHITE_ON_BLACK);
}

static void scroll_up(void)
{
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int y = SHELL_BOX_TOP; y < SHELL_BOX_BOTTOM - 1; y++)
        for (int x = SHELL_BOX_LEFT; x < SHELL_BOX_RIGHT; x++)
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
    for (int x = SHELL_BOX_LEFT; x < SHELL_BOX_RIGHT; x++)
        vga_put_char(x, SHELL_BOX_BOTTOM - 1, ' ', COLOR_WHITE_ON_BLACK);
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

static void shell_print_color(const char* str, unsigned char attr)
{
    if (cmd_row >= SHELL_BOX_BOTTOM) {
        cmd_row = SHELL_BOX_BOTTOM - 1;
    }
    if (cmd_row >= SHELL_BOX_BOTTOM - 1) {
        scroll_up();
        cmd_row = SHELL_BOX_BOTTOM - 2;
    }
    int x = SHELL_BOX_LEFT;
    while (*str && x < SHELL_BOX_RIGHT)
        vga_put_char(x++, cmd_row, *str++, attr);
    cmd_row++;
    if (cmd_row > SHELL_BOX_BOTTOM) {
        cmd_row = SHELL_BOX_BOTTOM;
    }
}

static void shell_print(const char* str) { shell_print_color(str, COLOR_WHITE_ON_BLACK); }
static void shell_ok(const char* str)    { shell_print_color(str, COLOR_GREEN_ON_BLACK); }
static void shell_err(const char* str)   { shell_print_color(str, COLOR_RED_ON_BLACK); }

static void show_prompt(void)
{
    if (cmd_row >= SHELL_BOX_BOTTOM - 1) {
        scroll_up();
        cmd_row = SHELL_BOX_BOTTOM - 2;
    }
    vga_put_char(1, cmd_row, '>', COLOR_WHITE_ON_BLACK);
    vga_put_char(2, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
    input_pos = 3;
    vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
}

static void execute_command(const char* cmd)
{
    if (str_len(cmd) == 0) return;

    if (str_cmp(cmd, "help") == 0 || str_cmp(cmd, "HELP") == 0) {
        shell_print("[!]" KERNEL_NAME " Shell Commands:");
        shell_print("  ABT      -    About micro kernel.");
        shell_print("  CAT      -    Display file contents.");
        shell_print("  CLS      -    Clear screen.");
        shell_print("  CPU      -    Show CPU information.");
        shell_print("  CRASH    -    Crash this kernel.");
        shell_print("  DATE     -    Show build date.");
        shell_print("  DIR      -    List files on disk.");
        shell_print("  ECHO     -    Echo text.");
        shell_print("  HELP     -    Show this help.");
        shell_print("  MEM      -    Show memory info.");
        shell_print("  RBT      -    Reboot system.");
        shell_print("  RUN      -    Run program (RUN filename).");
        shell_print("  SETUP    -    Install to hard disk.");
        shell_print("  SYSCALL  -    Test system call.");
        shell_print("  UPTIME   -    Show running time.");
        shell_print("  VER      -    Show version.");
    }
    else if (str_cmp(cmd, "cls") == 0 || str_cmp(cmd, "CLS") == 0) {
        shell_clear();
    }
    else if (str_cmp(cmd, "abt") == 0 || str_cmp(cmd, "ABT") == 0) {
        shell_ok("[!] " KERNEL_NAME " - A Micro kernel based on x86 structure.");
        shell_ok("[!] " KERNEL_COPYRIGHT);
        shell_ok("[!] Built on x86 Bare-metal Protected Mode");
    }
    else if (str_cmp(cmd, "cpu") == 0 || str_cmp(cmd, "CPU") == 0) {
        char *cpu_name = (char *)0x7F10;
        while (*cpu_name == ' ') cpu_name++;
        if (cpu_name[0] == '\0') cpu_name = (char *)0x7F00;
        char buf[60];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        for (int i = 0; cpu_name[i] && p < 55; i++) buf[p++] = cpu_name[i];
        buf[p] = '\0';
        shell_print(buf);
    }
    else if (str_cmp(cmd, "mem") == 0 || str_cmp(cmd, "MEM") == 0) {
        unsigned int total_mb = total_pages * 4096 / 1048576;
        unsigned int used_kb = (allocated_pages * 4096) / 1024;
        unsigned int free_kb = (total_pages * 4096) / 1024 - used_kb;
        char buf[80];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        char* s = "Total: "; while (*s) buf[p++] = *s++;
        char num[10];
        int np = 0;
        unsigned int n = total_mb;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + n % 10; n /= 10; }
        while (np > 0) buf[p++] = num[--np];
        buf[p++] = 'M'; buf[p++] = 'B'; buf[p++] = ' '; buf[p++] = '|'; buf[p++] = ' ';
        s = "Used: "; while (*s) buf[p++] = *s++;
        np = 0; n = used_kb;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + n % 10; n /= 10; }
        while (np > 0) buf[p++] = num[--np];
        buf[p++] = 'K'; buf[p++] = 'B'; buf[p++] = ' '; buf[p++] = '|'; buf[p++] = ' ';
        s = "Free: "; while (*s) buf[p++] = *s++;
        np = 0; n = free_kb;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + n % 10; n /= 10; }
        while (np > 0) buf[p++] = num[--np];
        buf[p++] = 'K'; buf[p++] = 'B';
        buf[p] = '\0';
        shell_print(buf);
    }
    else if (str_cmp(cmd, "ver") == 0 || str_cmp(cmd, "VER") == 0) {
        shell_print("[!] Ver. " KERNEL_VERSION);
    }
    else if (str_cmp(cmd, "date") == 0 || str_cmp(cmd, "DATE") == 0) {
        shell_print("[!] Built: " __DATE__ " " __TIME__);
    }
    else if (str_cmp(cmd, "syscall") == 0 || str_cmp(cmd, "SYSCALL") == 0) {
        shell_clear();
        __asm__ volatile("int $0x80" : : "a"(1), "b"("Hello from syscall!"));
        cmd_row = SHELL_BOX_TOP;
        show_prompt();
    }
    else if (str_cmp(cmd, "rbt") == 0 || str_cmp(cmd, "RBT") == 0) {
        while (inb(0x64) & 0x02);
        outb(0x64, 0xFE);
    }
    else if (str_cmp(cmd, "uptime") == 0 || str_cmp(cmd, "UPTIME") == 0) {
        unsigned int seconds = timer_ticks / 100;
        char buf[30];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        char* s = "Uptime: "; while (*s) buf[p++] = *s++;
        char num[10];
        int np = 0;
        unsigned int n = seconds;
        if (n == 0) num[np++] = '0';
        while (n > 0) { num[np++] = '0' + n % 10; n /= 10; }
        for (int i = np - 1; i >= 0; i--) buf[p++] = num[i];
        buf[p++] = 's';
        buf[p] = '\0';
        shell_print(buf);
    }
    else if (str_cmp(cmd, "crash") == 0 || str_cmp(cmd, "CRASH") == 0) {
        volatile int a = 1, b = 0;
        a = a / b;
    }
    else if (str_cmp(cmd, "setup") == 0 || str_cmp(cmd, "SETUP") == 0) {
        unsigned char* boot_drive = (unsigned char*)0x7F60;
        if (*boot_drive == 0x80) {
            shell_err("[X] Already running from hard disk!");
            return;
        }
        if (!disk_detect()) {
            shell_err("[X] No hard disk detected!");
            return;
        }
        shell_print("[!] Installing...");
        unsigned char buf[512];
        unsigned char* boot_sector = (unsigned char*)0x7C00;
        for (int i = 0; i < 512; i++) buf[i] = boot_sector[i];
        if (disk_write(0, buf) != 0) {
            shell_err("[X] Write boot sector failed!");
            return;
        }
        unsigned char* kernel = (unsigned char*)0x1000;
        for (int sec = 0; sec < 50; sec++)
            if (disk_write(1 + sec, kernel + sec * 512) != 0) {
                shell_err("[X] Write kernel failed!");
                return;
            }
        shell_ok("[!] Installation complete!");
    }
    else if (str_cmp(cmd, "dir") == 0 || str_cmp(cmd, "DIR") == 0) {
        unsigned int root_start = 1 + 2 * 16;  // reserved + num_fats * sectors_per_fat
        unsigned char root_buf[512];

        shell_print("[!] Disk contents:");
        shell_print("  Name         Type   Size");
        shell_print("  ------------ ------ ------------");

        int file_count = 0;
        for (unsigned int i = 0; i < 4; i++) {
            if (disk_read(root_start + i, root_buf) != 0) break;
            for (int j = 0; j < 16; j++) {
                unsigned char* entry = &root_buf[j * 32];
                if (entry[0] == 0x00) goto dir_done;
                if (entry[0] == 0xE5) continue;
                if (entry[11] == 0x0F) continue;
                if (entry[11] & 0x08) continue;

                char name[13];
                int p = 0;
                for (int k = 0; k < 8 && entry[k] != ' ' && entry[k] != 0; k++)
                    name[p++] = entry[k];
                if (entry[8] != ' ' && entry[8] != 0) {
                    name[p++] = '.';
                    for (int k = 8; k < 11 && entry[k] != ' ' && entry[k] != 0; k++)
                        name[p++] = entry[k];
                }
                name[p] = '\0';

                char line[80];
                int lp = 0;
                for (int k = 0; k < 13; k++)
                    line[lp++] = (k < p) ? name[k] : ' ';
                line[lp++] = ' ';
                if (entry[11] & 0x10) {
                    line[lp++]='<'; line[lp++]='D'; line[lp++]='I'; line[lp++]='R'; line[lp++]='>'; line[lp++]=' ';
                } else {
                    for (int k = 0; k < 6; k++) line[lp++] = ' ';
                }
                unsigned int size = *(unsigned int*)&entry[28];
                char num[10];
                int np = 0;
                if (size == 0) num[np++] = '0';
                while (size > 0) { num[np++] = '0' + (size % 10); size /= 10; }
                while (np > 0) line[lp++] = num[--np];
                line[lp++] = ' '; line[lp++] = 'b'; line[lp++] = 'y'; line[lp++] = 't'; line[lp++] = 'e'; line[lp++] = 's';
                line[lp] = '\0';
                shell_print(line);
                file_count++;
            }
        }
dir_done:
        char buf[30];
        int bp = 0;
        buf[bp++] = '['; buf[bp++] = '!'; buf[bp++] = ']'; buf[bp++] = ' ';
        if (file_count >= 10) buf[bp++] = '0' + file_count / 10;
        buf[bp++] = '0' + file_count % 10;
        buf[bp++] = ' '; buf[bp++] = 'f'; buf[bp++] = 'i'; buf[bp++] = 'l'; buf[bp++] = 'e';
        if (file_count != 1) buf[bp++] = 's';
        buf[bp] = '\0';
        shell_print(buf);
    }
    else if (str_cmp(cmd, "run") == 0 || str_cmp(cmd, "RUN") == 0) {
        shell_err("[X] Usage: RUN <filename>");
    }
    else if (str_len(cmd) >= 4 &&
             (cmd[0] == 'R' || cmd[0] == 'r') && (cmd[1] == 'U' || cmd[1] == 'u') &&
             (cmd[2] == 'N' || cmd[2] == 'n') && cmd[3] == ' ') {
        const char* fn = cmd + 4;
        while (*fn == ' ') fn++;
        if (!*fn) {
            shell_err("[X] Usage: RUN <filename>");
            return;
        }

        // 直接读根目录找文件
        unsigned int root_start = 1 + 2 * 16;
        unsigned char root_buf[512];
        unsigned int file_cluster = 0;
        unsigned int file_size = 0;
        int found = 0;

        char search_name[12];
        int sp = 0;
        for (int i = 0; fn[i] && sp < 11; i++) {
            if (fn[i] == '.') {
                while (sp < 8) search_name[sp++] = ' ';
                continue;
            }
            if (fn[i] >= 'a' && fn[i] <= 'z')
                search_name[sp++] = fn[i] - 'a' + 'A';
            else
                search_name[sp++] = fn[i];
        }
        while (sp < 11) search_name[sp++] = ' ';

        for (unsigned int i = 0; i < 4; i++) {
            if (disk_read(root_start + i, root_buf) != 0) break;
            for (int j = 0; j < 16; j++) {
                unsigned char* entry = &root_buf[j * 32];
                if (entry[0] == 0x00) goto search_done;
                if (entry[0] == 0xE5) continue;
                if (entry[11] == 0x0F) continue;
                if (entry[11] & 0x08) continue;

                int match = 1;
                for (int k = 0; k < 11; k++) {
                    if (entry[k] != search_name[k]) {
                        match = 0;
                        break;
                    }
                }
                if (match) {
                    file_cluster = *(unsigned short*)&entry[26];
                    file_size = *(unsigned int*)&entry[28];
                    found = 1;
                    goto search_done;
                }
            }
        }
search_done:
        if (!found) {
            shell_err("[X] File not found!");
            return;
        }

        unsigned int needed_pages = (file_size + 4095) / 4096;
        if (needed_pages < 1) needed_pages = 1;
        unsigned int load_addr = alloc_pages(needed_pages);
        if (!load_addr) {
            shell_err("[X] Out of memory!");
            return;
        }

        // 读文件数据
        unsigned char* load_ptr = (unsigned char*)load_addr;
        unsigned int cluster = file_cluster;
        unsigned int bytes_read = 0;

        while (bytes_read < file_size) {
            unsigned int sector = 33 + (cluster - 2) * 8;  // first_data_sector
            unsigned char sector_buf[512];
            for (int s = 0; s < 8 && bytes_read < file_size; s++) {
                if (disk_read(sector + s, sector_buf) != 0) goto read_done;
                unsigned int to_copy = 512;
                if (bytes_read + to_copy > file_size) to_copy = file_size - bytes_read;
                for (unsigned int k = 0; k < to_copy; k++)
                    load_ptr[bytes_read + k] = sector_buf[k];
                bytes_read += to_copy;
            }
            // 读 FAT 找下一个簇
            unsigned int fat_sector = 1 + cluster / 256;
            unsigned char fat_buf[512];
            disk_read(fat_sector, fat_buf);
            unsigned short* fat16 = (unsigned short*)fat_buf;
            unsigned short next = fat16[cluster];
            if (next >= 0xFFF8) break;
            cluster = next;
        }
read_done:
        char buf[80];
        int p = 0;
        buf[p++] = '['; buf[p++] = '!'; buf[p++] = ']'; buf[p++] = ' ';
        char* s = "Running "; while (*s) buf[p++] = *s++;
        while (*fn && p < 60) buf[p++] = *fn++;
        buf[p++] = ' '; buf[p++] = '@'; buf[p++] = ' '; buf[p++] = '0'; buf[p++] = 'x';
        char hex[] = "0123456789ABCDEF";
        for (int i = 28; i >= 0; i -= 4) buf[p++] = hex[(load_addr >> i) & 0xF];
        buf[p] = '\0';
        shell_ok(buf);

        void (*user_func)() = (void (*)())load_addr;
        user_func();
        shell_print("[!] Program returned.");
        for (unsigned int i = 0; i < needed_pages; i++) free_page(load_addr + i * 4096);
    }
    else if (str_len(cmd) >= 4 &&
             (cmd[0] == 'C' || cmd[0] == 'c') && (cmd[1] == 'A' || cmd[1] == 'a') &&
             (cmd[2] == 'T' || cmd[2] == 't') && cmd[3] == ' ') {
        shell_err("[X] CAT command temporarily disabled.");
    }
    else if (str_len(cmd) >= 5 &&
             (cmd[0] == 'E' || cmd[0] == 'e') && (cmd[1] == 'C' || cmd[1] == 'c') &&
             (cmd[2] == 'H' || cmd[2] == 'h') && (cmd[3] == 'O' || cmd[3] == 'o') && cmd[4] == ' ') {
        shell_print(cmd + 5);
    }
    else {
        shell_err("[X] Unknown command. Type HELP for list.");
    }
}

static char scancode_to_ascii(unsigned char scancode)
{
    int upper = shift_pressed ^ caps_lock;
    switch (scancode) {
        case SC_1: return upper ? '!' : '1';
        case SC_2: return upper ? '@' : '2';
        case SC_3: return upper ? '#' : '3';
        case SC_4: return upper ? '$' : '4';
        case SC_5: return upper ? '%' : '5';
        case SC_6: return upper ? '^' : '6';
        case SC_7: return upper ? '&' : '7';
        case SC_8: return upper ? '*' : '8';
        case SC_9: return upper ? '(' : '9';
        case SC_0: return upper ? ')' : '0';
        case SC_MINUS: return upper ? '_' : '-';
        case SC_EQUAL: return upper ? '+' : '=';
        case SC_Q: return upper ? 'Q' : 'q';
        case SC_W: return upper ? 'W' : 'w';
        case SC_E: return upper ? 'E' : 'e';
        case SC_R: return upper ? 'R' : 'r';
        case SC_T: return upper ? 'T' : 't';
        case SC_Y: return upper ? 'Y' : 'y';
        case SC_U: return upper ? 'U' : 'u';
        case SC_I: return upper ? 'I' : 'i';
        case SC_O: return upper ? 'O' : 'o';
        case SC_P: return upper ? 'P' : 'p';
        case SC_A: return upper ? 'A' : 'a';
        case SC_S: return upper ? 'S' : 's';
        case SC_D: return upper ? 'D' : 'd';
        case SC_F: return upper ? 'F' : 'f';
        case SC_G: return upper ? 'G' : 'g';
        case SC_H: return upper ? 'H' : 'h';
        case SC_J: return upper ? 'J' : 'j';
        case SC_K: return upper ? 'K' : 'k';
        case SC_L: return upper ? 'L' : 'l';
        case SC_Z: return upper ? 'Z' : 'z';
        case SC_X: return upper ? 'X' : 'x';
        case SC_C: return upper ? 'C' : 'c';
        case SC_V: return upper ? 'V' : 'v';
        case SC_B: return upper ? 'B' : 'b';
        case SC_N: return upper ? 'N' : 'n';
        case SC_M: return upper ? 'M' : 'm';
        case SC_LBRACKET: return upper ? '{' : '[';
        case SC_RBRACKET: return upper ? '}' : ']';
        case SC_SEMICOLON: return upper ? ':' : ';';
        case SC_QUOTE: return upper ? '"' : '\'';
        case SC_BACKTICK: return upper ? '~' : '`';
        case SC_BACKSLASH: return upper ? '|' : '\\';
        case SC_COMMA: return upper ? '<' : ',';
        case SC_DOT: return upper ? '>' : '.';
        case SC_SLASH: return upper ? '?' : '/';
        case SC_SPACE: return ' ';
        default: return 0;
    }
}

void shell_input(unsigned char scancode)
{
    if (!shell_open) return;

    if (scancode == SC_LSHIFT || scancode == SC_RSHIFT) {
        shift_pressed = 1;
        return;
    }
    if (scancode == SC_RELEASE_LSHIFT || scancode == SC_RELEASE_RSHIFT) {
        shift_pressed = 0;
        return;
    }
    if (scancode == SC_CAPSLOCK) {
        caps_lock = !caps_lock;
        return;
    }
    if (scancode & SC_RELEASE_MASK) return;

    if (scancode == SC_KP_8) {
        if (history_count > 0 && history_pos < 4) {
            if (history_pos == -1) history_pos = (history_count - 1) % 5;
            else history_pos = (history_pos - 1 + 5) % 5;
            for (int i = 3; i < SHELL_INNER_W - 1; i++)
                vga_put_char(i, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
            int h = history_pos;
            for (int i = 0; history[h][i] && i < 75; i++)
                vga_put_char(3 + i, cmd_row, history[h][i], COLOR_WHITE_ON_BLACK);
            input_pos = 3;
            for (int i = 0; history[h][i] && i < 75; i++) {
                input_buf[i] = history[h][i];
                input_pos++;
            }
            input_buf[input_pos - 3] = '\0';
            vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
        }
        return;
    }

    if (scancode == SC_KP_2) {
        if (history_pos != -1) {
            history_pos = (history_pos + 1) % 5;
            if (history_pos >= history_count || history_count == 0) {
                history_pos = -1;
                for (int i = 3; i < SHELL_INNER_W - 1; i++)
                    vga_put_char(i, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
                input_buf[0] = '\0';
                input_pos = 3;
                vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
            } else {
                for (int i = 3; i < SHELL_INNER_W - 1; i++)
                    vga_put_char(i, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
                int h = history_pos;
                for (int i = 0; history[h][i] && i < 75; i++)
                    vga_put_char(3 + i, cmd_row, history[h][i], COLOR_WHITE_ON_BLACK);
                input_pos = 3;
                for (int i = 0; history[h][i] && i < 75; i++) {
                    input_buf[i] = history[h][i];
                    input_pos++;
                }
                input_buf[input_pos - 3] = '\0';
                vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
            }
        }
        return;
    }

    if (scancode == SC_TAB) {
        if (input_pos <= 3) return;
        input_buf[input_pos - 3] = '\0';
        const char* cmds[] = {
            "abt", "cat", "cls", "cpu", "crash", "date", "dir", "echo",
            "help", "mem", "rbt", "run", "setup", "syscall", "uptime", "ver"
        };
        int match_count = 0, match_idx = 0, cmd_count = 16;
        for (int i = 0; i < cmd_count; i++) {
            int j;
            for (j = 0; input_buf[j] && cmds[i][j]; j++)
                if (input_buf[j] != cmds[i][j] && input_buf[j] != cmds[i][j] - 'a' + 'A') break;
            if (input_buf[j] == '\0') { match_count++; match_idx = i; }
        }
        if (match_count == 1) {
            for (int i = 3; i < SHELL_INNER_W - 1; i++)
                vga_put_char(i, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
            int i;
            for (i = 0; cmds[match_idx][i]; i++) {
                vga_put_char(3 + i, cmd_row, cmds[match_idx][i], COLOR_WHITE_ON_BLACK);
                input_buf[i] = cmds[match_idx][i];
            }
            input_buf[i] = '\0';
            input_pos = 3 + i;
            vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
        } else if (match_count > 1) {
            cmd_row++;
            if (cmd_row >= SHELL_BOX_BOTTOM - 1) {
                scroll_up();
                cmd_row = SHELL_BOX_BOTTOM - 2;
            }
            for (int i = 0; i < cmd_count; i++) {
                int j;
                for (j = 0; input_buf[j] && cmds[i][j]; j++)
                    if (input_buf[j] != cmds[i][j] && input_buf[j] != cmds[i][j] - 'a' + 'A') break;
                if (input_buf[j] == '\0') {
                    for (int k = 0; cmds[i][k]; k++)
                        vga_put_char(1 + k, cmd_row, cmds[i][k], COLOR_WHITE_ON_BLACK);
                    cmd_row++;
                    if (cmd_row >= SHELL_BOX_BOTTOM - 1) {
                        scroll_up();
                        cmd_row = SHELL_BOX_BOTTOM - 2;
                    }
                }
            }
            show_prompt();
        }
        return;
    }

    if (scancode == SC_BACKSPACE) {
        if (input_pos > 3) {
            vga_put_char(input_pos, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
            input_pos--;
            vga_put_char(input_pos, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
            vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
            input_buf[input_pos - 3] = '\0';
        }
        return;
    }

    if (scancode == SC_ENTER) {
        vga_put_char(input_pos, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
        input_buf[input_pos - 3] = '\0';
        if (input_buf[0] != '\0') {
            int h = history_count % 5;
            for (int i = 0; i < 80; i++) history[h][i] = input_buf[i];
            history_count++; history_pos = -1;
        }

        cmd_row++;

        execute_command(input_buf);

        input_buf[0] = '\0'; input_pos = 3;
        show_prompt();
        return;
    }

    char c = scancode_to_ascii(scancode);
    if (c && input_pos < SHELL_INNER_W - 2) {
        vga_put_char(input_pos, cmd_row, c, COLOR_WHITE_ON_BLACK);
        input_buf[input_pos - 3] = c;
        input_pos++;
        vga_put_char(input_pos, cmd_row, '_', COLOR_WHITE_ON_BLACK);
    }
}

void shell_init(void)
{
    shell_open = 0;
    input_pos = 3;
    cmd_row = SHELL_BOX_TOP;
    shift_pressed = 0;
    caps_lock = 0;
    input_buf[0] = '\0';
    history_count = 0;
    history_pos = -1;
}

void shell_show(void)
{
    if (shell_open) return;
    shell_open = 1;
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = VGA_WIDTH; i < VGA_WIDTH * 24; i++)
        screen_backup[i - VGA_WIDTH] = vga[i];
    for (int i = 0; i < VGA_WIDTH; i++)
        vga_put_char(i, 0, ' ', COLOR_BLACK_ON_WHITE);
    char *title = " T.B.M.K. Micro Kernel - Shell ";
    int title_len = 0;
    while (title[title_len]) title_len++;
    int title_start = (VGA_WIDTH - title_len) / 2;
    for (int i = 0; i < title_len; i++)
        vga_put_char(title_start + i, 0, title[i], COLOR_BLACK_ON_WHITE);
    draw_shell_box();
    for (int y = SHELL_BOX_TOP; y < SHELL_BOX_BOTTOM; y++)
        for (int x = SHELL_BOX_LEFT; x < SHELL_BOX_RIGHT; x++)
            vga_put_char(x, y, ' ', COLOR_WHITE_ON_BLACK);
    cmd_row = SHELL_BOX_TOP;
    input_pos = 3;
    input_buf[0] = '\0';
    show_prompt();
}

void shell_close(void)
{
    if (!shell_open) return;
    vga_put_char(1, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
    vga_put_char(2, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
    vga_put_char(3, cmd_row, ' ', COLOR_WHITE_ON_BLACK);
    shell_open = 0;
    volatile unsigned short *vga = (volatile unsigned short *)VGA_BUFFER;
    for (int i = VGA_WIDTH; i < VGA_WIDTH * 24; i++)
        vga[i] = screen_backup[i - VGA_WIDTH];
    for (int i = 0; i < VGA_WIDTH; i++)
        vga_put_char(i, 0, ' ', COLOR_BLACK_ON_WHITE);
    char *title = " T.B.M.K. Micro Kernel - Main Page ";
    int title_len = 0;
    while (title[title_len]) title_len++;
    int title_start = (VGA_WIDTH - title_len) / 2;
    for (int i = 0; i < title_len; i++)
        vga_put_char(title_start + i, 0, title[i], COLOR_BLACK_ON_WHITE);
}

void shell_clear(void)
{
    for (int y = SHELL_BOX_TOP; y < SHELL_BOX_BOTTOM; y++)
        for (int x = SHELL_BOX_LEFT; x < SHELL_BOX_RIGHT; x++)
            vga_put_char(x, y, ' ', COLOR_WHITE_ON_BLACK);
    cmd_row = SHELL_BOX_TOP;
    input_pos = 3;
    input_buf[0] = '\0';
    show_prompt();
}

int shell_is_open(void)
{
    return shell_open;
}