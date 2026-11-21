# 批量替换所有文件中的下划线
sed -i 's/_timer_ticks/timer_ticks/g' kernel.c
sed -i 's/_show_exception/show_exception/g' except.c
sed -i 's/_keyboard_callback/keyboard_callback/g' keyboard.c
sed -i 's/_sys_print/sys_print/g' syscall.c
sed -i 's/_sys_clear/sys_clear/g' syscall.c
sed -i 's/_sys_reboot/sys_reboot/g' syscall.c
sed -i 's/_sys_exec/sys_exec/g' syscall.c
sed -i 's/_sys_exit/sys_exit/g' syscall.c

# 清理并重新编译
make clean
make