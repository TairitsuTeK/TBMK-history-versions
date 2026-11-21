#ifndef WM_H
#define WM_H

void wm_init(void);
void wm_update(unsigned char* v, int mx, int my, int mb, int mb_press, int mb_release);
void wm_draw(unsigned char* v);
int wm_is_dragging(void);
int wm_need_mouse_reset(void);

#endif