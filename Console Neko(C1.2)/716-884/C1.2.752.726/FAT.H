#ifndef FAT_H
#define FAT_H

// FAT16 BPB (BIOS Parameter Block) - 位于引导扇区偏移 11 处
typedef struct {
    unsigned short bytes_per_sector;     // 偏移 11: 通常是 512
    unsigned char  sectors_per_cluster;  // 偏移 13
    unsigned short reserved_sectors;     // 偏移 14: 引导扇区数
    unsigned char  num_fats;             // 偏移 16: FAT 表数量 (通常是 2)
    unsigned short root_entries;         // 偏移 17: 根目录条目数
    unsigned short total_sectors_16;     // 偏移 19: 总扇区数 (< 65536)
    unsigned char  media_type;           // 偏移 21: 0xF8 = 硬盘
    unsigned short sectors_per_fat;      // 偏移 22: 每个 FAT 的扇区数
    unsigned short sectors_per_track;    // 偏移 24
    unsigned short num_heads;            // 偏移 26
    unsigned int   hidden_sectors;       // 偏移 28
    unsigned int   total_sectors_32;     // 偏移 32
} __attribute__((packed)) bpb_t;

// FAT16 根目录条目 - 32 字节
typedef struct {
    unsigned char  name[8];              // 文件名 (不足 8 字节用空格填充)
    unsigned char  ext[3];               // 扩展名
    unsigned char  attr;                 // 属性: 0x10=目录, 0x20=存档, 0x0F=长文件名
    unsigned char  reserved;             // 保留
    unsigned char  create_time_tenth;    // 创建时间(十分之一秒)
    unsigned short create_time;          // 创建时间
    unsigned short create_date;          // 创建日期
    unsigned short access_date;          // 最后访问日期
    unsigned short first_cluster_high;   // 起始簇号高 16 位 (FAT32 用)
    unsigned short write_time;           // 最后写入时间
    unsigned short write_date;           // 最后写入日期
    unsigned short first_cluster;        // 起始簇号
    unsigned int   file_size;            // 文件大小 (字节)
} __attribute__((packed)) dir_entry_t;

// FAT16 文件系统信息结构
typedef struct {
    unsigned char  sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char  num_fats;
    unsigned short root_entries;
    unsigned short sectors_per_fat;
    unsigned int   root_dir_sectors;
    unsigned int   first_data_sector;
    unsigned int   total_clusters;
    unsigned char  fat_loaded;
    unsigned short fat_cache[512 * 128];  // 缓存整个 FAT 表 (最多 128 扇区)
} fat_fs_t;

// 文件信息
typedef struct {
    char           name[13];          // 8.3 格式文件名
    unsigned char  attr;
    unsigned int   size;
    unsigned short first_cluster;
} file_info_t;

// 文件句柄
typedef struct {
    char           name[13];
    unsigned int   size;
    unsigned short first_cluster;
    unsigned short current_cluster;
    unsigned int   current_offset;     // 文件内偏移
    unsigned int   cluster_offset;     // 当前簇内偏移
    unsigned char  is_open;
} file_handle_t;

// 初始化文件系统，成功返回 0，失败返回 -1
int fat_init(void);

// 列出根目录所有文件，返回文件数量
int fat_list_root(void (*callback)(const file_info_t*));

// 按文件名查找，返回 0 表示找到，-1 表示未找到
int fat_find(const char* name, file_info_t* info);

// 打开文件，返回句柄指针，失败返回 0
file_handle_t* fat_open(const char* name);

// 读取文件数据，返回实际读取的字节数
int fat_read(file_handle_t* handle, unsigned char* buf, unsigned int size);

// 关闭文件
void fat_close(file_handle_t* handle);

// 打印调试信息
void fat_debug(void);

// 全局文件系统实例
extern fat_fs_t g_fs;

#endif