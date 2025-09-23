#!/usr/bin/env python3
import os

# 仓库根目录
ROOT_DIR = "./ytx-client"  # 根据你本地路径修改 

# 存放 CMake 列表的输出文件
OUTPUT_FILE = "./cmake_file_lists.txt"

# 要扫描的文件后缀
SOURCE_EXTS = [".cc", ".cpp"]
HEADER_EXTS = [".h", ".hpp"]
UI_EXTS = [".ui"]

# 存放扫描结果
sources = []
headers = []
uis = []

for root, dirs, files in os.walk(ROOT_DIR):
    if "external" in root.split(os.sep):
           continue

    for file in files:
        path = os.path.relpath(os.path.join(root, file), ROOT_DIR).replace("\\", "/")
        ext = os.path.splitext(file)[1].lower()
        if ext in SOURCE_EXTS:
            sources.append(path)
        elif ext in HEADER_EXTS:
            headers.append(path)
        elif ext in UI_EXTS:
            uis.append(path)

# 写入输出文件
with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
    f.write("# Auto-generated file lists for CMake\n\n")

    f.write("set(SOURCES\n")
    for s in sources:
        f.write(f"    {s}\n")
    f.write(")\n\n")

    f.write("set(HEADERS\n")
    for h in headers:
        f.write(f"    {h}\n")
    f.write(")\n\n")

    f.write("set(UIS\n")
    for u in uis:
        f.write(f"    {u}\n")
    f.write(")\n\n")

print(f"Done! Generated {OUTPUT_FILE} with {len(sources)} sources, {len(headers)} headers, {len(uis)} ui files.")
