"""
makedisk.py - 创建简单的 FAT16 磁盘镜像用于测试
用法: python makedisk.py
输出: disk.img (10MB FAT16 镜像，包含测试文件)
"""

import struct
import os

def create_fat16_image(filename='disk.img', size_mb=10):
    """创建一个最小的 FAT16 磁盘镜像"""

    SECTOR_SIZE = 512
    total_sectors = size_mb * 1024 * 1024 // SECTOR_SIZE
    sectors_per_cluster = 8  # 4KB 簇
    reserved_sectors = 1
    num_fats = 2
    root_entries = 512  # 32 个扇区的根目录
    sectors_per_fat = 16  # 每个 FAT 8KB

    img = bytearray(total_sectors * SECTOR_SIZE)

    # ===== BPB (从偏移 11 开始) =====
    bpb = bytearray(512)
    bpb[0:3] = b'\xEB\x3C\x90'  # 跳转指令 + NOP

    # OEM Name (8 bytes)
    bpb[3:11] = b'T.B.M.K. '

    # BPB 参数 (从偏移 11 开始)
    struct.pack_into('<H', bpb, 11, SECTOR_SIZE)       # bytes_per_sector
    struct.pack_into('<B', bpb, 13, sectors_per_cluster)
    struct.pack_into('<H', bpb, 14, reserved_sectors)
    struct.pack_into('<B', bpb, 16, num_fats)
    struct.pack_into('<H', bpb, 17, root_entries)
    struct.pack_into('<H', bpb, 19, total_sectors & 0xFFFF)  # total_sectors_16
    struct.pack_into('<B', bpb, 21, 0xF8)              # media (hard disk)
    struct.pack_into('<H', bpb, 22, sectors_per_fat)
    struct.pack_into('<H', bpb, 24, 63)                # sectors_per_track
    struct.pack_into('<H', bpb, 26, 16)                # num_heads
    struct.pack_into('<I', bpb, 28, 0)                 # hidden_sectors
    struct.pack_into('<I', bpb, 32, 0)                 # total_sectors_32 (0 = use 16-bit)

    # 引导扇区签名
    struct.pack_into('<H', bpb, 510, 0xAA55)

    # 复制到镜像
    img[0:512] = bpb

    # ===== 计算偏移 =====
    fat1_offset = reserved_sectors * SECTOR_SIZE       # FAT1 起始字节
    fat2_offset = fat1_offset + sectors_per_fat * SECTOR_SIZE  # FAT2 起始字节
    root_dir_sectors = (root_entries * 32 + SECTOR_SIZE - 1) // SECTOR_SIZE
    root_offset = (reserved_sectors + num_fats * sectors_per_fat) * SECTOR_SIZE  # 根目录起始字节
    data_offset_bytes = root_offset + root_dir_sectors * SECTOR_SIZE  # 数据区起始字节

    # 数据区起始扇区号 (用于簇号计算)
    data_start_sector = reserved_sectors + num_fats * sectors_per_fat + root_dir_sectors

    print(f"  FAT1 offset: 0x{fat1_offset:X}")
    print(f"  Root offset: 0x{root_offset:X}")
    print(f"  Data offset: 0x{data_offset_bytes:X}")
    print(f"  Data start sector: {data_start_sector}")

    # ===== FAT 表初始化 =====
    # FAT 簇 0-1 保留
    img[fat1_offset] = 0xF8
    img[fat1_offset + 1] = 0xFF
    img[fat1_offset + 2] = 0xFF
    img[fat1_offset + 3] = 0xFF
    img[fat2_offset:fat2_offset + 4] = img[fat1_offset:fat1_offset + 4]

    # ===== 根目录 =====
    # 卷标条目
    vol_entry = bytearray(32)
    vol_label = b'TBMK_DISK  '
    vol_entry[0:11] = vol_label[0:11]
    vol_entry[11] = 0x08  # 属性: 卷标
    img[root_offset:root_offset + 32] = vol_entry

    # ===== 创建文件辅助函数 =====
    # 当前数据区使用到的字节偏移 (从数据区起始开始)
    current_data_offset = 0

    def create_file_entry(name, ext, content, dir_entry_offset):
        """在根目录中创建文件条目，并在数据区写入内容"""
        nonlocal current_data_offset

        entry = bytearray(32)

        # 文件名 (空格填充)
        name_bytes = name.encode('ascii').upper()
        ext_bytes = ext.encode('ascii').upper()
        for i in range(8):
            entry[i] = name_bytes[i] if i < len(name_bytes) else 0x20
        for i in range(3):
            entry[8 + i] = ext_bytes[i] if i < len(ext_bytes) else 0x20

        entry[11] = 0x20  # 属性: 存档

        # 文件内容
        content_bytes = content.encode('ascii') if isinstance(content, str) else content

        # 文件大小
        struct.pack_into('<I', entry, 28, len(content_bytes))

        # 计算簇号
        # 簇号 = 2 + (数据区内的扇区偏移 / 每簇扇区数)
        sector_in_data = current_data_offset // SECTOR_SIZE
        cluster = 2 + sector_in_data // sectors_per_cluster

        if cluster > 65535:
            print(f"  [ERROR] 簇号 {cluster} 超出 FAT16 范围!")
            return 0

        struct.pack_into('<H', entry, 26, cluster)

        print(f"  Creating {name}.{ext}: cluster={cluster}, size={len(content_bytes)}")

        # 写入根目录
        entry_addr = root_offset + dir_entry_offset
        img[entry_addr:entry_addr + 32] = entry

        # 写入数据区
        data_addr = data_offset_bytes + current_data_offset
        for i in range(len(content_bytes)):
            img[data_addr + i] = content_bytes[i]

        # 更新 FAT 表 (标记为 EOF)
        fat_entry_offset = fat1_offset + cluster * 2
        struct.pack_into('<H', img, fat_entry_offset, 0xFFFF)
        fat_entry_offset = fat2_offset + cluster * 2
        struct.pack_into('<H', img, fat_entry_offset, 0xFFFF)

        # 更新偏移 (下一个文件从新簇开始)
        used_sectors = (len(content_bytes) + SECTOR_SIZE - 1) // SECTOR_SIZE
        used_clusters = (used_sectors + sectors_per_cluster - 1) // sectors_per_cluster
        current_data_offset += used_clusters * sectors_per_cluster * SECTOR_SIZE

        return len(content_bytes)

    # 创建测试文件 (每个文件占一个簇)
    print("\n  Creating test files...")

    create_file_entry('README', 'TXT',
                      'Welcome to T.B.M.K. FAT16 File System!\r\n'
                      'This is a test file.\r\n'
                      'Commands: DIR, TYPE, LOAD\r\n',
                      32)

    create_file_entry('HELLO', 'TXT',
                      'Hello from T.B.M.K. Kernel!\r\n'
                      'FAT16 driver loaded successfully.\r\n',
                      64)

    create_file_entry('CONFIG', 'SYS',
                      'TBMK_CONFIG=1\r\n'
                      'VERSION=C1.2.704.726\r\n'
                      'BOOT_MODE=DISK\r\n',
                      96)

    create_file_entry('TEST', 'BIN',
                      b'\x90\x90\x90\x90'
                      b'\xEB\xFE',
                      128)

    # 写入文件
    with open(filename, 'wb') as f:
        f.write(img)

    # 打印信息
    total_clusters = (total_sectors - data_start_sector) // sectors_per_cluster
    print(f"\n[✓] Created {filename}")
    print(f"    Size: {size_mb}MB ({total_sectors} sectors)")
    print(f"    Filesystem: FAT16")
    print(f"    Sectors per cluster: {sectors_per_cluster}")
    print(f"    Sectors per FAT: {sectors_per_fat}")
    print(f"    Root entries: {root_entries}")
    print(f"    Data clusters: {total_clusters}")
    print(f"    Files: README.TXT, HELLO.TXT, CONFIG.SYS, TEST.BIN")

if __name__ == '__main__':
    create_fat16_image()
    print("\nDone! Use: qemu-system-i386 -fda os.img -hda disk.img")