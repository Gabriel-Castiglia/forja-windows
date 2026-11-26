# Changelog

**English** · [Español](CHANGELOG.es.md) · [日本語](CHANGELOG.ja.md)

## 3.1.2 — 2026-11-27

- **A new icon**: an anvil and a hammer drawn as a flat picture, cream with a black outline on the tile of the chosen
  colour, with three sparks. It replaces the engraving of an 18th-century trade card, and still follows the colour
  chosen in *View → Theme and colour*; the setup pictures use it too. The drawing is in `assets/icon.svg`.

## 3.1.1 — 2026-11-26

- The menu bar keeps its colour after a drop-down list (profile, symbols, separator, clipboard time) has been
  opened: the brush it was painted with used to be deleted while the bar still used it.
- `/out`: a write that fails (a full disk, say) now also leaves the file empty, with exit code 2, as every other
  failure does.
- Updates: when the check or the download cannot be started, the status line no longer stays on "Checking for
  updates…" or "Downloading…".
- Help: the strength levels are given in full (below 36 bits very weak, below 50 weak, below 64 fair, below 80
  strong, 80 or more very strong); the update section tells of the first start's question and of the author's
  signature; a new section describes screen readers and per-monitor scaling. The update question says that the
  checksums are signed by the author. "Save as…" has an ellipsis like the other menu items.
- User guides: in the program's own colours, taken from its default theme, with the title band and the icon as at
  the top of the window; the strength levels corrected; the Japanese and Spanish wording matches the program.
- README in three complete files — `README.md` (English), `README.es.md` (Spanish), `README.ja.md` (Japanese) —
  and this changelog also in Spanish and Japanese.
- Internal: code repeated in several places merged into shared helpers (off-screen drawing, cached brushes, modal
  windows, default buttons, window classes, task dialogs, the text of a password), unused functions removed,
  memory locks checked, the window class no longer points at an icon destroyed at start-up, comments reviewed.

## 3.1.0 — 2026-11-26

- **The first start asks before checking for updates**: with no `CheckUpdates` in `forja.ini` (a new install, or
  settings from 3.0 and older) Forja asks once whether it may ask GitHub for a new version every day, before it
  asks anything. The answer is saved and can be changed in the Help menu; closing the question saves nothing and
  checks nothing that time. A start by the setup after an update keeps the daily check on, without asking.
- **Screen readers**: Narrator and other readers now name every control Forja draws itself and say what it is and
  how it is: modes and colours as options, chips and switches as check boxes that are on or off, the slider with
  its value, the drop-downs with what they hold, the number and text fields with their labels, the − and + of the
  count, the icon buttons. Each row of the list is read as its password (the list asks for it when a reader
  does; it still keeps no copy), the menus are read, and the status line is read out as it changes, without
  moving the focus.
- **Per-monitor scaling** (PerMonitorV2): moved to a screen with another scale, the window is drawn again at that
  scale — fonts, icons, rows and layout — instead of being stretched as a picture, blurry; Help, About and Theme
  open again at it. A window that opens on such a screen takes its scale before it is shown.
- Internal: the window's source, a single forja.c of about 5200 lines, is split into modules without any change of
  behaviour: forja.c (state, window procedure, entry), settings.c, theme.c, draw.c, controls.c, layout.c,
  dialogs.c, clipboard.c, filesave.c, updui.c, cli.c and selftest.c, sharing forja.h, plus access.c for screen
  readers. The string table of ui_strings.h is compiled once (in forja.c) instead of in every file that includes it.
- The source audit (`tools/audit_sources.py`) is clean again: a comment of 3.0.2 had an accented letter.

## 3.0.2 — 2026-11-25

- **Updates signed by the author**: the `SHA256SUMS.txt` of a release must now carry the author's signature
  (`release.sig`, ECDSA P-256, checked with Windows' own cryptography against two keys built into Forja) before the
  setup is downloaded. Before, the setup was checked only against hashes from the same page, so a release put there
  by someone else would have installed itself. A release without a valid signature is offered as a page, to download
  by hand. Anyone can still download, install and share Forja; Forja 3.0.1 and older install this one as usual.

## 3.0.1 — 2026-11-25

- Generate no longer freezes the window with very tight rules (for example only digits, no repeats and a length of
  128 took 14 seconds and then failed). With "no repeats" each character is drawn among all but the one before it,
  which keeps every valid password exactly as likely. Rules that leave so few passwords that drawing one would take
  too long are refused at once: Generate is greyed out and the status line says why, in English, Japanese and
  Spanish. The message no longer blames the Windows random number generator when the rules were the cause.
- `/out` exit codes: 6 when no draw followed the rules within the tries allowed, 3 when the random number generator
  failed (they were both 3); the file is left empty after any failure.
- The random numbers are asked for in blocks, and what is left of a block is wiped when the password is made.
- Updates: each download goes to a new folder with a random name under `%TEMP%`, the setup is kept locked against
  writing from the moment it is checked until it starts, and setups and cut downloads left by earlier updates are
  deleted when Forja starts.
- Turning "Clear clipboard after" off now stops the countdown already running; turning it on starts one if the
  clipboard still holds Forja's copy.
- Save to file writes everything or nothing: a write that fails leaves no half-written file.
- The window opens where it was also when the taskbar is at the top or on the left (it drifted a little each time).
- Copy all no longer adds a line break after the last password.
- Setup: if removing Wyrmhold fails, the setup says so; the uninstaller also offers to delete `%APPDATA%\Wyrmhold`.

## 3.0.0 — 2026-11-20

**Wyrmhold is now Forja**, a name easier to remember: the forge, where metal takes shape under the hammer, as each
password here takes shape from pure chance. The same number as Forja 3.0.0 for macOS. The icon is the anvil and
hammers of the trade card of Elwell and Taylor, smiths of London (c. 1766, British Museum, public domain). The
repository is now `forja-windows` (GitHub forwards the old address).

- Installing Forja removes Wyrmhold in silence. The first time Forja starts, `%APPDATA%\Wyrmhold\wyrmhold.ini`
  comes over as `%APPDATA%\Forja\forja.ini` (a portable copy takes the `wyrmhold.ini` beside it as `forja.ini`).
- The program is `forja.exe`; the installer is `Forja-Setup-<version>.exe`.
- Built with the GCC of MSYS2, as every version before 2.1.0 (2.1.0 had been built with the GCC of WinLibs; same GCC
  16.2.0 and linker 2.47, and the program no longer carries a threads library it does not use).

## 2.1.0 — 2026-11-20

- Clipboard: copies are now wiped after **2 minutes** by default (it was a fixed 30 seconds). A drop-down next to
  "Clear clipboard after" offers 10 s, 30 s, 1, 2, 5 and 10 minutes; the switch still turns clearing off.
  `wyrmhold.ini` keeps `ClearSeconds` (5 to 3600) beside `ClearClipboard`; `/clearafter:N[s|m]|off` sets it for
  one session without saving it.
- Clipboard: the copy is wiped only if the clipboard still holds exactly what Wyrmhold put there — the sequence
  number, or else a SHA-256 of the text (the text itself is never kept). The same text put back by a clipboard
  tool is still wiped; anything else copied afterwards is left alone. Closing the program, signing out or shutting
  down before the time is up wipes it at once.
- Updates that install themselves: Help → Check for updates, and a daily check (Help → Check for updates daily,
  `CheckUpdates` in the .ini). One question (Update now, What's new, Later); then the setup is downloaded from the
  GitHub release, checked against its `SHA256SUMS.txt` (deleted and never run if it does not match), run in silence
  keeping the previous install options, and Wyrmhold opens again. A portable copy is offered the download page.
- `wyrmhold.exe /selftest:<log>`: the clipboard wiping, its settings and the update's pure functions, with the
  user's clipboard saved and put back and a throw-away .ini; run by `tools/run_checks.ps1`.
  `tools/check_clipboard.ps1` now runs a portable copy in a temporary folder, so it never touches the real settings.
- Help, tooltips, README and user guides describe both in English, Japanese and Spanish.
- Small screens: a new window is sized to the screen's work area; rows that do not fit the width go
  on to the next row (minimum width 560 logical pixels, was 760); in a low window the band slims
  down or goes, rows and gaps tighten, and below that the part above the list scrolls while at
  least three password rows stay in view. The status line is a window of its own.

## 2.0.2 — 2026-11-10

- The setup creates the desktop shortcut unless you untick it (it was unticked by default).

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
