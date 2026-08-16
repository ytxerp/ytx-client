#!/usr/bin/env python3
"""convert_guard_to_pragma_once.py — 把传统 include guard 转成 #pragma once"""

import argparse
import re
import sys
from pathlib import Path

# 预处理指令
IF_RE = re.compile(r"^\s*#\s*if\b")
IFDEF_RE = re.compile(r"^\s*#\s*ifdef\b")
IFNDEF_RE = re.compile(r"^\s*#\s*ifndef\b")
ENDIF_RE = re.compile(r"^\s*#\s*endif\b")

# 标准 include guard
GUARD_RE = re.compile(r"^\s*#\s*ifndef\s+(\w+)\s*$")
DEFINE_RE = re.compile(r"^\s*#\s*define\s+(\w+)\s*$")

SOURCE_EXTENSIONS = {".h", ".hpp"}


def find_guard_start(lines):
    """查找标准 include guard：#ifndef XXX 紧跟 #define XXX。"""
    for i in range(len(lines) - 1):
        guard = GUARD_RE.fullmatch(lines[i])
        define = DEFINE_RE.fullmatch(lines[i + 1])

        if guard and define and guard.group(1) == define.group(1):
            return i

    return None


def find_matching_endif(lines, start):
    """找到与最外层 include guard 对应的 #endif。"""
    depth = 1

    for i in range(start, len(lines)):
        line = lines[i]

        if (
            IF_RE.match(line)
            or IFDEF_RE.match(line)
            or IFNDEF_RE.match(line)
        ):
            depth += 1
        elif ENDIF_RE.match(line):
            depth -= 1
            if depth == 0:
                return i

    return None


def convert_file(path: Path, dry_run=False):
    """转换单个文件。成功转换返回 True，否则返回 False。"""
    raw = path.read_bytes()

    # 已经使用 pragma once，无需处理。
    if b"#pragma once" in raw:
        return False

    newline = "\r\n" if b"\r\n" in raw else "\n"
    text = raw.decode("utf-8")

    lines = text.split(newline)

    trailing_newline = bool(lines and lines[-1] == "")
    if trailing_newline:
        lines.pop()

    guard_start = find_guard_start(lines)
    if guard_start is None:
        return False

    guard_end = find_matching_endif(lines, guard_start + 2)
    if guard_end is None:
        print(
            f"WARN: {path}: 未找到匹配的 #endif，跳过",
            file=sys.stderr,
        )
        return False

    new_lines = (
        lines[:guard_start]
        + ["#pragma once"]
        + lines[guard_start + 2 : guard_end]
        + lines[guard_end + 1 :]
    )

    new_text = newline.join(new_lines)

    if trailing_newline:
        new_text += newline

    if dry_run:
        print(f"would convert: {path}")
    else:
        path.write_bytes(new_text.encode("utf-8"))
        print(f"converted: {path}")

    return True


def iter_files(paths):
    """遍历命令行指定的文件和目录。"""
    for path in paths:
        if path.is_file():
            if path.suffix in SOURCE_EXTENSIONS:
                yield path
            continue

        if path.is_dir():
            for file in path.rglob("*"):
                if file.is_file() and file.suffix in SOURCE_EXTENSIONS:
                    yield file
            continue

        print(
            f"WARN: {path}: 文件或目录不存在，跳过",
            file=sys.stderr,
        )


def main():
    parser = argparse.ArgumentParser(
        description="把传统 include guard 转换成 #pragma once"
    )
    parser.add_argument(
        "paths",
        nargs="+",
        type=Path,
        help="要处理的文件或目录",
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="只显示将要转换的文件，不修改文件",
    )

    args = parser.parse_args()

    changed = 0

    for path in iter_files(args.paths):
        try:
            if convert_file(path, args.dry_run):
                changed += 1
        except UnicodeDecodeError as e:
            print(
                f"ERROR: {path}: UTF-8 解码失败: {e}",
                file=sys.stderr,
            )
        except OSError as e:
            print(
                f"ERROR: {path}: {e}",
                file=sys.stderr,
            )

    action = "would convert" if args.dry_run else "converted"
    print(f"{changed} file(s) {action}")


if __name__ == "__main__":
    main()