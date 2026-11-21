#include "fat.h"
#include "disk.h"

#define SECTOR_SIZE 512

fat_fs_t g_fs;
static file_handle_t g_handle;

static int str_casecmp(const char* a, const char* b, int n)
{
    for (int i = 0; i < n; i++) {
        char ca = a[i], cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca -= 32;
        if (cb >= 'a' && cb <= 'z') cb -= 32;
        if (ca != cb) return ca - cb;
        if (ca == 0) return 0;
    }
    return 0;
}

static int str_len(const char* s)
{
    int n = 0;
    while (s[n]) n++;
    return n;
}

static void str_copy(char* dst, const char* src, int max)
{
    int i;
    for (i = 0; i < max - 1 && src[i]; i++) dst[i] = src[i];
    dst[i] = '\0';
}

static int fat_read_fat(void)
{
    unsigned int fat_start = g_fs.reserved_sectors;
    int sectors_to_read = g_fs.sectors_per_fat;
    if (sectors_to_read > 128) sectors_to_read = 128;
    
    for (int i = 0; i < sectors_to_read; i++) {
        unsigned char buf[512];
        if (disk_read(fat_start + i, buf) != 0) return -1;
        unsigned short* buf_ptr = (unsigned short*)buf;
        for (int j = 0; j < 256; j++)
            g_fs.fat_cache[i * 256 + j] = buf_ptr[j];
    }
    g_fs.fat_loaded = 1;
    return 0;
}

static int fat_flush(void)
{
    unsigned int fat_start = g_fs.reserved_sectors;
    int fat_sectors = g_fs.sectors_per_fat;
    if (fat_sectors > 128) fat_sectors = 128;
    
    for (int i = 0; i < fat_sectors; i++) {
        unsigned char buf[512];
        for (int j = 0; j < 256; j++) {
            buf[j * 2] = g_fs.fat_cache[i * 256 + j] & 0xFF;
            buf[j * 2 + 1] = (g_fs.fat_cache[i * 256 + j] >> 8) & 0xFF;
        }
        if (disk_write(fat_start + i, buf) != 0) return -1;
    }
    return 0;
}

void fat_flush_all(void)
{
    fat_flush();
}

static unsigned short fat_next_cluster(unsigned short cluster)
{
    if (cluster < 2 || cluster >= g_fs.total_clusters + 2) return 0xFFFF;
    unsigned short value = g_fs.fat_cache[cluster];
    if (value >= 0xFFF8) return 0xFFFF;
    if (value == 0xFFF7) return 0xFFF7;
    if (value == 0)     return 0xFFFF;
    return value;
}

static unsigned int cluster_to_sector(unsigned short cluster)
{
    return g_fs.first_data_sector + (cluster - 2) * g_fs.sectors_per_cluster;
}

static void fat_name_to_str(const unsigned char* raw_name, char* out)
{
    int p = 0;
    for (int i = 0; i < 8; i++) {
        if (raw_name[i] == ' ' || raw_name[i] == 0) break;
        out[p++] = raw_name[i];
    }
    if (raw_name[8] != ' ' && raw_name[8] != 0) {
        out[p++] = '.';
        for (int i = 8; i < 11; i++) {
            if (raw_name[i] == ' ' || raw_name[i] == 0) break;
            out[p++] = raw_name[i];
        }
    }
    out[p] = '\0';
}

static void str_to_fat_name(const char* input, unsigned char* name, unsigned char* ext)
{
    for (int i = 0; i < 8; i++) name[i] = ' ';
    for (int i = 0; i < 3; i++) ext[i] = ' ';
    int dot_pos = -1, len = str_len(input);
    for (int i = 0; i < len; i++) {
        if (input[i] == '.') { dot_pos = i; break; }
    }
    int main_len = (dot_pos >= 0) ? dot_pos : len;
    if (main_len > 8) main_len = 8;
    for (int i = 0; i < main_len; i++) {
        char c = input[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        name[i] = c;
    }
    if (dot_pos >= 0) {
        int ext_start = dot_pos + 1, ext_len = len - ext_start;
        if (ext_len > 3) ext_len = 3;
        for (int i = 0; i < ext_len; i++) {
            char c = input[ext_start + i];
            if (c >= 'a' && c <= 'z') c -= 32;
            ext[i] = c;
        }
    }
}

int fat_init(void)
{
    unsigned char boot_sector[512];
    if (disk_read(0, boot_sector) != 0) return -1;

    bpb_t* bpb = (bpb_t*)&boot_sector[11];
    
    // 验证BPB
    if (bpb->bytes_per_sector != 512) return -1;
    if (bpb->sectors_per_cluster == 0) return -1;
    if (bpb->num_fats == 0 || bpb->num_fats > 4) return -1;
    if (bpb->root_entries == 0) return -1;
    
    g_fs.sectors_per_cluster = bpb->sectors_per_cluster;
    g_fs.reserved_sectors   = bpb->reserved_sectors;
    g_fs.num_fats           = bpb->num_fats;
    g_fs.root_entries       = bpb->root_entries;
    g_fs.sectors_per_fat    = bpb->sectors_per_fat;
    g_fs.root_dir_sectors   = (g_fs.root_entries * 32 + 511) / 512;

    unsigned int total_sectors = bpb->total_sectors_16;
    if (total_sectors == 0) total_sectors = bpb->total_sectors_32;
    if (total_sectors == 0) return -1;

    g_fs.first_data_sector = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat + g_fs.root_dir_sectors;

    // 加载FAT
    g_fs.fat_loaded = 0;
    if (fat_read_fat() != 0) return -1;
    
    return 0;
}

int fat_list_root(void (*callback)(const file_info_t*))
{
    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];
    file_info_t info;
    
    // 根目录扇区数
    unsigned int root_sectors = g_fs.root_dir_sectors;
    if (root_sectors > 128) root_sectors = 128;
    
    for (unsigned int i = 0; i < root_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) return -1;
        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];
            if (entry->name[0] == 0x00) return 0;  // 结束
            if (entry->name[0] == 0xE5) continue;  // 已删除
            if (entry->attr == 0x0F) continue;      // LFN
            if (entry->attr & 0x08) continue;       // 卷标
            
            fat_name_to_str(entry->name, info.name);
            info.attr = entry->attr;
            info.size = entry->file_size;
            info.first_cluster = entry->first_cluster;
            if (callback) callback(&info);
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
    
    unsigned int root_sectors = g_fs.root_dir_sectors;
    if (root_sectors > 128) root_sectors = 128;

    for (unsigned int i = 0; i < root_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) return -1;
        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];
            if (entry->name[0] == 0x00) return -1;
            if (entry->name[0] == 0xE5) continue;
            if (entry->attr == 0x0F) continue;
            if (entry->attr & 0x08) continue;
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
    if (fat_find(name, &info) != 0) return 0;
    if (info.attr & 0x10) return 0;
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
    unsigned int bytes_read = 0, remaining = size;
    if (handle->current_offset >= handle->size) return 0;
    if (handle->current_offset + size > handle->size) remaining = handle->size - handle->current_offset;

    unsigned char sector_buf[512];
    while (remaining > 0) {
        if (handle->cluster_offset >= g_fs.sectors_per_cluster * 512) {
            handle->current_cluster = fat_next_cluster(handle->current_cluster);
            if (handle->current_cluster == 0xFFFF || handle->current_cluster == 0xFFF7) break;
            handle->cluster_offset = 0;
        }
        unsigned int sector = cluster_to_sector(handle->current_cluster) + handle->cluster_offset / 512;
        unsigned int offset_in_sector = handle->cluster_offset % 512;
        if (disk_read(sector, sector_buf) != 0) break;
        unsigned int to_read = 512 - offset_in_sector;
        if (to_read > remaining) to_read = remaining;
        for (unsigned int i = 0; i < to_read; i++) buf[bytes_read + i] = sector_buf[offset_in_sector + i];
        bytes_read += to_read;
        remaining -= to_read;
        handle->current_offset += to_read;
        handle->cluster_offset += to_read;
    }
    return bytes_read;
}

void fat_close(file_handle_t* handle)
{
    if (handle) handle->is_open = 0;
}

int fat_delete(const char* name)
{
    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];
    unsigned char fat_name[8], fat_ext[3];
    str_to_fat_name(name, fat_name, fat_ext);
    
    unsigned int root_sectors = g_fs.root_dir_sectors;
    if (root_sectors > 128) root_sectors = 128;

    for (unsigned int i = 0; i < root_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) return -1;
        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];
            if (entry->name[0] == 0x00) return -1;
            if (entry->name[0] == 0xE5) continue;
            if (entry->attr == 0x0F) continue;
            if (entry->attr & 0x08) continue;
            if (str_casecmp((const char*)entry->name, (const char*)fat_name, 8) == 0 &&
                str_casecmp((const char*)&entry->name[8], (const char*)fat_ext, 3) == 0) {
                entry->name[0] = 0xE5;
                unsigned short cluster = entry->first_cluster;
                while (cluster >= 2 && cluster < 0xFFF8) {
                    unsigned short next = g_fs.fat_cache[cluster];
                    g_fs.fat_cache[cluster] = 0x0000;
                    if (next >= 0xFFF8) break;
                    cluster = next;
                }
                if (disk_write(root_start + i, buf) != 0) return -1;
                fat_flush();
                return 0;
            }
        }
    }
    return -1;
}

int fat_rename(const char* old_name, const char* new_name)
{
    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];
    unsigned char old_fat_name[8], old_fat_ext[3];
    unsigned char new_fat_name[8], new_fat_ext[3];
    str_to_fat_name(old_name, old_fat_name, old_fat_ext);
    str_to_fat_name(new_name, new_fat_name, new_fat_ext);

    file_info_t dummy;
    if (fat_find(new_name, &dummy) == 0) return -2;
    
    unsigned int root_sectors = g_fs.root_dir_sectors;
    if (root_sectors > 128) root_sectors = 128;

    for (unsigned int i = 0; i < root_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) return -1;
        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];
            if (entry->name[0] == 0x00) return -1;
            if (entry->name[0] == 0xE5) continue;
            if (entry->attr == 0x0F) continue;
            if (entry->attr & 0x08) continue;
            if (str_casecmp((const char*)entry->name, (const char*)old_fat_name, 8) == 0 &&
                str_casecmp((const char*)&entry->name[8], (const char*)old_fat_ext, 3) == 0) {
                for (int k = 0; k < 8; k++) entry->name[k] = new_fat_name[k];
                for (int k = 0; k < 3; k++) entry->ext[k] = new_fat_ext[k];
                if (disk_write(root_start + i, buf) != 0) return -1;
                return 0;
            }
        }
    }
    return -1;
}

int fat_create(const char* name)
{
    file_info_t dummy;
    if (fat_find(name, &dummy) == 0) return -2;

    unsigned int root_start = g_fs.reserved_sectors + g_fs.num_fats * g_fs.sectors_per_fat;
    unsigned char buf[512];
    unsigned char fat_name[8], fat_ext[3];
    str_to_fat_name(name, fat_name, fat_ext);
    
    unsigned int root_sectors = g_fs.root_dir_sectors;
    if (root_sectors > 128) root_sectors = 128;

    for (unsigned int i = 0; i < root_sectors; i++) {
        if (disk_read(root_start + i, buf) != 0) return -1;
        for (int j = 0; j < 16; j++) {
            dir_entry_t* entry = (dir_entry_t*)&buf[j * 32];
            if (entry->name[0] == 0x00 || entry->name[0] == 0xE5) {
                for (int k = 0; k < 32; k++) buf[j * 32 + k] = 0;
                for (int k = 0; k < 8; k++) entry->name[k] = fat_name[k];
                for (int k = 0; k < 3; k++) entry->ext[k] = fat_ext[k];
                entry->attr = 0x20;
                entry->first_cluster = 0;
                entry->file_size = 0;
                if (disk_write(root_start + i, buf) != 0) return -1;
                return 0;
            }
        }
    }
    return -3;
}