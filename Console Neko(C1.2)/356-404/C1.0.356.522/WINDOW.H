#ifndef WINDOW_H
#define WINDOW_H

// 初始化窗口模块
void window_init(void);

// 显示全屏窗口
void window_show(void);

// 关闭窗口，恢复主界面
void window_close(void);

// 窗口是否显示中
int window_is_open(void);

#endif