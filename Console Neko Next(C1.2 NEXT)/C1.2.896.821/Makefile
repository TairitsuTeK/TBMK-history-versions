# TBMK GRUB Multiboot kernel: ELF at 1 MB + ISO when grub-mkrescue exists.
# `make` builds $(OUT)/tbmk_grub.iso (or only kernel.elf without GRUB tools).

CC ?= gcc
LD ?= ld
NASM ?= nasm
GRUB_MKRESCUE ?= grub-mkrescue
OUT ?= build

C_SOURCES := kernel.c except.c keyboard.c disk.c syscall.c fat.c shell.c
ASM_SOURCES := boot/multiboot.asm
O_FILES := $(addprefix $(OUT)/obj/,$(C_SOURCES:.c=.o) $(ASM_SOURCES:.asm=.o))

CFLAGS := -m32 -ffreestanding -nostdlib -nostartfiles -fno-builtin \
          -fno-stack-protector -fno-pie -mno-mmx -mno-sse -mno-sse2 \
          -Os -Wall -Wextra -I.
LDFLAGS := -m elf_i386

GRUB_CFG := $(OUT)/iso/boot/grub/grub.cfg
GRUB_KERNEL := $(OUT)/iso/boot/kernel.elf
ISO := $(OUT)/tbmk_grub.iso
HAVE_GRUB := $(shell command -v $(GRUB_MKRESCUE) >/dev/null 2>&1 && echo yes)

.PHONY: all clean

ifeq ($(HAVE_GRUB),yes)
all: $(ISO)
else
all: $(OUT)/kernel.elf
endif

$(OUT):
	@mkdir -p $@

$(OUT)/obj/%.o: %.c | $(OUT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/obj/%.o: %.asm | $(OUT)
	@mkdir -p $(dir $@)
	$(NASM) -f elf32 $< -o $@

$(OUT)/kernel.elf: $(O_FILES) link_grub.ld | $(OUT)
	$(LD) $(LDFLAGS) -T link_grub.ld $(O_FILES) -o $@

$(GRUB_KERNEL): $(OUT)/kernel.elf
	@mkdir -p $(dir $@)
	@cp $< $@

$(GRUB_CFG):
	@mkdir -p $(dir $@)
	@printf '%s\n' 'set timeout=20' 'set default=0' '' \
	  'menuentry "Console Neko Next" {' '    multiboot /boot/kernel.elf' '    boot' '}' > $@

$(ISO): $(GRUB_KERNEL) $(GRUB_CFG)
	$(GRUB_MKRESCUE) -o $@ $(OUT)/iso

clean:
	rm -rf $(OUT)

