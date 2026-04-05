#!/usr/bin/env python3
import os

# Repository root directory
ROOT_DIR = "/ytx-client"

# Output file for CMake file lists
OUTPUT_FILE = "./qt_file_lists.txt"

# File extensions to scan
SOURCE_EXTS = [".cc", ".cpp"]
HEADER_EXTS = [".h", ".hpp"]
UI_EXTS = [".ui"]

# Directories to exclude from scanning
EXCLUDE_DIRS = {"external"}

# Collected file paths
sources = []
headers = []
uis = []

for root, dirs, files in os.walk(ROOT_DIR):
    # Skip excluded directories
    dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]

    for file in files:
        # Get relative path and normalize separators
        path = os.path.relpath(os.path.join(root, file), ROOT_DIR).replace("\\", "/")
        ext = os.path.splitext(file)[1].lower()

        if ext in SOURCE_EXTS:
            sources.append(path)
        elif ext in HEADER_EXTS:
            headers.append(path)
        elif ext in UI_EXTS:
            uis.append(path)

# Sort for stable output and clean git diffs
sources.sort()
headers.sort()
uis.sort()

# Write results to output file
with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
    f.write("# Auto-generated file lists for Qt project\n\n")

    f.write("set(HEADERS\n")
    for h in headers:
        f.write(f"    {h}\n")
    f.write(")\n\n")

    f.write("set(SOURCES\n")
    for s in sources:
        f.write(f"    {s}\n")
    f.write(")\n\n")

    f.write("set(UIS\n")
    for u in uis:
        f.write(f"    {u}\n")
    f.write(")\n\n")

print(
    f"Done! Generated {OUTPUT_FILE} with "
    f"{len(sources)} sources, {len(headers)} headers, {len(uis)} ui files."
)