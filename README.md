<p align="center"><img src="assets/app256.png" width="128" alt="Forja"></p>

<h1 align="center">Forja</h1>
<p align="center"><b>Strong passwords, passphrases and PINs — made on your computer, stored nowhere.</b></p>

<p align="center"><b>English</b> · <a href="README.es.md">Español</a> · <a href="README.ja.md">日本語</a></p>

<p align="center"><img src="docs/images/window_en.png" width="560" alt="The main window of Forja"></p>

> **Windows 10 (version 1809 or later) or Windows 11, on x64 (64-bit Intel or AMD) only.** There is no build for
> ARM processors.

*Forja* is Spanish for forge, where metal takes shape under the hammer, on the anvil — as each password here takes
shape from pure chance. Until version 2.1 it was called Wyrmhold.

- **Three kinds:** random characters (4–128), passphrases from the EFF list of 7,776 words (3–12 words) and PINs
  (4–128 digits); 1 to 50 at a time.
- **Profiles** for the usual cases, including sites that refuse symbols, sites that accept only `- _ .`, sites
  with a short maximum, Wi-Fi and PINs. Symbol sets: basic, all, minimal or your own; characters to leave out;
  no look-alikes, no repeats in a row, start with a letter; PINs without runs such as 123.
- **Uniform:** randomness from `BCryptGenRandom`; a password that breaks a rule is drawn again, never patched,
  so every valid password is equally likely. The entropy shown is counted exactly, every rule included.
- Copy one or all, or save them to a text file. Copies are kept out of the clipboard history (Win+V) and the
  cloud clipboard, and **wiped after 2 minutes** (10 s to 10 min in the window, or never) or as soon as the program
  closes — only if the clipboard still holds exactly what Forja put there, so whatever you copied afterwards is
  left alone. See [Clipboard](#clipboard).
- **Updates that install themselves:** Help → Check for updates, and once a day by itself if you say yes when Forja
  first starts (can be switched off). One question — Update now, What's new or Later — and then the setup is
  downloaded only if the release carries the author's signature, checked against the release's `SHA256SUMS.txt`
  (a file that does not match is deleted, never run), installed in silence with the same options, and Forja opens
  again. A portable copy only offers the download page. See [Updates](#updates).
- **Accessible:** Narrator and other screen readers name every control, with the state of switches, chips and
  modes, the value of the slider, the passwords of the list and the menus, and read the status line out as it
  changes. Everything works from the keyboard. Sharp on every screen: moved to a screen with another scale, the
  window is drawn again at that scale.
- English, Japanese and Spanish; light, dark or as-Windows theme and twelve colours, which also colour the icon.
- **Setup** in the three languages, per user with no administrator rights, with a clean uninstaller. Or portable:
  a single static `forja.exe` with an empty `forja.ini` beside it.
- [Command line](#command-line) for scripts: `forja.exe /out:passwords.txt [/mode:chars|words|pin] [/len:N] [/count:N] ...`

## Download and install

- **Setup:** `Forja-Setup-<version>.exe` from [Releases](https://github.com/Gabriel-Castiglia/forja-windows/releases/latest).
  It asks for its language (English, Japanese or Spanish), installs for your user only without administrator
  rights (the first page also offers all users), and adds Forja and the user guide to the Start menu and a desktop
  shortcut you can untick. Forja starts in the language chosen for the setup. Installing Forja removes Wyrmhold,
  and its settings come over.
- **Uninstall:** Settings → Apps → Installed apps → Forja → Uninstall. It removes every file it installed and asks
  whether to delete your settings too.
- **Portable:** copy `forja.exe` with an empty `forja.ini` beside it (a USB stick, a share); the settings are kept in
  that file. A portable copy does not update itself.
- **User guides** (PDF; Word in [`docs/`](docs)): [English](docs/forja_guide_en.pdf) ·
  [Español](docs/forja_guide_es.pdf) · [日本語](docs/forja_guide_jp.pdf). The setup installs the one in its
  language.
- **macOS version:** <https://github.com/Gabriel-Castiglia/forja> (Apple silicon only).

## Privacy

Nothing is stored or sent anywhere. The passwords exist only in the window until you copy or save them, and are
gone when Forja closes. `forja.ini` (in `%APPDATA%\Forja`, or beside a portable `forja.exe`) keeps the kind and its
settings, the clipboard clearing, the update check, theme, colour, language, the last folder used for saving and
the window's place — never a password or anything copied. The only connection Forja ever makes is the update
check, which asks GitHub for the number of the latest version, and only if you allow it.

## Profiles

The **Profile** button (or the **Passwords** menu) sets everything at once; after a change of your own it reads
**Custom**.

| Profile | Settings | Entropy |
|---|---|---|
| Standard | 20 characters, the four kinds, basic symbols `! @ # $ % & * + - = ?`, no look-alikes (the default) | 122 bits |
| Maximum | 32 characters, all 32 printable symbols, look-alikes allowed | 210 bits |
| No symbols | 20 characters: letters and digits | 117 bits |
| Minimal symbols | 20 characters, symbols only `- _ .`, starting with a letter | 117 bits |
| Short | 12 characters, starting with a letter, no repeats in a row | 72 bits |
| Wi-Fi | 20 characters, lowercase letters and digits: easy to type on a phone or TV | 101 bits |
| Passphrase | 6 words, hyphens, capital letters, one digit | 83 bits |
| PIN | 6 digits, no repeats, no runs | 19 bits |

Strength levels: below 36 bits very weak, below 50 weak, below 64 fair, below 80 strong, 80 or more very strong.
The guessing time assumes an attacker with the file trying 100 billion passwords a second. The scale runs from
"instantly" and "seconds" up to "billions" and "trillions of years"; beyond that, "far longer than the age of the
universe".

## Keyboard

| Keys | Action |
|---|---|
| `Ctrl+1`, `Ctrl+2`, `Ctrl+3` | Characters, Passphrase, PIN |
| `F5` | Generate a new set |
| `Enter`, `Ctrl+C`, double click | Copy the selected password |
| `Ctrl+Shift+C` | Copy all, one per line |
| `Ctrl+S` | Save all to a text file |
| `F1` | Help |
| `Esc` | Close Help, About or Theme and colour |

## Clipboard

- **Clear clipboard after** (next to the list) is on by default, with **2 minutes**; the drop-down beside it offers
  10 s, 30 s, 1, 2, 5 and 10 minutes, and choosing a time also switches clearing on. Switched off, copies stay
  until something else is copied.
- The copy is wiped only if the clipboard still holds **exactly** what Forja put there: Forja remembers the
  clipboard's sequence number and a SHA-256 of the text (never the text itself). Anything copied afterwards, by you
  or another program, is left alone; the very same text put back by a clipboard tool is still wiped.
- **Closing Forja before the time is up wipes the copy at once** (again only if it is still Forja's), and so
  does signing out or shutting down. With clearing off, closing leaves copies alone.
- Every copy carries the formats that keep it out of the clipboard history (Win+V), the cloud clipboard and
  clipboard monitors.
- Settings, in `forja.ini`: `ClearClipboard=1|0` and `ClearSeconds=120` (5 to 3600). For one session only:
  `forja.exe /clearafter:90` (seconds), `/clearafter:5m` (minutes) or `/clearafter:off` — not saved.

## Updates

- **Help → Check for updates…** asks `api.github.com` for the latest release; **Help → Check for updates daily**
  (`CheckUpdates=1|0` in `forja.ini`) does it by itself at most once a day. Only the version is asked for: nothing
  about you or your passwords is sent.
- On the first start (no `CheckUpdates` in `forja.ini` yet, also when coming from 3.0 or older) Forja asks once
  whether to check daily, before asking GitHub anything. The answer is saved; closing the question without an
  answer saves nothing, checks nothing that time and asks again on the next start. A start by the setup after an
  update keeps the daily check on, without asking.
- When the release is newer, one question: **Update now**, **What's new** (opens the release page) or **Later**.
- **Update now** first checks that the release's `SHA256SUMS.txt` carries the author's signature (`release.sig`,
  checked against two keys built into Forja); a release without a valid one installs nothing and can only be
  downloaded by hand from its page. Then it downloads `Forja-Setup-<version>.exe` and compares the SHA-256; if it
  does not match, the file is deleted and never run. Then Forja closes (a copy waiting to be
  wiped goes), the setup runs with `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART`, keeping the install folder,
  language and tasks chosen before, waits for Forja to have exited and starts it again.
- A **portable** copy (not the one the setup installed) cannot update itself: it offers the download page.
- Releases must therefore carry the setup, a `SHA256SUMS.txt` (`hash  file name` per line) and its `release.sig`.

## Command line

```bat
forja.exe /out:<file> [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [options]
```

`/out` writes the passwords, one per line, and exits without a window. Without options it uses the Standard
profile, whatever the settings say; nothing is written to `forja.ini`. `/count` goes up to 100,000.
Options: `/preset:0`–`7` (the profiles in the order above), `/classes:luds`, `/symbols:basic|all|min|custom:<chars>`,
`/exclude:<chars>`, `/ambiguous` (allow look-alikes), `/norepeat`, `/startletter`,
`/sep:hyphen|space|dot|underscore|none|digit`, `/nocap`, `/nonumber`, `/allowruns`. `/entropy:<file>` writes instead
the entropy in bits, the characters per position (or words in the list) and 0 when the settings can make a password.

Exit codes of `/out`: 0 done, 2 the file could not be written, 3 the random number generator failed, 5 the options
cannot make a password, 6 the rules are too tight (no draw followed them in time); after a failure the file is left
empty. For the window: `/lang:en|jp|es`, `/theme:system|light|dark`
(`system`, or `auto`, follows Windows), `/accent:0`–`11` (in the order of **Theme and colour**; 0 is the default black)
and `/clearafter:N[s|m]|off`. `/theme` and `/accent` win over what `forja.ini` holds, `/theme:system` and `/accent:0`
included.

## Build

Needs MSYS2 with mingw-w64 GCC in `C:\msys64` (or set `MINGW` to its `bin` folder), and Inno Setup 6 for the setup:

```bat
build.bat
rem -> build\forja.exe
powershell -File tools\make_installer.ps1
rem -> out\Forja-Setup-<version>.exe
```

The version is written only in `src/version.h`; the program, the setup and the guides read it from there.

Tested with: Windows 11 Pro x64, MSYS2 with mingw-w64 GCC 16.2.0 (package 16.2.0-6) and binutils (windres) 2.47
(2.47-4), Inno Setup 6.7.3, Python 3.14.8 with python-docx 1.2.0 (guides), LibreOffice 26.8.3.1 (guides to PDF),
Python 3.13.17 with Pillow 12.3.0, numpy 2.5.4, scipy 1.18.1 and opencv-python 5.0.0.93 (icon and setup pictures).

The icon is drawn from `assets/icon.svg` (in every colour, by the author's icon tool) and the setup pictures by
`tools/make_wizard.py`; the word list header by `tools/make_wordlist.py` from `assets/eff_large_wordlist.txt`.

## Checks

| Script | What it proves |
|---|---|
| `tools/run_checks.ps1` | The generator follows the rules, over several hundred thousand passwords. The checker is calibrated first: a correct file must pass and five spoiled copies must fail. Then it runs `forja.exe /selftest:<log>` (below). |
| `forja.exe /selftest:<log>` | About five seconds, no window: the clipboard wiping (wiped when the time is up, a later copy by another program left alone, exactly the same text still wiped, wiped on exit, nothing wiped with clearing off, history exclusion formats), the clipboard settings and `/clearafter` in a throw-away `.ini`, and the update's pure functions (version comparison, `SHA256SUMS.txt` lookup, release JSON, SHA-256, the author's signature), and every step of the guessing time with its text in the three languages. Your clipboard is saved first and put back; your settings are never touched. Exit code = failures. |
| `tools/check_options.py` | Every option (character sets, exclusions, no repeats, first letter, PIN runs, passphrase separators) over thousands of passwords, and the exact entropy against an independent count; for passphrases, also that none can be read as two different draws, not even with no separator, no capitals and words with a hyphen such as `t-shirt`. Calibrated first. |
| `tools/check_clipboard.ps1` | The same from outside, through the real window: a portable copy in a temporary folder with `/clearafter:5`; exclusion formats, wipe after the time, a later copy by another program left alone, wipe on exit, `/clearafter` not saved. Run it in an interactive session (Windows PowerShell, `-STA`). |
| `tools/audit_sources.py` | Copyright and licence header (GPL-2.0-only, SPDX) in every source file, comments in English, line endings, the version written only in `src/version.h`, every interface text in the three languages. |

The guides are generated by `tools/make_guide.py` (python-docx), in the program's own colours (read from
`src/theme.c`), with a cover, contents, numbered sections and the screenshots in `docs/images` (taken with
`forja.exe /shot`), and turned into PDF by `tools/make_pdf.ps1` (LibreOffice, or Microsoft Word when LibreOffice is
not installed).

---

## License

Free software under the **GNU General Public License, version 2 only** (GPL-2.0-only), with no warranty — see [`LICENSE`](LICENSE).

© 2026 Gabriel Castiglia. Plain C and the Win32 API, no third-party code.
Passphrase words: the EFF large word list, Electronic Frontier Foundation, CC BY 3.0 US (eff.org/dice).
Icon: an anvil and hammer drawn for Forja (2026), under the GPL-2.0 like the rest.
