"""
Random Password Generator -- writes the user guide in English, Japanese and Spanish
(docs/randompassgen_guide_<lang>.docx). The PDFs are made from them by tools/make_pdf.ps1.
Author: Gabriel Castiglia - 2026-11-09

    python tools/make_guide.py

The screenshots in docs/images are taken from the program itself with /shot (see the
header of src/randompassgen.c). The version is read from src/randompassgen.c, so the guide
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
VERSION = re.search(r'#define APP_VERSION\s+L"([\d.]+)"',
                    (ROOT / "src" / "randompassgen.c").read_text(encoding="utf-8")).group(1)
AUTHOR = "Gabriel Castiglia"
DATE = "2026-11-09"
ACCENT = RGBColor(0x2B, 0x4C, 0x7E)
GREY = RGBColor(0x5F, 0x6B, 0x78)

# Each guide: a list of blocks. ("h", text) heading, ("p", text) paragraph,
# ("b", text) bullet, ("n", text) numbered item, ("img", file, caption), ("code", text).
GUIDES = {
    "en": {
        "font": "Segoe UI", "east": "Segoe UI",
        "title": "Random Password Generator",
        "subtitle": f"User guide  \u00B7  version {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. What it is"),
            ("p", "A small Windows program that makes strong random passwords, for example to protect a zip, 7z or rar file sent by email. It follows the same rules as password_gen.py, the Python script in the same folder, and adds a window: settings, a list of passwords, one-click copy, and protection of the clipboard."),
            ("h", "2. Requirements and installation"),
            ("p", "Windows 10 or 11, 64-bit. There is nothing to install: the program is the single file randompassgen.exe. Copy it to any folder and run it. It needs no runtime and writes nothing to the registry."),
            ("p", "Windows only offers to pin a program to the taskbar when it runs from a local disk. To pin it, copy randompassgen.exe to a local folder (for example C:\\Tools\\RandomPassGen) and run it from there."),
            ("h", "3. The window"),
            ("img", "window_en.png", "The main window (light theme). The program follows the light or dark theme of Windows."),
            ("n", "Length: 4 to 128 characters (20 by default). How many: 1 to 50 passwords (5 by default). A new set is made a moment after either value changes."),
            ("n", "Avoid ambiguous characters: leaves out 0, O, 1, l and I, which are easy to mix up when a password is read out or typed by hand. On by default."),
            ("n", "Clear the clipboard 30 s after copying: see section 6. On by default."),
            ("n", "The strength line: how many different characters each position can take, the entropy, and a four-step bar. The entropy is length \u00D7 log2(characters): the number of bits a guesser has to cover. 80 bits or more is very strong for a file sent by email; the default 20 characters give about 122 bits."),
            ("n", "Generate (F5): a completely new set. A set is also made every time the program starts."),
            ("n", "The list: digits are shown in blue and symbols in amber, so a password is easier to read back. What is copied is plain text. A password longer than the row ends in \u2026 on screen; the copy is always complete."),
            ("h", "4. Copying and saving"),
            ("b", "One password: click the copy icon at the end of its row, double-click the row, or select it and press Enter or Ctrl+C. The icon turns into a check mark for a moment."),
            ("b", "All of them: the copy icon at the top right of the list, or Ctrl+Shift+C. They are copied one per line."),
            ("b", "Save as (the arrow icon, or Ctrl+S): writes all of them to a text file, one per line. The file is plain text; keep it somewhere safe or delete it once used."),
            ("h", "5. How the passwords are made"),
            ("b", "Four classes: lowercase letters, uppercase letters, digits, and the symbols ! @ # $ % & * + - = ?"),
            ("b", "Every password has at least one character of each class. The rest are drawn from all classes together, and the whole password is shuffled, so the guaranteed characters can be anywhere."),
            ("b", "The randomness comes from the Windows cryptographic random number generator (BCryptGenRandom). Each draw is uniform: every character of a class is exactly as likely as any other."),
            ("b", "Nothing is stored or sent anywhere. The passwords exist only in the window until they are copied or saved, and are erased from memory when the program closes."),
            ("h", "6. Clipboard protection"),
            ("p", "Every copy is marked so that Windows keeps it out of the clipboard history (Win+V) and the cloud clipboard. With the option on, a copied password is also wiped from the clipboard 30 seconds later, or when the program closes, whichever comes first. The status line counts down. If something else is copied in between, that is left alone: only the program's own copy is wiped."),
            ("h", "7. Settings file"),
            ("p", "Length, how many, the two options, the language and the window position are kept in randompassgen.ini, next to the program, when that folder can be written to. Delete the file to go back to the defaults. Passwords are never written to it."),
            ("h", "8. Command line"),
            ("p", "For scripts and tests the program also works without a window:"),
            ("code", "randompassgen.exe /out:passwords.txt [/len:N] [/count:N] [/ambiguous]"),
            ("p", "/out writes the passwords to the file, one per line, and exits (up to 100000 at a time). /ambiguous allows 0 O 1 l I. Other switches: /lang:en|jp|es and /theme:light|dark choose the language and theme for that run; /shot:file.bmp saves a picture of the window and exits. None of them changes the settings file."),
            ("h", "9. Building from source"),
            ("p", "The Source folder holds the code (C, Win32) and build.bat. With MSYS2 mingw-w64 GCC installed in C:\\msys64, run build.bat; the program is written to build\\randompassgen.exe. tools\\run_checks.ps1 checks the generator against the rules with several hundred thousand passwords, and tools\\check_clipboard.ps1 checks the clipboard behaviour."),
            ("h", "10. License"),
            ("p", "Free software under the GNU General Public License, version 2 (GPL-2.0-only): you may use, copy, study and change it, and anyone who passes it on must do so under the same license, with the source code. It comes with no warranty. The full text is in LICENSE.txt, next to the program. \u00A9 2026 Gabriel Castiglia."),
        ],
    },
    "jp": {
        "font": "Meiryo UI", "east": "Meiryo UI",
        "title": "ランダムパスワード生成ツール",
        "subtitle": f"ユーザーガイド  \u00B7  バージョン {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. 概要"),
            ("p", "強力なランダムパスワードを生成する Windows 用の小さなプログラムです。メールで送る zip・7z・rar ファイルの保護などに使います。同じフォルダーにある Python スクリプト password_gen.py と同じルールで生成し、設定・パスワード一覧・ワンクリックコピー・クリップボード保護を備えたウィンドウを追加しています。"),
            ("h", "2. 動作環境とインストール"),
            ("p", "Windows 10 または 11（64 ビット）。インストールは不要で、プログラムは randompassgen.exe の 1 ファイルだけです。任意のフォルダーにコピーして起動してください。ランタイムは不要で、レジストリにも何も書き込みません。"),
            ("p", "タスクバーへのピン留めは、ローカルディスクから起動した場合にのみ Windows が提供します。ピン留めする場合は、randompassgen.exe をローカルフォルダー（例: C:\\Tools\\RandomPassGen）にコピーし、そこから起動してください。"),
            ("h", "3. ウィンドウ"),
            ("img", "window_jp.png", "メインウィンドウ（ライトテーマ）。Windows のライト／ダークテーマに従います。"),
            ("n", "文字数: 4～128 文字（既定は 20）。生成数: 1～50 個（既定は 5）。どちらかを変更すると、少し後に新しいパスワードが生成されます。"),
            ("n", "紛らわしい文字を使わない: 読み上げや手入力で間違えやすい 0、O、1、l、I を除外します。既定でオンです。"),
            ("n", "コピーの 30 秒後にクリップボードを消去する: 6 章を参照してください。既定でオンです。"),
            ("n", "強度の行: 1 文字あたりの文字の種類、エントロピー、4 段階のバーを表示します。エントロピーは「文字数 \u00D7 log2(種類)」で、推測する側が試す必要のあるビット数です。メールで送るファイルには 80 ビット以上で十分に強力です。既定の 20 文字で約 122 ビットです。"),
            ("n", "[生成] (F5): まったく新しいパスワードを生成します。プログラムの起動時にも毎回生成されます。"),
            ("n", "一覧: 数字は青、記号はオレンジで表示され、読み返しやすくなっています。コピーされるのはプレーンテキストです。行に収まらない長いパスワードは画面上で … で終わりますが、コピーは常に全体です。"),
            ("h", "4. コピーと保存"),
            ("b", "1 つだけ: 行の右端のコピーアイコンをクリックするか、行をダブルクリックするか、行を選択して Enter または Ctrl+C を押します。アイコンが一瞬チェックマークに変わります。"),
            ("b", "すべて: 一覧の右上のコピーアイコン、または Ctrl+Shift+C。1 行に 1 つずつコピーされます。"),
            ("b", "名前を付けて保存（矢印アイコン、または Ctrl+S）: すべてをテキストファイルに 1 行ずつ書き込みます。プレーンテキストなので、安全な場所に保管するか、使用後に削除してください。"),
            ("h", "5. パスワードの生成方法"),
            ("b", "4 つの文字種: 小文字、大文字、数字、記号 ! @ # $ % & * + - = ?"),
            ("b", "各パスワードには各文字種が 1 文字以上含まれます。残りはすべての文字種から選ばれ、最後に全体をシャッフルするため、保証された文字の位置もランダムです。"),
            ("b", "乱数は Windows の暗号論的乱数生成器 (BCryptGenRandom) から取得します。各抽選は一様で、同じ文字種の文字はすべて同じ確率で選ばれます。"),
            ("b", "何も保存・送信されません。パスワードはコピーまたは保存するまでウィンドウの中にだけ存在し、プログラムを閉じるとメモリから消去されます。"),
            ("h", "6. クリップボードの保護"),
            ("p", "コピーは常に、Windows のクリップボード履歴 (Win+V) とクラウドクリップボードの対象外になるよう指定されます。オプションがオンの場合、コピーしたパスワードは 30 秒後、またはプログラムを閉じたときのどちらか早いほうでクリップボードから消去されます。ステータス行に残り秒数が表示されます。その間に別のものをコピーした場合、それは消去しません。消去するのはこのプログラムがコピーしたものだけです。"),
            ("h", "7. 設定ファイル"),
            ("p", "文字数・生成数・2 つのオプション・言語・ウィンドウ位置は、書き込みできる場合はプログラムと同じフォルダーの randompassgen.ini に保存されます。既定に戻すにはこのファイルを削除してください。パスワードが書き込まれることはありません。"),
            ("h", "8. コマンドライン"),
            ("p", "スクリプトやテスト用に、ウィンドウなしでも動作します:"),
            ("code", "randompassgen.exe /out:passwords.txt [/len:N] [/count:N] [/ambiguous]"),
            ("p", "/out はパスワードを 1 行ずつファイルに書き込んで終了します（1 回に最大 100000 個）。/ambiguous は 0 O 1 l I を許可します。その他: /lang:en|jp|es と /theme:light|dark はその回の言語とテーマを指定し、/shot:file.bmp はウィンドウの画像を保存して終了します。いずれも設定ファイルは変更しません。"),
            ("h", "9. ソースからのビルド"),
            ("p", "Source フォルダーにコード（C、Win32）と build.bat があります。MSYS2 の mingw-w64 GCC を C:\\msys64 にインストールした状態で build.bat を実行すると、build\\randompassgen.exe が生成されます。tools\\run_checks.ps1 は数十万個のパスワードで生成ルールを検証し、tools\\check_clipboard.ps1 はクリップボードの動作を検証します。"),
            ("h", "10. ライセンス"),
            ("p", "GNU 一般公衆利用許諾書 バージョン 2 (GPL-2.0-only) に基づくフリーソフトウェアです。使用・複製・研究・改変ができ、再配布する場合は同じライセンスでソースコードとともに配布する必要があります。無保証です。全文はプログラムと同じフォルダーの LICENSE.txt にあります。\u00A9 2026 Gabriel Castiglia"),
        ],
    },
    "es": {
        "font": "Segoe UI", "east": "Segoe UI",
        "title": "Generador de contrase\u00f1as aleatorias",
        "subtitle": f"Gu\u00eda de uso  \u00B7  versi\u00f3n {VERSION}  \u00B7  {DATE}  \u00B7  {AUTHOR}",
        "blocks": [
            ("h", "1. Qu\u00e9 es"),
            ("p", "Un programa chico para Windows que genera contrase\u00f1as aleatorias fuertes, por ejemplo para proteger un zip, 7z o rar que se manda por email. Sigue las mismas reglas que password_gen.py, el script de Python de la misma carpeta, y le agrega una ventana: opciones, una lista de contrase\u00f1as, copia con un clic y protecci\u00f3n del portapapeles."),
            ("h", "2. Requisitos e instalaci\u00f3n"),
            ("p", "Windows 10 u 11 de 64 bits. No hay nada que instalar: el programa es el \u00fanico archivo randompassgen.exe. Se copia a cualquier carpeta y se ejecuta. No necesita runtime y no escribe nada en el registro."),
            ("p", "Windows s\u00f3lo ofrece anclar un programa a la barra de tareas si corre desde un disco local. Para anclarlo, copi\u00e1 randompassgen.exe a una carpeta local (por ejemplo C:\\Tools\\RandomPassGen) y ejecutalo desde ah\u00ed."),
            ("h", "3. La ventana"),
            ("img", "window_es.png", "La ventana principal (tema oscuro). El programa sigue el tema claro u oscuro de Windows."),
            ("n", "Largo: de 4 a 128 caracteres (20 por omisi\u00f3n). Cantidad: de 1 a 50 contrase\u00f1as (5 por omisi\u00f3n). Al cambiar cualquiera de los dos, enseguida se genera un juego nuevo."),
            ("n", "Evitar caracteres ambiguos: saca 0, O, 1, l e I, que se confunden f\u00e1cil al dictar o teclear una contrase\u00f1a a mano. Activado por omisi\u00f3n."),
            ("n", "Borrar el portapapeles 30 s despu\u00e9s de copiar: ver la secci\u00f3n 6. Activado por omisi\u00f3n."),
            ("n", "La l\u00ednea de fuerza: cu\u00e1ntos caracteres distintos puede tomar cada posici\u00f3n, la entrop\u00eda y una barra de cuatro escalones. La entrop\u00eda es largo \u00D7 log2(caracteres): la cantidad de bits que tiene que cubrir quien intente adivinarla. Con 80 bits o m\u00e1s es muy fuerte para un archivo mandado por email; los 20 caracteres por omisi\u00f3n dan unos 122 bits."),
            ("n", "Generar (F5): un juego completamente nuevo. Tambi\u00e9n se genera uno cada vez que arranca el programa."),
            ("n", "La lista: los d\u00edgitos van en azul y los s\u00edmbolos en \u00e1mbar, para leer la contrase\u00f1a m\u00e1s f\u00e1cil. Lo que se copia es texto plano. Una contrase\u00f1a m\u00e1s larga que la fila termina en \u2026 en pantalla; la copia siempre es completa."),
            ("h", "4. Copiar y guardar"),
            ("b", "Una contrase\u00f1a: clic en el \u00edcono de copiar al final de su fila, doble clic en la fila, o seleccionarla y apretar Enter o Ctrl+C. El \u00edcono pasa a ser una tilde por un momento."),
            ("b", "Todas: el \u00edcono de copiar arriba a la derecha de la lista, o Ctrl+Shift+C. Se copian una por l\u00ednea."),
            ("b", "Guardar como (el \u00edcono de la flecha, o Ctrl+S): las escribe todas en un archivo de texto, una por l\u00ednea. El archivo es texto plano: guardalo en un lugar seguro o borralo despu\u00e9s de usarlo."),
            ("h", "5. C\u00f3mo se generan las contrase\u00f1as"),
            ("b", "Cuatro clases: min\u00fasculas, may\u00fasculas, d\u00edgitos y los s\u00edmbolos ! @ # $ % & * + - = ?"),
            ("b", "Cada contrase\u00f1a tiene al menos un car\u00e1cter de cada clase. El resto se saca de todas las clases juntas y despu\u00e9s se mezcla la contrase\u00f1a entera, as\u00ed que los caracteres garantizados pueden quedar en cualquier posici\u00f3n."),
            ("b", "El azar sale del generador criptogr\u00e1fico de Windows (BCryptGenRandom). Cada sorteo es uniforme: todos los caracteres de una clase tienen exactamente la misma probabilidad."),
            ("b", "No se guarda ni se env\u00eda nada. Las contrase\u00f1as existen s\u00f3lo en la ventana hasta que se copian o se guardan, y se borran de la memoria al cerrar el programa."),
            ("h", "6. Protecci\u00f3n del portapapeles"),
            ("p", "Cada copia va marcada para que Windows la deje fuera del historial del portapapeles (Win+V) y del portapapeles en la nube. Con la opci\u00f3n activada, adem\u00e1s, la contrase\u00f1a copiada se borra del portapapeles a los 30 segundos o al cerrar el programa, lo que pase primero. La l\u00ednea de estado muestra la cuenta regresiva. Si en el medio se copia otra cosa, no se toca: s\u00f3lo se borra la copia hecha por el programa."),
            ("h", "7. Archivo de configuraci\u00f3n"),
            ("p", "El largo, la cantidad, las dos opciones, el idioma y la posici\u00f3n de la ventana se guardan en randompassgen.ini, al lado del programa, si esa carpeta se puede escribir. Para volver a los valores por omisi\u00f3n, se borra el archivo. Las contrase\u00f1as nunca se escriben ah\u00ed."),
            ("h", "8. L\u00ednea de comandos"),
            ("p", "Para scripts y pruebas el programa tambi\u00e9n funciona sin ventana:"),
            ("code", "randompassgen.exe /out:passwords.txt [/len:N] [/count:N] [/ambiguous]"),
            ("p", "/out escribe las contrase\u00f1as en el archivo, una por l\u00ednea, y termina (hasta 100000 por vez). /ambiguous permite 0 O 1 l I. Otros modificadores: /lang:en|jp|es y /theme:light|dark eligen el idioma y el tema para esa corrida; /shot:archivo.bmp guarda una imagen de la ventana y termina. Ninguno cambia el archivo de configuraci\u00f3n."),
            ("h", "9. Compilar desde el c\u00f3digo"),
            ("p", "La carpeta Source tiene el c\u00f3digo (C, Win32) y build.bat. Con el GCC mingw-w64 de MSYS2 instalado en C:\\msys64, se ejecuta build.bat y el programa queda en build\\randompassgen.exe. tools\\run_checks.ps1 verifica el generador contra las reglas con varios cientos de miles de contrase\u00f1as, y tools\\check_clipboard.ps1 verifica el comportamiento del portapapeles."),
            ("h", "10. Licencia"),
            ("p", "Software libre bajo la Licencia P\u00fablica General GNU, versi\u00f3n 2 (GPL-2.0-only): se puede usar, copiar, estudiar y modificar, y quien lo redistribuya debe hacerlo con la misma licencia y con el c\u00f3digo. Se entrega sin ninguna garant\u00eda. El texto completo est\u00e1 en LICENSE.txt, junto al programa. \u00A9 2026 Gabriel Castiglia."),
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
    set_font(p.add_run(g["title"]), f, e, 22, True, ACCENT)
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
    out = DOCS / f"randompassgen_guide_{lang}.docx"
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
