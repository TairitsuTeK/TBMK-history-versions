#ifndef DISK_H
#define DISK_H

int disk_detect(void);
int disk_read(unsigned int lba, unsigned char* buf);
int disk_write(unsigned int lba, const unsigned char* buf);

// 新增函数
void disk_set_grub_mode(int mode);
int disk_get_grub_mode(void);

#endif