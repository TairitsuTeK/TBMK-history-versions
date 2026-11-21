#!/usr/bin/env python3
"""Generate VERSIONS.md from a finished build and the source tree."""

from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build"

FEATURES = [
    ("window.c", "窗口"),
    ("wm.c", "窗口管理"),
    ("mouse.c", "鼠标"),
    ("shell.c", "Shell"),
    ("fat.c", "FAT16"),
    ("disk.c", "磁盘"),
    ("calc.c", "计算器"),
    ("cpanel.c", "控制面板"),
    ("font.c", "字体"),
    ("filemanager.c", "文件管理"),
    ("except.c", "异常处理"),
    ("syscall.c", "系统调用"),
]

FAMILY_ORDER = [
    "Console Neko Next",
    "Console Neko",
    "Kitten Neko",
    "Pixel Neko Next",
    "Pixel Neko",
    "STABLE",
]


def family_of(path: str) -> str:
    top = path.split("/")[0]
    return re.sub(r"\([^)]*\)\s*$", "", top).strip()


def human_size(size: int) -> str:
    if size >= 1024 * 1024:
        return f"{size / 1024 / 1024:.1f} MB"
    if size >= 1024:
        return f"{size / 1024:.1f} KB"
    return f"{size} B"


def features_of(directory: Path) -> list[str]:
    names = {p.name.lower() for p in directory.rglob("*") if p.is_file()}
    found = [label for needle, label in FEATURES if needle in names]
    if not any(name.endswith(".c") for name in names) and "kernel.asm" in names:
        found.insert(0, "纯汇编")
    if "multiboot.asm" in names:
        found.append("Multiboot")
    return found


def main() -> None:
    manifest = json.loads((BUILD / "manifest.json").read_text(encoding="utf-8"))
    entries = manifest["versions"]
    lines = [
        "# TBMK 历史版本构建清单",
        "",
        f"- 生成时间（UTC）：{manifest['generated_at']}",
        f"- 构建工具链：`{manifest['toolchain']['cc']}` / `{manifest['toolchain']['ld']}` / `nasm`",
        f"- 结果：**{manifest['total'] - manifest['failed']}/{manifest['total']}** 构建成功",
        "",
        "## 总览",
        "",
        "| 世代 | 版本数 | 类型 | 产物 |",
        "| --- | ---: | --- | --- |",
    ]

    families: dict[str, list[dict]] = {}
    for entry in entries:
        families.setdefault(family_of(entry["path"]), []).append(entry)

    for family in FAMILY_ORDER:
        group = families.get(family, [])
        if not group:
            continue
        flavors = sorted({item["flavor"] for item in group})
        artifacts = sorted({Path(a).suffix for item in group for a in item.get("artifacts", [])})
        lines.append(
            f"| {family} | {len(group)} | {', '.join(flavors)} | {', '.join(artifacts)} |"
        )

    lines += [
        "",
        "类型：`asm` = 纯 NASM 引导扇区 + 内核拼接；`legacy` = NASM 引导 + 32 位 C 内核 + 软盘镜像；"
        "`grub` = GRUB Multiboot ELF/ISO。",
        "",
    ]

    for family in FAMILY_ORDER:
        group = families.get(family, [])
        if not group:
            continue
        lines += [f"## {family}", ""]
        lines += ["| 版本目录 | 类型 | 关键模块 | 构建产物 |", "| --- | --- | --- | --- |"]
        for entry in sorted(group, key=lambda item: item["path"].lower()):
            relative = entry["path"]
            directory = ROOT / relative
            tags = "、".join(features_of(directory)) or "—"
            artifacts = []
            for artifact in entry.get("artifacts", []):
                path = BUILD / artifact
                if path.exists():
                    artifacts.append(f"{path.name} ({human_size(path.stat().st_size)})")
            if entry["flavor"] == "grub" and not any(a.endswith(".iso") for a in entry.get("artifacts", [])):
                artifacts.append("`*.iso`（CI 生成）")
            artifact_text = "<br>".join(artifacts) if artifacts else "—"
            status = "" if entry["status"] == "built" else " ❌"
            lines.append(
                f"| {relative}{status} | {entry['flavor']} | {tags} | {artifact_text} |"
            )
        lines.append("")

    lines += [
        "## 说明",
        "",
        "- 表中路径与仓库目录一一对应；每个版本目录都保留了原始源码与当年的 Windows `编译.bat`。",
        "- 产物由 `tools/build.py` 重新生成，未提交进仓库；GitHub Actions 会在每次推送时重建。",
        "- 旧版 `os.img` 统一填充为 1.44 MB 软盘镜像，便于 `qemu-system-i386 -fda` 启动；"
        "GUI 的 Next 版本使用 GRUB ISO。",
        "- 原始构建脚本中的 Windows 专用步骤（`copy /b`、`fsutil`、`powershell`）已由跨平台构建器等价实现。",
        "",
    ]

    (ROOT / "VERSIONS.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"wrote {ROOT / 'VERSIONS.md'} ({len(entries)} versions)")


if __name__ == "__main__":
    main()
