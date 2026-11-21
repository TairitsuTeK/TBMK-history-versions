@echo off
chcp 65001 > nul
title TBMK GUI 编译

:: ===== Build计数器 =====
if not exist build.txt echo 1 > build.txt
for /f %%i in ('powershell -Command "Get-Content build.txt"') do set BUILD=%%i

:: 获取月日
for /f %%i in ('powershell -Command "Get-Date -Format 'Md'"') do set MD=%%i

:: 更新 version.h
echo #ifndef VERSION_H > version.h
echo #define VERSION_H >> version.h
echo #define VER_STRING1       "Codename Pixel Neko NEXT" >> version.h
echo #define KERNEL_NAME       "TBMK GUI Shell" >> version.h
echo #define KERNEL_CODENAME   "Pixel Neko" >> version.h
echo #define KERNEL_VERSION    "C2.0" >> version.h
echo #define KERNEL_BUILD      "%BUILD%.%MD%" >> version.h
echo #define KERNEL_COPYRIGHT  "(C) 2026 Tairitsu_tty" >> version.h
echo #define KERNEL_AUTHOR     "Tairitsu_tty" >> version.h
echo #define VER_STRING2       KERNEL_BUILD >> version.h
echo #define VER_STRING3       KERNEL_COPYRIGHT >> version.h
echo #endif >> version.h

echo Build: %BUILD%.%MD%

del *.o *.bin *.elf *.exe os.img pad.bin 2>nul

nasm -f bin boot.asm -o boot.bin || (pause & exit /b)
nasm -f elf32 kernel_entry.asm -o entry.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c kernel.c -o kernel.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c mouse.c -o mouse.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c font.c -o font.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c wm.c -o wm.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c calc.c -o calc.o || (pause & exit /b)
i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c cpanel.c -o cpanel.o || (pause & exit /b)

i686-elf-ld -T link.ld entry.o kernel.o mouse.o font.o wm.o calc.o cpanel.o -o kernel.elf || (pause & exit /b)

objcopy -O binary kernel.elf kernel.bin
copy /b boot.bin + kernel.bin os.img > nul

set /a BUILD+=1
echo %BUILD% > build.txt

for %%A in (kernel.bin) do echo 内核: %%~zA 字节
for %%A in (os.img) do echo 镜像: %%~zA 字节
del *.o *.elf 2>nul
echo [✓] 完成！

qemu-system-i386 -drive file=os.img,format=raw,if=floppy -vga std -boot a