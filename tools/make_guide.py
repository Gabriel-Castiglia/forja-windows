"""
Wyrmhold -- writes the user guide in English, Japanese and Spanish
(docs/wyrmhold_guide_<lang>.docx). The PDFs are made from them by tools/make_pdf.ps1.
Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
Date: November 9, 2026
SPDX-License-Identifier: GPL-2.0-only

    python tools/make_guide.py

The screenshots in docs/images are taken from the program itself with /shot (see the
header of src/wyrmhold.c). The version is read from src/wyrmhold.c, so the guide
cannot name another one.
"""

import re
from pathlib import Path

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml.ns import qn
from docx.shared import Cm, Pt, RGBColor

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
IMAGES = DOCS / "images"
_VERSION_H = (ROOT / "src" / "version.h").read_text(encoding="utf-8")
VERSION = re.search(r'#define WH_VERSION_STR\s+"([\d.]+)"', _VERSION_H).group(1)
DATE = re.search(r'#define WH_DATE_STR\s+"([\d-]+)"', _VERSION_H).group(1)
REPO = re.search(r'#define WH_REPO_STR\s+"([^"]+)"', _VERSION_H).group(1)
AUTHOR = "Gabriel Castiglia"
ACCENT = RGBColor(0x22, 0x24, 0x2C)       # the default black of the icon
GREY = RGBColor(0x5F, 0x6B, 0x78)

# Each guide: a list of blocks. ("h", text) heading, ("p", text) paragraph,
# ("b", text) bullet, ("n", text) numbered item, ("img", file, caption), ("code", text).
GUIDES = {
    "en": {
        "font": "Segoe UI", "east": "Segoe UI",
        "title": "Wyrmhold",
        "subtitle": f"User guide  \u00B7  version {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. What it is"),
            ("p", "Wyrmhold makes strong random passwords, passphrases and PINs on your own computer: for a zip, 7z or rar file sent by email, a new account, a Wi-Fi network or a lock. Nothing is stored or sent anywhere: the passwords exist only in the window until you copy or save them."),
            ("p", "The name: \u201Cwyrm\u201D is the Old English word for dragon, and \u201Chold\u201D a stronghold. A wyrmhold is the dragon\u2019s keep, where a dragon guards its hoard \u2014 as this program guards your passwords. The icon shows it: Athanasius Kircher\u2019s winged dragon (Mundus Subterraneus, 1665) watching over its tower, with a lit keyhole in the window."),
            ("h", "2. Installing and removing it"),
            ("p", "Windows 10 or 11 on a 64-bit Intel or AMD processor (x64); there is no version for ARM processors. Run Wyrmhold-Setup-" + VERSION + ".exe and choose the language of the setup: English, Japanese or Spanish. By default it installs for your user only, with no administrator rights; the first page also offers to install it for all users. It adds Wyrmhold and the user guide to the Start menu, and a desktop shortcut if you tick it. The program starts in the language you chose for the setup."),
            ("p", "To remove it: Settings \u2192 Apps \u2192 Installed apps \u2192 Wyrmhold \u2192 Uninstall. The uninstaller removes every file it installed and then asks whether to delete your settings too."),
            ("p", "Portable use: wyrmhold.exe is a single file that needs nothing else. To run it from a USB stick or a shared folder without installing, copy it there together with an empty file named wyrmhold.ini: the settings are then kept in that file."),
            ("h", "3. The window"),
            ("img", "window_en.png", "The main window: light theme, the default black colour."),
            ("n", "The kind of password: Characters, Passphrase or PIN (Ctrl+1, Ctrl+2, Ctrl+3). Next to it, the Profile button holds ready-made settings (section 5)."),
            ("n", "The settings of that kind. Every change makes a new set at once."),
            ("n", "The strength line: a five-step bar, the entropy in bits, how many characters each position can take and how long guessing would take (section 7)."),
            ("n", "Generate (F5): a completely new set."),
            ("n", "The list: up to 50 passwords. Digits are shown in blue and symbols in amber, so a password is easier to read back or dictate; what is copied is plain text."),
            ("h", "4. The three kinds"),
            ("b", "Characters (4 to 128): the strongest for their length; best when the password is pasted rather than typed. Choose which kinds to use (a\u2013z, A\u2013Z, 0\u20139, symbols) and which symbols: Basic (! @ # $ % & * + - = ?), All 32 (every printable symbol), Minimal (- _ .) or My own. Options: No look-alikes (0 O 1 l I | and quotes), No repeats in a row, Start with a letter, and the Leave out box for any other characters. Every kind that is on appears at least once."),
            ("b", "Passphrase (3 to 12 words): random words from the EFF list of 7,776, such as Copper-Showroom-Wrecking7-Salute. Longer, but far easier to remember, dictate or type on a phone. Separator: hyphen, space, dot, underscore, none, or a random digit (stronger). Options: Capital letters (each word), Add a digit. Six words give about 80 bits."),
            ("b", "PIN (4 to 128 digits): digits only. Options: No repeats in a row, No runs like 123 or 987. Safe only where wrong tries are limited, as on cards, phones and locks."),
            ("h", "5. Profiles, and sites that refuse some characters"),
            ("p", "The Profile button (also the Passwords menu) sets everything at once: Standard (20 characters, basic symbols), Maximum (32 characters, every symbol), No symbols, Minimal symbols (only - _ ., starting with a letter), Short (12 characters, for a low maximum), Wi-Fi (easy to type on a phone or TV), Passphrase (6 words) and PIN (6 digits). After any change of your own the button reads Custom."),
            ("b", "The site refuses symbols: profile No symbols, or switch off the #$% chip."),
            ("b", "It accepts only some symbols: Symbols \u2192 Minimal, or My own, and type exactly the symbols the site lists."),
            ("b", "It refuses particular characters: type them in Leave out."),
            ("b", "It has a maximum length: lower the length, or profile Short; make up for it with every kind of character."),
            ("b", "It wants a letter first, or no repeated characters: switch on Start with a letter, or No repeats in a row."),
            ("h", "6. Copying and saving"),
            ("b", "One password: the copy icon at the end of its row, a double click, or Enter or Ctrl+C on the selected row. The icon turns into a check mark for a moment."),
            ("b", "All of them: the copy icon at the top right of the list, or Ctrl+Shift+C, one per line."),
            ("b", "Save (the arrow icon, or Ctrl+S): all of them to a text file, one per line. Keep the file somewhere safe or delete it once used."),
            ("h", "7. How the passwords are made, and their strength"),
            ("p", "Every character or word comes from the Windows cryptographic random number generator (BCryptGenRandom), drawn without bias. When a password breaks one of the rules \u2014 a kind missing, a repeated neighbour, a run \u2014 it is thrown away and drawn again, never patched. So every password that follows the rules is exactly as likely as any other: no position, kind or pattern is favoured."),
            ("p", "The entropy is counted exactly, every rule included: n bits means 2^n possible passwords. Below 50 bits is weak, 64 fair, 80 or more very strong. The guessing time assumes an attacker who has the file and tries 100 billion passwords a second. The default 20 characters give about 122 bits."),
            ("h", "8. Clipboard protection"),
            ("p", "Every copy is kept out of the Windows clipboard history (Win+V) and the cloud clipboard. With Clear clipboard after 30 s on, a copied password is wiped 30 seconds later, or when the program closes, unless something else was copied in between."),
            ("h", "9. Theme and colour"),
            ("img", "theme_en.png", "View \u2192 Theme and colour: the appearance and twelve colours."),
            ("p", "View \u2192 Theme and colour chooses the appearance (as Windows, light or dark) and one of twelve colours. The colour paints the buttons, the band at the top and the icon\u2019s tile, in the window and on the taskbar. Black is the default."),
            ("h", "10. Settings"),
            ("p", "The kind, the options, the language, the theme, the colour and the window position are kept in %APPDATA%\\Wyrmhold\\wyrmhold.ini (or in the portable wyrmhold.ini, section 2). Passwords are never written to it. Delete it to go back to the defaults."),
            ("h", "11. Command line"),
            ("p", "For scripts and tests the program also works without a window:"),
            ("code", "wyrmhold.exe /out:passwords.txt [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [options]"),
            ("p", "/out writes the passwords to the file, one per line, and exits; /entropy:file writes the entropy instead. Options: /classes:luds, /symbols:basic|all|min|custom:..., /exclude:..., /ambiguous, /norepeat, /startletter, /sep:hyphen|space|dot|underscore|none|digit, /nocap, /nonumber, /allowruns, /preset:N. For the window: /lang:en|jp|es, /theme:light|dark, /accent:N. None of them changes the settings."),
            ("h", "12. Building from source"),
            ("p", "The source is C (Win32). With MSYS2 mingw-w64 GCC in C:\\msys64, build.bat writes build\\wyrmhold.exe; tools\\make_installer.ps1 builds the setup with Inno Setup 6. tools\\run_checks.ps1 and tools\\check_options.py check the passwords and the exact entropy against independent counts, and tools\\check_clipboard.ps1 the clipboard."),
            ("h", "13. License and credits"),
            ("p", "Free software under the GNU General Public License, version 2 (GPL-2.0-only): you may use, copy, study and change it, and anyone who passes it on must do so under the same license, with the source code. It comes with no warranty. The full text is in LICENSE.txt. \u00A9 2026 Gabriel Castiglia."),
            ("p", "Source code, releases and updates: https://" + REPO),
            ("p", "Passphrase words: the EFF large word list, by the Electronic Frontier Foundation, under CC BY 3.0 US (eff.org/dice). Icon: the winged dragon from Athanasius Kircher, Mundus Subterraneus (Amsterdam, 1665), public domain."),
        ],
    },
    "jp": {
        "font": "Meiryo UI", "east": "Meiryo UI",
        "title": "Wyrmhold",
        "subtitle": f"ユーザーガイド  \u00B7  バージョン {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. 概要"),
            ("p", "Wyrmhold は、強力なランダムパスワード・パスフレーズ・PIN をお使いのコンピューター上で作ります。メールで送る zip・7z・rar ファイル、新しいアカウント、Wi-Fi、ロックなどに使えます。何も保存・送信しません。パスワードはコピーまたは保存するまで、このウィンドウの中にだけ存在します。"),
            ("p", "名前の由来:「wyrm」は古英語で「竜」、「hold」は「砦」を意味します。Wyrmhold は竜が宝を守る砦のこと \u2014 このプログラムがあなたのパスワードを守るように。アイコンはそれを表しています。アタナシウス・キルヒャー『地下世界』(1665 年)の翼のある竜が塔を見張り、窓には鍵穴が灯っています。"),
            ("h", "2. インストールとアンインストール"),
            ("p", "Windows 10 または 11、64 ビットの Intel または AMD プロセッサー(x64)。ARM プロセッサー用の版はありません。Wyrmhold-Setup-" + VERSION + ".exe を実行し、セットアップの言語(英語・日本語・スペイン語)を選びます。既定では管理者権限なしで現在のユーザーにインストールされます。最初のページで全ユーザー向けのインストールも選べます。スタートメニューに Wyrmhold とユーザーガイドが追加され、チェックすればデスクトップにショートカットも作られます。プログラムはセットアップで選んだ言語で起動します。"),
            ("p", "削除するには:設定 \u2192 アプリ \u2192 インストールされているアプリ \u2192 Wyrmhold \u2192 アンインストール。インストールしたファイルをすべて削除したあと、設定も削除するかどうかを確認します。"),
            ("p", "ポータブルで使う:wyrmhold.exe はほかに何も必要としない 1 つのファイルです。USB メモリーや共有フォルダーからインストールせずに使うには、wyrmhold.exe と一緒に空の wyrmhold.ini を置いてください。設定はそのファイルに保存されます。"),
            ("h", "3. ウィンドウ"),
            ("img", "window_jp.png", "メインウィンドウ:ライトテーマ、既定の黒。"),
            ("n", "パスワードの種類:文字・パスフレーズ・PIN(Ctrl+1、Ctrl+2、Ctrl+3)。その横の「プロファイル」ボタンには既製の設定があります(5 章)。"),
            ("n", "その種類の設定。変更するとすぐに新しいセットが作られます。"),
            ("n", "強度の行:5 段階のバー、エントロピー(ビット)、1 文字あたりの候補数、推測にかかる時間(7 章)。"),
            ("n", "生成(F5):まったく新しいセットを作ります。"),
            ("n", "一覧:最大 50 個。数字は青、記号はオレンジで表示され、読み返しや読み上げが楽になります。コピーされるのは普通のテキストです。"),
            ("h", "4. 3 つの種類"),
            ("b", "文字(4\u2013128 文字):長さあたり最も強力です。入力せず貼り付ける場合に最適です。使う種類(a\u2013z、A\u2013Z、0\u20139、記号)と記号の組を選びます:基本(! @ # $ % & * + - = ?)、全32種(印字可能なすべての記号)、最小限(- _ .)、自分で指定。オプション:「紛らわしい文字なし」(0 O 1 l I | と引用符)、「同じ文字を連続させない」、「英字で始める」、ほかの文字は「除外」欄に入力。オンにした種類は必ず 1 文字以上含まれます。"),
            ("b", "パスフレーズ(3\u201312 単語):EFF の 7,776 語のリストから選んだランダムな単語。例:Copper-Showroom-Wrecking7-Salute。長くなりますが、覚える・読み上げる・スマホで入力するのがずっと楽です。区切り:ハイフン、スペース、ドット、アンダースコア、なし、ランダムな数字(より強力)。オプション:「大文字にする」(各単語の先頭)、「数字を1つ入れる」。6 単語で約 80 ビットです。"),
            ("b", "PIN(4\u2013128 桁):数字のみ。オプション:「同じ文字を連続させない」、「123 や 987 のような連番なし」。カード・スマホ・ロックのように誤入力の回数が制限される場所でのみ安全です。"),
            ("h", "5. プロファイルと、一部の文字を受け付けないサイト"),
            ("p", "「プロファイル」ボタン(「パスワード」メニューにもあります)ですべてを一度に設定できます:標準(20 文字、基本の記号)、最強(32 文字、すべての記号)、記号なし、最小限の記号(- _ . のみ、英字で始める)、短め(12 文字、最大文字数が少ない場合)、Wi-Fi(スマホやテレビで入力しやすい)、パスフレーズ(6 単語)、PIN(6 桁)。自分で変更するとボタンは「カスタム」になります。"),
            ("b", "記号を受け付けない:プロファイル「記号なし」、または #$% のチップをオフにします。"),
            ("b", "一部の記号だけ受け付ける:記号 \u2192 最小限、または「自分で指定」でサイトが示す記号だけを入力します。"),
            ("b", "特定の文字を受け付けない:その文字を「除外」に入力します。"),
            ("b", "最大文字数がある:長さを下げるか、プロファイル「短め」。すべての種類の文字で補ってください。"),
            ("b", "先頭は英字、または同じ文字の連続禁止:「英字で始める」または「同じ文字を連続させない」をオンにします。"),
            ("h", "6. コピーと保存"),
            ("b", "1 つ:行の末尾のコピーアイコン、ダブルクリック、または選択して Enter か Ctrl+C。アイコンが一瞬チェックマークに変わります。"),
            ("b", "すべて:一覧右上のコピーアイコン、または Ctrl+Shift+C。1 行に 1 つずつコピーされます。"),
            ("b", "保存(矢印アイコン、または Ctrl+S):すべてをテキストファイルに 1 行ずつ書き出します。安全な場所に置くか、使い終わったら削除してください。"),
            ("h", "7. パスワードの作り方と強度"),
            ("p", "すべての文字・単語は Windows の暗号用乱数生成器(BCryptGenRandom)から偏りなく選ばれます。規則に反するパスワード(種類が欠けている、同じ文字が隣り合う、連番など)は修正せずに捨てて引き直します。そのため規則を満たすすべてのパスワードが等しい確率で選ばれ、特定の位置・種類・パターンが有利になることはありません。"),
            ("p", "エントロピーはすべての規則を含めて正確に数えます。n ビットは 2^n 通りのパスワードを意味します。50 ビット未満は弱い、64 はまずまず、80 以上は非常に強力です。推測時間は、ファイルを持つ攻撃者が毎秒 1000 億回試す場合の目安です。既定の 20 文字で約 122 ビットです。"),
            ("h", "8. クリップボードの保護"),
            ("p", "コピーは常に Windows のクリップボード履歴(Win+V)とクラウドから除外されます。「30秒後にクリップボードを消去」がオンのとき、コピーしたパスワードは 30 秒後、またはプログラムを閉じたときに消去されます(その間にほかのものをコピーした場合を除く)。"),
            ("h", "9. テーマと色"),
            ("img", "theme_jp.png", "表示 \u2192 テーマと色:外観と 12 色。"),
            ("p", "「表示」\u2192「テーマと色」で、外観(Windows に合わせる・ライト・ダーク)と 12 色から 1 色を選べます。色はボタン、上部の帯、ウィンドウとタスクバーのアイコンの背景に使われます。既定は黒です。"),
            ("h", "10. 設定"),
            ("p", "種類・オプション・言語・テーマ・色・ウィンドウ位置は %APPDATA%\\Wyrmhold\\wyrmhold.ini(ポータブルの場合は 2 章の wyrmhold.ini)に保存されます。パスワードが書き込まれることはありません。削除すると既定に戻ります。"),
            ("h", "11. コマンドライン"),
            ("p", "スクリプトやテスト用に、ウィンドウなしでも使えます:"),
            ("code", "wyrmhold.exe /out:passwords.txt [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [options]"),
            ("p", "/out はパスワードを 1 行ずつファイルに書き出して終了します。/entropy:file は代わりにエントロピーを書き出します。オプション:/classes:luds、/symbols:basic|all|min|custom:...、/exclude:...、/ambiguous、/norepeat、/startletter、/sep:hyphen|space|dot|underscore|none|digit、/nocap、/nonumber、/allowruns、/preset:N。ウィンドウ用:/lang:en|jp|es、/theme:light|dark、/accent:N。いずれも設定は変更しません。"),
            ("h", "12. ソースからのビルド"),
            ("p", "ソースは C(Win32)です。C:\\msys64 に MSYS2 の mingw-w64 GCC があれば、build.bat が build\\wyrmhold.exe を作ります。tools\\make_installer.ps1 は Inno Setup 6 でセットアップを作ります。tools\\run_checks.ps1 と tools\\check_options.py はパスワードと正確なエントロピーを独立した計算と照合し、tools\\check_clipboard.ps1 はクリップボードを確認します。"),
            ("h", "13. ライセンスとクレジット"),
            ("p", "GNU 一般公衆利用許諾書 第 2 版(GPL-2.0-only)のもとで配布されるフリーソフトウェアです。使用・複製・研究・改変ができ、再配布する場合は同じライセンスでソースコードとともに行う必要があります。無保証です。全文は LICENSE.txt にあります。\u00A9 2026 Gabriel Castiglia."),
            ("p", "ソースコード・リリース・更新: https://" + REPO),
            ("p", "パスフレーズの単語:Electronic Frontier Foundation の EFF large word list(CC BY 3.0 US、eff.org/dice)。アイコン:アタナシウス・キルヒャー『地下世界』(アムステルダム、1665 年)の翼のある竜(パブリックドメイン)。"),
        ],
    },
    "es": {
        "font": "Segoe UI", "east": "Segoe UI",
        "title": "Wyrmhold",
        "subtitle": f"Gu\u00eda de uso  \u00B7  versi\u00f3n {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. Qué es"),
            ("p", "Wyrmhold genera contraseñas, frases y PIN fuertes y aleatorios en tu propia computadora: para un zip, 7z o rar que mandás por email, una cuenta nueva, una red Wi-Fi o una cerradura. No guarda ni envía nada: las contraseñas existen solo en la ventana hasta que las copiás o guardás."),
            ("p", "El nombre: \u00ABwyrm\u00BB es «dragón» en inglés antiguo y \u00ABhold\u00BB, «fortaleza». Un wyrmhold es el bastión donde un dragón custodia su tesoro \u2014 como este programa custodia tus contraseñas. El ícono lo muestra: el dragón alado de Athanasius Kircher (Mundus Subterraneus, 1665) vigilando su torre, con una cerradura encendida en la ventana."),
            ("h", "2. Instalar y desinstalar"),
            ("p", "Windows 10 u 11 con un procesador Intel o AMD de 64 bits (x64); no hay versión para procesadores ARM. Ejecutá Wyrmhold-Setup-" + VERSION + ".exe y elegí el idioma del instalador: inglés, japonés o español. Por defecto se instala solo para tu usuario, sin permisos de administrador; la primera página también ofrece instalarlo para todos los usuarios. Agrega Wyrmhold y la guía de uso al menú Inicio, y un acceso directo en el escritorio si lo marcás. El programa arranca en el idioma que elegiste para el instalador."),
            ("p", "Para desinstalarlo: Configuración \u2192 Aplicaciones \u2192 Aplicaciones instaladas \u2192 Wyrmhold \u2192 Desinstalar. El desinstalador borra todos los archivos que instaló y después pregunta si querés borrar también tu configuración."),
            ("p", "Uso portable: wyrmhold.exe es un único archivo que no necesita nada más. Para usarlo desde un pendrive o una carpeta compartida sin instalarlo, copialo ahí junto con un archivo vacío llamado wyrmhold.ini: la configuración se guarda entonces en ese archivo."),
            ("h", "3. La ventana"),
            ("img", "window_es.png", "La ventana principal: tema claro, el color negro de fábrica."),
            ("n", "El tipo de contraseña: Caracteres, Frase o PIN (Ctrl+1, Ctrl+2, Ctrl+3). Al lado, el botón Perfil tiene configuraciones listas (sección 5)."),
            ("n", "Las opciones de ese tipo. Cada cambio genera una tanda nueva al instante."),
            ("n", "La línea de fuerza: una barra de cinco pasos, la entropía en bits, cuántos caracteres puede tomar cada posición y cuánto tardaría adivinarla (sección 7)."),
            ("n", "Generar (F5): una tanda completamente nueva."),
            ("n", "La lista: hasta 50 contraseñas. Los dígitos van en azul y los símbolos en ámbar, para releer o dictar más fácil; lo que se copia es texto común."),
            ("h", "4. Los tres tipos"),
            ("b", "Caracteres (4 a 128): los más fuertes para su largo; ideales cuando la contraseña se pega en vez de tipearse. Elegís qué clases usar (a\u2013z, A\u2013Z, 0\u20139, símbolos) y qué símbolos: Básicos (! @ # $ % & * + - = ?), Los 32 (todos los símbolos imprimibles), Mínimos (- _ .) o Los míos. Opciones: Sin parecidos (0 O 1 l I | y comillas), Sin repetir seguidos, Empezar con letra y la casilla Excluir para cualquier otro carácter. Cada clase encendida aparece al menos una vez."),
            ("b", "Frase (3 a 12 palabras): palabras al azar de la lista de 7.776 de la EFF, como Copper-Showroom-Wrecking7-Salute. Más larga, pero mucho más fácil de recordar, dictar o tipear en el celular. Separador: guion, espacio, punto, guion bajo, ninguno o un dígito al azar (más fuerte). Opciones: Mayúsculas (en cada palabra), Agregar un dígito. Seis palabras dan unos 80 bits."),
            ("b", "PIN (4 a 128 dígitos): solo números. Opciones: Sin repetir seguidos, Sin seguidos tipo 123 o 987. Seguro solo donde los intentos fallidos están limitados, como en tarjetas, celulares y cerraduras."),
            ("h", "5. Perfiles, y sitios que no aceptan algunos caracteres"),
            ("p", "El botón Perfil (también el menú Contraseñas) configura todo de una vez: Estándar (20 caracteres, símbolos básicos), Máxima (32 caracteres, todos los símbolos), Sin símbolos, Símbolos mínimos (solo - _ ., empezando con letra), Corta (12 caracteres, para un máximo bajo), Wi-Fi (fácil de tipear en un celular o un televisor), Frase (6 palabras) y PIN (6 dígitos). Si cambiás algo por tu cuenta, el botón dice Personalizado."),
            ("b", "El sitio no acepta símbolos: perfil Sin símbolos, o apagá el chip #$%."),
            ("b", "Acepta solo algunos símbolos: Símbolos \u2192 Mínimos, o Los míos, y escribí exactamente los que el sitio permite."),
            ("b", "Rechaza caracteres puntuales: escribilos en Excluir."),
            ("b", "Tiene un largo máximo: bajá el largo, o perfil Corta; compensá usando todas las clases."),
            ("b", "Pide empezar con letra, o no repetir caracteres: encendé Empezar con letra, o Sin repetir seguidos."),
            ("h", "6. Copiar y guardar"),
            ("b", "Una contraseña: el ícono de copiar al final de su fila, doble clic, o Enter o Ctrl+C sobre la fila seleccionada. El ícono se vuelve una tilde por un momento."),
            ("b", "Todas: el ícono de copiar arriba a la derecha de la lista, o Ctrl+Shift+C, una por línea."),
            ("b", "Guardar (el ícono de la flecha, o Ctrl+S): todas a un archivo de texto, una por línea. Guardá el archivo en un lugar seguro o borralo después de usarlo."),
            ("h", "7. Cómo se generan, y su fuerza"),
            ("p", "Cada carácter o palabra sale del generador criptográfico de números aleatorios de Windows (BCryptGenRandom), sin sesgo. Cuando una contraseña rompe una regla \u2014 falta una clase, dos vecinos iguales, una seguidilla como 123 \u2014 se descarta y se sortea de nuevo, nunca se parcha. Así, todas las contraseñas que cumplen las reglas son igual de probables: ninguna posición, clase o patrón queda favorecido."),
            ("p", "La entropía se cuenta exacta, con todas las reglas: n bits son 2^n contraseñas posibles. Menos de 50 bits es débil, 64 aceptable, 80 o más muy fuerte. El tiempo para adivinarla supone un atacante que tiene el archivo y prueba 100.000 millones de contraseñas por segundo. Los 20 caracteres de fábrica dan unos 122 bits."),
            ("h", "8. Protección del portapapeles"),
            ("p", "Toda copia queda fuera del historial del portapapeles de Windows (Win+V) y de la nube. Con Borrar portapapeles a los 30 s encendido, una contraseña copiada se borra 30 segundos después, o al cerrar el programa, salvo que se haya copiado otra cosa mientras tanto."),
            ("h", "9. Tema y color"),
            ("img", "theme_es.png", "Ver \u2192 Tema y color: la apariencia y doce colores."),
            ("p", "Ver \u2192 Tema y color elige la apariencia (como Windows, claro u oscuro) y uno de doce colores. El color pinta los botones, la banda de arriba y el fondo del ícono, en la ventana y en la barra de tareas. El negro es el de fábrica."),
            ("h", "10. Configuración"),
            ("p", "El tipo, las opciones, el idioma, el tema, el color y la posición de la ventana se guardan en %APPDATA%\\Wyrmhold\\wyrmhold.ini (o en el wyrmhold.ini portable, sección 2). Nunca se guardan contraseñas ahí. Borralo para volver a la configuración de fábrica."),
            ("h", "11. Línea de comandos"),
            ("p", "Para scripts y pruebas, el programa también funciona sin ventana:"),
            ("code", "wyrmhold.exe /out:passwords.txt [/mode:chars|words|pin] [/len:N] [/words:N] [/count:N] [opciones]"),
            ("p", "/out escribe las contraseñas en el archivo, una por línea, y termina; /entropy:archivo escribe en cambio la entropía. Opciones: /classes:luds, /symbols:basic|all|min|custom:..., /exclude:..., /ambiguous, /norepeat, /startletter, /sep:hyphen|space|dot|underscore|none|digit, /nocap, /nonumber, /allowruns, /preset:N. Para la ventana: /lang:en|jp|es, /theme:light|dark, /accent:N. Ninguna cambia la configuración."),
            ("h", "12. Compilar desde el código"),
            ("p", "El código es C (Win32). Con el GCC mingw-w64 de MSYS2 en C:\\msys64, build.bat genera build\\wyrmhold.exe; tools\\make_installer.ps1 arma el instalador con Inno Setup 6. tools\\run_checks.ps1 y tools\\check_options.py verifican las contraseñas y la entropía exacta contra cálculos independientes, y tools\\check_clipboard.ps1 el portapapeles."),
            ("h", "13. Licencia y créditos"),
            ("p", "Software libre bajo la Licencia Pública General de GNU, versión 2 (GPL-2.0-only): podés usarlo, copiarlo, estudiarlo y modificarlo, y quien lo redistribuya debe hacerlo bajo la misma licencia, con el código fuente. Se ofrece sin garantía. El texto completo está en LICENSE.txt. \u00A9 2026 Gabriel Castiglia."),
            ("p", "Código fuente, versiones y actualizaciones: https://" + REPO),
            ("p", "Palabras de las frases: la lista grande de la EFF, de la Electronic Frontier Foundation, bajo CC BY 3.0 US (eff.org/dice). Ícono: el dragón alado de Athanasius Kircher, Mundus Subterraneus (Ámsterdam, 1665), de dominio público."),
        ],
    },
}


def set_font(run, latin, east, size=None, bold=None, color=None):
    run.font.name = latin
    rpr = run._element.get_or_add_rPr()
    fonts = rpr.find(qn("w:rFonts"))
    if fonts is None:
        fonts = rpr.makeelement(qn("w:rFonts"), {})
        rpr.append(fonts)
    for k in ("w:ascii", "w:hAnsi", "w:cs"):
        fonts.set(qn(k), latin)
    fonts.set(qn("w:eastAsia"), east)
    if size: run.font.size = Pt(size)
    if bold is not None: run.font.bold = bold
    if color is not None: run.font.color.rgb = color


def build(lang: str, g: dict) -> Path:
    doc = Document()
    sec = doc.sections[0]
    sec.page_width, sec.page_height = Cm(21.0), Cm(29.7)
    for side in ("left_margin", "right_margin"):
        setattr(sec, side, Cm(2.2))
    sec.top_margin, sec.bottom_margin = Cm(2.0), Cm(2.0)

    normal = doc.styles["Normal"]
    normal.font.size = Pt(10.5)
    normal.paragraph_format.space_after = Pt(6)
    normal.paragraph_format.line_spacing = 1.15
    f, e = g["font"], g["east"]

    p = doc.add_paragraph()
    p.add_run().add_picture(str(ROOT / "assets" / "app256.png"), width=Cm(1.6))
    set_font(p.add_run("  " + g["title"]), f, e, 24, True, ACCENT)
    p.paragraph_format.space_after = Pt(2)
    p = doc.add_paragraph()
    set_font(p.add_run(g["subtitle"]), f, e, 10, False, GREY)
    p.paragraph_format.space_after = Pt(14)

    n = 0
    for b in g["blocks"]:
        kind = b[0]
        if kind == "h":
            n = 0
            p = doc.add_paragraph()
            p.paragraph_format.space_before = Pt(12)
            p.paragraph_format.space_after = Pt(4)
            p.paragraph_format.keep_with_next = True
            set_font(p.add_run(b[1]), f, e, 13, True, ACCENT)
        elif kind == "p":
            set_font(doc.add_paragraph().add_run(b[1]), f, e)
        elif kind in ("b", "n"):
            n += 1
            p = doc.add_paragraph()
            p.paragraph_format.left_indent = Cm(0.8)
            p.paragraph_format.first_line_indent = Cm(-0.5)
            p.paragraph_format.space_after = Pt(4)
            set_font(p.add_run(("\u2022" if kind == "b" else f"{n}.") + "\u00A0\u00A0"), f, e, bold=(kind == "n"), color=ACCENT)
            set_font(p.add_run(b[1]), f, e)
        elif kind == "code":
            p = doc.add_paragraph()
            p.paragraph_format.left_indent = Cm(0.8)
            set_font(p.add_run(b[1]), "Consolas", "Consolas", 9.5)
        elif kind == "img":
            p = doc.add_paragraph()
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.keep_with_next = True
            p.add_run().add_picture(str(IMAGES / b[1]), width=Cm(12.5))
            c = doc.add_paragraph()
            c.alignment = WD_ALIGN_PARAGRAPH.CENTER
            set_font(c.add_run(b[2]), f, e, 9, False, GREY)

    footer = sec.footer.paragraphs[0]
    footer.alignment = WD_ALIGN_PARAGRAPH.CENTER
    set_font(footer.add_run(f"{g['title']}  \u00B7  v{VERSION}  \u00B7  {AUTHOR}"), f, e, 8.5, False, GREY)

    doc.core_properties.author = AUTHOR
    doc.core_properties.title = g["title"]
    doc.core_properties.company = ""
    out = DOCS / f"wyrmhold_guide_{lang}.docx"
    doc.save(out)
    return out


def main() -> None:
    DOCS.mkdir(exist_ok=True)
    for lang, g in GUIDES.items():
        for b in g["blocks"]:
            if b[0] == "img" and not (IMAGES / b[1]).exists():
                raise SystemExit(f"missing screenshot {IMAGES / b[1]}")
        print("written:", build(lang, g))


if __name__ == "__main__":
    main()
