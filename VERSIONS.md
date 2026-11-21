# TBMK 历史版本构建清单

- 统计来源：本地 `make` 全量构建 + GitHub Actions 复验
- 构建工具链：`i686-elf-gcc` / `i686-elf-ld` / `nasm`（CI 为 `gcc -m32` + `ld`）
- 结果：**78/78** 构建成功

## 总览

| 世代 | 版本数 | 类型 | 产物 |
| --- | ---: | --- | --- |
| Console Neko Next | 1 | grub | .elf |
| Console Neko | 48 | legacy | .elf, .img |
| Kitten Neko | 14 | asm | .img |
| Pixel Neko Next | 2 | grub, legacy | .elf, .img |
| Pixel Neko | 10 | legacy | .elf, .img |
| STABLE | 3 | asm, legacy | .elf, .img |

类型：`asm` = 纯 NASM 引导扇区 + 内核拼接；`legacy` = NASM 引导 + 32 位 C 内核 + 软盘镜像；`grub` = GRUB Multiboot ELF/ISO。

## Console Neko Next

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| Console Neko Next(C1.2 NEXT)/C1.2.896.821 | grub | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用、Multiboot | kernel.elf (31.3 KB)<br>`*.iso`（CI 生成） |

## Console Neko

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| Console Neko(C1.2)/320-332/C1.0.320.522 | legacy | — | os.img (1.4 MB)<br>kernel.elf (5.0 KB) |
| Console Neko(C1.2)/320-332/C1.0.332.522 | legacy | — | os.img (1.4 MB)<br>kernel.elf (6.6 KB) |
| Console Neko(C1.2)/356-404/C1.0.356.522 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (7.3 KB) |
| Console Neko(C1.2)/356-404/C1.0.368.522 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (8.0 KB) |
| Console Neko(C1.2)/356-404/C1.0.380.522 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (8.8 KB) |
| Console Neko(C1.2)/356-404/C1.0.392.523 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (8.7 KB) |
| Console Neko(C1.2)/356-404/C1.0.404.523 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (8.7 KB) |
| Console Neko(C1.2)/428-464/C1.1.428.523 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (12.6 KB) |
| Console Neko(C1.2)/428-464/C1.1.440.523 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (15.4 KB) |
| Console Neko(C1.2)/428-464/C1.1.452.523 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (15.4 KB) |
| Console Neko(C1.2)/428-464/C1.1.464.523 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (11.7 KB) |
| Console Neko(C1.2)/488-536/C1.2.488.524 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (11.7 KB) |
| Console Neko(C1.2)/488-536/C1.2.500.721 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (11.5 KB) |
| Console Neko(C1.2)/488-536/C1.2.512.721 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (12.0 KB) |
| Console Neko(C1.2)/488-536/C1.2.524.721 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (11.7 KB) |
| Console Neko(C1.2)/488-536/C1.2.536.721 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (12.1 KB) |
| Console Neko(C1.2)/560-716/C1.2.560.724 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (12.9 KB) |
| Console Neko(C1.2)/560-716/C1.2.572.724 | legacy | 窗口、鼠标、Shell | os.img (1.4 MB)<br>kernel.elf (13.7 KB) |
| Console Neko(C1.2)/560-716/C1.2.584.724 | legacy | 窗口、鼠标、Shell | os.img (1.4 MB)<br>kernel.elf (14.0 KB) |
| Console Neko(C1.2)/560-716/C1.2.596.724 | legacy | 窗口、鼠标、Shell、异常处理 | os.img (1.4 MB)<br>kernel.elf (16.3 KB) |
| Console Neko(C1.2)/560-716/C1.2.608.724 | legacy | 鼠标、Shell、异常处理 | os.img (1.4 MB)<br>kernel.elf (14.3 KB) |
| Console Neko(C1.2)/560-716/C1.2.620.724 | legacy | 鼠标、Shell、磁盘、异常处理 | os.img (1.4 MB)<br>kernel.elf (15.2 KB) |
| Console Neko(C1.2)/560-716/C1.2.632.724 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (16.2 KB) |
| Console Neko(C1.2)/560-716/C1.2.644.724 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (16.6 KB) |
| Console Neko(C1.2)/560-716/C1.2.656.724 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (17.8 KB) |
| Console Neko(C1.2)/560-716/C1.2.668.725 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (19.0 KB) |
| Console Neko(C1.2)/560-716/C1.2.680.725 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (19.0 KB) |
| Console Neko(C1.2)/560-716/C1.2.692.725 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (18.3 KB) |
| Console Neko(C1.2)/560-716/C1.2.704.726 | legacy | 鼠标、Shell、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (19.1 KB) |
| Console Neko(C1.2)/560-716/C1.2.716.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (22.8 KB) |
| Console Neko(C1.2)/716-884/C1.2.740.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (22.9 KB) |
| Console Neko(C1.2)/716-884/C1.2.752.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.0 KB) |
| Console Neko(C1.2)/716-884/C1.2.764.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (22.9 KB) |
| Console Neko(C1.2)/716-884/C1.2.776.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.4 KB) |
| Console Neko(C1.2)/716-884/C1.2.788.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.4 KB) |
| Console Neko(C1.2)/716-884/C1.2.800.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.3 KB) |
| Console Neko(C1.2)/716-884/C1.2.812.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.0 KB) |
| Console Neko(C1.2)/716-884/C1.2.824.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (23.3 KB) |
| Console Neko(C1.2)/716-884/C1.2.836.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (24.1 KB) |
| Console Neko(C1.2)/716-884/C1.2.848.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (24.2 KB) |
| Console Neko(C1.2)/716-884/C1.2.860.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (26.2 KB) |
| Console Neko(C1.2)/716-884/C1.2.872.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (26.4 KB) |
| Console Neko(C1.2)/716-884/C1.2.884.727 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (27.1 KB) |
| Console Neko(C1.2)/Stable/[stable]C1.0.344.522 | legacy | — | os.img (1.4 MB)<br>kernel.elf (6.6 KB) |
| Console Neko(C1.2)/Stable/[stable]C1.1.416.523 | legacy | 窗口 | os.img (1.4 MB)<br>kernel.elf (12.6 KB) |
| Console Neko(C1.2)/Stable/[stable]C1.2.476.524 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (11.7 KB) |
| Console Neko(C1.2)/Stable/[stable]C1.2.548.724 | legacy | 窗口、Shell | os.img (1.4 MB)<br>kernel.elf (12.9 KB) |
| Console Neko(C1.2)/Stable/[stable]C1.2.728.726 | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (22.9 KB) |

## Kitten Neko

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| Kitten Neko(1.2)/Beta Stage/1.0.16.520 | asm | 纯汇编 | TMOS.img (1.1 KB) |
| Kitten Neko(1.2)/Beta Stage/1.0.20.520 | asm | 纯汇编 | TMOS.img (1.1 KB) |
| Kitten Neko(1.2)/Beta Stage/1.0.24.520 | asm | 纯汇编 | TBMK.img (1.1 KB) |
| Kitten Neko(1.2)/Beta Stage/1.0.26.520 | asm | 纯汇编 | TMOS.img (1.1 KB) |
| Kitten Neko(1.2)/Beta Stage/1.0.30.520 | asm | 纯汇编 | TMOS.img (1.2 KB) |
| Kitten Neko(1.2)/Delta(Beta 3) Stage/1.2.66.521 | asm | 纯汇编 | TBMK.img (3.2 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.38.521 | asm | 纯汇编 | TBMK.img (1.2 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.42.521 | asm | 纯汇编 | TBMK.img (1.2 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.46.521 | asm | 纯汇编 | TBMK.img (1.3 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.50.521 | asm | 纯汇编 | TBMK.img (1.9 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.54.521 | asm | 纯汇编 | TBMK.img (1.8 KB) |
| Kitten Neko(1.2)/Gamma(Beta 2) Stage/1.1.58.521 | asm | 纯汇编 | TBMK.img (1.8 KB) |
| Kitten Neko(1.2)/Stable/[stable]1.1.34.521 | asm | 纯汇编 | TBMK.img (1.2 KB) |
| Kitten Neko(1.2)/Stable/[stable]1.2.62.521 | asm | 纯汇编 | TBMK.img (2.0 KB) |

## Pixel Neko Next

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| Pixel Neko Next(C2.0 Next)/GUI.C2.0.1001.729 | legacy | 窗口管理、鼠标、计算器、控制面板、字体 | os.img (1.4 MB)<br>kernel.elf (28.8 KB) |
| Pixel Neko Next(C2.0 Next)/GUI.C2.0.1009.822 | grub | 窗口管理、鼠标、FAT16、磁盘、计算器、控制面板、字体、文件管理、Multiboot | kernel.elf (42.0 KB)<br>`*.iso`（CI 生成） |

## Pixel Neko

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| Pixel Neko(C2.0)/GUI.C2.0.885.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (9.6 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.888.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (11.0 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.889.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (11.3 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.890.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (11.3 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.896.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (11.5 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.922.728 | legacy | 鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (14.1 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.935.729 | legacy | 窗口管理、鼠标、字体 | os.img (1.4 MB)<br>kernel.elf (16.6 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.947.729 | legacy | 窗口管理、鼠标、计算器、字体 | os.img (1.4 MB)<br>kernel.elf (19.5 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.963.729 | legacy | 窗口管理、鼠标、计算器、字体 | os.img (1.4 MB)<br>kernel.elf (20.5 KB) |
| Pixel Neko(C2.0)/GUI.C2.0.984.729 | legacy | 窗口管理、鼠标、计算器、字体 | os.img (1.4 MB)<br>kernel.elf (23.6 KB) |

## STABLE

| 版本目录 | 类型 | 关键模块 | 构建产物 |
| --- | --- | --- | --- |
| STABLE/T.B.M.K 1.2 Code | asm | 纯汇编 | TBMK.img (2.0 KB) |
| STABLE/T.B.M.K C1.2 Code | legacy | 鼠标、Shell、FAT16、磁盘、异常处理、系统调用 | os.img (1.4 MB)<br>kernel.elf (27.1 KB) |
| STABLE/T.B.M.K C2.0 Code | legacy | 窗口管理、鼠标、计算器、字体 | os.img (1.4 MB)<br>kernel.elf (23.6 KB) |

## 说明

- 表中路径与仓库目录一一对应；每个版本目录都有自己的 `Makefile`，原始 Windows `编译.bat` 已按 Makefile 化并移除。
- 产物由 `make` / `make release` 生成到 `dist/`，不提交进仓库；GitHub Actions 会在每次推送到 `main` 时重建并发布 Release。
- 旧版 `os.img` 统一填充为 1.44 MB 软盘镜像，便于 `qemu-system-i386 -fda` 启动；GUI 的 Next 版本使用 GRUB ISO。
- Windows 专用步骤（`copy /b`、`fsutil`、`powershell`）以及原先的 Python 辅助脚本已由 Makefile 规则等价实现。
