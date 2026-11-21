#ifndef SYSCALL_H
#define SYSCALL_H

void syscall_handler(void);

// 系统调用号
#define SYS_PRINT  1
#define SYS_CLEAR  2
#define SYS_REBOOT 3
#define SYS_EXEC   4
#define SYS_EXIT   5

// 用户程序入口
void user_program_entry(void);

#endif