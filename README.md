# TBMK 历史版本归档

[![build](https://github.com/TairitsuTeK/TBMK-history-versions/actions/workflows/build.yml/badge.svg)](https://github.com/TairitsuTeK/TBMK-history-versions/actions/workflows/build.yml)

[TBMK（Tiny Bare-metal Micro Kernel）](https://github.com/TairitsuTeK) 历代版本的完整源码归档。仓库保留当年的目录结构，**每个版本目录都有自己的 Makefile，不存在任何 `.bat` / `.sh` 构建脚本**；GitHub Actions 负责全量编译，并在每次 push 到 `main` 时自动发布 Release，同时提供源码包与镜像包。

- Release 资产：`tbmk-sources.zip`（全部版本源码）与 `tbmk-images.zip`（全部镜像）
- 镜像包含 `os.img` / `TBMK.img` / `TMOS.img` 软盘镜像、`kernel.elf`，以及两个 Next 版本的 GRUB `.iso`
- 每个版本的源码、类型与产物见 **[VERSIONS.md](VERSIONS.md)**
- 发布页：<https://github.com/TairitsuTeK/TBMK-history-versions/releases>

## 版本谱系

TBMK 的版本号形如 `C1.2.884.727`：主版本 + 构建号 + 日期（7 月 27 日）。时间线大致如下：

| 世代 | 目录 | 版本数 | 时间 | 技术特征 |
| --- | --- | ---: | --- | --- |
| Kitten Neko 1.2 | `Kitten Neko(1.2)` | 14 | 5 月 20–21 日 | 纯 NASM；16 位引导扇区加载 20 个扇区到 `0x1000`，进入保护模式后跳转 32 位内核；VGA 文本输出 |
| Console Neko C1.0 | `Console Neko(C1.2)/320-332`、`356-404` | 7 | 5 月 22–23 日 | 进入 C 内核时代：NASM 引导 + `link.ld` 链接到 `0x1000`；`window.c` 早期窗口、键盘驱动；MinGW 符号 ABI |
| Console Neko C1.1 | `Console Neko(C1.2)/428-464` | 4 | 5 月 23 日 | 引入 `shell.c`，命令行交互与多窗口 |
| Console Neko C1.2 | `Console Neko(C1.2)/488-536`、`560-716`、`716-884` | 32 | 5 月 24 日–7 月 27 日 | Shell 逐步完善；加入鼠标（572+）、异常处理/IDT（596+）、磁盘与 FAT16（644+）、系统调用（668+）；后期支持 FAT16 测试盘 |
| Console Neko Stable | `Console Neko(C1.2)/Stable` | 5 | 5 月 22 日–7 月 26 日 | 各阶段精选快照 |
| Pixel Neko C2.0 | `Pixel Neko(C2.0)` | 10 | 7 月 28–29 日 | VGA Mode 13h 图形界面：鼠标驱动、窗口管理器、字体渲染、计算器、控制面板；后期加入磁盘/FAT16/文件管理 |
| Pixel Neko Next | `Pixel Neko Next(C2.0 Next)` | 2 | 7 月 29 日、8 月 22 日 | 1001.729 仍是自举软盘镜像（i686-elf 工具链）；1009.822 改为 GRUB Multiboot + ISO，并带 FAT16 磁盘支持 |
| Console Neko Next | `Console Neko Next(C1.2 NEXT)` | 1 | 8 月 21 日 | GRUB Multiboot、ELF 链接到 1 MB、ISO 启动 |
| STABLE | `STABLE` | 3 | — | 纯汇编 1.2、C1.2、C2.0 的定版快照 |

合计 **78 个版本**：15 个纯汇编、61 个 C 内核软盘镜像、2 个 GRUB 版本。

## 自动构建与发布

`.github/workflows/build.yml` 在 push 到 `main`、PR 和手动触发时运行：

1. 安装 `make`、`nasm`、`gcc-multilib`、`binutils`、GRUB 工具和 `zip`；
2. 执行 `make release`：递归编译全部 78 个版本，并生成 `dist/tbmk-sources.zip` 与 `dist/tbmk-images.zip`；
3. 上传 `tbmk-release` workflow artifact；
4. 非 PR 构建会以 `build-<run_number>` 为 tag 创建 GitHub Release，附带上述两个压缩包与 `BUILD-INFO.txt`。

Release 里的 `tbmk-sources.zip` 是当前提交的完整源码树（78 个版本，每个都带 Makefile）；`tbmk-images.zip` 包含 76 个可启动软盘镜像、2 个 GRUB ISO，以及每个版本的 `kernel.elf`。任一版本编译失败都会让整个 workflow 失败。

## 本地构建

依赖：

- GNU Make
- NASM
- 32 位 freestanding 工具链：Linux 用 `gcc-multilib`，或任意平台的 `i686-elf-gcc`
- 构建 GRUB 版本需要 `grub-mkrescue`、`xorriso`、`mtools`
- 打包 Release 需要 `zip`

Linux（Debian/Ubuntu）：

```bash
sudo apt install make nasm gcc-multilib binutils grub-pc-bin grub-common xorriso mtools zip
```

Windows：安装 NASM、GNU Make 与 i686-elf 工具链，把它们的 `bin` 加入 `PATH`。Makefile 的 shell 逻辑（大小写别名、填充）依赖 POSIX 工具；Windows 上建议在 Git Bash / MSYS2 环境中执行，或直接用 WSL。

常用命令：

```bash
make              # 编译所有版本到 dist/<版本目录>/
make sources      # 把当前提交的全部源码打包为 dist/tbmk-sources.zip
make release      # 编译并打包 dist/tbmk-sources.zip + dist/tbmk-images.zip
make list         # 列出 78 个版本
make clean        # 清理 dist/ 与本地 build/
```

单独构建某个版本：

```bash
make -C "Console Neko(C1.2)/560-716/C1.2.716.726" OUT=dist/C1.2.716.726
```

产物布局：`dist/<把目录名中的 / 和空格替换后的 slug>/`，例如
`dist/Console_NekoC1.2__560-716__C1.2.716.726/os.img`。

## 构建适配说明

这些项目当年都在 Windows 上用 MinGW + NASM + 批处理构建。现在每个版本目录的 `Makefile` 直接承担全部构建步骤，并处理了以下跨平台差异：

- **区分 C/C++**：很多源码是大写 `.C`，在区分大小写的 Linux 上会被 GCC 当作 C++ 并改名修饰。Makefile 统一用 `-x c` 强制按 C 编译。
- **大小写不敏感的文件名**：旧代码里常见 `#include "keyboard.h"` 而磁盘上是 `KEYBOARD.H`，Windows 能自动匹配、Linux 不能。Makefile 在 `$(OUT)/.stage` 里为每个大写文件名生成小写别名，源码本身保持原样。
- **MinGW 符号前缀**：旧版的 `KERNEL_ENTRY.ASM` 声明 `_kernel_main`，C 里的 `kernel_main` 在 MinGW 下正好是 `_kernel_main`。ELF 目标没有这个前缀，因此 Makefile 检测到 `_kernel_main` 时追加 `-fleading-underscore`，让源码里的下划线调用（如 `call _keyboard_callback`）保持成立。
- **现代 GCC 的默认行为**：追加 `-fno-pie -fno-stack-protector`，避免 freestanding 链接引用宿主运行库符号。
- **C1.2.632.724 的特殊情况**：`SYSCALL.C` 的 inline asm 调用 `static` 函数，`-Os` 会把未被 C 引用的静态函数删除。该版本的 Makefile 只对该文件使用 `-O0` 保留符号。
- **镜像生成**：`boot.bin + kernel.bin` 后填充到 1.44 MB，保证 BIOS 按固定扇区数读取时不会越界；GRUB 版本使用 `link_grub.ld`，由 `grub-mkrescue` 生成 ISO。
- **无原始构建脚本的版本**：Pixel Neko 896/922/935/947/963/984、STABLE C1.2/C2.0 等快照没有当年的 bat，已按同族相邻版本补上 Makefile 配方。

## 运行

```bash
# 控制台版
qemu-system-i386 -fda dist/Console_NekoC1.2__356-404__C1.0.404.523/os.img

# 纯汇编版
qemu-system-i386 -fda dist/Kitten_Neko1.2__Stable__stable1.2.62.521/TBMK.img

# GRUB 版（由 CI 生成 ISO）
qemu-system-i386 -cdrom dist/Pixel_Neko_NextC2.0_Next__GUI.C2.0.1009.822/pixel_neko.iso -m 128M
```

GUI 的 Next 版本若要测试 FAT16 文件管理，可自己创建一个 FAT16 磁盘镜像（原仓库的 2 GB `disk.img` 未纳入版本库）：

```bash
truncate -s 64M disk.img
mkfs.fat -F 16 disk.img
qemu-system-i386 -cdrom pixel_neko.iso -hda disk.img -m 128M
```

## 验证情况

- 本地（Windows + GNU Make + i686-elf-gcc 15.2，Git Bash shell）：**78/78 全部构建成功**；
- GitHub Actions（Ubuntu + gcc-multilib）：**78/78 全部构建成功**，Release 同时包含源码包与镜像包；
- 抽样 QEMU 冒烟测试（Kitten 1.2.62、Console 404/716、Pixel 984 等）：构建镜像与原始 `os.img` 的 VGA 文本/帧缓冲输出一致；
- 未对所有版本做完整运行时回归：GUI 功能、FAT16 读写等需要人工在 QEMU 中测试。

## 目录结构

```
Console Neko Next(C1.2 NEXT)/    GRUB 控制台内核
Console Neko(C1.2)/              Console 系列（按构建区间分组）
Kitten Neko(1.2)/                纯汇编系列
Pixel Neko Next(C2.0 Next)/      GRUB GUI 内核
Pixel Neko(C2.0)/                GUI 系列
STABLE/                          定版快照
Makefile                         根构建入口（遍历所有版本并打包）
*/Makefile（含多级子目录）        每个版本自己的编译规则
.github/workflows/build.yml      GitHub Actions：构建 + 发布 Release
VERSIONS.md                      78 个版本清单
```

## 版权

代码版权归原作者 **Tairitsu_tty / TairitsuTeK** 所有。本仓库用于历史归档与构建复现，未附加额外许可证；如需转载或复用，请先联系作者。
