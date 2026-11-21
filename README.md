# TBMK 历史版本归档

[![build](https://github.com/TairitsuTeK/TBMK-history-versions/actions/workflows/build.yml/badge.svg)](https://github.com/TairitsuTeK/TBMK-history-versions/actions/workflows/build.yml)

[TBMK（Tiny Bare-metal Micro Kernel）](https://github.com/TairitsuTeK) 历代版本的完整源码归档。仓库保留当年的目录结构与 Windows `编译.bat`，同时新增一套跨平台构建器和 GitHub Actions，让 **78 个历史版本** 都能在云端自动构建。

- 构建产物：`os.img` / `TBMK.img` 软盘镜像、`kernel.elf`，以及两个 Next 版本的 GRUB `.iso`
- 每个版本的源码、构建类型与产物见 **[VERSIONS.md](VERSIONS.md)**
- 原始源码与脚本未做改动；新增的只是构建层（`tools/`、`.github/`、本 README）

## 版本谱系

TBMK 的版本号形如 `C1.2.884.727`：主版本 + 构建号 + 日期（7 月 27 日）。时间线大致如下：

| 世代 | 目录 | 版本数 | 时间 | 技术特征 |
| --- | --- | ---: | --- | --- |
| Kitten Neko 1.2 | `Kitten Neko(1.2)` | 14 | 5 月 20–21 日 | 纯 NASM；16 位引导扇区加载 20 个扇区到 `0x1000`，进入保护模式后跳转 32 位内核；VGA 文本输出 |
| Console Neko C1.0 | `Console Neko(C1.2)/320-332`、`356-404` | 7 | 5 月 22–23 日 | 进入 C 内核时代：NASM 引导 + `link.ld` 链接到 `0x1000`；`window.c` 早期窗口、键盘驱动；MinGW 符号 ABI |
| Console Neko C1.1 | `Console Neko(C1.2)/428-464` | 4 | 5 月 23 日 | 引入 `shell.c`，命令行交互与多窗口 |
| Console Neko C1.2 | `Console Neko(C1.2)/488-536`、`560-716`、`716-884` | 32 | 5 月 24 日–7 月 27 日 | Shell 逐步完善；加入鼠标（572+）、异常处理/IDT（596+）、磁盘与 FAT16（644+）、系统调用（668+）；后期配套 Python FAT16 测试盘 |
| Console Neko Stable | `Console Neko(C1.2)/Stable` | 5 | 5 月 22 日–7 月 26 日 | 各阶段精选快照 |
| Pixel Neko C2.0 | `Pixel Neko(C2.0)` | 10 | 7 月 28–29 日 | VGA Mode 13h 图形界面：鼠标驱动、窗口管理器、字体渲染、计算器、控制面板；后期加入磁盘/FAT16/文件管理 |
| Pixel Neko Next | `Pixel Neko Next(C2.0 Next)` | 2 | 7 月 29 日、8 月 22 日 | 1001.729 仍是自举软盘镜像（i686-elf 工具链）；1009.822 改为 GRUB Multiboot + ISO，并带构建号计数器与 FAT16 磁盘支持 |
| Console Neko Next | `Console Neko Next(C1.2 NEXT)` | 1 | 8 月 21 日 | GRUB Multiboot、ELF 链接到 1 MB、ISO 启动 |
| STABLE | `STABLE` | 3 | — | 纯汇编 1.2、C1.2、C2.0 的定版快照 |

合计 **78 个版本**：15 个纯汇编、61 个 C 内核软盘镜像、2 个 GRUB 版本。

## 自动构建

`.github/workflows/build.yml` 在每次 push、PR 和手动触发时：

1. 安装 `nasm`、`gcc-multilib`、`binutils`、GRUB 工具；
2. 运行 `python3 tools/build.py --all`；
3. 把镜像、ELF、ISO、构建日志和 `build/manifest.json` 作为 **`tbmk-images`** artifact 上传。

构建失败会直接让 workflow 失败，并在日志中列出具体版本与编译器输出；artifact 仍然会以 `if: always()` 上传，便于排查。

## 本地构建

依赖：

- Python 3.9+
- NASM
- 32 位 freestanding 工具链：Linux 用 `gcc-multilib`，或任意平台的 `i686-elf-gcc`
- 构建 GRUB 版本需要 `grub-mkrescue`、`xorriso`、`mtools`

Linux（Debian/Ubuntu）：

```bash
sudo apt install nasm gcc-multilib binutils grub-pc-bin grub-common xorriso mtools python3
```

Windows：安装 NASM、Python 与 i686-elf 工具链（构建器会自动优先使用 `i686-elf-gcc`），例如把工具链的 `bin` 加入 `PATH`。

常用命令：

```bash
python3 tools/build.py --list          # 列出 78 个版本与构建类型
python3 tools/build.py --all           # 全量构建到 build/
python3 tools/build.py C1.2.716        # 只构建路径中包含该子串的版本
python3 tools/build.py --clean         # 清理 build/
python3 tools/report.py                # 重新生成 VERSIONS.md
make all                               # 等价于 --all
```

产物布局：`build/<版本目录 slug>/`，例如
`build/Console_Neko_C1.2_560-716_C1.2.716.726/os.img`。

## 构建适配说明

这些项目当年都在 Windows 上用 MinGW + NASM + 批处理构建。为在 Linux CI 上复现，同时不改动历史源码，构建器做了以下处理：

- **区分 C/C++**：很多源码是大写 `.C`，在区分大小写的 Linux 上会被 GCC 当作 C++ 并改名修饰。构建器统一用 `-x c` 强制按 C 编译。
- **MinGW 符号前缀**：旧版的 `KERNEL_ENTRY.ASM` 声明 `_kernel_main`，C 里的 `kernel_main` 在 MinGW 下正好是 `_kernel_main`。ELF 目标没有这个前缀，因此对这类版本追加 `-fleading-underscore`，让源码里的下划线调用（如 `call _keyboard_callback`）保持成立。
- **现代 GCC 的默认行为**：追加 `-fno-pie -fno-stack-protector`，避免 freestanding 链接引用宿主运行库符号。
- **C1.2.632.724 的特殊情况**：`SYSCALL.C` 的 inline asm 调用 `static` 函数，`-Os` 会把未被 C 引用的静态函数删除。构建器只对该文件使用 `-O0` 保留符号。
- **镜像生成**：`boot.bin + kernel.bin` 后填充到 1.44 MB，保证 BIOS 按固定扇区数读取时不会越界；GRUB 版本沿用仓库自带的 Makefile 和 `link_grub.ld`，由 `grub-mkrescue` 生成 ISO。
- **无构建脚本的版本**：Pixel Neko 896/922/935/947/963/984、STABLE C1.2/C2.0 等快照没有当年的 bat，构建器按同族相邻版本复原了编译配方。

## 运行

```bash
# 控制台版
qemu-system-i386 -fda build/Console_Neko_C1.2_356-404_C1.0.404.523/os.img

# 纯汇编版
qemu-system-i386 -fda build/Kitten_Neko_1.2_Stable_stable_1.2.62.521/TBMK.img

# GRUB 版（CI 产物）
qemu-system-i386 -cdrom build/Pixel_Neko_Next_C2.0_Next_GUI.C2.0.1009.822/pixel_neko.iso -m 128M
```

GUI 的 Next 版本若要测试 FAT16 文件管理，原仓库里还有一个 2 GB 的 `disk.img`（未纳入版本库）。可在对应版本目录运行：

```bash
bash create_disk.sh 64      # 需要 qemu-img 与 mkfs.fat
qemu-system-i386 -cdrom pixel_neko.iso -hda disk.img -m 128M
```

## 验证情况

- 本地（Windows + i686-elf-gcc 15.2）：**78/78 全部构建成功**；
- 抽样 QEMU 冒烟测试（Kitten 1.2.62、Console 404/716、Pixel 984 等）：构建镜像与原始 `os.img` 的 VGA 文本/帧缓冲输出一致；
- CI 使用 Ubuntu `gcc -m32` + `-fleading-underscore` 的等价路径，首次 push 后由 Actions 实际验证；
- 未对所有版本做完整运行时回归：GUI 功能、FAT16 读写等需要人工在 QEMU 中测试。

## 目录结构

```
Console Neko Next(C1.2 NEXT)/    GRUB 控制台内核
Console Neko(C1.2)/              Console 系列（按构建区间分组）
Kitten Neko(1.2)/                纯汇编系列
Pixel Neko Next(C2.0 Next)/      GRUB GUI 内核
Pixel Neko(C2.0)/                GUI 系列
STABLE/                          定版快照
tools/build.py                   跨平台构建器
tools/report.py                  生成 VERSIONS.md
.github/workflows/build.yml      GitHub Actions
VERSIONS.md                      78 个版本清单
```

## 版权

代码版权归原作者 **Tairitsu_tty / TairitsuTeK** 所有。本仓库用于历史归档与构建复现，未附加额外许可证；如需转载或复用，请先联系作者。
