#ifndef WINDOW_H
#define WINDOW_H

void window_init(void);
void window_show(void);
void window_close(void);
int window_is_open(void);
void window_input(unsigned char scancode);

#endif