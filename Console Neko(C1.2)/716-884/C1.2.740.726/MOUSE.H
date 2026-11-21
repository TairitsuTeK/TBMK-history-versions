#ifndef MOUSE_H
#define MOUSE_H

extern int mouse_x;
extern int mouse_y;
extern unsigned short mouse_backup;
extern int mouse_moved;

void mouse_init(void);
void mouse_handler(void);
void mouse_redraw(void);

#endif