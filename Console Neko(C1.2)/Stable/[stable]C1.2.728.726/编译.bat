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
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c except.c -o except.o
if %errorlevel% neq 0 (echo [x] except.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c keyboard.c -o keyboard.o
if %errorlevel% neq 0 (echo [x] keyboard.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c disk.c -o disk.o
if %errorlevel% neq 0 (echo [x] disk.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c syscall.c -o syscall.o
if %errorlevel% neq 0 (echo [x] syscall.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c fat.c -o fat.o
if %errorlevel% neq 0 (echo [x] fat.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c shell.c -o shell.o
if %errorlevel% neq 0 (echo [x] shell.c 失败！& pause & exit /b 1)

gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c kernel.c -o kernel.o
if %errorlevel% neq 0 (echo [x] kernel.c 失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [4/5] 链接内核...
gcc -m32 -ffreestanding -nostdlib -nostartfiles -T link.ld entry.o kernel.o except.o keyboard.o disk.o syscall.o fat.o shell.o -o kernel.elf
if %errorlevel% neq 0 (echo [x] 链接失败！& pause & exit /b 1)
echo [✓] 完成!

echo.
echo [5/5] 生成镜像...
objcopy -O binary kernel.elf kernel.bin
copy /b boot.bin + kernel.bin os.img > nul

echo 内核大小:
dir kernel.bin | find "kernel.bin"

set IMG_SIZE=0
for %%A in (os.img) do set IMG_SIZE=%%~zA
if %IMG_SIZE% LSS 1474560 (
    set /a PAD_SIZE=1474560 - %IMG_SIZE%
    fsutil file createnew pad.bin %PAD_SIZE% > nul 2>&1
    copy /b os.img + pad.bin os.img /b > nul
    del pad.bin > nul 2>&1
    echo [✓] 镜像已填充到 1.44MB
) else (
    echo [✓] 镜像大小足够
)

del /q *.o *.elf 2>nul
echo [✓] 完成！文件：os.img

echo.
echo ======================================
echo   测试: qemu-system-i386 -fda os.img -hda disk.img -vga std
echo   Ctrl+S Shell ^| Ctrl+Alt+Del 重启
echo ======================================

if not exist disk.img python makedisk.py

qemu-system-i386 -fda os.img -hda harddisk.img -vga std -boot order=ac