#ifndef DISK_H
#define DISK_H

int disk_detect(void);
int disk_read(unsigned int lba, unsigned char* buf);
int disk_write(unsigned int lba, const unsigned char* buf);

#endif