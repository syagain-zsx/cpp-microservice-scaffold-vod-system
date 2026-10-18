#!/usr/bin/env python3
"""检查 Markdown 文档里的相对链接是否有效（不检查外链）。

用法:
    python3 scripts/check-doc-links.py
"""
import glob
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    broken = []
    files = glob.glob(os.path.join(ROOT, "**", "*.md"), recursive=True)
    for path in files:
        text = open(path, encoding="utf-8").read()
        for match in re.finditer(r"!?\[[^\]]*\]\(([^)]+)\)", text):
            link = match.group(1).split("#")[0].strip()
            if not link or "://" in link or link.startswith("mailto:"):
                continue
            target = os.path.normpath(os.path.join(os.path.dirname(path), link))
            if not os.path.exists(target):
                broken.append((os.path.relpath(path, ROOT), link))

    print(f"markdown files: {len(files)}, broken links: {len(broken)}")
    for f, link in broken:
        print(f"  [BROKEN] {f} -> {link}")
    return 0 if not broken else 1


if __name__ == "__main__":
    sys.exit(main())
