# TBMK legacy C kernel: NASM boot sector + 32-bit freestanding C kernel.
# `make` builds $(OUT)/os.img (1.44 MB floppy) and $(OUT)/kernel.elf.

CC ?= gcc
LD ?= ld
NASM ?= nasm
OBJCOPY ?= objcopy
OUT ?= build
STAGE := $(OUT)/.stage

BOOT_ASM := $(firstword $(wildcard BOOT.ASM boot.asm))
ENTRY_ASM := $(firstword $(wildcard KERNEL_ENTRY.ASM kernel_entry.asm))
C_FILES := $(wildcard *.c) $(wildcard *.C)
C_OBJS := $(patsubst %,$(OUT)/%.o,$(basename $(notdir $(C_FILES))))
SOURCE_FILES := $(wildcard *.c *.C *.h *.H *.asm *.ASM *.inc *.INC *.ld)

CFLAGS := -x c -ffreestanding -nostdlib -nostartfiles -fno-builtin \
          -fno-pie -fno-stack-protector -Os -I$(STAGE)
ifneq ($(findstring i686-elf,$(CC)),i686-elf)
CFLAGS += -m32
endif
ifneq ($(wildcard $(ENTRY_ASM)),)
CFLAGS += $(shell grep -qi '_kernel_main' $(ENTRY_ASM) 2>/dev/null && echo -fleading-underscore)
endif

.PHONY: all clean
all: $(OUT)/os.img $(OUT)/kernel.elf

$(OUT):
	@mkdir -p $@

# The Windows-era tree relies on a case-insensitive file system for includes
# such as "keyboard.h" -> KEYBOARD.H. Mirror the sources with lowercase
# aliases so Linux compilers resolve the same headers.
$(STAGE)/.stamp: $(SOURCE_FILES) | $(OUT)
	@rm -rf $(STAGE)
	@mkdir -p $(STAGE)
	@cp $(SOURCE_FILES) $(STAGE)/ 2>/dev/null || true
	@cd $(STAGE) && for f in *; do l=$$(echo "$$f" | tr 'A-Z' 'a-z'); \
	  if [ "$$f" != "$$l" ] && [ ! -e "$$l" ]; then ln -s "$$f" "$$l"; fi; done
	@touch $@

$(OUT)/boot.bin: $(BOOT_ASM) | $(STAGE)/.stamp
	$(NASM) -f bin $< -o $@

$(OUT)/entry.o: $(ENTRY_ASM) | $(STAGE)/.stamp
	$(NASM) -f elf32 $< -o $@

$(OUT)/%.o: %.c | $(STAGE)/.stamp
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/%.o: %.C | $(STAGE)/.stamp
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/kernel.elf: $(OUT)/entry.o $(C_OBJS) link.ld | $(STAGE)/.stamp
	$(LD) -m elf_i386 -T link.ld $(OUT)/entry.o $(C_OBJS) -o $@

$(OUT)/kernel.bin: $(OUT)/kernel.elf
	$(OBJCOPY) -O binary $< $@

$(OUT)/os.img: $(OUT)/boot.bin $(OUT)/kernel.bin
	@cat $^ > $@
	@truncate -s 1474560 $@

clean:
	rm -rf $(OUT)
