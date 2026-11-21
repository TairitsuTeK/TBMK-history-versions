# TBMK pure NASM kernel: 16-bit boot sector + 32-bit kernel.
# `make` builds $(OUT)/TMOS.img.

NASM ?= nasm
OUT ?= build
TARGET := TMOS.img

.PHONY: all clean
all: $(OUT)/$(TARGET)

$(OUT):
	@mkdir -p $@

$(OUT)/boot.bin: BOOT.ASM | $(OUT)
	$(NASM) -f bin $< -o $@

$(OUT)/kernel.bin: KERNEL.ASM | $(OUT)
	$(NASM) -f bin $< -o $@

$(OUT)/$(TARGET): $(OUT)/boot.bin $(OUT)/kernel.bin
	@cat $^ > $@

clean:
	rm -rf $(OUT)
