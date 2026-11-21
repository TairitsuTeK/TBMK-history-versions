import os

# 要收集的源文件列表
files = [
    "boot.asm",
    "kernel_entry.asm",
    "kernel.c",
    "link.ld",
    "port.h",
    "mouse.h",
    "mouse.c",
    "version.h",
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