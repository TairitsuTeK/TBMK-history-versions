# Console Neko Next — C1.2.896.821

**版本目录**：`Console Neko Next(C1.2 NEXT)/C1.2.896.821`  
**构建类型**：GRUB Multiboot 内核  
**版本日期**：8 月 21 日  
**构建号**：896  
**阶段**：—  
**源码规模**：8 个 C 文件、1 个汇编文件、约 2585 行  
**主要产物**：`tbmk_grub.iso`

## 这一版的特色

- FAT16 文件系统；Shell；异常处理；磁盘驱动；系统调用；键盘驱动；鼠标驱动在本家族中首次出现。
- 本版包含：FAT16 文件系统、Shell、异常处理、磁盘驱动、系统调用、键盘驱动、鼠标驱动。
- 使用 GRUB 引导，可由 grub-mkrescue 直接生成 ISO。

## 相对上一版

- 这是 Console Neko Next 的首个版本，建立引导与内核骨架。

## 技术参数

| 项目 | 值 |
| --- | --- |
| 引导方式 | GRUB Multiboot → ELF 链接到 1 MB |
| 内核链接地址 | `1M (0x100000)` |
| 引导加载扇区数 | — |
| 图形/控制台 | VGA 文本控制台 |
| 模块 | FAT16 文件系统、Shell、异常处理、磁盘驱动、系统调用、键盘驱动、鼠标驱动 |

## 构建与运行

```bash
make -C "Console Neko Next(C1.2 NEXT)/C1.2.896.821"
qemu-system-i386 -cdrom "dist/Console_Neko_NextC1.2_NEXT__C1.2.896.821/tbmk_grub.iso" -m 128M
```

产物位于 `dist/Console_Neko_NextC1.2_NEXT__C1.2.896.821/`。

## 家族背景

Console Neko Next 把控制台内核迁移到 GRUB Multiboot：ELF 链接到 1 MB，由 grub-mkrescue 生成 ISO，不再依赖自举软盘。

> 本说明由历史源码自动分析生成：模块变化来自文件名对比，规模来自源码行数，用于版本定位与取舍参考。
