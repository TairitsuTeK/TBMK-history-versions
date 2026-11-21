#include "filemanager.h"
#include "font.h"
#include "fat.h"
#include "disk.h"

static file_info_t files[64];
static int file_count = 0;
static int selected = -1;
static int scroll_offset = 0;
static int no_disk = 0;  // 0=正常, 1=无硬盘
static int show_error = 0;  // 是否显示错误弹窗
static int error_timer = 0;  // 错误弹窗计时器

static void file_callback(const file_info_t* info)
{
    if (file_count < 64) {
        files[file_count] = *info;
        file_count++;
    }
}

static void load_files(void)
{
    file_count = 0;
    selected = -1;
    scroll_offset = 0;
    no_disk = 0;
    
    // 检测硬盘
    if (!disk_detect()) {
        no_disk = 1;
        show_error = 1;
        error_timer = 300;  // 显示约5秒
        return;
    }
    
    // 初始化FAT
    if (!g_fs.fat_loaded) {
        if (fat_init() != 0) {
            no_disk = 1;
            show_error = 1;
            error_timer = 300;
            return;
        }
    }
    
    fat_list_root(file_callback);
}

void fm_init(void)
{
    load_files();
}

// 绘制错误弹窗
static void draw_error_dialog(unsigned char* v, int wx, int wy, int ww, int wh)
{
    // 弹窗背景
    int dx = wx + ww/2 - 60;
    int dy = wy + wh/2 - 25;
    int dw = 120;
    int dh = 50;
    
    // 背景
    for (int y = dy; y < dy+dh && y < 200; y++)
        for (int x = dx; x < dx+dw && x < 320; x++)
            v[y*320+x] = 8;
    
    // 边框
    for (int x = dx; x < dx+dw; x++) {
        v[dy*320+x] = 15;
        v[(dy+dh-1)*320+x] = 0;
    }
    for (int y = dy; y < dy+dh; y++) {
        v[y*320+dx] = 15;
        v[y*320+dx+dw-1] = 0;
    }
    
    // 错误图标
    for (int y = dy+10; y < dy+25; y++)
        for (int x = dx+10; x < dx+25; x++)
            v[y*320+x] = 4;  // 红色
    
    // 白色X
    font_draw_text(v, dx+15, dy+14, "X", 15);
    
    // 错误文字
    font_draw_text(v, dx+30, dy+12, "No Disk!", 15);
    font_draw_text(v, dx+30, dy+24, "No hard disk", 0);
    font_draw_text(v, dx+30, dy+34, "detected!", 0);
}

void fm_draw(unsigned char* v, int wx, int wy, int ww, int wh)
{
    // 标题
    font_draw_text(v, wx+5, wy+20, "File Manager", 0);
    
    // 列标题
    font_draw_text(v, wx+8, wy+32, "Name", 0);
    font_draw_text(v, wx+ww-45, wy+32, "Size", 0);
    
    // 分隔线
    for (int x = wx+4; x < wx+ww-4; x++) {
        if (wy+40 < 200 && x < 320) v[(wy+40)*320+x] = 8;
    }
    
    // 文件列表
    int max_display = (wh - 50) / 10;
    if (max_display > 10) max_display = 10;
    
    for (int i = 0; i < max_display && (i + scroll_offset) < file_count; i++) {
        int idx = i + scroll_offset;
        int y = wy + 42 + i * 10;
        
        if (idx == selected) {
            for (int yy = y; yy < y+9 && yy < 200; yy++)
                for (int xx = wx+4; xx < wx+ww-4 && xx < 320; xx++)
                    v[yy*320+xx] = 9;
        }
        
        int fg = (idx == selected) ? 15 : 0;
        font_draw_text(v, wx+8, y, files[idx].name, fg);
        
        if (!(files[idx].attr & 0x10)) {
            char size_str[12];
            unsigned int size = files[idx].size;
            int p = 0;
            if (size == 0) { size_str[p++] = '0'; }
            else {
                char temp[10];
                int tp = 0;
                while (size > 0) { temp[tp++] = '0' + size % 10; size /= 10; }
                while (tp > 0) size_str[p++] = temp[--tp];
            }
            size_str[p++] = 'B';
            size_str[p] = 0;
            font_draw_text(v, wx+ww-50, y, size_str, fg);
        }
    }
    
    // 底部信息
    if (no_disk) {
        font_draw_text(v, wx+8, wy+wh-12, "No disk mounted", 8);
    } else {
        char info[15];
        info[0] = '0' + file_count / 10;
        info[1] = '0' + file_count % 10;
        info[2] = ' ';
        info[3] = 'f';
        info[4] = 'i';
        info[5] = 'l';
        info[6] = 'e';
        info[7] = 's';
        info[8] = 0;
        font_draw_text(v, wx+8, wy+wh-12, info, 0);
    }
    
    // 错误弹窗
    if (show_error && error_timer > 0) {
        draw_error_dialog(v, wx, wy, ww, wh);
        error_timer--;
        if (error_timer == 0) {
            show_error = 0;
        }
    }
}

void fm_click(unsigned char* v, int mx, int my, int wx, int wy, int ww, int wh)
{
    // 如果显示错误弹窗，点击关闭
    if (show_error) {
        int dx = wx + ww/2 - 60;
        int dy = wy + wh/2 - 25;
        int dw = 120;
        int dh = 50;
        
        if (mx >= dx && mx < dx+dw && my >= dy && my < dy+dh) {
            show_error = 0;
        }
        return;
    }
    
    // 检测点击文件
    int max_display = (wh - 50) / 10;
    if (max_display > 10) max_display = 10;
    
    for (int i = 0; i < max_display && (i + scroll_offset) < file_count; i++) {
        int idx = i + scroll_offset;
        int y = wy + 42 + i * 10;
        
        if (mx >= wx+4 && mx < wx+ww-4 && my >= y && my < y+9) {
            selected = idx;
            return;
        }
    }
    
    selected = -1;
}