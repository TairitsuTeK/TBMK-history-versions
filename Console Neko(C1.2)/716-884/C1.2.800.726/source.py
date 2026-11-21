import os

# 要收集的源文件列表
files = [
    "boot.asm",
    "kernel_entry.asm",
    "kernel.c",
    "keyboard.c",
    "keyboard.h",
    "shell.c",
    "shell.h",
    "except.c",
    "except.h",
    "syscall.c",
    "syscall.h",
    "disk.c",
    "disk.h",
    "version.h",
    "link.ld",
    "fat.c",       # ← 加逗号
    "fat.h",       # ← 加逗号
    "编译.bat",
]

output = open("TBMK_source.txt", "w", encoding="utf-8")

for fname in files:
    if os.path.exists(fname):
        output.write(f"\n{'='*60}\n")
        output.write(f"[{fname}]\n")
        output.write(f"{'='*60}\n\n")
        with open(fname, "r", encoding="utf-8", errors="ignore") as f:
            output.write(f.read())
        output.write("\n")
    else:
        output.write(f"\n[{fname}] - NOT FOUND\n\n")

output.close()
print("Done! Saved to TBMK_source.txt")