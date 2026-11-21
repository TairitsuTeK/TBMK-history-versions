#ifndef SHELL_H
#define SHELL_H

void shell_init(void);
void shell_show(void);
void shell_close(void);
int shell_is_open(void);
void shell_input(unsigned char scancode);
void shell_clear(void);

#endif