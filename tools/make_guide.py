"""
Forja -- writes the user guide in English, Japanese and Spanish
(docs/forja_guide_<lang>.docx). The PDFs are made from them by tools/make_pdf.ps1.
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 27, 2026
SPDX-License-Identifier: GPL-2.0-only

    python tools/make_guide.py

The screenshots in docs/images are taken from the program itself with /shot (see the
header of src/forja.c), at /shotdpi:144 with a large /shotarea so that the whole band
shows; window_callouts_<lang>.png is window_<lang>.png with the numbered marks that
section 3 explains. The version and its date are read from src/version.h, so the guide
cannot name another one, and the colours from the default theme in src/theme.c (light,
black accent), so the guide looks like the program.

Each guide is a cover, a table of contents and numbered sections. The contents list the
page of each section through PAGEREF fields pointing at bookmarks on the headings, so
LibreOffice and Word fill in the numbers themselves; the headings use the Heading 1 and
Heading 2 styles, so the PDF carries them as bookmarks.

Inline marks in the texts: **bold** (the names of buttons, menus and options, exactly as
the program shows them in that language) and `code` (file names, commands and keys).
"""

import re
from datetime import datetime
from pathlib import Path

from docx import Document
from docx.enum.section import WD_SECTION
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_CELL_VERTICAL_ALIGNMENT
from docx.enum.text import WD_ALIGN_PARAGRAPH, WD_BREAK, WD_TAB_ALIGNMENT, WD_TAB_LEADER
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
IMAGES = DOCS / "images"
ICON = ROOT / "assets" / "app256.png"
_VERSION_H = (ROOT / "src" / "version.h").read_text(encoding="utf-8")
VERSION = re.search(r'#define WH_VERSION_STR\s+"([\d.]+)"', _VERSION_H).group(1)
DATE = re.search(r'#define WH_DATE_STR\s+"([\d-]+)"', _VERSION_H).group(1)
REPO = re.search(r'#define WH_REPO_STR\s+"([^"]+)"', _VERSION_H).group(1)
AUTHOR = "Gabriel Castiglia"
SETUP = f"Forja-Setup-{VERSION}.exe"
RELEASES = f"https://{REPO}/releases/latest"


def _theme_palette() -> dict:
    """The colours of the program's default look, read from src/theme.c: the light branch of
    makePalette(), the first (black) accent and the ember accent (used for the numbered marks
    on the screenshot). Returned as 'RRGGBB' strings."""
    src = (ROOT / "src" / "theme.c").read_text(encoding="utf-8")
    rgb = r"RGB\(0x([0-9A-Fa-f]{2}),\s*0x([0-9A-Fa-f]{2}),\s*0x([0-9A-Fa-f]{2})\)"
    black = re.search(r"\{\s*S_ACCENT_BLACK,\s*" + rgb + r",\s*" + rgb + r",\s*" + rgb, src).groups()
    ember = re.search(r"\{\s*S_ACCENT_EMBER,\s*" + rgb, src).groups()
    light = re.search(r"else\s*\{(.*?)\n\t\}", src[src.index("void makePalette"):], re.S).group(1)
    pal = {k: (int(r, 16), int(g, 16), int(b, 16)) for k, r, g, b in re.findall(r"PAL\.(\w+)\s*=\s*" + rgb, light)}
    accent, tile = tuple(int(x, 16) for x in black[0:3]), tuple(int(x, 16) for x in black[6:9])
    m = re.search(r"PAL\.band\s*=\s*mix\(t,\s*" + rgb + r",\s*([\d.]+)\)", light)
    to, t = tuple(int(x, 16) for x in m.groups()[:3]), float(m.group(4))
    mix = lambda a, b, f: tuple(int(x + (y - x) * f + 0.5) for x, y in zip(a, b))
    pal["accent"] = accent
    pal["ember"] = tuple(int(x, 16) for x in ember)
    pal["band"] = mix(tile, to, t)                                   # as makePalette() mixes it
    pal["bandSub"] = mix((0xFF, 0xFF, 0xFF), pal["band"], 0.32)
    pal["accentSoft"] = mix(pal["card"], accent, 0.11)
    pal["emberSoft"] = mix(pal["card"], pal["ember"], 0.10)
    return {k: "%02X%02X%02X" % v for k, v in pal.items()}


PAL = _theme_palette()
COLOR = {k: RGBColor.from_string(v) for k, v in PAL.items()}
ACCENT = COLOR["accent"]                  # headings, bullets and numbers: the default accent
TEXT = COLOR["text"]                      # body text
GREY = COLOR["hint"]                      # subtitle, captions and footer: the hint colour
WHITE = RGBColor(0xFF, 0xFF, 0xFF)
WIDTH = 16.6                              # text width in cm: A4 less two margins of 2.2 cm
MONO = "Consolas"

_d = datetime.strptime(DATE, "%Y-%m-%d")
_MONTHS_EN = "January February March April May June July August September October November December".split()
_MONTHS_ES = "enero febrero marzo abril mayo junio julio agosto septiembre octubre noviembre diciembre".split()
DATE_EN = f"{_d.day} {_MONTHS_EN[_d.month - 1]} {_d.year}"
DATE_ES = f"{_d.day} de {_MONTHS_ES[_d.month - 1]} de {_d.year}"
DATE_JP = f"{_d.year}年{_d.month}月{_d.day}日"

# ---------------------------------------------------------------------------------------- texts
# Blocks: ("h1", text) numbered section, ("h2", text) numbered subsection, ("p", text),
# ("b", text) bullet, ("n", text) numbered step, ("img", file, caption, width in cm),
# ("table", widths in cm, header row, rows), ("legend", [(mark, text), ...]) the marks of a
# screenshot, ("code", text), ("note", text) a framed remark, ("page",) a page break.

GUIDES = {}

GUIDES["en"] = {
    "font": "Segoe UI", "east": "Yu Gothic UI", "size": 10.5,
    "title": "Forja",
    "tagline": "Strong passwords, passphrases and PINs \u2014 made here, stored nowhere",
    "kind": "User guide",
    "meta": f"Version {VERSION}  \u00B7  {DATE_EN}",
    "byline": f"{AUTHOR}  \u00B7  {REPO}",
    "licence": "Free software under the GNU General Public License, version 2 only. No warranty.",
    "contents": "Contents",
    "figure": "Figure",
    "page": "Page",
    "blocks": [
        ("h1", "What Forja is"),
        ("p", "Forja makes strong random passwords, passphrases and PINs on your own computer: for a zip, 7z or rar file sent by email, a new account, a Wi-Fi network or a lock."),
        ("note", "**Nothing is stored or sent anywhere.** The passwords exist only in the window until you copy or save them, and are gone when Forja closes. The settings file keeps your choices, never a password. The only connection Forja ever makes is to ask GitHub for the number of the latest version, and only if you allow it (section 9)."),
        ("b", "**Three kinds:** random characters (4 to 128), passphrases of 3 to 12 words from the EFF list of 7,776 words, and PINs."),
        ("b", "**Profiles** for the usual cases, including sites that refuse symbols, accept only `- _ .` or have a short maximum length."),
        ("b", "**Uniform and exact:** every password that follows the rules is equally likely, and the strength shown is counted exactly, every rule included."),
        ("b", "**Clipboard protection:** copies are kept out of the clipboard history and wiped after 2 minutes, or when Forja closes."),
        ("b", "**Updates signed by the author**, installed by themselves once you say so."),
        ("b", "English, Japanese and Spanish; light, dark or follow-Windows theme and twelve colours; read by screen readers; sharp at every display scale."),
        ("p", "The name: a forja (Spanish for forge) is where metal takes shape under the hammer, on the anvil, as each password here takes shape from pure chance. Until version 2.1 it was called Wyrmhold."),

        ("h1", "Installing and removing it"),
        ("h2", "Requirements"),
        ("p", "Windows 10 (version 1809 or later) or Windows 11, on a 64-bit Intel or AMD processor (x64). There is no version for ARM processors. Forja is a single program of about 1 MB that needs nothing else installed."),
        ("h2", "Installing"),
        ("p", f"Download `{SETUP}` from the Releases page ({RELEASES}), run it and choose the language of the setup: English, Japanese or Spanish. By default Forja is installed for your user only, with no administrator rights; the first page also offers to install it for all users. The setup adds Forja and this guide to the Start menu, and a desktop shortcut, which you can untick. The program starts in the language you chose for the setup."),
        ("p", "Installing Forja removes Wyrmhold, its former name, and its settings come over the first time Forja starts. If removing Wyrmhold fails, the setup says so, and Wyrmhold can be removed by hand as below."),
        ("h2", "Removing"),
        ("p", "**Settings \u2192 Apps \u2192 Installed apps \u2192 Forja \u2192 Uninstall.** The uninstaller removes every file it installed and then asks whether to delete your settings too (and those Wyrmhold left, if any)."),
        ("h2", "Portable use"),
        ("p", "`forja.exe` needs nothing else. To run it from a USB stick or a shared folder without installing it, copy it there together with an empty file named `forja.ini`: the settings are then kept in that file. A portable copy cannot update itself (section 9)."),

        ("h1", "The window"),
        ("img", "window_callouts_{lang}.png", "The main window: light theme, the default black colour.", 13.0),
        ("legend", [
            ("1", "**Characters**, **Passphrase** or **PIN**: the kind of password (`Ctrl+1`, `Ctrl+2`, `Ctrl+3`)."),
            ("2", "**Profile**: ready-made settings for common cases (section 5). After a change of your own it reads **Custom**."),
            ("3", "The settings of that kind (section 4), and **How many** passwords to make, 1 to 50. Every change makes a new set at once."),
            ("4", "The strength line: a five-step bar, the level and the entropy in bits; below, the possible characters per position and how long guessing would take (section 7). When the settings cannot make a password, it says why, in red."),
            ("5", "**Generate** (`F5`): a completely new set."),
            ("6", "**Clear clipboard after** and its time (section 8)."),
            ("7", "Save all the passwords to a text file (`Ctrl+S`)."),
            ("8", "Copy all the passwords, one per line (`Ctrl+Shift+C`)."),
            ("9", "The list. Each row has its copy icon. Digits are shown in blue and symbols in amber, so a password is easier to read back or dictate; what is copied is plain text."),
            ("10", "The status line: what has just happened (passwords made, copied, the clipboard cleared, an update) and the version."),
        ]),
        ("h2", "The menus"),
        ("table", [3.0, 13.6], ["Menu", "Commands"], [
            ["**File**", "**Generate** (`F5`), **Copy selected** (`Ctrl+C`), **Copy all** (`Ctrl+Shift+C`), **Save as\u2026** (`Ctrl+S`), **Exit**"],
            ["**Passwords**", "**Characters**, **Passphrase**, **PIN** (`Ctrl+1` to `Ctrl+3`) and the eight profiles"],
            ["**View**", "**Theme: follow Windows**, **Theme: light**, **Theme: dark**, **Theme and colour\u2026**"],
            ["**Language**", "**English**, **日本語**, **Español**: the window changes at once"],
            ["**Help**", "**Help** (`F1`), **Check for updates\u2026**, **Check for updates daily**, **About**"],
        ]),
        ("h2", "Screen readers, scaling and small screens"),
        ("p", "Narrator and other screen readers read the whole window: each control by name, the modes and colours as options, the chips and switches as check boxes that are on or off, the slider with its value, each row of the list as its password, and the menus; the status line is read out whenever it changes, without moving the focus. Everything can be done from the keyboard (section 12)."),
        ("p", "Moved to a screen with another scale, the window is drawn again at that scale, sharp. On a small screen the window fits the screen: rows that do not fit the width go on to the next row, the band at the top slims down or goes, and in a very low window the upper part scrolls while at least three passwords stay in view."),

        ("h1", "The three kinds"),
        ("h2", "Characters"),
        ("p", "Random characters are the strongest for their length; best when the password is pasted rather than typed."),
        ("table", [3.6, 13.0], ["Setting", "What it does"], [
            ["**Length**", "4 to 128 characters (20 by default). The slider reaches 64; type up to 128 in the field next to it."],
            ["**Use**", "The kinds of character: **a\u2013z**, **A\u2013Z**, **0\u20139**, **#$%**. Every kind that is on appears at least once in each password."],
            ["**Symbols**", "**Basic** (`! @ # $ % & * + - = ?`), **All 32** (every printable symbol), **Minimal** (`- _ .`) or **My own\u2026**, which shows the **My symbols** field: type exactly the symbols a site allows (letters and digits typed there are ignored)."],
            ["**No look-alikes**", "Leaves out `0 O 1 l I`, and ``| ` ' \"`` among the symbols: they look alike in many fonts. On by default."],
            ["**No repeats in a row**", "Never the same character twice in a row (`aa`, `11`)."],
            ["**Start with a letter**", "The first character is always a letter."],
            ["**Leave out**", "Any other characters that must never appear, typed one after the other."],
        ]),
        ("h2", "Passphrase"),
        ("img", "words_{lang}.png", "A passphrase: six words, hyphens, capital letters and one digit.", 14.0),
        ("p", "Random words from the EFF list of 7,776 English words, such as `Copper-Showroom-Wrecking7-Salute`. Longer, but far easier to remember, dictate or type on a phone."),
        ("table", [3.6, 13.0], ["Setting", "What it does"], [
            ["**Words**", "3 to 12 (6 by default). Each word adds 12.9 bits."],
            ["**Separator**", "**Hyphen**, **Space**, **Dot**, **Underscore**, **None**, or **Random digit**, which adds about 3.3 bits for every gap."],
            ["**Capital letters**", "Each Word Starts With A Capital, for sites that require uppercase letters. On by default."],
            ["**Add a digit**", "One digit after one of the words, for sites that require a number. On by default."],
        ]),
        ("p", "With the defaults (six words, hyphens, capitals, a digit) a passphrase has 83 bits: very strong."),
        ("h2", "PIN"),
        ("img", "pin_{lang}.png", "A PIN: six digits, no repeats and no runs.", 14.0),
        ("table", [3.6, 13.0], ["Setting", "What it does"], [
            ["**Digits**", "4 to 128 (6 by default). The slider reaches 20; type up to 128 in the field next to it."],
            ["**No repeats in a row**", "Never the same digit twice in a row. On by default."],
            ["**No runs like 123 or 987**", "No three digits in a row going up or down by one: the first ones people try. On by default."],
            ["**Leave out**", "Digits that must never appear."],
        ]),
        ("note", "A PIN is only safe where wrong tries are limited, as on cards, phones and locks. Instead of a level, the strength line says **PIN: only where tries are limited**, in amber."),

        ("h1", "Profiles, and sites that refuse some characters"),
        ("p", "The **Profile** button, and the **Passwords** menu, set everything at once. After any change of your own the button reads **Custom**; settings that match a profile again show its name."),
        ("table", [3.3, 9.6, 3.7], ["Profile", "Settings", "Strength"], [
            ["**Standard**", "20 characters, the four kinds, **Basic** symbols, no look-alikes (the default)", "122 bits"],
            ["**Maximum**", "32 characters, **All 32** symbols, look-alikes allowed", "210 bits"],
            ["**No symbols**", "20 characters: letters and digits, no look-alikes", "117 bits"],
            ["**Minimal symbols**", "20 characters, symbols only `- _ .`, starting with a letter", "117 bits"],
            ["**Short**", "12 characters, starting with a letter, no repeats in a row: for a low maximum", "72 bits"],
            ["**Wi-Fi**", "20 characters, lowercase letters and digits only: easy to type on a phone or TV", "101 bits"],
            ["**Passphrase**", "6 words, hyphens, capital letters, a digit", "83 bits"],
            ["**PIN**", "6 digits, no repeats, no runs", "19 bits"],
        ]),
        ("h2", "When a site refuses some characters"),
        ("b", "It refuses symbols: profile **No symbols**, or switch off the **#$%** chip."),
        ("b", "It accepts only some symbols: **Symbols \u2192 Minimal**, or **My own\u2026**, and type exactly the symbols the site lists."),
        ("b", "It refuses particular characters: type them in **Leave out**."),
        ("b", "It has a maximum length: lower the length, or profile **Short**; make up for it with every kind of character."),
        ("b", "It wants a letter first, or no repeated characters: switch on **Start with a letter** or **No repeats in a row**."),

        ("h1", "Copying and saving"),
        ("b", "**One password:** the copy icon at the end of its row, a double click on the row, or `Enter` or `Ctrl+C` on the selected row. The icon turns into a check mark for a moment, and the status line says which password was copied."),
        ("b", "**All of them:** the copy icon at the top right of the list, or `Ctrl+Shift+C`: one per line."),
        ("b", "**Save:** the arrow icon, or `Ctrl+S`, writes them all to a text file, one per line (`passwords.txt` is proposed, in the folder used last time). The file is written whole or not at all. Keep it somewhere safe, or delete it once used."),
        ("p", "Every start and every change of the settings makes a fresh set; nothing of an earlier set is kept."),
        ("note", "**Good practice:** send the password through a different channel from the file \u2014 the file by email, the password in a separate message, a chat or a call."),

        ("h1", "How the passwords are made, and their strength"),
        ("h2", "Pure chance, without bias"),
        ("p", "Every character or word comes from the Windows cryptographic random number generator (`BCryptGenRandom`), drawn without bias. When a password breaks one of the rules \u2014 a kind missing, a repeated neighbour, a run \u2014 it is thrown away and drawn again, never patched. So every password that follows the rules is exactly as likely as any other: no position, kind or pattern is favoured."),
        ("h2", "Entropy, counted exactly"),
        ("p", "The entropy is the number of different passwords the settings can produce, written as a power of two: n bits means 2\u207F equally likely passwords, and each bit more doubles the work of whoever tries to guess it. Forja counts it exactly, every rule included:"),
        ("b", "**Characters:** not simply length \u00D7 log\u2082(characters), which would count passwords the rules forbid. Forja counts, position by position, the passwords in which every kind that is on appears, with no repeated neighbours and a letter first if asked. The standard 20 characters, from 68 possible ones, give 121.6 bits."),
        ("b", "**Passphrase:** 12.9 bits per word (log\u2082 7,776), plus 3.3 bits per gap with **Random digit**, plus, with **Add a digit**, log\u2082(10 \u00D7 words) for which digit and after which word. **Capital letters** add nothing: every word gets one. The count assumes that the attacker knows the list and the settings. It counts different passphrases, not just different draws: no word of the list is the beginning of another (nor are the ones with a hyphen, such as `t-shirt` or `drop-down`), so with any separator, **None** included, and with or without capitals, each passphrase comes from one draw only."),
        ("b", "**PIN:** the strings of the digits allowed, counted exactly without repeats or runs: six digits give 19.1 bits."),
        ("h2", "Levels and guessing time"),
        ("table", [4.0, 4.0, 8.6], ["Level", "Entropy", "Shown in"], [
            ["**Very weak**", "below 36 bits", "red"],
            ["**Weak**", "36 to 49 bits", "red"],
            ["**Fair**", "50 to 63 bits", "amber"],
            ["**Strong**", "64 to 79 bits", "green"],
            ["**Very strong**", "80 bits or more", "green"],
        ]),
        ("p", "The guessing time assumes an attacker who has the file and tries 100 billion passwords a second, finding it on average halfway: from **instantly**, **seconds** and **minutes** up to **millions of years**, **billions of years**, **trillions of years** and **far longer than the age of the universe**."),
        ("table", [8.0, 3.0, 5.6], ["Settings", "Entropy", "Guessing it"], [
            ["Standard profile, 20 characters", "122 bits", "far longer than the age of the universe"],
            ["Passphrase of 8 words", "110 bits", "trillions of years"],
            ["16 characters, otherwise standard", "97 bits", "billions of years"],
            ["Short profile, 12 characters", "72 bits", "centuries"],
            ["Passphrase of 6 words (default)", "83 bits", "millions of years"],
            ["Passphrase of 4 words", "57 bits", "days to months"],
            ["Passphrase of 3 words, no capitals, no digit", "39 bits", "seconds"],
            ["PIN of 6 digits", "19 bits", "instantly"],
        ]),
        ("h2", "When the settings cannot make a password"),
        ("p", "The strength line then says why, in red, **Generate** is greyed out and the list stays empty. Relax one of the rules:"),
        ("b", "**Turn on at least one kind of character.**"),
        ("b", "**Nothing is left of the symbols: allow some, or turn that kind off.** (also for letters or digits, when **Leave out** or **No look-alikes** removes them all)"),
        ("b", "**No password can follow all these rules. Relax one of them.**"),
        ("b", "**Every digit is left out.** (PIN)"),
        ("b", "**These rules leave so few passwords that making one would take too long. Relax one of them.** For example: digits only, no repeats and a length of 128."),

        ("h1", "Clipboard protection"),
        ("p", "Every copy carries the formats that keep it out of the Windows clipboard history (`Win+V`), the cloud clipboard and clipboard monitors. Above the list, **Clear clipboard after** is on by default with **2 min**; the drop-down beside it offers 10 s, 30 s, 1, 2, 5 and 10 minutes, and choosing a time also switches clearing on. The status line counts down the seconds left."),
        ("b", "When the time is up, the copy is wiped \u2014 but only if the clipboard still holds exactly what Forja put there. Forja remembers the clipboard\u2019s sequence number and a SHA-256 of the text, never the text itself. Whatever you or another program copied afterwards is left alone; the very same text put back by a clipboard tool is still wiped."),
        ("b", "Closing Forja before the time is up wipes the copy at once, and so does signing out or shutting down \u2014 again only if it is still Forja\u2019s."),
        ("b", "Switched off, copies stay until you copy something else, also after Forja closes."),
        ("b", "In `forja.ini`: `ClearClipboard=1` or `0` and `ClearSeconds=120` (5 to 3600). For one session only, not saved: `/clearafter:90` (seconds), `/clearafter:5m` (minutes) or `/clearafter:off`."),

        ("h1", "Updates"),
        ("h2", "The question on the first start"),
        ("p", "The first time Forja starts, it asks once **Check for new versions automatically?**: **Check daily** or **Don\u2019t check**. Nothing is asked of GitHub before you answer. The answer is kept and can be changed at any time with **Help \u2192 Check for updates daily**; closing the question without answering asks again the next time. After an update the daily check stays as it was, without asking."),
        ("h2", "Checking"),
        ("p", "**Help \u2192 Check for updates\u2026** asks GitHub whether a newer version has been published; with **Check for updates daily** on, Forja also does it by itself, at most once a day. Only the number of the latest version is asked for: nothing about you, your passwords or your settings is sent. When there is a newer version, a single question: **Update now**, **What\u2019s new** (opens the release page) or **Later**."),
        ("h2", "Installing an update"),
        ("n", "**Update now** first checks that the release\u2019s list of checksums (`SHA256SUMS.txt`) carries the author\u2019s signature (`release.sig`), against two keys built into Forja. A release without a valid signature installs nothing; it can only be downloaded by hand from its page."),
        ("n", "It downloads the setup into a new folder of its own and compares its SHA-256 with the signed list. If it does not match, the file is deleted and never run."),
        ("n", "Forja closes (a copy waiting to be wiped is wiped), the setup installs in silence keeping the folder, language and options chosen the first time, and Forja opens again; the status line says **Forja was updated to** the new version."),
        ("p", "A portable copy (one not installed by the setup) cannot update itself: it offers to open the download page instead."),

        ("h1", "Theme, colour and language"),
        ("img", "theme_{lang}.png", "View \u2192 Theme and colour: the appearance and twelve colours.", 12.0),
        ("p", "**View \u2192 Theme and colour\u2026** chooses the **Appearance** (**As Windows**, **Light** or **Dark**) and one of twelve colours: Black (the default), Blue, Gold, Purple, Sky, Ember, Red, Yellow, Green, Teal, Pink and White. The colour paints the buttons, the band at the top and the icon\u2019s tile, in the window and on the taskbar. The **View** menu also switches the theme directly."),
        ("img", "window_dark_{lang}.png", "The dark theme with the Ember colour.", 11.0),
        ("p", "The **Language** menu switches the window, the menus, Help and About between English, Japanese and Spanish at once."),

        ("h1", "Settings and privacy"),
        ("p", "Forja keeps your choices in `%APPDATA%\\Forja\\forja.ini` (or in the portable `forja.ini`, section 2): the kind and the settings of each kind, how many passwords, the clipboard clearing and its time, the daily update check, the theme, the colour, the language, the folder last used for saving, and the window\u2019s position and size."),
        ("p", "**Passwords are never written to it**, nor anything copied. Delete the file to go back to the defaults. The command line (section 13) never writes it."),

        ("h1", "Keyboard shortcuts"),
        ("table", [5.0, 11.6], ["Keys", "Action"], [
            ["`Ctrl+1`, `Ctrl+2`, `Ctrl+3`", "**Characters**, **Passphrase**, **PIN**"],
            ["`F5`", "**Generate** a new set"],
            ["`Enter`, `Ctrl+C`, double click", "Copy the selected password"],
            ["`Ctrl+Shift+C`", "Copy all, one per line"],
            ["`Ctrl+S`", "Save all to a text file"],
            ["`F1`", "Help"],
            ["`Esc`", "Close Help, About or Theme and colour"],
            ["`Tab`, `Shift+Tab`", "Move between the controls; `Space` switches a chip or switch"],
            ["Arrows, `PgUp`, `PgDn`, `Home`, `End`", "Move the slider (by one, by four, to either end)"],
            ["`Alt` + underlined letter", "Open a menu"],
        ]),

        ("h1", "Command line"),
        ("p", "For scripts and tests the program also works without a window:"),
        ("code", "forja.exe /out:<file> [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [options]"),
        ("p", "`/out` writes the passwords to the file, one per line, and exits. Without options it uses the **Standard** profile, whatever the settings say; the options do not change the settings, and nothing is written to `forja.ini`."),
        ("table", [5.4, 11.2], ["Option", "Meaning"], [
            ["`/out:<file>`", "Write the passwords and exit"],
            ["`/entropy:<file>`", "Write instead one line: the entropy in bits, the characters per position (or words in the list) and 0 when the settings can make a password"],
            ["`/mode:chars|words|pin`", "The kind (characters by default)"],
            ["`/len:N`", "Length of the characters, or digits of the PIN"],
            ["`/words:N`", "Words of the passphrase"],
            ["`/count:N`", "How many: 1 to 100,000 (5 by default)"],
            ["`/preset:N`", "A profile: 0 Standard, 1 Maximum, 2 No symbols, 3 Minimal symbols, 4 Short, 5 Wi-Fi, 6 Passphrase, 7 PIN (it replaces the other options)"],
            ["`/classes:luds`", "The kinds to use: l lower, u upper, d digits, s symbols"],
            ["`/symbols:basic|all|min|custom:<chars>`", "The symbol set"],
            ["`/exclude:<chars>`", "Characters (or PIN digits) to leave out"],
            ["`/ambiguous`", "Allow the look-alikes (left out by default)"],
            ["`/norepeat`, `/startletter`", "No repeats in a row; start with a letter"],
            ["`/sep:hyphen|space|dot|underscore|none|digit`", "Separator of the passphrase"],
            ["`/nocap`, `/nonumber`", "Passphrase without capital letters; without the digit"],
            ["`/allowruns`", "PIN with runs such as 123 allowed"],
        ]),
        ("p", "For the window: `/lang:en|jp|es`, `/theme:system|light|dark` (`system`, or `auto`, follows Windows), `/accent:N` (0 to 11, in the order of **Theme and colour**; 0 is the default black) and `/clearafter:N[s|m]|off` (section 8). `/theme` and `/accent` win over what `forja.ini` holds, `/theme:system` and `/accent:0` included. `/selftest:<file>` checks the clipboard wiping, its settings, the update\u2019s checks and the guessing-time scale in about five seconds and writes a report (section 14)."),
        ("table", [2.4, 14.2], ["Exit code", "Meaning (/out)"], [
            ["0", "Done"],
            ["2", "The file could not be written"],
            ["3", "The random number generator failed"],
            ["5", "The options cannot make a password"],
            ["6", "The rules are too tight: no draw followed them in time"],
        ]),
        ("p", "After a failure the file is left empty. Examples:"),
        ("code", "forja.exe /out:wifi.txt /preset:5 /count:3\nforja.exe /out:phrases.txt /mode:words /words:5 /sep:space /nocap\nforja.exe /out:pins.txt /mode:pin /len:8 /count:10"),

        ("h1", "Building from source"),
        ("p", "The source is C and the Win32 API, with no third-party code. With MSYS2 mingw-w64 GCC in `C:\\msys64`, `build.bat` writes `build\\forja.exe`; `tools\\make_installer.ps1` builds the setup with Inno Setup 6. The version is written only in `src\\version.h`."),
        ("p", "`tools\\run_checks.ps1` and `tools\\check_options.py` check the passwords and the exact entropy against independent counts, over hundreds of thousands of passwords, and `tools\\check_clipboard.ps1` the clipboard through the real window. `run_checks.ps1` also runs `forja.exe /selftest`, which saves your clipboard first and puts it back, and never touches your settings."),

        ("h1", "License and credits"),
        ("p", "Free software under the GNU General Public License, version 2 only (GPL-2.0-only): you may use, copy, study and change it, and anyone who passes it on must do so under the same license, with the source code. It comes with no warranty. The full text is in `LICENSE.txt`, in the installation folder. \u00A9 2026 Gabriel Castiglia."),
        ("p", f"Source code, releases and updates: https://{REPO}"),
        ("p", "Passphrase words: the EFF large word list, by the Electronic Frontier Foundation, under CC BY 3.0 US (eff.org/dice). Icon: an anvil and hammer drawn for Forja (2026), under the GPL-2.0 like the rest."),
    ],
}

GUIDES["jp"] = {
    "font": "Yu Gothic UI", "east": "Yu Gothic UI", "size": 10,
    "title": "Forja",
    "tagline": "強力なパスワード・パスフレーズ・PIN \u2014 ここで作り、どこにも保存しません",
    "kind": "ユーザーガイド",
    "meta": f"バージョン {VERSION}  \u00B7  {DATE_JP}",
    "byline": f"{AUTHOR}  \u00B7  {REPO}",
    "licence": "GNU 一般公衆利用許諾書 第 2 版のみ（GPL-2.0-only）のもとで配布されるフリーソフトウェアです。無保証です。",
    "contents": "目次",
    "figure": "図",
    "page": "ページ",
    "blocks": [
        ("h1", "Forja とは"),
        ("p", "Forja は、強力なランダムパスワード・パスフレーズ・PIN をお使いのコンピューター上で作ります。メールで送る zip・7z・rar ファイル、新しいアカウント、Wi-Fi、錠前などに使えます。"),
        ("note", "**何も保存せず、どこにも送信しません。** パスワードはコピーまたは保存するまでウィンドウの中にだけ存在し、Forja を閉じると消えます。設定ファイルに残るのは選んだ設定だけで、パスワードは決して書き込まれません。Forja が行う通信は、最新バージョンの番号を GitHub に問い合わせることだけで、それも許可した場合に限ります（9 章）。"),
        ("b", "**3 つの種類：** ランダムな文字（4〜128 文字）、EFF の 7,776 語のリストから選ぶ 3〜12 単語のパスフレーズ、PIN。"),
        ("b", "**プロファイル：** 記号を受け付けないサイト、`- _ .` だけを受け付けるサイト、最大文字数が少ないサイトなど、よくある用途向けの設定。"),
        ("b", "**一様で正確：** 規則を満たすパスワードはすべて同じ確率で選ばれ、表示される強度はすべての規則を含めて正確に数えた値です。"),
        ("b", "**クリップボードの保護：** コピーはクリップボードの履歴に残らず、2 分後または Forja を閉じたときに消去されます。"),
        ("b", "**作者の署名付きの更新：** 許可すれば自動でインストールされます。"),
        ("b", "英語・日本語・スペイン語、ライト・ダーク・Windows に合わせるのテーマと 12 色、スクリーンリーダー対応、どの表示倍率でもくっきり表示。"),
        ("p", "名前の由来：Forja はスペイン語で「鍛冶場」。金床の上で槌を打って金属を形づくるように、ここでは一つひとつのパスワードが純粋な偶然から生まれます。バージョン 2.1 までは Wyrmhold という名前でした。"),

        ("h1", "インストールとアンインストール"),
        ("h2", "動作環境"),
        ("p", "Windows 10（バージョン 1809 以降）または Windows 11、64 ビットの Intel または AMD プロセッサー（x64）。ARM プロセッサー用の版はありません。Forja は約 1 MB の 1 つのプログラムで、ほかに何もインストールする必要はありません。"),
        ("h2", "インストール"),
        ("p", f"Releases のページ（{RELEASES}）から `{SETUP}` をダウンロードして実行し、セットアップの言語（英語・日本語・スペイン語）を選びます。既定では管理者権限なしで現在のユーザーにインストールされます。最初のページで全ユーザー向けのインストールも選べます。スタート メニューに Forja とこのガイドが追加され、デスクトップにもショートカットが作られます（不要ならチェックを外してください）。プログラムはセットアップで選んだ言語で起動します。"),
        ("p", "Forja をインストールすると、旧名の Wyrmhold は削除され、その設定は Forja の初回起動時に引き継がれます。Wyrmhold を削除できなかった場合はセットアップがそう表示するので、下の手順で手動で削除できます。"),
        ("h2", "アンインストール"),
        ("p", "**設定 \u2192 アプリ \u2192 インストールされているアプリ \u2192 Forja \u2192 アンインストール。** インストールしたファイルをすべて削除したあと、設定も削除するかどうかを確認します（Wyrmhold の設定が残っていればそれも）。"),
        ("h2", "ポータブルで使う"),
        ("p", "`forja.exe` はほかに何も必要としません。USB メモリーや共有フォルダーからインストールせずに使うには、`forja.exe` と一緒に空の `forja.ini` というファイルを置いてください。設定はそのファイルに保存されます。ポータブル版は自動では更新できません（9 章）。"),

        ("h1", "ウィンドウ"),
        ("img", "window_callouts_{lang}.png", "メインウィンドウ：ライトテーマ、既定の黒。", 13.0),
        ("legend", [
            ("1", "**文字**・**パスフレーズ**・**PIN**：パスワードの種類（`Ctrl+1`、`Ctrl+2`、`Ctrl+3`）。"),
            ("2", "**プロファイル**：よくある用途向けの既製の設定（5 章）。自分で変更すると **カスタム** と表示されます。"),
            ("3", "その種類の設定（4 章）と、作るパスワードの **生成数**（1〜50）。変更するとすぐに新しいセットが作られます。"),
            ("4", "強度の行：5 段階のバー、強度、エントロピー（ビット）。その下に 1 文字あたりの候補数と推測にかかる時間（7 章）。設定ではパスワードを作れない場合は、その理由が赤で表示されます。"),
            ("5", "**生成**（`F5`）：まったく新しいセットを作ります。"),
            ("6", "**クリップボードを自動消去** とその時間（8 章）。"),
            ("7", "すべてのパスワードをテキストファイルに保存（`Ctrl+S`）。"),
            ("8", "すべてのパスワードを 1 行に 1 つずつコピー（`Ctrl+Shift+C`）。"),
            ("9", "一覧。各行にコピーアイコンがあります。数字は青、記号はオレンジで表示され、読み返しや読み上げが楽になります。コピーされるのは普通のテキストです。"),
            ("10", "状態表示：直前の操作（生成、コピー、クリップボードの消去、更新）とバージョン。"),
        ]),
        ("h2", "メニュー"),
        ("table", [3.0, 13.6], ["メニュー", "コマンド"], [
            ["**ファイル**", "**生成**（`F5`）、**選択をコピー**（`Ctrl+C`）、**すべてコピー**（`Ctrl+Shift+C`）、**名前を付けて保存\u2026**（`Ctrl+S`）、**終了**"],
            ["**パスワード**", "**文字**・**パスフレーズ**・**PIN**（`Ctrl+1`〜`Ctrl+3`）と 8 つのプロファイル"],
            ["**表示**", "**テーマ: Windows に合わせる**、**テーマ: ライト**、**テーマ: ダーク**、**テーマと色\u2026**"],
            ["**言語**", "**English**・**日本語**・**Español**：ウィンドウがすぐに切り替わります"],
            ["**ヘルプ**", "**ヘルプ**（`F1`）、**更新の確認\u2026**、**毎日自動で更新を確認**、**バージョン情報**"],
        ]),
        ("h2", "スクリーンリーダー・表示倍率・小さい画面"),
        ("p", "ナレーターなどのスクリーンリーダーはウィンドウ全体を読み上げます。各コントロールの名前、モードと色は選択肢として、チップとスイッチはオン・オフのあるチェックボックスとして、スライダーはその値、一覧の各行はそのパスワード、そしてメニューも読み上げます。下部の状態表示は、フォーカスを動かさずに変わるたびに読み上げられます。すべての操作はキーボードでも行えます（12 章）。"),
        ("p", "表示倍率の異なる画面にウィンドウを移すと、その倍率でくっきり描き直されます。小さい画面ではウィンドウが画面に収まるように調整されます。幅に収まらない行は次の行に回り、上部の帯は細くなるか消え、非常に低いウィンドウでは、パスワードを 3 行以上表示したまま上部がスクロールします。"),

        ("h1", "3 つの種類"),
        ("h2", "文字"),
        ("p", "ランダムな文字は、長さあたり最も強力です。入力せず貼り付ける場合に最適です。"),
        ("table", [3.6, 13.0], ["設定", "内容"], [
            ["**文字数**", "4〜128 文字（既定は 20）。スライダーは 64 まで。横の欄には 128 まで入力できます。"],
            ["**使う文字**", "文字の種類：**a\u2013z**、**A\u2013Z**、**0\u20139**、**#$%**。オンにした種類は、どのパスワードにも必ず 1 文字以上入ります。"],
            ["**記号**", "**基本**（`! @ # $ % & * + - = ?`）、**全32種**（印字可能なすべての記号）、**最小限**（`- _ .`）、**自分で指定\u2026**。「自分で指定」を選ぶと **使う記号** 欄が表示され、サイトが許可する記号だけを入力できます（英字と数字は無視されます）。"],
            ["**紛らわしい文字なし**", "`0 O 1 l I` と、記号では ``| ` ' \"`` を使いません。多くのフォントで見分けにくい文字です。既定でオン。"],
            ["**同じ文字を連続させない**", "同じ文字を 2 回続けません（`aa`、`11`）。"],
            ["**英字で始める**", "最初の文字を必ず英字にします。"],
            ["**除外**", "絶対に使わないほかの文字。続けて入力します。"],
        ]),
        ("h2", "パスフレーズ"),
        ("img", "words_{lang}.png", "パスフレーズ：6 単語、ハイフン、大文字、数字 1 つ。", 14.0),
        ("p", "EFF の 7,776 語の英単語リストから選んだランダムな単語です。例：`Copper-Showroom-Wrecking7-Salute`。長くなりますが、覚える・読み上げる・スマートフォンで入力するのがずっと楽です。"),
        ("table", [3.6, 13.0], ["設定", "内容"], [
            ["**単語数**", "3〜12（既定は 6）。1 単語ごとに 12.9 ビット。"],
            ["**区切り**", "**ハイフン**、**スペース**、**ドット**、**アンダースコア**、**なし**、**ランダムな数字**。ランダムな数字にすると、区切りごとに約 3.3 ビット強くなります。"],
            ["**大文字にする**", "各単語の先頭を大文字にします。大文字が必要なサイト向け。既定でオン。"],
            ["**数字を1つ入れる**", "いずれかの単語の後ろに数字を 1 つ入れます。数字が必要なサイト向け。既定でオン。"],
        ]),
        ("p", "既定（6 単語、ハイフン、大文字、数字 1 つ）のパスフレーズは 83 ビットで、非常に強力です。"),
        ("h2", "PIN"),
        ("img", "pin_{lang}.png", "PIN：6 桁、繰り返しなし、連番なし。", 14.0),
        ("table", [3.6, 13.0], ["設定", "内容"], [
            ["**桁数**", "4〜128（既定は 6）。スライダーは 20 まで。横の欄には 128 まで入力できます。"],
            ["**同じ文字を連続させない**", "同じ数字を 2 回続けません。既定でオン。"],
            ["**123 や 987 のような連番なし**", "1 ずつ増える・減る 3 桁の並びを使いません。最初に試される番号です。既定でオン。"],
            ["**除外**", "絶対に使わない数字。"],
        ]),
        ("note", "PIN が安全なのは、カード・スマートフォン・錠前のように誤入力の回数が制限される場合だけです。強度の行には段階の代わりに **PIN: 試行回数が制限される場合のみ** とオレンジで表示されます。"),

        ("h1", "プロファイルと、一部の文字を受け付けないサイト"),
        ("p", "**プロファイル** ボタン（**パスワード** メニューにもあります）で、すべてを一度に設定できます。自分で変更するとボタンは **カスタム** になり、設定がまたプロファイルと一致するとその名前に戻ります。"),
        ("table", [3.6, 9.4, 3.6], ["プロファイル", "設定", "強度"], [
            ["**標準**", "20 文字、4 種類すべて、**基本** の記号、紛らわしい文字なし（既定）", "122 ビット"],
            ["**最強**", "32 文字、**全32種** の記号、紛らわしい文字も使う", "210 ビット"],
            ["**記号なし**", "20 文字：英字と数字、紛らわしい文字なし", "117 ビット"],
            ["**最小限の記号**", "20 文字、記号は `- _ .` のみ、英字で始める", "117 ビット"],
            ["**短め**", "12 文字、英字で始める、同じ文字を連続させない：最大文字数が少ない場合", "72 ビット"],
            ["**Wi-Fi**", "20 文字、英小文字と数字のみ：スマートフォンやテレビで入力しやすい", "101 ビット"],
            ["**パスフレーズ**", "6 単語、ハイフン、大文字、数字 1 つ", "83 ビット"],
            ["**PIN**", "6 桁、繰り返しなし、連番なし", "19 ビット"],
        ]),
        ("h2", "一部の文字を受け付けないサイト"),
        ("b", "記号を受け付けない：プロファイル **記号なし**、または **#$%** のチップをオフにします。"),
        ("b", "一部の記号だけを受け付ける：**記号 \u2192 最小限**、または **自分で指定\u2026** でサイトが示す記号だけを入力します。"),
        ("b", "特定の文字を受け付けない：その文字を **除外** に入力します。"),
        ("b", "最大文字数がある：文字数を下げるか、プロファイル **短め**。すべての種類の文字で補ってください。"),
        ("b", "先頭は英字、または同じ文字の連続禁止：**英字で始める** または **同じ文字を連続させない** をオンにします。"),

        ("h1", "コピーと保存"),
        ("b", "**1 つ：** 行の末尾のコピーアイコン、行のダブルクリック、または選択した行で `Enter` か `Ctrl+C`。アイコンが一瞬チェックマークに変わり、どのパスワードをコピーしたかが状態表示に出ます。"),
        ("b", "**すべて：** 一覧右上のコピーアイコン、または `Ctrl+Shift+C`。1 行に 1 つずつコピーされます。"),
        ("b", "**保存：** 矢印アイコン、または `Ctrl+S` で、すべてをテキストファイルに 1 行ずつ書き出します（前回使ったフォルダーに `passwords.txt` という名前が提案されます）。ファイルは完全に書き込まれるか、まったく書き込まれないかのどちらかです。安全な場所に置くか、使い終わったら削除してください。"),
        ("p", "起動するたび、設定を変えるたびに新しいセットが作られます。前のセットは何も残りません。"),
        ("note", "**安全な使い方：** パスワードはファイルとは別の経路で送ってください。ファイルはメールで、パスワードは別のメッセージ・チャット・電話で。"),

        ("h1", "パスワードの作り方と強度"),
        ("h2", "偏りのない純粋な偶然"),
        ("p", "すべての文字・単語は Windows の暗号用乱数生成器（`BCryptGenRandom`）から偏りなく選ばれます。規則に反するパスワード（種類が欠けている、同じ文字が隣り合う、連番など）は修正せずに捨てて引き直します。そのため、規則を満たすすべてのパスワードが等しい確率で選ばれ、特定の位置・種類・パターンが有利になることはありません。"),
        ("h2", "正確に数えるエントロピー"),
        ("p", "エントロピーは、その設定で作れる異なるパスワードの数を 2 のべき乗で表したものです。n ビットは等しい確率の 2\u207F 通りのパスワードを意味し、1 ビット増えるごとに推測の手間は 2 倍になります。Forja はすべての規則を含めて正確に数えます。"),
        ("b", "**文字：** 単純な「文字数 \u00D7 log\u2082（候補数）」では、規則で除かれるパスワードまで数えてしまいます。Forja は 1 文字ずつ、オンにした種類がすべて現れ、同じ文字が隣り合わず、指定があれば英字で始まるパスワードを数えます。標準の 20 文字（候補 68 種類）で 121.6 ビットです。"),
        ("b", "**パスフレーズ：** 1 単語あたり 12.9 ビット（log\u2082 7,776）。**ランダムな数字** の区切りは 1 か所ごとに 3.3 ビット、**数字を1つ入れる** はどの数字をどの単語の後ろに入れるかで log\u2082（10 \u00D7 単語数）を加えます。**大文字にする** はすべての単語に適用されるので強度は増えません。攻撃者がリストと設定を知っている前提で数えます。数えているのは抽選の数ではなく、異なるパスフレーズの数です。リストには、ほかの単語の先頭部分になる単語がありません（`t-shirt` や `drop-down` のようなハイフン入りの単語も同様です）。そのため、区切りが **なし** でも、大文字の有無にかかわらず、どのパスフレーズも 1 通りの抽選からしか生まれません。"),
        ("b", "**PIN：** 使える数字の並びを、繰り返しと連番の規則を含めて正確に数えます。6 桁で 19.1 ビットです。"),
        ("h2", "強度の段階と推測時間"),
        ("table", [4.0, 4.0, 8.6], ["段階", "エントロピー", "表示色"], [
            ["**非常に弱い**", "36 ビット未満", "赤"],
            ["**弱い**", "36〜49 ビット", "赤"],
            ["**普通**", "50〜63 ビット", "オレンジ"],
            ["**強い**", "64〜79 ビット", "緑"],
            ["**非常に強い**", "80 ビット以上", "緑"],
        ]),
        ("p", "推測時間は、ファイルを持つ攻撃者が毎秒 1000 億回試し、平均して半分を試したところで当たると仮定した目安です。**一瞬**・**数秒**・**数分** から **数百万年〜数億年**・**数十億年〜数千億年**・**数兆年〜数百兆年**・**宇宙の年齢をはるかに超える** まであります。"),
        ("table", [8.0, 3.0, 5.6], ["設定", "エントロピー", "推測にかかる時間"], [
            ["プロファイル「標準」、20 文字", "122 ビット", "宇宙の年齢をはるかに超える"],
            ["8 単語のパスフレーズ", "110 ビット", "数兆年〜数百兆年"],
            ["16 文字、ほかは標準", "97 ビット", "数十億年〜数千億年"],
            ["プロファイル「短め」、12 文字", "72 ビット", "数世紀"],
            ["6 単語のパスフレーズ（既定）", "83 ビット", "数百万年〜数億年"],
            ["4 単語のパスフレーズ", "57 ビット", "数日〜数か月"],
            ["3 単語、大文字なし・数字なしのパスフレーズ", "39 ビット", "数秒"],
            ["6 桁の PIN", "19 ビット", "一瞬"],
        ]),
        ("h2", "設定ではパスワードを作れない場合"),
        ("p", "その場合、強度の行に理由が赤で表示され、**生成** は無効になり、一覧は空のままです。どれかの規則を緩めてください。"),
        ("b", "**文字の種類を1つ以上オンにしてください。**"),
        ("b", "**記号 が1文字も残っていません。除外を減らすか、その種類をオフにしてください。**（**除外** や **紛らわしい文字なし** で英字や数字がなくなった場合も同様）"),
        ("b", "**すべての規則を満たすパスワードは作れません。どれかを緩めてください。**"),
        ("b", "**すべての数字が除外されています。**（PIN）"),
        ("b", "**規則が厳しすぎて、パスワードを作るのに時間がかかりすぎます。どれかを緩めてください。** 例：数字のみ・繰り返しなし・長さ 128。"),

        ("h1", "クリップボードの保護"),
        ("p", "コピーには、Windows のクリップボードの履歴（`Win+V`）、クラウド クリップボード、クリップボード監視ツールから除外するための形式が必ず付きます。一覧の上の **クリップボードを自動消去** は既定でオン、時間は **2分後** です。横のドロップダウンで 10 秒・30 秒・1 分・2 分・5 分・10 分後を選べ、時間を選ぶと自動消去もオンになります。状態表示に残り秒数が表示されます。"),
        ("b", "時間がたつとコピーは消去されます。ただし、クリップボードの中身が Forja の入れたものとまったく同じ場合だけです。Forja はクリップボードのシーケンス番号とテキストの SHA-256 を覚えておき、テキストそのものは保持しません。後からあなたやほかのプログラムがコピーしたものには手を触れません。クリップボードツールが同じテキストを戻した場合は消去します。"),
        ("b", "時間がたつ前に Forja を閉じると、コピーはその場で消去されます。サインアウトやシャットダウンのときも同じです（これも Forja の入れたものが残っている場合だけ）。"),
        ("b", "オフにすると、コピーはほかのものをコピーするまで残ります。Forja を閉じた後も同じです。"),
        ("b", "`forja.ini` では `ClearClipboard=1` または `0` と `ClearSeconds=120`（5〜3600）。そのセッションだけ（保存されません）なら `/clearafter:90`（秒）、`/clearafter:5m`（分）、`/clearafter:off`。"),

        ("h1", "更新"),
        ("h2", "初回起動時の質問"),
        ("p", "Forja を初めて起動したときに、**新しいバージョンを自動で確認しますか？** と一度だけ尋ねます（**毎日確認する** または **確認しない**）。答えるまで GitHub には何も問い合わせません。答えは保存され、**ヘルプ \u2192 毎日自動で更新を確認** でいつでも変更できます。答えずに閉じると、次の起動時にもう一度尋ねます。更新後は尋ねずにそれまでの設定のままです。"),
        ("h2", "確認"),
        ("p", "**ヘルプ \u2192 更新の確認\u2026** で、新しいバージョンが公開されているかを GitHub に問い合わせます。**毎日自動で更新を確認** がオンなら、1 日に 1 回まで自動でも確認します。問い合わせるのは最新バージョンの番号だけで、あなたやパスワード、設定に関する情報は何も送られません。新しいバージョンがあれば、**今すぐ更新**・**更新内容を見る**（リリースのページを開きます）・**後で** の質問が 1 回だけ表示されます。"),
        ("h2", "更新のインストール"),
        ("n", "**今すぐ更新** は、まずリリースのチェックサム一覧（`SHA256SUMS.txt`）に作者の署名（`release.sig`）があることを、Forja に組み込まれた 2 つの鍵で確かめます。有効な署名がないリリースは何もインストールせず、ページから手動でダウンロードするしかありません。"),
        ("n", "セットアップを専用の新しいフォルダーにダウンロードし、その SHA-256 を署名済みの一覧と照合します。一致しなければファイルは削除され、実行されることはありません。"),
        ("n", "Forja が閉じ（消去待ちのコピーは消去されます）、最初に選んだフォルダー・言語・オプションのままセットアップが自動でインストールし、Forja がもう一度開きます。状態表示に **Forja を** 新しいバージョン **に更新しました** と表示されます。"),
        ("p", "ポータブル版（セットアップでインストールしていないもの）は自動では更新できないため、代わりにダウンロードページを開くことを提案します。"),

        ("h1", "テーマ・色・言語"),
        ("img", "theme_{lang}.png", "表示 \u2192 テーマと色：外観と 12 色。", 12.0),
        ("p", "**表示 \u2192 テーマと色\u2026** で、**外観**（**Windows に合わせる**・**ライト**・**ダーク**）と 12 色から 1 色を選べます：黒（既定）、青、金、紫、空色、オレンジ、赤、黄、緑、青緑、ピンク、白。色はボタン、上部の帯、ウィンドウとタスク バーのアイコンの背景に使われます。テーマは **表示** メニューから直接切り替えることもできます。"),
        ("img", "window_dark_{lang}.png", "ダークテーマとオレンジ。", 11.0),
        ("p", "**言語** メニューで、ウィンドウ・メニュー・ヘルプ・バージョン情報を英語・日本語・スペイン語にすぐ切り替えられます。"),

        ("h1", "設定とプライバシー"),
        ("p", "Forja は選んだ設定を `%APPDATA%\\Forja\\forja.ini`（ポータブルの場合は 2 章の `forja.ini`）に保存します：種類と各種類の設定、生成数、クリップボードの自動消去とその時間、毎日の更新確認、テーマ、色、言語、最後に保存に使ったフォルダー、ウィンドウの位置と大きさ。"),
        ("p", "**パスワードやコピーした内容が書き込まれることはありません。** ファイルを削除すると既定に戻ります。コマンドライン（13 章）がこのファイルに書き込むことはありません。"),

        ("h1", "キーボード ショートカット"),
        ("table", [5.0, 11.6], ["キー", "操作"], [
            ["`Ctrl+1`、`Ctrl+2`、`Ctrl+3`", "**文字**・**パスフレーズ**・**PIN**"],
            ["`F5`", "新しいセットを **生成**"],
            ["`Enter`、`Ctrl+C`、ダブルクリック", "選択したパスワードをコピー"],
            ["`Ctrl+Shift+C`", "すべてを 1 行に 1 つずつコピー"],
            ["`Ctrl+S`", "すべてをテキストファイルに保存"],
            ["`F1`", "ヘルプ"],
            ["`Esc`", "ヘルプ・バージョン情報・テーマと色を閉じる"],
            ["`Tab`、`Shift+Tab`", "コントロール間を移動。`Space` でチップやスイッチを切り替え"],
            ["方向キー、`PgUp`、`PgDn`、`Home`、`End`", "スライダーを動かす（1 ずつ、4 ずつ、端まで）"],
            ["`Alt` + 下線付きの文字", "メニューを開く"],
        ]),

        ("h1", "コマンドライン"),
        ("p", "スクリプトやテスト用に、ウィンドウなしでも使えます。"),
        ("code", "forja.exe /out:<file> [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [options]"),
        ("p", "`/out` はパスワードを 1 行ずつファイルに書き出して終了します。オプションを付けなければ、設定に関係なくプロファイル **標準** を使います。オプションで設定が変わることはなく、`forja.ini` には何も書き込まれません。"),
        ("table", [5.4, 11.2], ["オプション", "意味"], [
            ["`/out:<file>`", "パスワードを書き出して終了"],
            ["`/entropy:<file>`", "代わりに 1 行を書き出す：エントロピー（ビット）、1 文字あたりの候補数（パスフレーズではリストの単語数）、パスワードを作れる設定なら 0"],
            ["`/mode:chars|words|pin`", "種類（既定は文字）"],
            ["`/len:N`", "文字数、または PIN の桁数"],
            ["`/words:N`", "パスフレーズの単語数"],
            ["`/count:N`", "生成数：1〜100,000（既定は 5）"],
            ["`/preset:N`", "プロファイル：0 標準、1 最強、2 記号なし、3 最小限の記号、4 短め、5 Wi-Fi、6 パスフレーズ、7 PIN（ほかのオプションより優先）"],
            ["`/classes:luds`", "使う種類：l 小文字、u 大文字、d 数字、s 記号"],
            ["`/symbols:basic|all|min|custom:<chars>`", "記号の組"],
            ["`/exclude:<chars>`", "除外する文字（PIN では数字）"],
            ["`/ambiguous`", "紛らわしい文字も使う（既定では除外）"],
            ["`/norepeat`、`/startletter`", "同じ文字を連続させない、英字で始める"],
            ["`/sep:hyphen|space|dot|underscore|none|digit`", "パスフレーズの区切り"],
            ["`/nocap`、`/nonumber`", "パスフレーズを大文字にしない、数字を入れない"],
            ["`/allowruns`", "PIN で 123 のような連番を許可"],
        ]),
        ("p", "ウィンドウ用：`/lang:en|jp|es`、`/theme:system|light|dark`（`system` または `auto` で Windows に合わせる）、`/accent:N`（0〜11、**テーマと色** の並び順。0 は既定の黒）、`/clearafter:N[s|m]|off`（8 章）。`/theme` と `/accent` は `forja.ini` の保存内容より優先されます（`/theme:system` と `/accent:0` も同様です）。`/selftest:<file>` は、クリップボードの自動消去とその設定、更新の確認処理、推測時間の段階を約 5 秒でテストし、結果をファイルに書き出します（14 章）。"),
        ("table", [2.4, 14.2], ["終了コード", "意味（/out）"], [
            ["0", "完了"],
            ["2", "ファイルに書き込めない"],
            ["3", "乱数生成器の失敗"],
            ["5", "このオプションではパスワードを作れない"],
            ["6", "規則が厳しすぎる：時間内に規則を満たす抽選がない"],
        ]),
        ("p", "失敗したとき、ファイルは空のままです。例："),
        ("code", "forja.exe /out:wifi.txt /preset:5 /count:3\nforja.exe /out:phrases.txt /mode:words /words:5 /sep:space /nocap\nforja.exe /out:pins.txt /mode:pin /len:8 /count:10"),

        ("h1", "ソースからのビルド"),
        ("p", "ソースは C と Win32 API で、サードパーティのコードはありません。`C:\\msys64` に MSYS2 の mingw-w64 GCC があれば、`build.bat` が `build\\forja.exe` を作ります。`tools\\make_installer.ps1` は Inno Setup 6 でセットアップを作ります。バージョンは `src\\version.h` にだけ書かれています。"),
        ("p", "`tools\\run_checks.ps1` と `tools\\check_options.py` は、数十万個のパスワードで規則と正確なエントロピーを独立した計算と照合し、`tools\\check_clipboard.ps1` は実際のウィンドウでクリップボードを確認します。`run_checks.ps1` は `forja.exe /selftest` も実行します。テスト前にクリップボードの中身を保存して最後に元に戻し、あなたの設定には一切触れません。"),

        ("h1", "ライセンスとクレジット"),
        ("p", "GNU 一般公衆利用許諾書 第 2 版のみ（GPL-2.0-only）のもとで配布されるフリーソフトウェアです。使用・複製・研究・改変ができ、再配布する場合は同じライセンスでソースコードとともに行う必要があります。無保証です。全文はインストール先フォルダーの `LICENSE.txt` にあります。\u00A9 2026 Gabriel Castiglia。"),
        ("p", f"ソースコード・リリース・更新：https://{REPO}"),
        ("p", "パスフレーズの単語：Electronic Frontier Foundation の EFF large word list（CC BY 3.0 US、eff.org/dice）。アイコン：Forja のために描いた金床と槌（2026 年）。ほかと同じく GPL-2.0。"),
    ],
}

GUIDES["es"] = {
    "font": "Segoe UI", "east": "Yu Gothic UI", "size": 10.5,
    "title": "Forja",
    "tagline": "Contraseñas, frases y PIN fuertes \u2014 se crean aquí y no se guardan en ninguna parte",
    "kind": "Guía de uso",
    "meta": f"Versión {VERSION}  \u00B7  {DATE_ES}",
    "byline": f"{AUTHOR}  \u00B7  {REPO}",
    "licence": "Software libre bajo la Licencia Pública General de GNU, solo versión 2. Sin garantía.",
    "contents": "Índice",
    "figure": "Figura",
    "page": "Página",
    "blocks": [
        ("h1", "Qué es Forja"),
        ("p", "Forja genera contraseñas, frases y PIN fuertes y aleatorios en tu propio equipo: para un zip, 7z o rar que envías por correo electrónico, una cuenta nueva, una red Wi-Fi o una cerradura."),
        ("note", "**No guarda ni envía nada.** Las contraseñas existen solo en la ventana hasta que las copias o guardas, y desaparecen al cerrar Forja. El archivo de configuración guarda tus elecciones, nunca una contraseña. La única conexión que hace Forja es preguntarle a GitHub el número de la última versión, y solo si lo permites (sección 9)."),
        ("b", "**Tres tipos:** caracteres al azar (de 4 a 128), frases de 3 a 12 palabras de la lista de 7.776 de la EFF, y PIN."),
        ("b", "**Perfiles** para los casos de siempre, incluidos los sitios que no aceptan símbolos, que aceptan solo `- _ .` o que tienen una longitud máxima corta."),
        ("b", "**Uniforme y exacta:** todas las contraseñas que cumplen las reglas son igual de probables, y la fuerza que muestra se cuenta exacta, con todas las reglas."),
        ("b", "**Protección del portapapeles:** lo copiado queda fuera del historial y se borra a los 2 minutos o al cerrar Forja."),
        ("b", "**Actualizaciones firmadas por el autor**, que se instalan solas cuando lo decides."),
        ("b", "Inglés, japonés y español; tema claro, oscuro o como Windows y doce colores; compatible con lectores de pantalla; nítida en cualquier escala de pantalla."),
        ("p", "El nombre: en la forja el metal toma forma a golpes de martillo sobre el yunque, como aquí cada contraseña sale del puro azar. Hasta la versión 2.1 se llamaba Wyrmhold."),

        ("h1", "Instalar y desinstalar"),
        ("h2", "Requisitos"),
        ("p", "Windows 10 (versión 1809 o posterior) u 11, con un procesador Intel o AMD de 64 bits (x64). No hay versión para procesadores ARM. Forja es un único programa de alrededor de 1 MB que no necesita nada más instalado."),
        ("h2", "Instalar"),
        ("p", f"Descarga `{SETUP}` de la página de versiones ({RELEASES}), ejecútalo y elige el idioma del instalador: inglés, japonés o español. Por defecto Forja se instala solo para tu usuario, sin permisos de administrador; la primera página también ofrece instalarla para todos los usuarios. El instalador agrega Forja y esta guía al menú Inicio, y un acceso directo en el escritorio, que puedes desmarcar. El programa arranca en el idioma que elegiste para el instalador."),
        ("p", "Instalar Forja desinstala Wyrmhold, su nombre anterior, y su configuración pasa a Forja la primera vez que la abres. Si no se puede desinstalar Wyrmhold, el instalador lo avisa, y Wyrmhold se puede quitar manualmente como se explica abajo."),
        ("h2", "Desinstalar"),
        ("p", "**Configuración \u2192 Aplicaciones \u2192 Aplicaciones instaladas \u2192 Forja \u2192 Desinstalar.** El desinstalador borra todos los archivos que instaló y después pregunta si quieres borrar también tu configuración (y la que haya dejado Wyrmhold)."),
        ("h2", "Uso portable"),
        ("p", "`forja.exe` no necesita nada más. Para usarlo desde una memoria USB o una carpeta compartida sin instalarlo, cópialo allí junto con un archivo vacío llamado `forja.ini`: la configuración se guarda entonces en ese archivo. Una copia portable no puede actualizarse sola (sección 9)."),

        ("h1", "La ventana"),
        ("img", "window_callouts_{lang}.png", "La ventana principal: tema claro, el color negro de fábrica.", 13.0),
        ("legend", [
            ("1", "**Caracteres**, **Frase** o **PIN**: el tipo de contraseña (`Ctrl+1`, `Ctrl+2`, `Ctrl+3`)."),
            ("2", "**Perfil**: configuraciones listas para los casos comunes (sección 5). Si cambias algo por tu cuenta, muestra **Personalizado**."),
            ("3", "Las opciones de ese tipo (sección 4) y la **Cantidad** de contraseñas, de 1 a 50. Cada cambio genera un juego nuevo al instante."),
            ("4", "La línea de fuerza: una barra de cinco pasos, el nivel y la entropía en bits; debajo, los caracteres posibles por posición y cuánto tardaría adivinarla (sección 7). Si las opciones no permiten generar una contraseña, dice por qué, en rojo."),
            ("5", "**Generar** (`F5`): un juego completamente nuevo."),
            ("6", "**Borrar portapapeles a los** y su tiempo (sección 8)."),
            ("7", "Guardar todas las contraseñas en un archivo de texto (`Ctrl+S`)."),
            ("8", "Copiar todas las contraseñas, una por línea (`Ctrl+Mayús+C`)."),
            ("9", "La lista. Cada fila tiene su ícono de copiar. Los dígitos van en azul y los símbolos en ámbar, para releer o dictar más fácil; lo que se copia es texto común."),
            ("10", "La línea de estado: lo que acaba de pasar (contraseñas generadas, copiadas, el portapapeles borrado, una actualización) y la versión."),
        ]),
        ("h2", "Los menús"),
        ("table", [3.0, 13.6], ["Menú", "Comandos"], [
            ["**Archivo**", "**Generar** (`F5`), **Copiar la seleccionada** (`Ctrl+C`), **Copiar todas** (`Ctrl+Mayús+C`), **Guardar como\u2026** (`Ctrl+S`), **Salir**"],
            ["**Contraseñas**", "**Caracteres**, **Frase**, **PIN** (`Ctrl+1` a `Ctrl+3`) y los ocho perfiles"],
            ["**Ver**", "**Tema: como Windows**, **Tema: claro**, **Tema: oscuro**, **Tema y color\u2026**"],
            ["**Idioma**", "**English**, **日本語**, **Español**: la ventana cambia al instante"],
            ["**Ayuda**", "**Ayuda** (`F1`), **Buscar actualizaciones\u2026**, **Buscar actualizaciones todos los días**, **Acerca de**"],
        ]),
        ("h2", "Lectores de pantalla, escala y pantallas pequeñas"),
        ("p", "El Narrador y los otros lectores de pantalla leen toda la ventana: cada control por su nombre, los modos y los colores como opciones, las fichas y los interruptores como casillas activadas o desactivadas, el deslizador con su valor, cada fila de la lista como su contraseña, y los menús; la línea de estado se lee cada vez que cambia, sin mover el foco. Todo se puede hacer con el teclado (sección 12)."),
        ("p", "Si la ventana pasa a una pantalla con otra escala, se vuelve a dibujar nítida a esa escala. En una pantalla pequeña la ventana se ajusta a la pantalla: las filas que no caben a lo ancho pasan a la línea siguiente, la banda de arriba se reduce o desaparece, y en una ventana muy baja la parte de arriba se desplaza mientras quedan a la vista al menos tres contraseñas."),

        ("h1", "Los tres tipos"),
        ("h2", "Caracteres"),
        ("p", "Los caracteres al azar son los más fuertes para su longitud; ideales cuando la contraseña se pega en vez de escribirse."),
        ("table", [3.6, 13.0], ["Opción", "Qué hace"], [
            ["**Longitud**", "De 4 a 128 caracteres (20 de fábrica). El deslizador llega a 64; en el campo de al lado se puede escribir hasta 128."],
            ["**Usar**", "Las clases de caracteres: **a\u2013z**, **A\u2013Z**, **0\u20139**, **#$%**. Cada clase activada aparece al menos una vez en cada contraseña."],
            ["**Símbolos**", "**Básicos** (`! @ # $ % & * + - = ?`), **Los 32** (todos los símbolos imprimibles), **Mínimos** (`- _ .`) o **Los míos\u2026**, que muestra el campo **Mis símbolos**: escribe exactamente los símbolos que permite el sitio (las letras y los dígitos que escribas ahí se ignoran)."],
            ["**Sin parecidos**", "Deja fuera `0 O 1 l I`, y ``| ` ' \"`` entre los símbolos: se confunden en muchas tipografías. Activada de fábrica."],
            ["**Sin repetir seguidos**", "Nunca el mismo carácter dos veces seguidas (`aa`, `11`)."],
            ["**Empezar con letra**", "El primer carácter es siempre una letra."],
            ["**Excluir**", "Cualquier otro carácter que no debe aparecer nunca, escritos uno detrás de otro."],
        ]),
        ("h2", "Frase"),
        ("img", "words_{lang}.png", "Una frase: seis palabras, guiones, mayúsculas y un dígito.", 14.0),
        ("p", "Palabras al azar de la lista de 7.776 palabras en inglés de la EFF, como `Copper-Showroom-Wrecking7-Salute`. Más larga, pero mucho más fácil de recordar, dictar o escribir en el teléfono."),
        ("table", [3.6, 13.0], ["Opción", "Qué hace"], [
            ["**Palabras**", "De 3 a 12 (6 de fábrica). Cada palabra suma 12,9 bits."],
            ["**Separador**", "**Guion**, **Espacio**, **Punto**, **Guion bajo**, **Ninguno** o **Dígito al azar**, que suma unos 3,3 bits por cada espacio entre palabras."],
            ["**Mayúsculas**", "Cada Palabra Empieza Con Mayúscula, para los sitios que las exigen. Activada de fábrica."],
            ["**Agregar un dígito**", "Un dígito después de una de las palabras, para los sitios que exigen un número. Activada de fábrica."],
        ]),
        ("p", "Con la configuración de fábrica (seis palabras, guiones, mayúsculas, un dígito) una frase tiene 83 bits: muy fuerte."),
        ("h2", "PIN"),
        ("img", "pin_{lang}.png", "Un PIN: seis dígitos, sin repetidos ni series.", 14.0),
        ("table", [3.6, 13.0], ["Opción", "Qué hace"], [
            ["**Dígitos**", "De 4 a 128 (6 de fábrica). El deslizador llega a 20; en el campo de al lado se puede escribir hasta 128."],
            ["**Sin repetir seguidos**", "Nunca el mismo dígito dos veces seguidas. Activada de fábrica."],
            ["**Sin series como 123 o 987**", "Nunca tres dígitos seguidos que suben o bajan de uno en uno: son los primeros que se prueban. Activada de fábrica."],
            ["**Excluir**", "Dígitos que no deben aparecer nunca."],
        ]),
        ("note", "Un PIN solo es seguro donde los intentos fallidos están limitados, como en tarjetas, teléfonos y cerraduras. En vez de un nivel, la línea de fuerza dice **PIN: solo con intentos limitados**, en ámbar."),

        ("h1", "Perfiles, y sitios que no aceptan algunos caracteres"),
        ("p", "El botón **Perfil**, y el menú **Contraseñas**, configuran todo de una vez. Si cambias algo por tu cuenta, el botón muestra **Personalizado**; si la configuración vuelve a coincidir con un perfil, muestra su nombre."),
        ("table", [3.3, 9.6, 3.7], ["Perfil", "Configuración", "Fuerza"], [
            ["**Estándar**", "20 caracteres, las cuatro clases, símbolos **Básicos**, sin parecidos (la de fábrica)", "122 bits"],
            ["**Máxima**", "32 caracteres, **Los 32** símbolos, con parecidos", "210 bits"],
            ["**Sin símbolos**", "20 caracteres: letras y dígitos, sin parecidos", "117 bits"],
            ["**Símbolos mínimos**", "20 caracteres, solo los símbolos `- _ .`, empezando con letra", "117 bits"],
            ["**Corta**", "12 caracteres, empezando con letra, sin repetir seguidos: para un máximo bajo", "72 bits"],
            ["**Wi-Fi**", "20 caracteres, solo minúsculas y dígitos: fácil de escribir en un teléfono o un televisor", "101 bits"],
            ["**Frase**", "6 palabras, guiones, mayúsculas, un dígito", "83 bits"],
            ["**PIN**", "6 dígitos, sin repetidos ni series", "19 bits"],
        ]),
        ("h2", "Cuando un sitio no acepta algunos caracteres"),
        ("b", "No acepta símbolos: perfil **Sin símbolos**, o desactiva la ficha **#$%**."),
        ("b", "Acepta solo algunos símbolos: **Símbolos \u2192 Mínimos**, o **Los míos\u2026**, y escribe exactamente los que el sitio permite."),
        ("b", "Rechaza caracteres concretos: escríbelos en **Excluir**."),
        ("b", "Tiene una longitud máxima: reduce la longitud, o elige el perfil **Corta**; compénsalo usando todas las clases."),
        ("b", "Pide empezar con letra, o no repetir caracteres: activa **Empezar con letra** o **Sin repetir seguidos**."),

        ("h1", "Copiar y guardar"),
        ("b", "**Una contraseña:** el ícono de copiar al final de su fila, doble clic sobre la fila, o `Enter` o `Ctrl+C` sobre la fila seleccionada. El ícono se convierte en una marca de verificación por un momento y la línea de estado dice qué contraseña se copió."),
        ("b", "**Todas:** el ícono de copiar arriba a la derecha de la lista, o `Ctrl+Mayús+C`: una por línea."),
        ("b", "**Guardar:** el ícono de la flecha, o `Ctrl+S`, las escribe todas en un archivo de texto, una por línea (propone `contraseñas.txt`, en la última carpeta usada). El archivo se escribe entero o no se escribe. Guárdalo en un lugar seguro o bórralo después de usarlo."),
        ("p", "Cada vez que abres el programa y cada cambio de opciones generan un juego nuevo; no queda nada de la anterior."),
        ("note", "**Buena práctica:** envía la contraseña por un canal distinto del archivo \u2014 el archivo por correo electrónico, la contraseña en otro mensaje, un chat o una llamada."),

        ("h1", "Cómo se generan, y su fuerza"),
        ("h2", "Puro azar, sin sesgo"),
        ("p", "Cada carácter o palabra sale del generador criptográfico de números aleatorios de Windows (`BCryptGenRandom`), sin sesgo. Cuando una contraseña rompe una regla \u2014 falta una clase, dos vecinos iguales, una secuencia como 123 \u2014 se descarta y se sortea de nuevo, nunca se corrige. Así, todas las contraseñas que cumplen las reglas son igual de probables: ninguna posición, clase o patrón queda favorecido."),
        ("h2", "La entropía, contada exacta"),
        ("p", "La entropía es la cantidad de contraseñas distintas que pueden salir con esas opciones, escrita como potencia de dos: n bits son 2\u207F contraseñas igual de probables, y cada bit más duplica el trabajo de quien intente adivinarla. Forja la cuenta exacta, con todas las reglas:"),
        ("b", "**Caracteres:** no es simplemente longitud \u00D7 log\u2082(caracteres), que contaría contraseñas que las reglas prohíben. Forja cuenta, posición por posición, las contraseñas en las que aparece cada clase activada, sin vecinos iguales y con una letra primero si se pide. Los 20 caracteres de fábrica, entre 68 posibles, dan 121,6 bits."),
        ("b", "**Frase:** 12,9 bits por palabra (log\u2082 7.776), más 3,3 bits por espacio con **Dígito al azar**, más, con **Agregar un dígito**, log\u2082(10 \u00D7 palabras) por qué dígito y después de qué palabra. **Mayúsculas** no suma nada: todas las palabras la llevan. La cuenta supone que el atacante conoce la lista y las opciones. Cuenta frases distintas, no solo sorteos distintos: ninguna palabra de la lista es el comienzo de otra (tampoco las que llevan guion, como `t-shirt` o `drop-down`), así que, con cualquier separador, incluso **Ninguno**, y con o sin mayúsculas, cada frase sale de un único sorteo."),
        ("b", "**PIN:** las combinaciones de los dígitos permitidos, contadas exactas sin repetidos ni series: seis dígitos dan 19,1 bits."),
        ("h2", "Niveles y tiempo para adivinarla"),
        ("table", [4.0, 4.0, 8.6], ["Nivel", "Entropía", "Se muestra en"], [
            ["**Muy débil**", "menos de 36 bits", "rojo"],
            ["**Débil**", "de 36 a 49 bits", "rojo"],
            ["**Aceptable**", "de 50 a 63 bits", "ámbar"],
            ["**Fuerte**", "de 64 a 79 bits", "verde"],
            ["**Muy fuerte**", "80 bits o más", "verde"],
        ]),
        ("p", "El tiempo para adivinarla supone un atacante que tiene el archivo y prueba 100.000 millones de contraseñas por segundo, y que en promedio acierta a mitad de camino: desde **al instante**, **segundos** y **minutos** hasta **millones de años**, **miles de millones de años**, **billones de años** y **mucho más que la edad del universo**."),
        ("table", [8.0, 3.0, 5.6], ["Opciones", "Entropía", "Adivinarla"], [
            ["Perfil Estándar, 20 caracteres", "122 bits", "mucho más que la edad del universo"],
            ["Frase de 8 palabras", "110 bits", "billones de años"],
            ["16 caracteres, el resto estándar", "97 bits", "miles de millones de años"],
            ["Perfil Corta, 12 caracteres", "72 bits", "siglos"],
            ["Frase de 6 palabras (de fábrica)", "83 bits", "millones de años"],
            ["Frase de 4 palabras", "57 bits", "días a meses"],
            ["Frase de 3 palabras, sin mayúsculas ni dígito", "39 bits", "segundos"],
            ["PIN de 6 dígitos", "19 bits", "al instante"],
        ]),
        ("h2", "Cuando las opciones no permiten generar una contraseña"),
        ("p", "La línea de fuerza dice entonces por qué, en rojo, **Generar** queda deshabilitado y la lista queda vacía. Flexibiliza alguna regla:"),
        ("b", "**Activa al menos un tipo de carácter.**"),
        ("b", "**No queda nada de los símbolos: permite alguno o desactiva ese tipo.** (también con letras o dígitos, cuando **Excluir** o **Sin parecidos** los quitan todos)"),
        ("b", "**Ninguna contraseña puede cumplir todas estas reglas. Flexibiliza alguna.**"),
        ("b", "**Están excluidos todos los dígitos.** (PIN)"),
        ("b", "**Estas reglas dejan tan pocas contraseñas que generar una tardaría demasiado. Flexibiliza alguna.** Por ejemplo: solo dígitos, sin repetir y longitud 128."),

        ("h1", "Protección del portapapeles"),
        ("p", "Toda copia lleva los formatos que la dejan fuera del historial del portapapeles de Windows (`Win+V`), del portapapeles en la nube y de los monitores de portapapeles. Arriba de la lista, **Borrar portapapeles a los** viene activado con **2 min**; el desplegable de al lado ofrece 10 s, 30 s, 1, 2, 5 y 10 minutos, y elegir un tiempo también activa el borrado. La línea de estado muestra los segundos que faltan."),
        ("b", "Cuando pasa el tiempo, lo copiado se borra \u2014 pero solo si el portapapeles sigue teniendo exactamente lo que puso Forja. Forja recuerda el número de secuencia del portapapeles y un SHA-256 del texto, nunca el texto. Lo que tú u otro programa hayan copiado después no se toca; el mismo texto devuelto por una herramienta de portapapeles se borra igual."),
        ("b", "Si cierras Forja antes de que pase el tiempo, lo copiado se borra en el acto, y lo mismo al cerrar sesión o apagar el equipo \u2014 otra vez, solo si sigue siendo de Forja."),
        ("b", "Desactivado, lo copiado queda hasta que copies otra cosa, también después de cerrar Forja."),
        ("b", "En `forja.ini`: `ClearClipboard=1` o `0` y `ClearSeconds=120` (de 5 a 3600). Solo para una sesión, sin guardarse: `/clearafter:90` (segundos), `/clearafter:5m` (minutos) o `/clearafter:off`."),

        ("h1", "Actualizaciones"),
        ("h2", "La pregunta del primer arranque"),
        ("p", "La primera vez que abres Forja te pregunta una sola vez **¿Buscar versiones nuevas automáticamente?**: **Buscar todos los días** o **No buscar**. No se le consulta nada a GitHub antes de que contestes. La respuesta se guarda y puedes cambiarla cuando quieras con **Ayuda \u2192 Buscar actualizaciones todos los días**; si cierras la pregunta sin contestar, se vuelve a preguntar la próxima vez. Después de una actualización la búsqueda diaria queda como estaba, sin preguntar."),
        ("h2", "Buscar"),
        ("p", "**Ayuda \u2192 Buscar actualizaciones\u2026** le pregunta a GitHub si se publicó una versión nueva; con **Buscar actualizaciones todos los días** activado, Forja también lo hace sola, como mucho una vez por día. Solo se pide el número de la última versión: no se envía nada sobre ti, tus contraseñas ni tu configuración. Si hay una versión nueva, una sola pregunta: **Actualizar ahora**, **Ver novedades** (abre la página de la versión) o **Más tarde**."),
        ("h2", "Instalar una actualización"),
        ("n", "**Actualizar ahora** comprueba primero que la lista de sumas de verificación de la versión (`SHA256SUMS.txt`) lleve la firma del autor (`release.sig`), contra dos claves incorporadas en Forja. Una versión sin firma válida no instala nada; solo se puede descargar manualmente desde su página."),
        ("n", "Descarga el instalador en una carpeta nueva propia y compara su SHA-256 con la lista firmada. Si no coincide, el archivo se borra y nunca se ejecuta."),
        ("n", "Forja se cierra (lo copiado que esperaba borrarse se borra), el instalador instala en silencio conservando la carpeta, el idioma y las opciones elegidos la primera vez, y Forja vuelve a abrirse; la línea de estado dice **Forja se actualizó a la** versión nueva."),
        ("p", "Una copia portable (no instalada con el instalador) no puede actualizarse sola: en su lugar ofrece abrir la página de descarga."),

        ("h1", "Tema, color e idioma"),
        ("img", "theme_{lang}.png", "Ver \u2192 Tema y color: la apariencia y doce colores.", 12.0),
        ("p", "**Ver \u2192 Tema y color\u2026** elige la **Apariencia** (**Como Windows**, **Claro** u **Oscuro**) y uno de doce colores: Negro (el de fábrica), Azul, Dorado, Morado, Celeste, Naranja, Rojo, Amarillo, Verde, Verde azulado, Rosa y Blanco. El color pinta los botones, la banda de arriba y el fondo del ícono, en la ventana y en la barra de tareas. El menú **Ver** también cambia el tema directamente."),
        ("img", "window_dark_{lang}.png", "El tema oscuro con el color Naranja.", 11.0),
        ("p", "El menú **Idioma** cambia al instante la ventana, los menús, la Ayuda y Acerca de entre inglés, japonés y español."),

        ("h1", "Configuración y privacidad"),
        ("p", "Forja guarda tus elecciones en `%APPDATA%\\Forja\\forja.ini` (o en el `forja.ini` portable, sección 2): el tipo y las opciones de cada tipo, la cantidad de contraseñas, el borrado del portapapeles y su tiempo, la búsqueda diaria de actualizaciones, el tema, el color, el idioma, la última carpeta usada para guardar, y la posición y el tamaño de la ventana."),
        ("p", "**Nunca se guardan contraseñas ahí**, ni nada de lo copiado. Borra el archivo para volver a la configuración de fábrica. La línea de comandos (sección 13) nunca lo escribe."),

        ("h1", "Atajos de teclado"),
        ("table", [5.0, 11.6], ["Teclas", "Acción"], [
            ["`Ctrl+1`, `Ctrl+2`, `Ctrl+3`", "**Caracteres**, **Frase**, **PIN**"],
            ["`F5`", "**Generar** un juego nuevo"],
            ["`Enter`, `Ctrl+C`, doble clic", "Copiar la contraseña seleccionada"],
            ["`Ctrl+Mayús+C`", "Copiar todas, una por línea"],
            ["`Ctrl+S`", "Guardar todas en un archivo de texto"],
            ["`F1`", "Ayuda"],
            ["`Esc`", "Cerrar Ayuda, Acerca de o Tema y color"],
            ["`Tab`, `Mayús+Tab`", "Pasar de un control a otro; `Espacio` activa o desactiva una ficha o un interruptor"],
            ["Flechas, `RePág`, `AvPág`, `Inicio`, `Fin`", "Mover el deslizador (de uno en uno, de cuatro en cuatro, hasta un extremo)"],
            ["`Alt` + letra subrayada", "Abrir un menú"],
        ]),

        ("h1", "Línea de comandos"),
        ("p", "Para scripts y pruebas, el programa también funciona sin ventana:"),
        ("code", "forja.exe /out:<archivo> [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [opciones]"),
        ("p", "`/out` escribe las contraseñas en el archivo, una por línea, y termina. Sin opciones usa el perfil **Estándar**, diga lo que diga la configuración; las opciones no cambian la configuración y no se escribe nada en `forja.ini`."),
        ("table", [5.4, 11.2], ["Opción", "Significado"], [
            ["`/out:<archivo>`", "Escribir las contraseñas y terminar"],
            ["`/entropy:<archivo>`", "Escribir en cambio una línea: la entropía en bits, los caracteres por posición (o las palabras de la lista) y 0 si las opciones permiten generar una contraseña"],
            ["`/mode:chars|words|pin`", "El tipo (caracteres por defecto)"],
            ["`/len:N`", "Longitud de los caracteres, o dígitos del PIN"],
            ["`/words:N`", "Palabras de la frase"],
            ["`/count:N`", "Cantidad: de 1 a 100.000 (5 por defecto)"],
            ["`/preset:N`", "Un perfil: 0 Estándar, 1 Máxima, 2 Sin símbolos, 3 Símbolos mínimos, 4 Corta, 5 Wi-Fi, 6 Frase, 7 PIN (reemplaza las otras opciones)"],
            ["`/classes:luds`", "Las clases a usar: l minúsculas, u mayúsculas, d dígitos, s símbolos"],
            ["`/symbols:basic|all|min|custom:<caracteres>`", "El juego de símbolos"],
            ["`/exclude:<caracteres>`", "Caracteres (o dígitos del PIN) a excluir"],
            ["`/ambiguous`", "Permitir los parecidos (excluidos por defecto)"],
            ["`/norepeat`, `/startletter`", "Sin repetir seguidos; empezar con letra"],
            ["`/sep:hyphen|space|dot|underscore|none|digit`", "Separador de la frase"],
            ["`/nocap`, `/nonumber`", "Frase sin mayúsculas; sin el dígito"],
            ["`/allowruns`", "PIN con series como 123 permitidas"],
        ]),
        ("p", "Para la ventana: `/lang:en|jp|es`, `/theme:system|light|dark` (`system`, o `auto`, sigue a Windows), `/accent:N` (de 0 a 11, en el orden de **Tema y color**; 0 es el negro de fábrica) y `/clearafter:N[s|m]|off` (sección 8). `/theme` y `/accent` mandan sobre lo guardado en `forja.ini`, también `/theme:system` y `/accent:0`. `/selftest:<archivo>` prueba en unos cinco segundos el borrado del portapapeles, su configuración, las verificaciones de la actualización y la escala del tiempo para adivinarla, y escribe un informe (sección 14)."),
        ("table", [2.4, 14.2], ["Código de salida", "Significado (/out)"], [
            ["0", "Listo"],
            ["2", "No se pudo escribir el archivo"],
            ["3", "Falló el generador de números aleatorios"],
            ["5", "Las opciones no permiten generar una contraseña"],
            ["6", "Las reglas son demasiado estrictas: ningún sorteo las cumplió a tiempo"],
        ]),
        ("p", "Si falla, el archivo queda vacío. Ejemplos:"),
        ("code", "forja.exe /out:wifi.txt /preset:5 /count:3\nforja.exe /out:frases.txt /mode:words /words:5 /sep:space /nocap\nforja.exe /out:pines.txt /mode:pin /len:8 /count:10"),

        ("h1", "Compilar desde el código"),
        ("p", "El código es C y la API Win32, sin código de terceros. Con el GCC mingw-w64 de MSYS2 en `C:\\msys64`, `build.bat` genera `build\\forja.exe`; `tools\\make_installer.ps1` arma el instalador con Inno Setup 6. La versión está escrita solo en `src\\version.h`."),
        ("p", "`tools\\run_checks.ps1` y `tools\\check_options.py` verifican las contraseñas y la entropía exacta contra cálculos independientes, sobre cientos de miles de contraseñas, y `tools\\check_clipboard.ps1` el portapapeles a través de la ventana real. `run_checks.ps1` también ejecuta `forja.exe /selftest`, que guarda tu portapapeles antes y lo restaura después, y nunca toca tu configuración."),

        ("h1", "Licencia y créditos"),
        ("p", "Software libre bajo la Licencia Pública General de GNU, solo versión 2 (GPL-2.0-only): puedes usarlo, copiarlo, estudiarlo y modificarlo, y quien lo redistribuya debe hacerlo bajo la misma licencia, con el código fuente. Se ofrece sin garantía. El texto completo está en `LICENSE.txt`, en la carpeta de instalación. \u00A9 2026 Gabriel Castiglia."),
        ("p", f"Código fuente, versiones y actualizaciones: https://{REPO}"),
        ("p", "Palabras de las frases: la lista grande de la EFF, de la Electronic Frontier Foundation, bajo CC BY 3.0 US (eff.org/dice). Ícono: un yunque y un martillo dibujados para Forja (2026), bajo la GPL-2.0 como el resto."),
    ],
}

# ---------------------------------------------------------------------------------------- docx

_INLINE = re.compile(r"(\*\*.+?\*\*|``.+?``|`[^`]+`)")     # ``...`` is code that holds a backtick


def set_font(run, latin, east, size=None, bold=None, color=None):
    run.font.name = latin
    rpr = run._element.get_or_add_rPr()
    fonts = rpr.find(qn("w:rFonts"))
    if fonts is None:
        fonts = rpr.makeelement(qn("w:rFonts"), {})
        rpr.insert(0, fonts)
    for k in ("w:ascii", "w:hAnsi", "w:cs"):
        fonts.set(qn(k), latin)
    fonts.set(qn("w:eastAsia"), east)
    if size: run.font.size = Pt(size)
    if bold is not None: run.font.bold = bold
    if color is not None: run.font.color.rgb = color


def rich(paragraph, text, g, size=None, color=None, bold=False):
    """Adds text with **bold** and `code` marks to a paragraph."""
    f, e = g["font"], g["east"]
    for part in _INLINE.split(text):
        if not part:
            continue
        if part.startswith("**") and part.endswith("**"):
            set_font(paragraph.add_run(part[2:-2]), f, e, size, True, color)
        elif part.startswith("`") and part.endswith("`"):
            code = part[2:-2] if part.startswith("``") else part[1:-1]
            set_font(paragraph.add_run(code), MONO, e, (size or g["size"]) - 0.5, bold, color)
        else:
            set_font(paragraph.add_run(part), f, e, size, bold, color)


def shade(element, fill: str) -> None:
    """Fills a paragraph or a table cell with a solid colour ('RRGGBB')."""
    pr = element.get_or_add_pPr() if element.tag == qn("w:p") else element.get_or_add_tcPr()
    shd = OxmlElement("w:shd")
    shd.set(qn("w:val"), "clear")
    shd.set(qn("w:color"), "auto")
    shd.set(qn("w:fill"), fill)
    pr.append(shd)


def border(paragraph, side: str, color: str, size: int = 6, space: int = 2) -> None:
    """A line on one side of a paragraph (size in eighths of a point)."""
    ppr = paragraph._p.get_or_add_pPr()
    bdr = ppr.find(qn("w:pBdr"))
    if bdr is None:
        bdr = OxmlElement("w:pBdr")
        ppr.append(bdr)
    line = OxmlElement(f"w:{side}")
    for k, v in (("val", "single"), ("sz", str(size)), ("space", str(space)), ("color", color)):
        line.set(qn(f"w:{k}"), v)
    bdr.append(line)


def cell_borders(cell, color: str, sides=("top", "left", "bottom", "right"), size: int = 4) -> None:
    tcpr = cell._tc.get_or_add_tcPr()
    b = OxmlElement("w:tcBorders")
    for side in ("top", "left", "bottom", "right"):
        el = OxmlElement(f"w:{side}")
        if side in sides:
            for k, v in (("val", "single"), ("sz", str(size)), ("space", "0"), ("color", color)):
                el.set(qn(f"w:{k}"), v)
        else:
            el.set(qn("w:val"), "nil")
        b.append(el)
    tcpr.append(b)


def cell_margins(table, top=60, bottom=60, left=110, right=110) -> None:
    """Inner margins of every cell, in twentieths of a point."""
    tblpr = table._tbl.tblPr
    mar = OxmlElement("w:tblCellMar")
    for side, v in (("top", top), ("left", left), ("bottom", bottom), ("right", right)):
        el = OxmlElement(f"w:{side}")
        el.set(qn("w:w"), str(v))
        el.set(qn("w:type"), "dxa")
        mar.append(el)
    tblpr.append(mar)


def fixed_table(doc, rows, widths):
    t = doc.add_table(rows=rows, cols=len(widths))
    t.alignment = WD_TABLE_ALIGNMENT.CENTER
    t.autofit = False
    layout = OxmlElement("w:tblLayout")
    layout.set(qn("w:type"), "fixed")
    t._tbl.tblPr.append(layout)
    for row in t.rows:
        for cell, w in zip(row.cells, widths):
            cell.width = Cm(w)
    for col, w in zip(t.columns, widths):
        col.width = Cm(w)
    return t


def row_flags(row, header=False) -> None:
    trpr = row._tr.get_or_add_trPr()
    trpr.append(OxmlElement("w:cantSplit"))
    if header:
        trpr.append(OxmlElement("w:tblHeader"))


def field(paragraph, instr: str, cached: str, g, size=None, color=None) -> None:
    """A Word field (PAGE, PAGEREF ...) with its cached result."""
    f, e = g["font"], g["east"]
    def fld(kind):
        r = paragraph.add_run()
        set_font(r, f, e, size, None, color)
        c = OxmlElement("w:fldChar")
        c.set(qn("w:fldCharType"), kind)
        r._r.append(c)
    fld("begin")
    r = paragraph.add_run()
    set_font(r, f, e, size, None, color)
    it = OxmlElement("w:instrText")
    it.set(qn("xml:space"), "preserve")
    it.text = f" {instr} "
    r._r.append(it)
    fld("separate")
    set_font(paragraph.add_run(cached), f, e, size, None, color)
    fld("end")


_bm = [0]


def bookmark(paragraph, name: str) -> None:
    _bm[0] += 1
    start = OxmlElement("w:bookmarkStart")
    start.set(qn("w:id"), str(_bm[0]))
    start.set(qn("w:name"), name)
    end = OxmlElement("w:bookmarkEnd")
    end.set(qn("w:id"), str(_bm[0]))
    paragraph._p.insert(1 if paragraph._p.pPr is not None else 0, start)
    paragraph._p.append(end)


def picture_frame(run, color: str) -> None:
    """A thin line around a picture, so that a light screenshot does not melt into the page."""
    for sppr in run._r.iter(qn("pic:spPr")):
        ln = OxmlElement("a:ln")
        ln.set("w", "9525")
        fill = OxmlElement("a:solidFill")
        clr = OxmlElement("a:srgbClr")
        clr.set("val", color)
        fill.append(clr)
        ln.append(fill)
        sppr.append(ln)


def style_headings(doc, g) -> None:
    f, e = g["font"], g["east"]
    for name, size, before, after in (("Heading 1", 15, 18, 6), ("Heading 2", 12, 12, 4)):
        st = doc.styles[name]
        st.font.name = f
        st.font.size = Pt(size)
        st.font.bold = True
        st.font.italic = False
        st.font.color.rgb = ACCENT
        rpr = st.element.get_or_add_rPr()
        fonts = rpr.find(qn("w:rFonts"))
        if fonts is None:
            fonts = OxmlElement("w:rFonts")
            rpr.insert(0, fonts)
        for k in list(fonts.attrib):
            del fonts.attrib[k]
        for k in ("w:ascii", "w:hAnsi", "w:cs", "w:eastAsia"):
            fonts.set(qn(k), f if k != "w:eastAsia" else e)
        pf = st.paragraph_format
        pf.space_before, pf.space_after = Pt(before), Pt(after)
        pf.keep_with_next = True
        pf.line_spacing = 1.0


def cover(doc, g, lang) -> None:
    """Page 1: the title band as at the top of the window, the main window and the credits."""
    f, e = g["font"], g["east"]
    # one cell (two cells leave a hairline between them in some PDF viewers): the icon, the
    # name raised beside it so that both share a centre line, and the tagline below
    t = fixed_table(doc, 1, [WIDTH])
    cell = t.rows[0].cells[0]
    shade(cell._tc, PAL["band"])
    cell_borders(cell, PAL["band"], ())
    p = cell.paragraphs[0]
    p.paragraph_format.space_before = Pt(16)
    p.paragraph_format.space_after = Pt(4)
    p.paragraph_format.left_indent = Cm(0.5)
    p.paragraph_format.line_spacing = 1.0
    p.add_run().add_picture(str(ICON), width=Cm(2.2))
    name = p.add_run(" " + g["title"])
    set_font(name, f, e, 36, True, WHITE)
    pos = OxmlElement("w:position")
    pos.set(qn("w:val"), "26")                    # half-points: the name raised to the icon's middle
    name._r.get_or_add_rPr().append(pos)
    p = cell.add_paragraph()
    p.paragraph_format.left_indent = Cm(0.5)
    p.paragraph_format.space_after = Pt(16)
    p.paragraph_format.line_spacing = 1.1
    set_font(p.add_run(g["tagline"]), f, e, 12, False, COLOR["bandSub"])

    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(26)
    p.paragraph_format.space_after = Pt(2)
    set_font(p.add_run(g["kind"]), f, e, 22, True, ACCENT)
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(18)
    set_font(p.add_run(g["meta"]), f, e, 11, False, GREY)

    p = doc.add_paragraph()
    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p.paragraph_format.space_after = Pt(20)
    run = p.add_run()
    run.add_picture(str(IMAGES / f"window_{lang}.png"), width=Cm(13.0))
    picture_frame(run, PAL["fieldBorder"])

    p = doc.add_paragraph()
    p.paragraph_format.space_before = Pt(10)
    p.paragraph_format.space_after = Pt(2)
    border(p, "top", PAL["border"], 6, 8)
    set_font(p.add_run(g["byline"]), f, e, 9.5, False, TEXT)
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(0)
    set_font(p.add_run(g["licence"]), f, e, 9, False, GREY)
    p.add_run().add_break(WD_BREAK.PAGE)


def contents(doc, g, heads) -> None:
    """Page 2: the sections and their pages (PAGEREF fields to the bookmarks of the headings)."""
    f, e = g["font"], g["east"]
    p = doc.add_paragraph()
    p.paragraph_format.space_after = Pt(14)
    border(p, "bottom", PAL["border"], 6, 4)
    set_font(p.add_run(g["contents"]), f, e, 18, True, ACCENT)
    for level, number, text, bm in heads:
        p = doc.add_paragraph()
        pf = p.paragraph_format
        pf.tab_stops.add_tab_stop(Cm(WIDTH), WD_TAB_ALIGNMENT.RIGHT, WD_TAB_LEADER.DOTS)
        pf.tab_stops.add_tab_stop(Cm(1.0 if level == 1 else 1.9))
        pf.left_indent = Cm(0 if level == 1 else 1.0)
        pf.space_before = Pt(6 if level == 1 else 0)
        pf.space_after = Pt(1)
        pf.line_spacing = 1.0
        size, bold, color = (11, True, TEXT) if level == 1 else (10, False, GREY)
        link = OxmlElement("w:hyperlink")
        link.set(qn("w:anchor"), bm)
        link.set(qn("w:history"), "1")
        p._p.append(link)
        tmp = doc.add_paragraph()                 # runs are made here, then moved into the link
        set_font(tmp.add_run(f"{number}\t{text}\t"), f, e, size, bold, color)
        field(tmp, f"PAGEREF {bm} \\h", "#", g, size, color)
        for r in list(tmp._p):
            if r.tag == qn("w:r"):
                link.append(r)
        tmp._p.getparent().remove(tmp._p)
    # the first section starts a new page itself (page_break_before): a break of its own here
    # would leave an empty page whenever the contents fill theirs exactly


def table_block(doc, g, widths, header, rows) -> None:
    t = fixed_table(doc, 1 + len(rows), widths)
    cell_margins(t)
    size = g["size"] - 1
    for i, row in enumerate([header] + rows):
        tr = t.rows[i]
        row_flags(tr, header=(i == 0))
        for j, (cell, text) in enumerate(zip(tr.cells, row)):
            cell_borders(cell, PAL["border"], ("bottom",) if i else ("bottom",), 4)
            if i == 0:
                shade(cell._tc, PAL["band"])
            elif i % 2 == 0:
                shade(cell._tc, PAL["page"])
            p = cell.paragraphs[0]
            p.paragraph_format.space_after = Pt(0)
            p.paragraph_format.line_spacing = 1.1
            # a short table stays on one page; a long one may break, its header repeated
            p.paragraph_format.keep_with_next = (i < len(rows)) if len(rows) <= 10 else (i == 0)
            if i == 0:
                set_font(p.add_run(text), g["font"], g["east"], size, True, WHITE)
            else:
                rich(p, text, g, size)
    doc.add_paragraph().paragraph_format.space_after = Pt(2)


def legend_block(doc, g, items) -> None:
    t = fixed_table(doc, len(items), [1.0, WIDTH - 1.0])
    cell_margins(t, 40, 40, 60, 80)
    for i, (mark, text) in enumerate(items):
        tr = t.rows[i]
        row_flags(tr)
        a, b = tr.cells
        keep = i < len(items) - 1                                 # the marks stay together
        for c in (a, b):
            cell_borders(c, PAL["border"], ("bottom",) if i < len(items) - 1 else (), 4)
        p = a.paragraphs[0]
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.keep_with_next = keep
        set_font(p.add_run(mark), g["font"], g["east"], g["size"], True, COLOR["ember"])
        p = b.paragraphs[0]
        p.paragraph_format.space_after = Pt(0)
        p.paragraph_format.keep_with_next = keep
        p.paragraph_format.line_spacing = 1.1
        rich(p, text, g, g["size"] - 0.5)
    doc.add_paragraph().paragraph_format.space_after = Pt(2)


def build(lang: str, g: dict) -> Path:
    _bm[0] = 0
    doc = Document()
    sec = doc.sections[0]
    sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
    for side in ("left_margin", "right_margin"):
        setattr(sec, side, Cm(2.2))
    sec.top_margin, sec.bottom_margin = Cm(2.0), Cm(2.0)
    sec.footer_distance = Cm(1.0)
    sec.different_first_page_header_footer = True

    f, e = g["font"], g["east"]
    normal = doc.styles["Normal"]
    normal.font.name = f
    normal.font.size = Pt(g["size"])
    normal.font.color.rgb = TEXT
    normal.element.get_or_add_rPr()
    normal.paragraph_format.space_after = Pt(6)
    normal.paragraph_format.line_spacing = 1.2 if lang == "jp" else 1.15
    style_headings(doc, g)

    # the numbers and bookmarks of the headings, needed by the contents before the body exists
    heads, h1, h2 = [], 0, 0
    for b in g["blocks"]:
        if b[0] == "h1":
            h1, h2 = h1 + 1, 0
            heads.append((1, f"{h1}.", b[1], f"s{h1}"))
        elif b[0] == "h2":
            h2 += 1
            heads.append((2, f"{h1}.{h2}", b[1], f"s{h1}_{h2}"))

    cover(doc, g, lang)
    contents(doc, g, heads)

    it = iter(heads)
    n, figure = 0, 0
    blocks = g["blocks"]
    for k, b in enumerate(blocks):
        kind = b[0]
        # a paragraph that introduces a table, a picture or a command stays on its page
        glue = k + 1 < len(blocks) and blocks[k + 1][0] in ("table", "img", "legend", "code")
        if kind in ("h1", "h2"):
            n = 0
            level, number, text, bm = next(it)
            p = doc.add_paragraph(style="Heading 1" if level == 1 else "Heading 2")
            p.paragraph_format.page_break_before = number == "1."
            if level == 1:
                border(p, "bottom", PAL["border"], 6, 3)
            set_font(p.add_run(f"{number}\u2002{text}"), f, e)
            bookmark(p, bm)
        elif kind == "p":
            p = doc.add_paragraph()
            p.paragraph_format.keep_with_next = glue
            rich(p, b[1], g)
        elif kind in ("b", "n"):
            n += 1
            p = doc.add_paragraph()
            p.paragraph_format.left_indent = Cm(0.8)
            p.paragraph_format.first_line_indent = Cm(-0.5)
            p.paragraph_format.space_after = Pt(4)
            set_font(p.add_run(("\u2022" if kind == "b" else f"{n}.") + "\u00A0\u00A0"), f, e, bold=(kind == "n"), color=ACCENT)
            rich(p, b[1], g)
        elif kind == "code":
            # a text field of the program: the field colour, framed in the field border
            p = doc.add_paragraph()
            p.paragraph_format.left_indent = Cm(0.3)
            p.paragraph_format.right_indent = Cm(0.3)
            p.paragraph_format.line_spacing = 1.15
            shade(p._p, PAL["field"])
            for side in ("top", "left", "bottom", "right"):
                border(p, side, PAL["fieldBorder"], 4, 5)
            lines = b[1].split("\n")
            for i, line in enumerate(lines):
                r = p.add_run(line)
                set_font(r, MONO, MONO, 9, color=TEXT)
                if i < len(lines) - 1:
                    r.add_break()
        elif kind == "note":
            p = doc.add_paragraph()
            p.paragraph_format.left_indent = Cm(0.3)
            p.paragraph_format.right_indent = Cm(0.3)
            p.paragraph_format.space_before = Pt(4)
            p.paragraph_format.space_after = Pt(10)
            shade(p._p, PAL["emberSoft"])
            border(p, "left", PAL["ember"], 24, 8)
            for side in ("top", "bottom", "right"):
                border(p, side, PAL["emberSoft"], 4, 5)
            rich(p, b[1], g)
        elif kind == "legend":
            legend_block(doc, g, b[1])
        elif kind == "table":
            table_block(doc, g, b[1], b[2], b[3])
        elif kind == "img":
            figure += 1
            p = doc.add_paragraph()
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.keep_with_next = True
            p.paragraph_format.space_before = Pt(4)
            p.paragraph_format.space_after = Pt(4)
            run = p.add_run()
            run.add_picture(str(IMAGES / b[1].format(lang=lang)), width=Cm(b[3]))
            picture_frame(run, PAL["fieldBorder"])
            c = doc.add_paragraph()
            c.alignment = WD_ALIGN_PARAGRAPH.CENTER
            c.paragraph_format.space_after = Pt(10)
            label = f"{g['figure']} {figure}" + ("\u3000" if lang == "jp" else ".  ")
            set_font(c.add_run(label), f, e, 9, True, GREY)
            set_font(c.add_run(b[2]), f, e, 9, False, GREY)
        elif kind == "page":
            doc.add_paragraph().add_run().add_break(WD_BREAK.PAGE)

    # the footer of every page but the cover: the guide on the left, the page on the right
    footer = sec.footer.paragraphs[0]
    footer.style = doc.styles["Normal"]               # the Footer style brings tab stops of its own
    footer.paragraph_format.space_after = Pt(0)
    footer.paragraph_format.tab_stops.add_tab_stop(Cm(WIDTH), WD_TAB_ALIGNMENT.RIGHT)
    border(footer, "top", PAL["border"], 4, 6)
    set_font(footer.add_run(f"{g['title']} {VERSION}  \u00B7  {g['kind']}\t"), f, e, 8.5, False, GREY)
    field(footer, "PAGE", "1", g, 8.5, GREY)
    sec.first_page_footer.paragraphs[0].text = ""

    # the document's own properties name only the author; nothing else is carried over
    cp = doc.core_properties
    cp.author = cp.last_modified_by = AUTHOR
    cp.title = f"{g['title']} \u2014 {g['kind']}"
    cp.subject = g["tagline"]
    cp.comments = cp.keywords = cp.category = ""
    cp.language = {"en": "en-GB", "es": "es-AR", "jp": "ja-JP"}[lang]
    cp.created = cp.modified = datetime.strptime(DATE, "%Y-%m-%d")     # the release date, not the template's
    _set_language(doc, cp.language)
    out = DOCS / f"forja_guide_{lang}.docx"
    doc.save(out)
    return out


def _set_language(doc, tag: str) -> None:
    """The proofing language of the text, so that a spelling checker reads it in the right one."""
    rpr = doc.styles["Normal"].element.get_or_add_rPr()
    lang = OxmlElement("w:lang")
    lang.set(qn("w:val"), tag if not tag.startswith("ja") else "en-US")
    lang.set(qn("w:eastAsia"), tag if tag.startswith("ja") else "ja-JP")
    rpr.append(lang)


def main() -> None:
    DOCS.mkdir(exist_ok=True)
    for lang, g in GUIDES.items():
        files = [f"window_{lang}.png"] + [b[1].format(lang=lang) for b in g["blocks"] if b[0] == "img"]
        for name in files:
            if not (IMAGES / name).exists():
                raise SystemExit(f"missing screenshot {IMAGES / name}")
        print("written:", build(lang, g))


if __name__ == "__main__":
    main()
