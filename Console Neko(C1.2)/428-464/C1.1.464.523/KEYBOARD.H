#ifndef KEYBOARD_H
#define KEYBOARD_H

// 初始化键盘中断
void keyboard_init(void);

// 键盘回调（由汇编调用）
void keyboard_callback(void);

#endif