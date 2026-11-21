@echo off
chcp 65001 > nul
title TBMK GUI 编译

del *.o *.bin *.elf os.img pad.bin 2>nul

nasm -f bin boot.asm -o boot.bin || (pause & exit /b)
nasm -f elf32 kernel_entry.asm -o entry.o || (pause & exit /b)
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c kernel.c -o kernel.o || (pause & exit /b)
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c mouse.c -o mouse.o || (pause & exit /b)
gcc -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin -Os -c font.c -o font.o || (pause & exit /b)
gcc -m32 -ffreestanding -nostdlib -nostartfiles -T link.ld entry.o kernel.o mouse.o font.o -o kernel.elf || (pause & exit /b)
objcopy -O binary kernel.elf kernel.bin
copy /b boot.bin + kernel.bin os.img > nul

for %%A in (kernel.bin) do echo 内核: %%~zA 字节
del *.o *.elf 2>nul
echo [✓] 完成！

qemu-system-i386 -drive file=os.img,format=raw,if=floppy -vga std -boot a