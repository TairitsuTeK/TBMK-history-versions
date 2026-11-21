#include "disk.h"
#include "port.h"

int disk_detect(void)
{
    outb(0x1F6, 0xA0);
    outb(0x1F7, 0xEC);
    unsigned char status = inb(0x1F7);
    if (status == 0xFF || status == 0x00) return 0;
    int timeout = 100000;
    while ((inb(0x1F7) & 0x80) && timeout > 0) timeout--;
    if (timeout == 0) return 0;
    if (inb(0x1F7) & 0x01) return 0;
    for (int i = 0; i < 256; i++) inw(0x1F0);
    return 1;
}

int disk_read(unsigned int lba, unsigned char* buf)
{
    while (inb(0x1F7) & 0x80);
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x20);
    while (!(inb(0x1F7) & 0x08));
    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++) buf16[i] = inw(0x1F0);
    return 0;
}

int disk_write(unsigned int lba, const unsigned char* buf)
{
    while (inb(0x1F7) & 0x80);
    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x30);
    while (!(inb(0x1F7) & 0x08));
    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++) outw(0x1F0, buf16[i]);
    outb(0x1F7, 0xE7);
    while (inb(0x1F7) & 0x80);
    return 0;
}