#include "disk.h"
#include "port.h"

#define ATA_PORT_DATA       0x1F0
#define ATA_PORT_ERROR      0x1F1
#define ATA_PORT_SECT_COUNT 0x1F2
#define ATA_PORT_LBA_LO     0x1F3
#define ATA_PORT_LBA_MID    0x1F4
#define ATA_PORT_LBA_HI     0x1F5
#define ATA_PORT_DRIVE      0x1F6
#define ATA_PORT_STATUS     0x1F7
#define ATA_PORT_CMD        0x1F7

#define ATA_CMD_READ        0x20
#define ATA_CMD_WRITE       0x30
#define ATA_CMD_IDENTIFY    0xEC
#define ATA_CMD_CACHE_FLUSH 0xE7

#define ATA_SR_BSY  0x80  // 忙
#define ATA_SR_DRDY 0x40  // 驱动器就绪
#define ATA_SR_DF   0x20  // 驱动器故障
#define ATA_SR_DRQ  0x08  // 数据请求
#define ATA_SR_ERR  0x01  // 错误

#define ATA_TIMEOUT  100000  // 超时计数

// 等待 BSY 清零（返回 0=成功, -1=超时, -2=故障）
static int ata_wait_bsy(void)
{
    int timeout = ATA_TIMEOUT;
    while ((inb(ATA_PORT_STATUS) & ATA_SR_BSY) && timeout > 0)
        timeout--;
    if (timeout == 0) return -1;      // 超时
    if (inb(ATA_PORT_STATUS) & ATA_SR_DF) return -2;  // 驱动器故障
    return 0;
}

// 等待 DRQ 置位（返回 0=成功, -1=超时, -2=错误）
static int ata_wait_drq(void)
{
    int timeout = ATA_TIMEOUT;
    unsigned char status;
    while (timeout > 0) {
        status = inb(ATA_PORT_STATUS);
        if (status & ATA_SR_DRQ) return 0;  // 数据就绪
        if (status & ATA_SR_ERR) return -2; // 错误
        timeout--;
    }
    return -1;  // 超时
}

int disk_detect(void)
{
    outb(ATA_PORT_DRIVE, 0xA0);         // 选择主盘
    outb(ATA_PORT_CMD, ATA_CMD_IDENTIFY);

    unsigned char status = inb(ATA_PORT_STATUS);
    if (status == 0xFF || status == 0x00) return 0;  // 无设备

    if (ata_wait_bsy() != 0) return 0;

    // 读取 IDENTIFY 数据（256个字）
    for (int i = 0; i < 256; i++) inw(ATA_PORT_DATA);
    return 1;
}

int disk_read(unsigned int lba, unsigned char* buf)
{
    // 1. 等待控制器空闲
    if (ata_wait_bsy() != 0) return -1;

    // 2. 设置参数
    outb(ATA_PORT_SECT_COUNT, 1);                      // 1 个扇区
    outb(ATA_PORT_LBA_LO, lba & 0xFF);
    outb(ATA_PORT_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PORT_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PORT_DRIVE, 0xE0 | ((lba >> 24) & 0x0F)); // LBA模式, 主盘

    // 3. 发送读命令
    outb(ATA_PORT_CMD, ATA_CMD_READ);

    // 4. 等待数据就绪
    if (ata_wait_drq() != 0) return -1;

    // 5. 读取数据
    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++)
        buf16[i] = inw(ATA_PORT_DATA);

    return 0;
}

int disk_write(unsigned int lba, const unsigned char* buf)
{
    // 1. 等待控制器空闲
    if (ata_wait_bsy() != 0) return -1;

    // 2. 设置参数
    outb(ATA_PORT_SECT_COUNT, 1);
    outb(ATA_PORT_LBA_LO, lba & 0xFF);
    outb(ATA_PORT_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PORT_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PORT_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    // 3. 发送写命令
    outb(ATA_PORT_CMD, ATA_CMD_WRITE);

    // 4. 等待数据请求
    if (ata_wait_drq() != 0) return -1;

    // 5. 写入数据
    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++)
        outw(ATA_PORT_DATA, buf16[i]);

    // 6. 刷新缓存
    outb(ATA_PORT_CMD, ATA_CMD_CACHE_FLUSH);

    // 7. 等待写入完成
    if (ata_wait_bsy() != 0) return -1;

    return 0;
}