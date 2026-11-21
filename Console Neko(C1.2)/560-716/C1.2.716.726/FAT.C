#include "fat.h"
#include "disk.h"

#define SECTOR_SIZE 512

// 全局文件系统实例
fat_fs_t g_fs;
static file_handle_t g_handle;

// 字符串比较 (不区分大小写)
static int str_casecmp(const char* a, const char* b, int n)
{
    for (int i = 0; i < n; i++) {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
    }
    return 0;
}

// 字符串长度
static int str_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

// 字符串复制
static void str_copy(char* dst, const char* src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

// 读取 FAT 表到缓存
static int fat_read_fat(void)
{
    unsigned int fat_start = g_fs.reserved_sectors;
    int sectors_to_read = g_fs.sectors_per_fat;

    // 限制读取扇区数，防止溢出缓存
    if (sectors_to_read > 128) {
        sectors_to_read = 128;
    }

    for (int i = 0; i < sectors_to_read; i++) {
        unsigned char buf[512];
        if (disk_read(fat_start + i, buf) != 0) {
            return -1;
        }
        // 复制到缓存
        unsigned short* cache_ptr = &g_fs.fat_cache[i * 256];
        unsigned short* buf_ptr = (unsigned short*)buf;
        for (int j = 0; j < 256; j++) {
            cache_ptr[j] = buf_ptr[j];
        }
    }

    g_fs.fat_loaded = 1;
    return 0;
}

// 获取 FAT 表中指定簇的下一个簇号
static unsigned short fat_next_cluster(unsigned short cluster)
{
    if (cluster < 2 || cluster >= g_fs.total_clusters + 2) {
        return 0xFFFF;  // 无效簇
    }

    unsigned short value = g_fs.fat_cache[cluster];

    // FAT16 簇值含义
    if (value >= 0xFFF8) return 0xFFFF;  // 最后一个簇
    if (value == 0xFFF7) return 0xFFF7;  // 坏簇
    if (value == 0)     return 0xFFFF;   // 空闲簇 (不应出现在链中)

    return value;
}

// 将簇号转换为扇区号
static unsigned int cluster_to_sector(unsigned short cluster)
{
    return g_fs.first_data_sector + (cluster - 2) * g_fs.sectors_per_cluster;
}

// 将 8.3 格式的文件名转换为可读字符串
// "KERNEL  BIN" -> "KERNEL.BIN"
static void fat_name_to_str(const unsigned char* raw_name, char* out)
{
    int p = 0;

    // 复制主文件名 (去除尾部空格)
    for (int i = 0; i < 8; i++) {
        if (raw_name[i] == ' ' || raw_name[i] == 0) break;
        out[p++] = raw_name[i];
    }

    // 如果有扩展名
    if (raw_name[8] != ' ' && raw_name[8] != 0) {
        out[p++] = '.';
        for (int i = 8; i < 11; i++) {
            if (raw_name[i] == ' ' || raw_name[i] == 0) break;
            out[p++] = raw_name[i];
        }
    }

    out[p] = '\0';
}

// 将用户输入的文件名转换为 8.3 格式
// "kernel.bin" -> name="KERNEL  ", ext="BIN"
static void str_to_fat_name(const char* input, unsigned char* name, unsigned char* ext)
{
    // 初始化
    for (int i = 0; i < 8; i++) name[i] = ' ';
    for (int i = 0; i < 3; i++) ext[i] = ' ';

    // 查找点的位置
    int dot_pos = -1;
    int len = str_len(input);
    for (int i = 0; i < len; i++) {
        if (input[i] == '.') {
            dot_pos = i;
            break;
        }
    }

    // 复制主文件名
    int main_len = (dot_pos >= 0) ? dot_pos : len;
    if (main_len > 8) main_len = 8;
    for (int i = 0; i < main_len; i++) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c -= 32;  // 转大写
        name[i] = c;
    }

    // 复制扩展名
    if (dot_pos >= 0) {
        int ext_start = dot_pos + 1;
        int ext_len = len - ext_start;
        if (ext_len > 3) ext_len = 3;
        for (int i = 0; i < ext_len; i++) {
            char c = input[ext_start + i];
            if (c >= 'a' && c <= 'z') c -= 32;
            ext[i] = c;
        }
    }
}

// ==================== 公开函数 ====================

int fat_init(void)
{
    unsigned char boot_sector[512];

    // 读取硬盘的第一个扇区 (MBR 或引导扇区)
    if (disk_read(0, boot_sector) != 0) {
        return -1;
    }

    bpb_t* bpb = 0;
    unsigned int partition_start = 0;

    // 检查扇区 0 是否有有效的 BPB (看 bytes_per_sector = 512)
    bpb = (bpb_t*)&boot_sector[11];

    if (bpb->bytes_per_sector == 512 && bpb->sectors_per_cluster > 0 && bpb->sectors_per_cluster <= 128) {
        // 扇区 0 直接是引导扇区
        partition_start = 0;
    } else {
        // 可能是 MBR 分区表，读取第一个分区的引导扇区
        // 分区表位于扇区 0 偏移 0x1BE
        // 第一个分区起始扇区在偏移 0x1C6 (4 字节)
        unsigned int* part_start = (unsigned int*)&boot_sector[0x1C6];
        partition_start = *part_start;

        if (partition_start == 0) {
            // MBR 无效分区，尝试常见偏移
            partition_start = 63;  // 传统 CHS 对齐
        }

        if (disk_read(partition_start, boot_sector) != 0) {
            return -1;
        }
        bpb = (bpb_t*)&boot_sector[11];
    }

    // 验证 BPB
    if (bpb->bytes_per_sector != 512) {
        // 再尝试 LBA 对齐的 2048
        partition_start = 2048;
        if (disk_read(partition_start, boot_sector) != 0) {
            return -1;
        }
        bpb = (bpb_t*)&boot_sector[11];
        if (bpb->bytes_per_sector != 512) {
            return -1;
        }
    }

    // 解析 BPB
    g_fs.sectors_per_cluster = bpb->sectors_per_cluster;
    g_fs.reserved_sectors   = bpb->reserved_sectors + partition_start;
    g_fs.num_fats           = bpb->num_fats;
    g_fs.root_entries       = bpb->root_entries;
    g_fs.sectors_per_fat    = bpb->sectors_per_fat;

    // 计算
    g_fs.root_dir_sectors = (g_fs.root_entries * 32 + 511) / 512;

    unsigned int total_sectors = bpb->total_sectors_16;
    if (total_sectors == 0) total_sectors = bpb->total_sectors_32;

    unsigned int data_sectors = total_sectors
                              - g_fs.reserved_sectors
                              - g_fs.num_fats * g_fs.sectors_per_fat
                              - g_fs.root_dir_sectors;

    g_fs.total_clusters = data_sectors / g_fs.sectors_per_cluster;
    g_fs.first_data_sector = g_fs.reserved_sectors
                           + g_fs.num_fats * g_fs.sectors_per_fat
                           + g_fs.root_dir_sectors;

    g_fs.fat_loaded = 0;

    // 读取 FAT 表
    if (fat_read_fat() != 0) {
        return -1;
    }

    return 0;
}

int fat_list_root(void (*callback)(const file_info_t*))
{
    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];
    file_info_t info;

    for (unsigned int i = 0; i < g_fs.root_dir_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) {
            return -1;
        }

        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];

            // 空条目 = 目录结束
            if (entry->name[0] == 0x00) {
                return 0;
            }

            // 0xE5 = 已删除
            if (entry->name[0] == 0xE5) {
                continue;
            }

            // 跳过卷标 (0x08) 和长文件名 (0x0F)
            if (entry->attr == 0x0F) {
                continue;
            }

            // 也跳过卷标
            if (entry->attr & 0x08) {
                continue;
            }

            fat_name_to_str(entry->name, info.name);
            info.attr = entry->attr;
            info.size = entry->file_size;
            info.first_cluster = entry->first_cluster;

            if (callback) {
                callback(&info);
            }
        }
    }

    return 0;
}

int fat_find(const char* name, file_info_t* info)
{
    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];

    unsigned char fat_name[8], fat_ext[3];
    str_to_fat_name(name, fat_name, fat_ext);

    for (unsigned int i = 0; i < g_fs.root_dir_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) {
            return -1;
        }

        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];

            if (entry->name[0] == 0x00) {
                return -1;  // 未找到
            }

            if (entry->name[0] == 0xE5) continue;
            if (entry->attr == 0x0F) continue;
            if (entry->attr & 0x08) continue;  // 跳过卷标

            // 比较文件名
            if (str_casecmp((const char*)entry->name, (const char*)fat_name, 8) == 0 &&
                str_casecmp((const char*)&entry->name[8], (const char*)fat_ext, 3) == 0) {

                fat_name_to_str(entry->name, info->name);
                info->attr = entry->attr;
                info->size = entry->file_size;
                info->first_cluster = entry->first_cluster;
                return 0;
            }
        }
    }

    return -1;
}

file_handle_t* fat_open(const char* name)
{
    file_info_t info;

    if (fat_find(name, &info) != 0) {
        return 0;  // 文件不存在
    }

    if (info.attr & 0x10) {
        return 0;  // 是目录
    }

    // 初始化句柄
    str_copy(g_handle.name, info.name, 13);
    g_handle.size = info.size;
    g_handle.first_cluster = info.first_cluster;
    g_handle.current_cluster = info.first_cluster;
    g_handle.current_offset = 0;
    g_handle.cluster_offset = 0;
    g_handle.is_open = 1;

    return &g_handle;
}

int fat_read(file_handle_t* handle, unsigned char* buf, unsigned int size)
{
    if (!handle->is_open) return -1;

    unsigned int bytes_read = 0;
    unsigned int remaining = size;

    if (handle->current_offset >= handle->size) {
        return 0;  // 已到文件末尾
    }

    if (handle->current_offset + size > handle->size) {
        remaining = handle->size - handle->current_offset;
    }

    unsigned char sector_buf[512];

    while (remaining > 0) {
        // 当前簇已读完，获取下一个簇
        if (handle->cluster_offset >= g_fs.sectors_per_cluster * 512) {
            handle->current_cluster = fat_next_cluster(handle->current_cluster);
            if (handle->current_cluster == 0xFFFF || handle->current_cluster == 0xFFF7) {
                break;  // 文件结束或坏簇
            }
            handle->cluster_offset = 0;
        }

        // 计算当前簇内的扇区号和偏移
        unsigned int sector = cluster_to_sector(handle->current_cluster)
                            + (handle->cluster_offset / 512);
        unsigned int offset_in_sector = handle->cluster_offset % 512;

        // 读取扇区
        if (disk_read(sector, sector_buf) != 0) {
            break;
        }

        // 计算本次可以读取的字节数
        unsigned int to_read = 512 - offset_in_sector;
        if (to_read > remaining) to_read = remaining;

        // 复制数据
        for (unsigned int i = 0; i < to_read; i++) {
            buf[bytes_read + i] = sector_buf[offset_in_sector + i];
        }

        bytes_read += to_read;
        remaining -= to_read;
        handle->current_offset += to_read;
        handle->cluster_offset += to_read;
    }

    return bytes_read;
}

void fat_close(file_handle_t* handle)
{
    if (handle) {
        handle->is_open = 0;
    }
}

void fat_debug(void)
{
    // 调试函数，暂时为空
}