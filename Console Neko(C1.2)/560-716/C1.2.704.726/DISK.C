#include "disk.h"

static inline void outb(unsigned short port, unsigned char value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline unsigned short inw(unsigned short port)
{
    unsigned short value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(unsigned short port, unsigned short value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

int disk_detect(void)
{
    outb(0x1F6, 0xA0);
    outb(0x1F7, 0xEC);

    unsigned char status = inb(0x1F7);

    if (status == 0xFF || status == 0x00) {
        return 0;
    }

    int timeout = 100000;
    while ((inb(0x1F7) & 0x80) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        return 0;
    }

    if (inb(0x1F7) & 0x01) {
        return 0;
    }

    for (int i = 0; i < 256; i++) {
        inw(0x1F0);
    }

    return 1;
}

int disk_read(unsigned int lba, unsigned char* buf)
{
    int timeout;

    timeout = 100000;
    while (inb(0x1F7) & 0x80) {
        if (--timeout <= 0) return -1;
    }

    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x20);

    timeout = 100000;
    while (!(inb(0x1F7) & 0x08)) {
        if (inb(0x1F7) & 0x01) return -1;
        if (--timeout <= 0) return -1;
    }

    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(0x1F0);
    }

    return 0;
}

int disk_write(unsigned int lba, const unsigned char* buf)
{
    int timeout;

    timeout = 100000;
    while (inb(0x1F7) & 0x80) {
        if (--timeout <= 0) return -1;
    }

    outb(0x1F2, 1);
    outb(0x1F3, lba & 0xFF);
    outb(0x1F4, (lba >> 8) & 0xFF);
    outb(0x1F5, (lba >> 16) & 0xFF);
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F7, 0x30);

    timeout = 100000;
    while (!(inb(0x1F7) & 0x08)) {
        if (inb(0x1F7) & 0x01) return -1;
        if (--timeout <= 0) return -1;
    }

    unsigned short* buf16 = (unsigned short*)buf;
    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buf16[i]);
    }

    outb(0x1F7, 0xE7);

    timeout = 100000;
    while (inb(0x1F7) & 0x80) {
        if (--timeout <= 0) return -1;
    }

    return 0;
}