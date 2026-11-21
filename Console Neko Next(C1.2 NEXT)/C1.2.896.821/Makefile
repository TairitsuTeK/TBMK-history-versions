CC = gcc
LD = ld
NASM = nasm
GRUB_MKRESCUE = grub-mkrescue
QEMU = qemu-system-i386

CFLAGS = -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin \
         -fno-stack-protector -fno-pie -mno-mmx -mno-sse -mno-sse2 \
         -Os -Wall -Wextra -I.

LDFLAGS = -m elf_i386
ASMFLAGS = -f elf32

C_SOURCES = kernel.c except.c keyboard.c disk.c syscall.c fat.c shell.c
ASM_SOURCES = boot/multiboot.asm
HEADERS = keyboard.h shell.h except.h syscall.h disk.h fat.h \
          version.h port.h scancode.h vga.h

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

KERNEL_ELF = iso/boot/kernel.elf
ISO_FILE = tbmk_grub.iso

.PHONY: all clean run debug rebuild help

all: $(ISO_FILE)

%.o: %.c $(HEADERS)
	@echo "  CC    $<"
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.asm
	@echo "  NASM  $<"
	$(NASM) $(ASMFLAGS) $< -o $@

$(KERNEL_ELF): $(OBJECTS) link_grub.ld
	@echo "  LD    $@"
	@mkdir -p iso/boot/grub
	$(LD) $(LDFLAGS) -T link_grub.ld $(OBJECTS) -o $@

iso/boot/grub/grub.cfg:
	@mkdir -p iso/boot/grub
	@echo 'set timeout=20' > $@
	@echo 'set default=0' >> $@
	@echo '' >> $@
	@echo 'menuentry "Console Neko Next" {' >> $@
	@echo '    multiboot /boot/kernel.elf' >> $@
	@echo '    boot' >> $@
	@echo '}' >> $@

$(ISO_FILE): $(KERNEL_ELF) iso/boot/grub/grub.cfg
	@echo "  ISO   $@"
	$(GRUB_MKRESCUE) -o $@ iso/

run: $(ISO_FILE)
	$(QEMU) -cdrom $(ISO_FILE) -m 128M

debug: $(ISO_FILE)
	$(QEMU) -cdrom $(ISO_FILE) -m 128M -s -S

clean:
	rm -f $(C_OBJECTS) $(ASM_OBJECTS)
	rm -f $(KERNEL_ELF)
	rm -f $(ISO_FILE)
	rm -rf iso

rebuild: clean all

help:
	@echo "make          - 编译内核"
	@echo "make run      - 编译并运行"
	@echo "make debug    - 调试模式"
	@echo "make clean    - 清理"
	@echo "make rebuild  - 重新编译"