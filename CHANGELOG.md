# Changelog

## 2.0.1 — 2026-11-10

- The About window shows the project page; the user guides link to it and say that Wyrmhold is for x64 Windows
  (there is no ARM build).
- `/out`, `/entropy` and `/shot` accept file names in any language (they were read in the ANSI code page).
- Using only the command line no longer creates an empty settings folder in `%APPDATA%`.
- The version is written once, in `src/version.h`; every source file carries the GPL-2.0-only header.

## 2.0.0 — 2026-11-10

- New name: **Wyrmhold** — Old English *wyrm*, dragon, and *hold*, stronghold: the dragon's keep. New icon:
  Athanasius Kircher's winged dragon (1665) guarding its tower, in engraving style.
- Three kinds of password: characters, passphrases (EFF large word list, 3–12 words, six separators) and PINs
  (no repeats, no runs).
- Profiles: Standard, Maximum, No symbols, Minimal symbols, Short, Wi-Fi, Passphrase, PIN.
- Character options: which kinds to use, symbol set (basic, all, minimal, your own), characters to leave out,
  no repeats in a row, start with a letter.
- Uniform generation over every password that follows the rules (rejection of the whole password), and the
  exact entropy, counted with every rule included; strength in five steps with a guessing time.
- New look: anti-aliased drawing, rounded fields and buttons, a band with the icon, owner-drawn menus.
- View → Theme and colour: light, dark or as Windows, and twelve colours that also colour the icon; black by default.
- Setup in English, Japanese and Spanish with an uninstaller; settings in `%APPDATA%\Wyrmhold`, or portable
  with a `wyrmhold.ini` beside the program.
- Command line: `/mode`, `/words`, `/classes`, `/symbols`, `/exclude`, `/norepeat`, `/startletter`, `/sep`,
  `/nocap`, `/nonumber`, `/allowruns`, `/preset`, `/entropy`, `/accent`, `/open`.
- User guides rewritten; new check `tools/check_options.py`.

## 1.0.1 — 2026-11-10

- The About window shows the license (GPL-2.0-only, no warranty) in English, Japanese and Spanish.
- User guides: new section 10, License.
- Project files: README (English, Spanish, Japanese), LICENSE, CHANGELOG, `.gitignore`, `.gitattributes`.

## 1.0.0 — 2026-11-09

- First release: Win32 window in C, static and portable; length 4–128, 1–50 passwords, avoid ambiguous
  characters, strength line, copy one / all, save to file.
- Clipboard kept out of Win+V and the cloud, wiped after 30 s or on exit.
- English, Japanese and Spanish; light and dark theme; Help and About.
- Command line `/out`, `/len`, `/count`, `/ambiguous`, `/lang`, `/theme`, `/shot`.
- User guides in three languages (Word and PDF).
