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

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF   0x20
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

#define ATA_TIMEOUT  1000000  // 增加超时时间

static int grub_mode = 1;

void disk_set_grub_mode(int mode)
{
    grub_mode = mode;
}

int disk_get_grub_mode(void)
{
    return grub_mode;
}

static int ata_wait_bsy(void)
{
    int timeout = ATA_TIMEOUT;
    while ((inb(ATA_PORT_STATUS) & ATA_SR_BSY) && timeout > 0)
        timeout--;
    if (timeout == 0) return -1;
    if (inb(ATA_PORT_STATUS) & ATA_SR_DF) return -2;
    return 0;
}

static int ata_wait_drq(void)
{
    int timeout = ATA_TIMEOUT;
    unsigned char status;
    while (timeout > 0) {
        status = inb(ATA_PORT_STATUS);
        if (status & ATA_SR_DRQ) return 0;
        if (status & ATA_SR_ERR) return -2;
        if (status & ATA_SR_DF) return -2;
        timeout--;
    }
    return -1;
}

int disk_detect(void)
{
    if (grub_mode) {
        // 尝试多个可能的硬盘
        unsigned char buf[512];
        
        // 尝试主盘
        if (disk_read(0, buf) == 0) {
            if (buf[510] == 0x55 && buf[511] == 0xAA) {
                return 1;
            }
            // 即使没有MBR签名，也可能有FAT引导扇区
            if (buf[11] == 0x00 && buf[12] == 0x02) {
                return 1;  // bytes_per_sector = 512
            }
        }
        
        return 0;
    }
    
    // 传统检测
    outb(ATA_PORT_DRIVE, 0xA0);
    outb(ATA_PORT_CMD, ATA_CMD_IDENTIFY);

    unsigned char status = inb(ATA_PORT_STATUS);
    if (status == 0xFF || status == 0x00) return 0;

    if (ata_wait_bsy() != 0) return 0;

    for (int i = 0; i < 256; i++) inw(ATA_PORT_DATA);
    return 1;
}

int disk_read(unsigned int lba, unsigned char* buf)
{
    // 等待控制器空闲
    int wait_result = ata_wait_bsy();
    if (wait_result != 0) return -1;

    // 设置参数
    outb(ATA_PORT_SECT_COUNT, 1);
    outb(ATA_PORT_LBA_LO, lba & 0xFF);
    outb(ATA_PORT_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PORT_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PORT_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));  // LBA模式, 主盘

    // 发送读命令
    outb(ATA_PORT_CMD, ATA_CMD_READ);

    // 等待数据就绪
    if (ata_wait_drq() != 0) return -1;

    // 读取数据
    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++)
        buf16[i] = inw(ATA_PORT_DATA);

    return 0;
}

int disk_write(unsigned int lba, const unsigned char* buf)
{
    if (ata_wait_bsy() != 0) return -1;

    outb(ATA_PORT_SECT_COUNT, 1);
    outb(ATA_PORT_LBA_LO, lba & 0xFF);
    outb(ATA_PORT_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_PORT_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_PORT_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_PORT_CMD, ATA_CMD_WRITE);

    if (ata_wait_drq() != 0) return -1;

    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++)
        outw(ATA_PORT_DATA, buf16[i]);

    outb(ATA_PORT_CMD, ATA_CMD_CACHE_FLUSH);
    
    if (ata_wait_bsy() != 0) return -1;
    
    for (volatile int i = 0; i < 10000; i++);

    return 0;
}