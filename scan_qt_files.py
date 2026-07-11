#!/usr/bin/env python3
import os
import re

# Repository root directory — script is run from the project root
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
# Target CMakeLists.txt to update in place
CMAKE_FILE = os.path.join(ROOT_DIR, "CMakeLists.txt")
# File extensions to scan
SOURCE_EXTS = [".cc", ".cpp"]
HEADER_EXTS = [".h", ".hpp"]
UI_EXTS = [".ui"]
# Directories to exclude from scanning
EXCLUDE_DIRS = {"external", "build", ".git"}


def collect_files():
    sources, headers, uis = [], [], []
    for root, dirs, files in os.walk(ROOT_DIR):
        dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]
        for file in files:
            path = os.path.relpath(os.path.join(root, file), ROOT_DIR).replace("\\", "/")
            ext = os.path.splitext(file)[1].lower()
            if ext in SOURCE_EXTS:
                sources.append(path)
            elif ext in HEADER_EXTS:
                headers.append(path)
            elif ext in UI_EXTS:
                uis.append(path)
    sources.sort()
    headers.sort()
    uis.sort()
    return sources, headers, uis


def render_block(var_name, files):
    lines = [f"set({var_name}"]
    lines += [f"    {f}" for f in files]
    lines.append(")")
    return "\n".join(lines)


def replace_block(content, var_name, new_block):
    pattern = re.compile(rf"set\(\s*{var_name}\b.*?\n\)", re.DOTALL)
    if not pattern.search(content):
        raise RuntimeError(
            f"Could not find 'set({var_name} ...)' block in {CMAKE_FILE}. "
            f"Make sure it exists (even empty) before running this script."
        )
    return pattern.sub(new_block, content, count=1)


def main():
    if not os.path.isfile(CMAKE_FILE):
        raise RuntimeError(
            f"CMakeLists.txt not found at {CMAKE_FILE}. "
            f"Run this script from the project root directory."
        )

    sources, headers, uis = collect_files()

    with open(CMAKE_FILE, "r", encoding="utf-8") as f:
        content = f.read()

    content = replace_block(content, "HEADERS", render_block("HEADERS", headers))
    content = replace_block(content, "SOURCES", render_block("SOURCES", sources))
    content = replace_block(content, "UIS", render_block("UIS", uis))

    with open(CMAKE_FILE, "w", encoding="utf-8") as f:
        f.write(content)

    print(
        f"Updated {CMAKE_FILE}: "
        f"{len(sources)} sources, {len(headers)} headers, {len(uis)} ui files."
    )


if __name__ == "__main__":
    main()