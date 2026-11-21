#!/usr/bin/env python3
"""Build every TBMK history version from source.

The repository keeps the original Windows batch scripts for reference. This
script is the portable replacement used by GitHub Actions and by local
developers on Linux, macOS, or Windows.

Three build flavours are detected automatically:

* ``grub``   - a version with ``link_grub.ld`` and a Makefile (GRUB ISO).
* ``legacy`` - a version with ``link.ld`` and C sources: NASM boot sector,
               ELF32 entry stub, freestanding C objects, flat binary, padded
               floppy image.
* ``asm``    - a pure NASM version with ``BOOT.ASM`` and ``KERNEL.ASM``.

Usage:
    python3 tools/build.py --list
    python3 tools/build.py --all
    python3 tools/build.py "C1.2.716"
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FLOPPY_SIZE = 1474560


class BuildError(RuntimeError):
    pass


def find_tool(env_name: str, candidates: list[str]) -> str | None:
    explicit = os.environ.get(env_name)
    if explicit:
        return explicit
    for candidate in candidates:
        if shutil.which(candidate):
            return candidate
    return None


def detect_toolchain() -> dict[str, str]:
    cc = find_tool("CC", ["i686-elf-gcc", "gcc", "clang"])
    if cc is None:
        raise BuildError("No C compiler found (install i686-elf-gcc or gcc-multilib)")
    is_elf = "i686-elf" in Path(cc).name
    prefix = str(Path(cc))[: -len(Path(cc).name)]
    elf_prefix = prefix + "i686-elf-"
    ld = find_tool("LD", [elf_prefix + "ld", "ld"] if is_elf else ["ld"])
    objcopy = find_tool("OBJCOPY", [elf_prefix + "objcopy", "objcopy"] if is_elf else ["objcopy"])
    if ld is None or objcopy is None:
        raise BuildError("No linker/objcopy found")
    return {
        "cc": cc,
        "ld": ld,
        "objcopy": objcopy,
        "nasm": find_tool("NASM", ["nasm"]) or "nasm",
        "grub": find_tool("GRUB_MKRESCUE", ["grub-mkrescue"]) or "",
        "is_elf": "1" if is_elf else "0",
    }


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def lower_file_map(directory: Path) -> dict[str, Path]:
    return {p.name.lower(): p for p in directory.iterdir() if p.is_file()}


def discover_versions() -> list[Path]:
    found: list[Path] = []
    for dirpath, dirnames, filenames in os.walk(ROOT):
        current = Path(dirpath)
        dirnames[:] = [d for d in dirnames if d not in {".git", "build", "tools", "__pycache__"}]
        if current == ROOT:
            continue
        files = {name.lower() for name in filenames}
        if "link.ld" in files or "link_grub.ld" in files:
            found.append(current)
        elif "boot.asm" in files and "kernel.asm" in files:
            found.append(current)
    return sorted(found, key=lambda p: str(p.relative_to(ROOT)).lower())


def flavor_of(directory: Path) -> str:
    files = lower_file_map(directory)
    if "link_grub.ld" in files:
        return "grub"
    if "link.ld" in files and any(p.suffix.lower() == ".c" for p in files.values()):
        return "legacy"
    if "boot.asm" in files and "kernel.asm" in files:
        return "asm"
    return "unsupported"


def slug_for(relative: Path) -> str:
    text = str(relative).replace("\\", "/")
    text = re.sub(r"[^A-Za-z0-9._-]+", "_", text)
    text = re.sub(r"_+", "_", text).strip("_")
    return text[:150]


def run(command: list[str], cwd: Path, log) -> None:
    log.write("$ " + " ".join(str(part) for part in command) + "\n")
    log.flush()
    started = time.time()
    proc = subprocess.run(
        command,
        cwd=str(cwd),
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
    )
    log.write(proc.stdout)
    log.flush()
    if proc.returncode != 0:
        tail = "\n".join(proc.stdout.strip().splitlines()[-20:])
        raise BuildError(
            f"command failed ({proc.returncode}, {time.time() - started:.1f}s): "
            f"{' '.join(command[:3])} ...\n{tail}"
        )


def build_asm(directory: Path, out: Path, tools: dict[str, str], log) -> list[Path]:
    staged, originals = stage_sources(directory, out)
    files = root_files_of(originals)
    target_name = "TBMK.img"
    for script in directory.glob("*.bat"):
        text = read_text(script)
        match = re.search(r"(?is)copy\s+/b\s+[^\r\n]*?\s+([A-Za-z0-9_.-]+\.img)", text)
        if match:
            target_name = match.group(1)
            break

    boot_bin = out / "boot.bin"
    kernel_bin = out / "kernel.bin"
    run([tools["nasm"], "-f", "bin", str(staged / files["boot.asm"]), "-o", str(boot_bin)], staged, log)
    run([tools["nasm"], "-f", "bin", str(staged / files["kernel.asm"]), "-o", str(kernel_bin)], staged, log)

    image = out / target_name
    image.write_bytes(boot_bin.read_bytes() + kernel_bin.read_bytes())
    return [image]


def stage_sources(directory: Path, out: Path) -> tuple[Path, list[Path]]:
    """Mirror a version into the build directory with case-folded aliases.

    The Windows-era sources mix cases freely (``#include "keyboard.h"`` for
    ``KEYBOARD.H``) and the original builds relied on a case-insensitive file
    system. The repository keeps the files exactly as they were, so the build
    directory carries a lowercase alias for every uppercase path segment.
    """
    staged = out / "src"
    if staged.exists():
        shutil.rmtree(staged)
    originals: list[Path] = []
    for item in sorted(directory.rglob("*")):
        if not item.is_file():
            continue
        relative = item.relative_to(directory)
        target = staged / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(item, target)
        originals.append(relative)
        lowercase = Path(*[part.lower() for part in relative.parts])
        if lowercase != relative:
            alias = staged / lowercase
            if not alias.exists():
                alias.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(item, alias)
    return staged, originals


def root_files_of(originals: list[Path]) -> dict[str, Path]:
    return {
        relative.name.lower(): relative
        for relative in originals
        if len(relative.parts) == 1
    }


def static_functions_referenced_by_inline_asm(source: Path) -> bool:
    """True when inline assembly calls a static function by name.

    GCC removes unused static functions at -Os, which also removes the symbol
    an inline-asm string refers to (seen in C1.2.632.724/SYSCALL.C). Building
    that single file at -O0 keeps the symbol without touching the source.
    """
    text = read_text(source)
    statics = {
        match.group(1)
        for match in re.finditer(r"\bstatic\b[^\n;{}]*?\b([A-Za-z_]\w*)\s*\(", text)
    }
    for name in statics:
        if re.search(r'"[^"]*\bcall\s+_' + re.escape(name) + r'\b', text, re.S):
            return True
    return False


def build_legacy(directory: Path, out: Path, tools: dict[str, str], log) -> list[Path]:
    staged, originals = stage_sources(directory, out)
    files = root_files_of(originals)
    if "boot.asm" not in files or "kernel_entry.asm" not in files or "link.ld" not in files:
        raise BuildError("legacy build needs BOOT.ASM, KERNEL_ENTRY.ASM and link.ld")
    paths = {name: staged / relative for name, relative in files.items()}

    entry_source = read_text(paths["kernel_entry.asm"])
    leading_underscore = re.search(r"(?i)\b_kernel_main\b", entry_source) is not None

    boot_bin = out / "boot.bin"
    entry_obj = out / "entry.o"
    run([tools["nasm"], "-f", "bin", str(paths["boot.asm"]), "-o", str(boot_bin)], staged, log)
    run([tools["nasm"], "-f", "elf32", str(paths["kernel_entry.asm"]), "-o", str(entry_obj)], staged, log)

    c_sources = sorted(
        (staged / relative for relative in originals
         if len(relative.parts) == 1 and relative.suffix.lower() == ".c"),
        key=lambda p: p.name.lower(),
    )
    if not c_sources:
        raise BuildError("no C sources found")

    flags = [
        "-x", "c",
        "-ffreestanding", "-nostdlib", "-nostartfiles", "-fno-builtin",
        "-fno-pie", "-fno-stack-protector", "-Os",
        f"-I{staged}",
    ]
    if tools["is_elf"] != "1":
        flags.insert(0, "-m32")
    if leading_underscore:
        flags.append("-fleading-underscore")

    objects: list[Path] = [entry_obj]
    for source in c_sources:
        obj = out / (source.stem + ".o")
        file_flags = list(flags)
        if static_functions_referenced_by_inline_asm(source):
            file_flags.append("-O0")
            log.write(f"note: {source.name} keeps static symbols referenced by inline asm (-O0)\n")
        run([tools["cc"], *file_flags, "-c", str(source), "-o", str(obj)], staged, log)
        objects.append(obj)

    kernel_elf = out / "kernel.elf"
    run([tools["ld"], "-m", "elf_i386", "-T", str(paths["link.ld"]), "-o", str(kernel_elf), *map(str, objects)], staged, log)
    kernel_bin = out / "kernel.bin"
    run([tools["objcopy"], "-O", "binary", str(kernel_elf), str(kernel_bin)], directory, log)

    payload = boot_bin.read_bytes() + kernel_bin.read_bytes()
    if len(payload) < FLOPPY_SIZE:
        payload += b"\x00" * (FLOPPY_SIZE - len(payload))
    image = out / "os.img"
    image.write_bytes(payload)
    return [image, kernel_elf]


def build_grub(directory: Path, out: Path, tools: dict[str, str], log) -> list[Path]:
    source_copy = out / "src"
    if source_copy.exists():
        shutil.rmtree(source_copy)
    shutil.copytree(directory, source_copy)

    counter = source_copy / "build_counter.sh"
    version_header = source_copy / "version.h"
    if counter.exists() and version_header.exists():
        bash = shutil.which("bash")
        if bash:
            subprocess.run([bash, str(counter)], cwd=str(source_copy),
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            os.utime(version_header, None)

    make = shutil.which("make") or "make"
    command = [
        make,
        f"CC={tools['cc']}",
        f"LD={tools['ld']}",
        f"NASM={tools['nasm']}",
        "QEMU=true",
    ]
    if tools["grub"]:
        command.append(f"GRUB_MKRESCUE={tools['grub']}")
        command.append("all")
    else:
        command.append("iso/boot/kernel.elf")
    run(command, source_copy, log)

    artifacts: list[Path] = []
    if tools["grub"]:
        for iso in sorted(source_copy.glob("*.iso")):
            target = out / iso.name
            shutil.copy2(iso, target)
            artifacts.append(target)
    kernel = source_copy / "iso" / "boot" / "kernel.elf"
    if kernel.exists():
        target = out / "kernel.elf"
        shutil.copy2(kernel, target)
        artifacts.append(target)
    if not artifacts:
        raise BuildError("Makefile finished but produced no ISO or kernel ELF")
    return artifacts


def build_one(directory: Path, tools: dict[str, str], out_root: Path) -> dict:
    relative = directory.relative_to(ROOT)
    flavor = flavor_of(directory)
    out = out_root / slug_for(relative)
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True, exist_ok=True)
    log = open(out / "build.log", "w", encoding="utf-8")
    started = time.time()
    entry = {
        "path": str(relative).replace("\\", "/"),
        "flavor": flavor,
        "compiler": tools["cc"],
        "status": "failed",
        "artifacts": [],
        "seconds": 0,
    }
    try:
        if flavor == "asm":
            artifacts = build_asm(directory, out, tools, log)
        elif flavor == "legacy":
            artifacts = build_legacy(directory, out, tools, log)
        elif flavor == "grub":
            artifacts = build_grub(directory, out, tools, log)
        else:
            raise BuildError("unsupported project layout")
        entry["status"] = "built"
        entry["artifacts"] = [str(p.relative_to(out_root)).replace("\\", "/") for p in artifacts]
    except Exception as error:  # noqa: BLE001 - report every failing version
        entry["status"] = "failed"
        entry["error"] = str(error)
    finally:
        entry["seconds"] = round(time.time() - started, 2)
        log.close()
    return entry


def main() -> int:
    parser = argparse.ArgumentParser(description="Build TBMK history versions")
    parser.add_argument("selectors", nargs="*", help="substrings of version paths to build")
    parser.add_argument("--all", action="store_true", help="build every discovered version")
    parser.add_argument("--list", action="store_true", help="list discovered versions and exit")
    parser.add_argument("--out", default=str(ROOT / "build"), help="output directory")
    parser.add_argument("--clean", action="store_true", help="remove the output directory and exit")
    args = parser.parse_args()

    out_root = Path(args.out).resolve()
    if args.clean:
        shutil.rmtree(out_root, ignore_errors=True)
        print(f"removed {out_root}")
        return 0

    versions = discover_versions()
    if args.list:
        for version in versions:
            print(f"{flavor_of(version):8} {version.relative_to(ROOT)}")
        print(f"\n{len(versions)} versions")
        return 0

    if not args.all:
        if not args.selectors:
            parser.error("pass --all, --list, or one or more version path substrings")
        versions = [
            version for version in versions
            if any(sel.lower() in str(version.relative_to(ROOT)).lower() for sel in args.selectors)
        ]
        if not versions:
            print("no version matched", file=sys.stderr)
            return 1

    out_root.mkdir(parents=True, exist_ok=True)
    tools = detect_toolchain()
    print(f"toolchain: {tools['cc']} | ld: {tools['ld']} | nasm: {tools['nasm']} | grub: {tools['grub'] or 'missing'}")
    print(f"building {len(versions)} versions into {out_root}\n")

    entries = []
    failures = 0
    for index, version in enumerate(versions, 1):
        label = str(version.relative_to(ROOT))
        print(f"[{index:>3}/{len(versions)}] {label}", end=" ... ", flush=True)
        entry = build_one(version, tools, out_root)
        entries.append(entry)
        print(f"{entry['status']} ({entry['seconds']}s)")
        if entry["status"] != "built":
            failures += 1
            print(f"      {entry.get('error', '')}")

    manifest = {
        "generated_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "toolchain": tools,
        "total": len(entries),
        "failed": failures,
        "versions": entries,
    }
    (out_root / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")

    print(f"\nbuilt {len(entries) - failures}/{len(entries)} versions; {failures} failed")
    print(f"manifest: {out_root / 'manifest.json'}")
    return 1 if failures else 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except BuildError as error:
        print(f"error: {error}", file=sys.stderr)
        sys.exit(2)
