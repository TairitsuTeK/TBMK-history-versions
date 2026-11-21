#ifndef MOUSE_H
#define MOUSE_H

void mouse_init(void);
void mouse_handle_byte(unsigned char d);
void mouse_get(int* x, int* y, int* btn);
void mouse_draw(unsigned char* vga);
void mouse_reset(void);

#endif