"""
Wyrmhold -- audits the source tree before it is published.
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 9, 2026
SPDX-License-Identifier: GPL-2.0-only

    python tools/audit_sources.py [root]

  1. every source file carries the copyright and licence header (GPL-2.0-only, SPDX);
  2. comments are in English: outside ui_strings.h and make_guide.py (the translated interface
     and guide text), no Japanese characters and no Spanish accented letters;
  3. line endings: .bat files CRLF only, every other text file LF only;
  4. the version is written only in src/version.h, and its numbers and string agree;
  5. every string id of the enum has a text in all three languages.
Exit 0 = clean.
"""

import re
import sys
from pathlib import Path

ROOT = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent
TEXT = {".c", ".h", ".rc", ".py", ".ps1", ".bat", ".xml", ".txt", ".md", ".iss"}
NEEDS_AUTHOR = {".c", ".h", ".rc", ".py", ".ps1", ".bat"}
SKIP = set()
TRANSLATED = {"ui_strings.h", "make_guide.py", "README.md", "Wyrmhold.iss"}   # text in three languages

JAPANESE = "[" + chr(0x3040) + "-" + chr(0x30FF) + chr(0x4E00) + "-" + chr(0x9FFF) + "]"
SPANISH = "[" + "".join(chr(c) for c in (0xE1, 0xE9, 0xED, 0xF3, 0xFA, 0xF1, 0xC1, 0xC9, 0xCD, 0xD3, 0xDA, 0xD1, 0xBF, 0xA1)) + "]"

problems = []


def fail(msg):
    problems.append(msg)


files = [p for p in ROOT.rglob("*") if p.is_file() and p.suffix.lower() in TEXT and not {"build", "_internal", ".git"} & set(p.parts)]
for p in files:
    rel = p.relative_to(ROOT)
    raw = p.read_bytes()
    text = raw.decode("utf-8")

    if p.name not in SKIP and p.suffix.lower() in NEEDS_AUTHOR and not ("Copyright (C) 2026 Gabriel Castiglia" in text and "SPDX-License-Identifier: GPL-2.0-only" in text):
        fail(f"{rel}: no copyright and licence header")


    if p.suffix.lower() == ".bat":
        if re.search(rb"(?<!\r)\n", raw):
            fail(f"{rel}: LF without CR in a .bat")
    elif b"\r" in raw:
        fail(f"{rel}: CR in a non-.bat file")

    if p.name not in TRANSLATED | SKIP and p.suffix.lower() != ".xml":
        for n, line in enumerate(text.splitlines(), 1):
            if re.search(JAPANESE, line):
                fail(f"{rel}:{n}: Japanese text outside ui_strings.h")
            if re.search(SPANISH, line):
                fail(f"{rel}:{n}: Spanish text outside ui_strings.h")

# versions
src = ROOT / "src"
vh = (src / "version.h").read_text(encoding="utf-8")
num = lambda k: re.search(rf"#define {k}\s+(\d+)", vh).group(1)
v_num = ".".join(num(k) for k in ("WH_VER_MAJOR", "WH_VER_MINOR", "WH_VER_PATCH"))
v_str = re.search(r'#define WH_VERSION_STR\s+"([\d.]+)"', vh).group(1)
if v_num != v_str:
    fail(f"version.h disagrees with itself: numbers {v_num}, string {v_str}")
for f in ("wyrmhold.c", "app.rc"):
    if re.search(r'"\d+\.\d+\.\d+"|\b\d+,\d+,\d+,\d+\b', (src / f).read_text(encoding="utf-8")):
        fail(f"{f}: a version written by hand (it belongs in version.h)")

# string table completeness
ui = (src / "ui_strings.h").read_text(encoding="utf-8")
enum_body = re.search(r"enum \{\s*S_WINDOW_TITLE = 0,(.*?)S_COUNT", ui, re.S).group(1)
ids = ["S_WINDOW_TITLE"] + re.findall(r"S_[A-Z0-9_]+", enum_body)
columns = ui.split("/* ------------------------------------------------------------------ ")[1:]
if len(columns) != 3:
    fail(f"string table: {len(columns)} language columns, expected 3")
for col in columns:
    name = col.split()[0]
    have = set(re.findall(r"\[(S_[A-Z0-9_]+)\]\s*=", col))
    missing = [i for i in ids if i not in have]
    extra = sorted(have - set(ids))
    if missing:
        fail(f"string table {name}: missing {missing}")
    if extra:
        fail(f"string table {name}: not in the enum {extra}")

print(f"audited {len(files)} files under {ROOT}")
for p in problems:
    print("  FAIL", p)
print("  CLEAN" if not problems else f"  {len(problems)} problem(s)")
sys.exit(1 if problems else 0)
