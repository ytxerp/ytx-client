#!/usr/bin/env python3
"""convert_guard_to_pragma_once.py — 把传统 include guard 转成 #pragma once"""
import sys
import re
from pathlib import Path

IFDEF_RE = re.compile(r'^\s*#\s*(if|ifdef|ifndef)\b')
ENDIF_RE = re.compile(r'^\s*#\s*endif\b')
IFNDEF_RE = re.compile(r'^\s*#\s*ifndef\s+(\w+)\s*$')
DEFINE_RE = re.compile(r'^\s*#\s*define\s+(\w+)\s*$')


def find_guard_start(lines):
    """跳过开头的空行 / 行注释 / 块注释（版权声明），
    定位真正的 #ifndef，并要求它和紧跟的 #define 是同一个宏名。"""
    i, n = 0, len(lines)
    while i < n:
        stripped = lines[i].strip()
        if stripped == '' or stripped.startswith('//'):
            i += 1
            continue
        if stripped.startswith('/*'):
            while i < n and '*/' not in lines[i]:
                i += 1
            i += 1
            continue
        break

    if i + 1 < n:
        m1 = IFNDEF_RE.match(lines[i])
        m2 = DEFINE_RE.match(lines[i + 1])
        if m1 and m2 and m1.group(1) == m2.group(1):
            return i
    return None


def find_matching_endif(lines, start_idx):
    """从 #define 之后开始，用嵌套深度追踪，找到真正配对的 #endif，
    不管它后面有没有跟注释。"""
    depth = 1  # 对应最外层的 #ifndef
    for i in range(start_idx, len(lines)):
        if IFDEF_RE.match(lines[i]):
            depth += 1
        elif ENDIF_RE.match(lines[i]):
            depth -= 1
            if depth == 0:
                return i
    return None


def convert_file(path: Path) -> bool:
    raw = path.read_bytes()
    newline = '\r\n' if b'\r\n' in raw else '\n'
    text = raw.decode('utf-8')

    lines = text.split(newline)
    trailing_newline = lines and lines[-1] == ''
    if trailing_newline:
        lines.pop()

    guard_idx = find_guard_start(lines)
    if guard_idx is None:
        return False  # 没有 guard，跳过

    endif_idx = find_matching_endif(lines, guard_idx + 2)
    if endif_idx is None:
        print(f"WARN: {path}: 未找到匹配的 #endif，跳过", file=sys.stderr)
        return False

    new_lines = (
        lines[:guard_idx]
        + ['#pragma once']
        + lines[guard_idx + 2:endif_idx]   # guard 内部原有内容
        + lines[endif_idx + 1:]
    )

    new_text = newline.join(new_lines)
    if trailing_newline:
        new_text += newline
    path.write_bytes(new_text.encode('utf-8'))
    return True


def main():
    changed = 0
    for filename in sys.argv[1:]:
        path = Path(filename)
        try:
            if convert_file(path):
                changed += 1
                print(f"converted: {path}")
        except Exception as e:
            print(f"ERROR: {path}: {e}", file=sys.stderr)
    print(f"{changed} file(s) converted, 其余未匹配或跳过")


if __name__ == "__main__":
    main()