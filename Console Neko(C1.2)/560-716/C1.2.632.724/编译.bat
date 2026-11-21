@echo off
chcp 65001 > nul
title TairitsuOS 一键编译

echo ======================================
echo       TairitsuOS 一键编译脚本
echo ======================================

if exist *.o del /q *.o
if exist *.bin del /q *.bin
if exist *.elf del /q *.elf
if exist os.img del /q os.img

echo.
echo [1/5] 编译引导扇区...
nasm -f bin boot.asm -o boot.bin
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [2/5] 编译内核入口汇编...
nasm -f elf32 kernel_entry.asm -o entry.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [3/5] 编译C源文件...
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c except.c -o except.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c keyboard.c -o keyboard.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c disk.c -o disk.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c syscall.c -o syscall.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c shell.c -o shell.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -O0 -c kernel.c -o kernel.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [4/5] 链接内核...
gcc -m32 -ffreestanding -nostdlib -nostartfiles -T link.ld entry.o kernel.o except.o keyboard.o disk.o syscall.o shell.o -o kernel.elf
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [5/5] 生成镜像...
objcopy -O binary kernel.elf kernel.bin
copy /b boot.bin + kernel.bin os.img > nul
del /q *.o *.elf 2>nul
echo [✓] 完成！文件：os.img

echo.
echo ======================================
echo   测试: qemu-system-i386 -fda os.img -hda disk.img
echo   Ctrl+S Shell ^| Ctrl+Alt+Del 重启
echo ======================================
pause