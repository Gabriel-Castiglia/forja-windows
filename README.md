<p align="center"><img src="assets/app256.png" width="128" alt="Wyrmhold"></p>

<h1 align="center">Wyrmhold</h1>
<p align="center"><b>Strong passwords, passphrases and PINs — made on your computer, stored nowhere.</b><br>
<i>Contraseñas, frases y PIN fuertes — se hacen en tu computadora y no se guardan en ningún lado.</i><br>
<i>強力なパスワード・パスフレーズ・PIN — このコンピューターで作り、どこにも保存しません。</i></p>

<p align="center"><img src="docs/images/window_en.png" width="520" alt="Main window"></p>

[English](#english) · [Español](#español) · [日本語](#日本語)

> **Windows 10/11 on x64 (64-bit Intel or AMD) only.** There is no build for ARM processors.
> Solo para Windows 10/11 en x64 (Intel o AMD de 64 bits); no hay versión para procesadores ARM.
> x64(64 ビットの Intel / AMD)の Windows 10/11 専用です。ARM 版はありません。

---

## English

*Wyrm* is the Old English word for dragon, and *hold* a stronghold: a wyrmhold is the dragon's keep, where a
dragon guards its hoard — as this program guards your passwords.

- **Three kinds:** random characters (4–128), passphrases from the EFF list of 7,776 words (3–12 words) and PINs.
- **Profiles** for the usual cases, including sites that refuse symbols, sites that accept only `- _ .`, sites
  with a short maximum, Wi-Fi and PINs. Symbol sets: basic, all, minimal or your own; characters to leave out;
  no look-alikes, no repeats in a row, start with a letter; PINs without runs such as 123.
- **Uniform:** randomness from `BCryptGenRandom`; a password that breaks a rule is drawn again, never patched,
  so every valid password is equally likely. The entropy shown is counted exactly, every rule included.
- Copy one or all, or save them to a text file. Copies are kept out of the clipboard history (Win+V) and the
  cloud clipboard, and **wiped after 2 minutes** (10 s to 10 min in the window, or never) or as soon as the program
  closes — only if the clipboard still holds exactly what Wyrmhold put there, so whatever you copied afterwards is
  left alone. See [Clipboard](#clipboard).
- **Updates that install themselves:** Help → Check for updates, and once a day by itself (can be switched off).
  One question — Update now, What's new or Later — and then the setup is downloaded, checked against the
  release's `SHA256SUMS.txt` (a file that does not match is deleted, never run), installed in silence with the
  same options, and Wyrmhold opens again. A portable copy only offers the download page. See [Updates](#updates).
- English, Japanese and Spanish; light, dark or as-Windows theme and twelve colours, which also colour the icon.
- **Setup** in the three languages, per user with no administrator rights, with a clean uninstaller. Or portable:
  a single static `wyrmhold.exe` with an empty `wyrmhold.ini` beside it.
- Command line for scripts: `wyrmhold.exe /out:passwords.txt [/mode:chars|words|pin] [/len:N] [/count:N] ...`

### Clipboard

- **Clear clipboard after** (next to the list) is on by default, with **2 minutes**; the drop-down beside it offers
  10 s, 30 s, 1, 2, 5 and 10 minutes, and choosing a time also switches clearing on. Switched off, copies stay
  until something else is copied.
- The copy is wiped only if the clipboard still holds **exactly** what Wyrmhold put there: Wyrmhold remembers the
  clipboard's sequence number and a SHA-256 of the text (never the text itself). Anything copied afterwards, by you
  or another program, is left alone; the very same text put back by a clipboard tool is still wiped.
- **Closing Wyrmhold before the time is up wipes the copy at once** (again only if it is still Wyrmhold's), and so
  does signing out or shutting down. With clearing off, closing leaves copies alone.
- Every copy carries the formats that keep it out of the clipboard history (Win+V), the cloud clipboard and
  clipboard monitors.
- Settings, in `wyrmhold.ini`: `ClearClipboard=1|0` and `ClearSeconds=120` (5 to 3600). For one session only:
  `wyrmhold.exe /clearafter:90` (seconds), `/clearafter:5m` (minutes) or `/clearafter:off` — not saved.

### Updates

- **Help → Check for updates…** asks `api.github.com` for the latest release; **Help → Check for updates daily**
  (on by default, `CheckUpdates=1|0` in `wyrmhold.ini`) does it by itself at most once a day. Only the version is
  asked for: nothing about you or your passwords is sent.
- When the release is newer, one question: **Update now**, **What's new** (opens the release page) or **Later**.
- **Update now** downloads `Wyrmhold-Setup-<version>.exe` and `SHA256SUMS.txt` from the release and compares the
  SHA-256; if it does not match, the file is deleted and never run. Then Wyrmhold closes (a copy waiting to be
  wiped goes), the setup runs with `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART`, keeping the install folder,
  language and tasks chosen before, waits for Wyrmhold to have exited and starts it again.
- A **portable** copy (not the one the setup installed) cannot update itself: it offers the download page.
- Releases must therefore carry the setup and a `SHA256SUMS.txt` (`hash  file name` per line).

**Download:** the setup is in [Releases](https://github.com/Gabriel-Castiglia/wyrmhold-windows/releases/latest).

**macOS version:** <https://github.com/Gabriel-Castiglia/wyrmhold> (Apple silicon only).

**User guides:** [`docs/`](docs) — English, Japanese and Spanish, in PDF and Word.

### Build

Needs MSYS2 with mingw-w64 GCC in `C:\msys64` (or set `MINGW` to its `bin` folder), and Inno Setup 6 for the setup:

```bat
build.bat
rem -> build\wyrmhold.exe
powershell -File tools\make_installer.ps1
rem -> out\Wyrmhold-Setup-<version>.exe
```

The version is written only in `src/version.h`; the program, the setup and the guides read it from there.

Tested with: Windows 11 Pro x64, MSYS2 with mingw-w64 GCC 16.2.0 (package 16.2.0-6) and binutils (windres) 2.47 (2.47-4),
Inno Setup 6.7.3, Python 3.13.17 with python-docx 1.2.0 (guides), Pillow 12.3.0, numpy 2.5.4, scipy 1.18.1 and
opencv-python 5.0.0.93 (icon and setup pictures), LibreOffice 26.8.2.2 (guides to PDF).

The icon and the setup pictures are drawn by `tools/make_icon.py` and `tools/make_wizard.py`; the word list
header by `tools/make_wordlist.py` from `assets/eff_large_wordlist.txt`.

### Checks

| Script | What it proves |
|---|---|
| `tools/run_checks.ps1` | The generator follows the rules, over several hundred thousand passwords. The checker is calibrated first: a correct file must pass and five spoiled copies must fail. Then it runs `wyrmhold.exe /selftest:<log>` (below). |
| `wyrmhold.exe /selftest:<log>` | About five seconds, no window: the clipboard wiping (wiped when the time is up, a later copy by another program left alone, exactly the same text still wiped, wiped on exit, nothing wiped with clearing off, history exclusion formats), the clipboard settings and `/clearafter` in a throw-away `.ini`, and the update's pure functions (version comparison, `SHA256SUMS.txt` lookup, release JSON, SHA-256). Your clipboard is saved first and put back; your settings are never touched. Exit code = failures. |
| `tools/check_options.py` | Every option (character sets, exclusions, no repeats, first letter, PIN runs, passphrase separators) over thousands of passwords, and the exact entropy against an independent count. Calibrated first. |
| `tools/check_clipboard.ps1` | The same from outside, through the real window: a portable copy in a temporary folder with `/clearafter:5`; exclusion formats, wipe after the time, a later copy by another program left alone, wipe on exit, `/clearafter` not saved. Run it in an interactive session (Windows PowerShell, `-STA`). |
| `tools/audit_sources.py` | Copyright and licence header (GPL-2.0-only, SPDX) in every source file, comments in English, line endings, the version written only in `src/version.h`, every interface text in the three languages. |

The guides are generated by `tools/make_guide.py` (python-docx) and printed to PDF by `tools/make_pdf.ps1` (Word).

---

## Español

*Wyrm* es «dragón» en inglés antiguo y *hold*, «fortaleza»: un wyrmhold es el bastión donde un dragón custodia
su tesoro — como este programa custodia tus contraseñas.

- **Tres tipos:** caracteres al azar (4–128), frases con la lista de 7.776 palabras de la EFF (3–12 palabras) y PIN.
- **Perfiles** para los casos de siempre, incluidos los sitios que no aceptan símbolos, los que aceptan solo
  `- _ .`, los de largo máximo corto, Wi-Fi y PIN. Juegos de símbolos: básicos, todos, mínimos o los tuyos;
  caracteres a excluir; sin parecidos, sin repetir seguidos, empezar con letra; PIN sin escaleras como 123.
- **Uniforme:** el azar sale de `BCryptGenRandom`; una contraseña que rompe una regla se sortea de nuevo, nunca
  se parcha, así que todas las válidas son igual de probables. La entropía que muestra es exacta, con todas las reglas.
- Copiar una o todas, o guardarlas en un archivo de texto. Lo copiado queda fuera del historial del portapapeles
  (Win+V) y de la nube, y **se borra a los 2 minutos** (de 10 s a 10 min en la ventana, o nunca) o apenas se cierra
  el programa — solo si el portapapeles sigue teniendo exactamente lo que puso Wyrmhold, así que lo que hayas copiado
  después no se toca. Ver [Portapapeles](#portapapeles).
- **Actualizaciones que se instalan solas:** Ayuda → Buscar actualizaciones, y una vez por día por su cuenta (se
  puede apagar). Una sola pregunta — Actualizar ahora, Ver novedades o Más tarde — y después se descarga el
  instalador, se verifica contra el `SHA256SUMS.txt` de la versión publicada (si no coincide se borra y nunca se
  ejecuta), se instala en silencio con las mismas opciones y Wyrmhold vuelve a abrirse. Una copia portable solo
  ofrece abrir la página de descarga. Ver [Actualizaciones](#actualizaciones).
- Inglés, japonés y español; tema claro, oscuro o como Windows y doce colores, que también pintan el ícono.
- **Instalador** en los tres idiomas, por usuario y sin permisos de administrador, con un desinstalador limpio.
  O portable: un único `wyrmhold.exe` estático con un `wyrmhold.ini` vacío al lado.
- Línea de comandos para scripts: `wyrmhold.exe /out:claves.txt [/mode:chars|words|pin] [/len:N] [/count:N] ...`

### Portapapeles

- **Borrar portapapeles a los** (junto a la lista) viene encendido, con **2 minutos**; el desplegable de al lado
  ofrece 10 s, 30 s, 1, 2, 5 y 10 minutos, y elegir un tiempo también enciende el borrado. Apagado, lo copiado
  queda hasta que se copie otra cosa.
- Se borra solo si el portapapeles sigue teniendo **exactamente** lo que puso Wyrmhold: Wyrmhold recuerda el número
  de secuencia del portapapeles y un SHA-256 del texto (nunca el texto). Lo que se copie después, por vos o por otro
  programa, no se toca; el mismo texto vuelto a poner por una herramienta de portapapeles sí se borra.
- **Si cerrás Wyrmhold antes de que pase el tiempo, lo copiado se borra en el acto** (de nuevo, solo si sigue siendo
  de Wyrmhold), y lo mismo al cerrar sesión o apagar. Con el borrado apagado, cerrar no toca lo copiado.
- Cada copia lleva los formatos que la dejan fuera del historial (Win+V), del portapapeles en la nube y de los
  monitores de portapapeles.
- Configuración, en `wyrmhold.ini`: `ClearClipboard=1|0` y `ClearSeconds=120` (de 5 a 3600). Solo para una sesión:
  `wyrmhold.exe /clearafter:90` (segundos), `/clearafter:5m` (minutos) o `/clearafter:off` — no se guarda.

### Actualizaciones

- **Ayuda → Buscar actualizaciones…** le pregunta a `api.github.com` por la última versión publicada; **Ayuda →
  Buscar actualizaciones todos los días** (encendido de fábrica, `CheckUpdates=1|0` en `wyrmhold.ini`) lo hace
  solo, como mucho una vez por día. Solo se pide la versión: no se envía nada sobre vos ni tus contraseñas.
- Si la versión publicada es más nueva, una sola pregunta: **Actualizar ahora**, **Ver novedades** (abre la página
  de la versión) o **Más tarde**.
- **Actualizar ahora** descarga `Wyrmhold-Setup-<versión>.exe` y `SHA256SUMS.txt` de la versión publicada y compara
  el SHA-256; si no coincide, el archivo se borra y nunca se ejecuta. Después Wyrmhold se cierra (lo copiado que
  esperaba borrarse se borra), el instalador corre con `/VERYSILENT /SUPPRESSMSGBOXES /NORESTART`, conservando la
  carpeta, el idioma y las tareas elegidos antes, espera a que Wyrmhold haya terminado y lo vuelve a abrir.
- Una copia **portable** (no la que instaló el instalador) no puede actualizarse sola: ofrece abrir la página de descarga.

**Descarga:** el instalador está en [Releases](https://github.com/Gabriel-Castiglia/wyrmhold-windows/releases/latest).

**Versión para macOS:** <https://github.com/Gabriel-Castiglia/wyrmhold> (solo Apple silicon).

**Guías de uso:** [`docs/`](docs) — en inglés, japonés y español, en PDF y Word.

### Compilar

Hace falta MSYS2 con mingw-w64 GCC en `C:\msys64` (o la variable `MINGW` apuntando a su carpeta `bin`), e
Inno Setup 6 para el instalador:

```bat
build.bat
rem -> build\wyrmhold.exe
powershell -File tools\make_installer.ps1
rem -> out\Wyrmhold-Setup-<version>.exe
```

La versión se escribe solo en `src/version.h`. Las versiones de las herramientas con que se compiló están en la
sección en inglés.

Las verificaciones están en la tabla de la versión en inglés; `wyrmhold.exe /selftest:<log>` prueba el borrado
del portapapeles (guarda tu portapapeles antes y lo restaura después, sin tocar tu configuración) y las funciones
de la actualización.

---

## 日本語

*Wyrm* は古英語で「竜」、*hold* は「砦」。Wyrmhold は竜が宝を守る砦のこと — このプログラムがあなたのパスワードを守るように。

- **3 つの種類:** ランダムな文字(4〜128)、EFF の 7,776 語リストによるパスフレーズ(3〜12 単語)、PIN。
- **プロファイル:** 記号を受け付けないサイト、`- _ .` だけ受け付けるサイト、最大文字数が少ないサイト、Wi-Fi、PIN など。
  記号の組(基本・すべて・最小限・自分で指定)、除外する文字、紛らわしい文字を除く、同じ文字を続けない、英字で始める、連番のない PIN。
- **一様:** 乱数は `BCryptGenRandom`。規則に反するパスワードは修正せず引き直すため、有効なパスワードはすべて同じ確率です。
  表示されるエントロピーはすべての規則を含めた正確な値です。
- 1 つまたは全部をコピー、またはテキストファイルに保存。コピーはクリップボード履歴(Win+V)とクラウドに残らず、**2 分後**(ウィンドウでは 10 秒〜10 分、またはオフ)またはプログラム終了時に消去されます。消去するのはクリップボードの中身が Wyrmhold の入れたものとまったく同じ場合だけなので、後からコピーしたものには手を触れません。設定は `wyrmhold.ini` の `ClearClipboard=1|0` と `ClearSeconds=120`(5〜3600)、そのセッションだけなら `/clearafter:90`、`/clearafter:5m`、`/clearafter:off`(保存されません)。
- **自動でインストールされる更新:** 「ヘルプ」→「更新の確認」、および 1 日 1 回の自動確認(オフにできます)。「今すぐ更新」「更新内容を見る」「後で」の質問が 1 回だけ表示され、その後はセットアップのダウンロード、リリースの `SHA256SUMS.txt` との照合(一致しないファイルは削除され、実行されません)、同じ設定での自動インストール、Wyrmhold の再起動まで自動で行われます。ポータブル版はダウンロードページを開くことだけを提案します。
- 英語・日本語・スペイン語。ライト・ダーク・Windows に合わせるテーマと 12 色(アイコンの色も変わります)。
- 3 か国語の**セットアップ**(ユーザー単位、管理者権限不要、きれいに削除できるアンインストーラー)。またはポータブル:
  静的リンクの `wyrmhold.exe` と空の `wyrmhold.ini`。

**ダウンロード:** セットアップは [Releases](https://github.com/Gabriel-Castiglia/wyrmhold-windows/releases/latest) にあります。

**macOS 版:** <https://github.com/Gabriel-Castiglia/wyrmhold>(Apple silicon のみ)。

**ユーザーガイド:** [`docs/`](docs) — 英語・日本語・スペイン語(PDF と Word)。

ビルドと検証は英語版を参照してください。

---

## License / Licencia / ライセンス

Free software under the **GNU General Public License, version 2 only** (GPL-2.0-only), with no warranty — see [`LICENSE`](LICENSE).
Software libre bajo la **GPL versión 2 únicamente** (GPL-2.0-only), sin garantía. GNU GPL バージョン 2 のみ(GPL-2.0-only)、無保証。

© 2026 Gabriel Castiglia. Plain C and the Win32 API, no third-party code.
Passphrase words: the EFF large word list, Electronic Frontier Foundation, CC BY 3.0 US (eff.org/dice).
Icon: the winged dragon from Athanasius Kircher, *Mundus Subterraneus* (Amsterdam, 1665), public domain.
