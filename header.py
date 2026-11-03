#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
from pathlib import Path

# 期望的版权声明（精确匹配，忽略首尾空白）
EXPECTED_HEADER = """/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */"""

# 常见的头文件扩展名
HEADER_EXTENSIONS = {'.h', '.hpp', '.hh', '.hxx', '.H'}

# 需要跳过的目录（可根据项目调整）
SKIP_DIRS = {
    '.git', '.svn', '.hg',
    'build', 'Build', 'BUILD', 'external',
    'cmake-build-debug', 'cmake-build-release',
    'out', 'bin', 'obj',
    'node_modules', 'venv', '.venv',
    '__pycache__', '.idea', '.vscode'
}


def normalize_text(text: str) -> str:
    """去除首尾空白，并统一换行符"""
    return text.strip().replace('\r\n', '\n').replace('\r', '\n')


def check_header(file_path: Path) -> bool:
    """检查文件是否以期望的版权声明开头"""
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read(2048)  # 只读前 2KB 足够判断

        content = normalize_text(content)
        expected = normalize_text(EXPECTED_HEADER)

        return content.startswith(expected)
    except Exception as e:
        print(f"[错误] 无法读取 {file_path}: {e}")
        return False


def main():
    root = Path('.').resolve()
    print(f"正在检查项目: {root}\n")

    matched = []
    mismatched = []
    errors = []

    for dirpath, dirnames, filenames in os.walk(root):
        # 跳过不需要的目录
        dirnames[:] = [d for d in dirnames if d not in SKIP_DIRS]

        for filename in filenames:
            if Path(filename).suffix in HEADER_EXTENSIONS:
                file_path = Path(dirpath) / filename
                rel_path = file_path.relative_to(root)

                if check_header(file_path):
                    matched.append(str(rel_path))
                else:
                    mismatched.append(str(rel_path))

    # 输出结果
    print("=" * 60)
    print(f"检查完成！共找到 {len(matched) + len(mismatched)} 个头文件\n")

    if matched:
        print(f"✅ 符合要求的头文件 ({len(matched)} 个):")
        for f in sorted(matched):
            print(f"   {f}")
        print()

    if mismatched:
        print(f"❌ 不符合要求的头文件 ({len(mismatched)} 个):")
        for f in sorted(mismatched):
            print(f"   {f}")
        print()
    else:
        print("🎉 所有头文件都符合要求！")

    print("=" * 60)


if __name__ == "__main__":
    main()