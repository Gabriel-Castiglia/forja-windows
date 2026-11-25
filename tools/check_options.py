"""
Forja -- checks the generator options through the command line,
independently of the C code.
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 10, 2026
SPDX-License-Identifier: GPL-2.0-only

    python tools/check_options.py <forja.exe>

For each case it asks the program for passwords (/out) and checks every one against the
rules in Python: alphabet, required kinds, exclusions, no repeated neighbours, first letter,
PIN runs, passphrase words from the EFF list. Then it asks for the entropy (/entropy) and
compares it with a count made here by brute force or inclusion-exclusion. The checker is
calibrated first: spoiled passwords and a wrong entropy must be caught.
Exit 0 = all good.
"""

import itertools
import math
import string
import subprocess
import sys
import tempfile
from pathlib import Path

EXE = sys.argv[1]
ROOT = Path(__file__).resolve().parent.parent
WORDS = {line.split("\t")[1] for line in (ROOT / "assets" / "eff_large_wordlist.txt").read_text("ascii").splitlines() if line}
SYM = {"basic": "!@#$%&*+-=?", "all": "".join(c for c in map(chr, range(33, 127)) if not c.isalnum()), "min": "-_."}
AMB = "0O1lI|`'\""
TMP = Path(tempfile.mkdtemp(prefix="rpg_opt_"))
fails = 0


def run(args, out):
    r = subprocess.run([EXE] + args + [out], capture_output=True)
    return r.returncode


def classes(c):
    on = c.get("classes", "luds")
    sym = c.get("custom") or SYM[c.get("symbols", "basic")]
    sym = "".join(sorted(set(ch for ch in sym if 32 < ord(ch) < 127 and not ch.isalnum())))
    ex, avoid = c.get("exclude", ""), c.get("avoid", True)
    keep = lambda s: "".join(ch for ch in s if ch not in ex and not (avoid and ch in AMB))
    return [keep(s) if k in on else "" for k, s in zip("luds", (string.ascii_lowercase, string.ascii_uppercase, string.digits, sym))]


def chars_ok(pw, c):
    cls = classes(c)
    if len(pw) != c["len"]:
        return "length"
    if set(pw) - set("".join(cls)):
        return "outside the alphabet"
    for s in cls:
        if s and not any(ch in s for ch in pw):
            return "a kind missing"
    if c.get("norepeat") and any(a == b for a, b in zip(pw, pw[1:])):
        return "repeated neighbour"
    if c.get("startletter") and (cls[0] + cls[1]) and pw[0] not in cls[0] + cls[1]:
        return "does not start with a letter"
    return None


def pin_ok(pw, c):
    ex = c.get("exclude", "")
    if len(pw) != c["len"] or not pw.isdigit() or any(ch in ex for ch in pw):
        return "not a PIN of the length"
    if c.get("norepeat") and any(a == b for a, b in zip(pw, pw[1:])):
        return "repeated neighbour"
    if c.get("noseq", True):
        for i in range(2, len(pw)):
            s1, s2 = int(pw[i - 1]) - int(pw[i - 2]), int(pw[i]) - int(pw[i - 1])
            if s1 == s2 and s1 in (1, -1):
                return "a run"
    return None


def words_ok(pw, c):
    sep = {"hyphen": "-", "space": " ", "dot": ".", "underscore": "_"}.get(c.get("sep", "hyphen"))
    if sep is None:
        return None                       # no separator / digits: only the count of letters is checked below
    parts = pw.split(sep) if sep != "-" else None
    if sep == "-":
        # words may contain hyphens themselves (t-shirt): greedy match from the left
        parts, rest = [], pw
        while rest:
            for n in range(len(rest), 0, -1):
                head = rest[:n]
                core = head.rstrip("0123456789").lower()
                if core in WORDS and (n == len(rest) or rest[n] == "-"):
                    parts.append(head)
                    rest = rest[n + 1:]
                    break
            else:
                return "not made of list words"
    if len(parts) != c["words"]:
        return f"{len(parts)} words"
    digits = sum(ch.isdigit() for ch in pw)
    if digits != (1 if c.get("number", True) else 0):
        return "wrong number of digits"
    for p in parts:
        core = p.rstrip("0123456789")
        if core.lower() not in WORDS:
            return "not a list word"
        if c.get("cap", True) and not core[0].isupper():
            return "no capital"
    return None


def entropy_expected(c):
    if c["mode"] == "words":
        bits = c["words"] * math.log2(7776)
        if c.get("sep") == "digit":
            bits += (c["words"] - 1) * math.log2(10)
        if c.get("number", True):
            bits += math.log2(10 * c["words"])
        return bits
    if c["mode"] == "pin":
        dg = [d for d in "0123456789" if d not in c.get("exclude", "")]
        n = 0
        for p in itertools.product(dg, repeat=c["len"]):
            if pin_ok("".join(p), c) is None:
                n += 1
        return math.log2(n) if n else 0
    n = [len(s) for s in classes(c)]
    L = c["len"]
    if not c.get("norepeat") and not c.get("startletter"):
        N, req = sum(n), [k for k in range(4) if n[k]]
        total = sum((-1) ** len(S) * (N - sum(n[k] for k in S)) ** L for r in range(len(req) + 1) for S in itertools.combinations(req, r))
        return math.log2(total)
    req = [k for k in range(4) if n[k]]
    total = 0
    for seq in itertools.product(req, repeat=L):            # class sequences: exact for small lengths
        if set(seq) != set(req):
            continue
        if c.get("startletter") and (n[0] + n[1]) and seq[0] not in (0, 1):
            continue
        w = n[seq[0]]
        for i in range(1, L):
            w *= n[seq[i]] - (1 if c.get("norepeat") and seq[i] == seq[i - 1] else 0)
        total += w
    return math.log2(total) if total else 0


def args_of(c):
    a = [f"/mode:{c['mode']}"]
    if c["mode"] == "words":
        a += [f"/words:{c['words']}", f"/sep:{c.get('sep', 'hyphen')}"]
        a += [] if c.get("cap", True) else ["/nocap"]
        a += [] if c.get("number", True) else ["/nonumber"]
    else:
        a += [f"/len:{c['len']}"]
        if c["mode"] == "chars":
            a += [f"/classes:{c.get('classes', 'luds')}"]
            a += [f"/symbols:custom:{c['custom']}"] if c.get("custom") else [f"/symbols:{c.get('symbols', 'basic')}"]
            a += [] if c.get("avoid", True) else ["/ambiguous"]
            a += ["/startletter"] if c.get("startletter") else []
        else:
            a += [] if c.get("noseq", True) else ["/allowruns"]
        a += ["/norepeat"] if c.get("norepeat") else []
        a += [f"/exclude:{c['exclude']}"] if c.get("exclude") else []
    return a


def check_case(c, count=3000):
    global fails
    a = args_of(c)
    out = TMP / "out.txt"
    if run(a + [f"/count:{count}"], f"/out:{out}") != 0:
        print("BAD ", " ".join(a), "exit code"); fails += 1; return
    lines = out.read_bytes().decode("ascii").split("\r\n")[:-1]
    test = chars_ok if c["mode"] == "chars" else pin_ok if c["mode"] == "pin" else words_ok
    bad = [(p, test(p, c)) for p in lines if test(p, c)]
    ent = TMP / "ent.txt"
    run(a, f"/entropy:{ent}")
    got = float(ent.read_text().split()[0])
    want = entropy_expected(c)
    ok = not bad and len(lines) == count and abs(got - want) < 1e-4
    fails += not ok
    print("ok  " if ok else "BAD ", " ".join(a), f"| {len(lines)} passwords, entropy {got:.3f} (expected {want:.3f})",
          f"| first problem: {bad[0]}" if bad else "")


# --- calibration: the checks must catch spoiled answers -----------------------------------
cal = {"mode": "chars", "len": 8, "norepeat": True, "startletter": True, "exclude": "xyz"}
caught = [chars_ok("aB3$efgh", cal) is None,          # a correct one passes
          chars_ok("aB3$effh", cal) is not None,      # repeated neighbour
          chars_ok("3aB$efgh", cal) is not None,      # starts with a digit
          chars_ok("aB3$efgx", cal) is not None,      # excluded character
          pin_ok("5123", {"mode": "pin", "len": 4}) is not None,
          words_ok("Copper-Showroom-Wrecking7-Salute-Knoll-Preteen", {"mode": "words", "words": 6}) is None,
          words_ok("Copper-Showroom-Wreckinx7-Salute-Knoll-Preteen", {"mode": "words", "words": 6}) is not None,
          abs(entropy_expected({"mode": "chars", "len": 4, "classes": "d", "avoid": False}) - math.log2(10 ** 4)) < 1e-9]
print("calibration:", "ok" if all(caught) else f"BAD {caught}")
fails += not all(caught)

# --- the program ------------------------------------------------------------------------
for c in [
    {"mode": "chars", "len": 20},
    {"mode": "chars", "len": 6, "norepeat": True, "startletter": True},
    {"mode": "chars", "len": 7, "classes": "lud", "exclude": "abcXYZ59"},
    {"mode": "chars", "len": 8, "symbols": "min", "startletter": True},
    {"mode": "chars", "len": 7, "symbols": "all", "avoid": False, "norepeat": True},
    {"mode": "chars", "len": 6, "custom": "#_!", "classes": "lds"},
    {"mode": "chars", "len": 32, "symbols": "all", "avoid": False},
    {"mode": "chars", "len": 12, "classes": "ld"},
    {"mode": "pin", "len": 6, "norepeat": True},
    {"mode": "pin", "len": 5, "noseq": False, "exclude": "0"},
    {"mode": "words", "words": 6},
    {"mode": "words", "words": 4, "sep": "space", "cap": False, "number": False},
    {"mode": "words", "words": 5, "sep": "dot"},
]:
    check_case(c)


# --- rules so tight that most draws fail -----------------------------------------------------
# Up to 3.0.0 these froze the window for seconds or minutes, and some ended blaming the random
# number generator. Now they are made at once, or refused by pw_check (PW_ERR_TOOTIGHT = 5).
def check_fast(c, count, limit=5.0):
    global fails
    import time
    a = args_of(c)
    out = TMP / "out.txt"
    t = time.perf_counter()
    rc = run(a + [f"/count:{count}"], f"/out:{out}")
    secs = time.perf_counter() - t
    lines = out.read_bytes().decode("ascii").split("\r\n")[:-1] if rc == 0 else []
    test = chars_ok if c["mode"] == "chars" else pin_ok
    bad = [(p, test(p, c)) for p in lines if test(p, c)]
    ok = rc == 0 and not bad and len(lines) == count and secs < limit
    fails += not ok
    print("ok  " if ok else "BAD ", " ".join(a), f"| exit {rc}, {len(lines)} passwords in {secs:.2f} s",
          f"| first problem: {bad[0]}" if bad else "")


def check_refused(c):
    global fails
    a = args_of(c)
    ent, out = TMP / "ent.txt", TMP / "out.txt"
    run(a, f"/entropy:{ent}")
    check = int(ent.read_text().split()[2])
    rc = run(a + ["/count:5"], f"/out:{out}")
    ok = check == 5 and rc == 5
    fails += not ok
    print("ok  " if ok else "BAD ", " ".join(a), f"| refused as too tight: check {check}, exit {rc} (expected 5 and 5)")


check_fast({"mode": "chars", "len": 128, "classes": "d", "norepeat": True}, 50)
check_fast({"mode": "pin", "len": 128, "norepeat": True}, 50)
check_fast({"mode": "pin", "len": 30, "norepeat": True, "exclude": "3456789"}, 50)
check_fast({"mode": "pin", "len": 45, "norepeat": True, "exclude": "3456789"}, 50)   # the tightest still accepted
check_refused({"mode": "pin", "len": 46, "norepeat": True, "exclude": "3456789"})
check_refused({"mode": "pin", "len": 128, "norepeat": True, "exclude": "3456789"})


# --- uniformity: every valid password equally likely ----------------------------------------
# Letters a b and digits 2 3 9, 5 characters, no repeated neighbours, first a letter: 510 valid
# passwords; 100,000 draws (about 196 each) must spread evenly (chi-square, |z| < 4). The same
# letters and digits with no other rule: 2,850 valid passwords, about 35 draws each.
def check_uniform(c, count):
    global fails
    a = args_of(c)
    out = TMP / "out.txt"
    rc = run(a + [f"/count:{count}"], f"/out:{out}")
    lines = out.read_bytes().decode("ascii").split("\r\n")[:-1] if rc == 0 else []
    cls = classes(c)
    alpha = "".join(cls)
    valid = ["".join(p) for p in itertools.product(alpha, repeat=c["len"]) if chars_ok("".join(p), c) is None]
    seen = {v: 0 for v in valid}
    stray = 0
    for p in lines:
        if p in seen: seen[p] += 1
        else: stray += 1
    e = len(lines) / len(valid) if valid else 0
    chi = sum((n - e) ** 2 / e for n in seen.values()) if e else 0
    z = (chi - (len(valid) - 1)) / math.sqrt(2 * (len(valid) - 1)) if len(valid) > 1 else 99
    ok = rc == 0 and len(lines) == count and not stray and abs(z) < 4
    fails += not ok
    print("ok  " if ok else "BAD ", " ".join(a), f"| {len(valid)} valid passwords, {count} drawn, chi-square z = {z:.2f}")


check_uniform({"mode": "chars", "len": 5, "classes": "ld", "exclude": "cdefghijklmnopqrstuvwxyz0145678",
               "avoid": False, "norepeat": True, "startletter": True}, 100000)
check_uniform({"mode": "chars", "len": 5, "classes": "ld", "exclude": "cdefghijklmnopqrstuvwxyz0145678",
               "avoid": False}, 100000)

print()
print("ALL OPTION CHECKS PASSED" if not fails else f"FAILED: {fails}")
sys.exit(1 if fails else 0)
