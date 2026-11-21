#ifndef DISK_H
#define DISK_H

void disk_read(unsigned int lba, unsigned char* buf);
void disk_write(unsigned int lba, const unsigned char* buf);

#endif