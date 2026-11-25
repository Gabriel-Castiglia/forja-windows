# Changelog

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
