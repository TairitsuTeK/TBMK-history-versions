#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
创建 FAT16 磁盘镜像
"""

import os
import struct
import sys

# 常量
BYTES_PER_SECTOR = 512
SECTORS_PER_CLUSTER = 8
RESERVED_SECTORS = 1
NUM_FATS = 2
ROOT_ENTRIES = 512
SECTORS_PER_FAT = 16
SECTORS_PER_TRACK = 63
NUM_HEADS = 16
TOTAL_SECTORS = 2880

def create_fat16_image(boot_bin, kernel_bin, output_image):
    """创建 FAT16 磁盘镜像"""

    # 读取引导扇区
    with open(boot_bin, 'rb') as f:
        boot_sector = bytearray(f.read())

    # 读取内核
    with open(kernel_bin, 'rb') as f:
        kernel_data = bytearray(f.read())

    # 计算内核占用的扇区数
    kernel_sectors = (len(kernel_data) + BYTES_PER_SECTOR - 1) // BYTES_PER_SECTOR

    print(f"[INFO] 内核大小: {len(kernel_data)} 字节")
    print(f"[INFO] 内核占用扇区: {kernel_sectors} 扇区")

    # 创建空白镜像
    image = bytearray(TOTAL_SECTORS * BYTES_PER_SECTOR)

    # 1. 写入引导扇区（BPB 在 boot.asm 中已设置）
    image[0:512] = boot_sector

    # 2. 计算 FAT 和根目录位置
    fat_start = RESERVED_SECTORS
    root_start = fat_start + NUM_FATS * SECTORS_PER_FAT
    root_sectors = (ROOT_ENTRIES * 32 + BYTES_PER_SECTOR - 1) // BYTES_PER_SECTOR
    data_start = root_start + root_sectors

    print(f"[INFO] FAT 起始: 扇区 {fat_start}")
    print(f"[INFO] 根目录起始: 扇区 {root_start}")
    print(f"[INFO] 数据区起始: 扇区 {data_start}")

    # 3. 初始化 FAT（FAT16）
    fat_size = SECTORS_PER_FAT * 256  # 每个扇区256个表项
    fat = [0] * fat_size

    # FAT 表项 0: 媒体描述
    fat[0] = 0xFFF8
    fat[1] = 0xFFFF

    # 4. 分配内核簇
    first_cluster = 2
    cluster_count = (kernel_sectors + SECTORS_PER_CLUSTER - 1) // SECTORS_PER_CLUSTER

    print(f"[INFO] 内核簇数: {cluster_count}")

    # 构建 FAT 链
    for i in range(cluster_count):
        cluster = first_cluster + i
        if i == cluster_count - 1:
            fat[cluster] = 0xFFFF
        else:
            fat[cluster] = cluster + 1

    # 写入 FAT（两份）
    for fat_num in range(NUM_FATS):
        fat_offset = (fat_start + fat_num * SECTORS_PER_FAT) * BYTES_PER_SECTOR
        for i, value in enumerate(fat[:SECTORS_PER_FAT * 256]):
            struct.pack_into('<H', image, fat_offset + i * 2, value)

    # 5. 创建根目录项
    root_dir_offset = root_start * BYTES_PER_SECTOR

    # 文件名: KERNEL  BIN (8.3 格式，必须11字节)
    filename = b"KERNEL  BIN"

    # 目录项结构
    # 0-7: 文件名, 8-10: 扩展名, 11: 属性, 26-27: 起始簇, 28-31: 文件大小
    struct.pack_into('<11s', image, root_dir_offset, filename)
    image[root_dir_offset + 11] = 0x20  # 文件属性
    struct.pack_into('<H', image, root_dir_offset + 26, first_cluster)
    struct.pack_into('<I', image, root_dir_offset + 28, len(kernel_data))

    # 6. 写入内核数据到数据区
    kernel_offset = data_start * BYTES_PER_SECTOR
    image[kernel_offset:kernel_offset + len(kernel_data)] = kernel_data

    # 7. 写入 BPB 信息
    struct.pack_into('<H', image, 11, BYTES_PER_SECTOR)
    image[13] = SECTORS_PER_CLUSTER
    struct.pack_into('<H', image, 14, RESERVED_SECTORS)
    image[16] = NUM_FATS
    struct.pack_into('<H', image, 17, ROOT_ENTRIES)
    struct.pack_into('<H', image, 19, 0)
    image[21] = 0xF8
    struct.pack_into('<H', image, 22, SECTORS_PER_FAT)
    struct.pack_into('<H', image, 24, SECTORS_PER_TRACK)
    struct.pack_into('<H', image, 26, NUM_HEADS)
    struct.pack_into('<I', image, 32, TOTAL_SECTORS)

    # 8. 写入镜像
    with open(output_image, 'wb') as f:
        f.write(image)

    print(f"[SUCCESS] 镜像已创建: {output_image}")
    print(f"[INFO] 总大小: {TOTAL_SECTORS * BYTES_PER_SECTOR} 字节")

    return True

def main():
    # 检查文件是否存在
    if not os.path.exists('boot.bin'):
        print("[ERROR] boot.bin 不存在，请先编译")
        sys.exit(1)

    if not os.path.exists('kernel.bin'):
        print("[ERROR] kernel.bin 不存在，请先编译")
        sys.exit(1)

    print("=" * 50)
    print("  T.B.M.K. 磁盘镜像构建工具")
    print("=" * 50)
    print()

    create_fat16_image('boot.bin', 'kernel.bin', 'os.img')
    print()
    print("=" * 50)

if __name__ == '__main__':
    main()