@echo off
chcp 65001 > nul
title TairitsuOS GUI 编译

echo ======================================
echo     TairitsuOS GUI 一键编译
echo ======================================

del *.o *.bin *.elf os.img pad.bin 2>nul

echo [1/5] 编译引导扇区...
nasm -f bin boot.asm -o boot.bin
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

echo [2/5] 编译内核入口...
nasm -f elf32 kernel_entry.asm -o entry.o
if %errorlevel% neq 0 (echo [x] 失败！& pause & exit /b 1)

echo [3/5] 编译C源文件...
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c kernel.c -o kernel.o
if %errorlevel% neq 0 (echo [x] kernel.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c mouse.c -o mouse.o
if %errorlevel% neq 0 (echo [x] mouse.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c font.c -o font.o
if %errorlevel% neq 0 (echo [x] font.c 失败！& pause & exit /b 1)

echo [4/5] 链接内核...
gcc -m32 -ffreestanding -nostdlib -nostartfiles -T link.ld entry.o kernel.o mouse.o font.o -o kernel.elf
if %errorlevel% neq 0 (echo [x] 链接失败！& pause & exit /b 1)

echo [5/5] 生成镜像...
objcopy -O binary kernel.elf kernel.bin
copy /b boot.bin + kernel.bin os.img > nul

for %%A in (kernel.bin) do echo 内核大小: %%~zA 字节

del *.o *.elf 2>nul
echo [✓] 完成！

qemu-system-i386 -drive file=os.img,format=raw,if=floppy -vga std -boot a