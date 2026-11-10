/**
  Random Password Generator -- every piece of text the window shows.
  Author: Gabriel Castiglia - 2026-11-09

  One table, three columns (English, Japanese, Spanish). Nothing user-visible is written
  anywhere else in the program. Each entry is a designated initialiser, so a column cannot
  drift out of step with the enum. The interface uses the wide Win32 API throughout, so
  Japanese renders on any Windows regardless of its code page.

  Format strings: a %d or %ls in one language is in the same order in the others.
**/

#ifndef UI_STRINGS_H
#define UI_STRINGS_H

enum { LANG_EN = 0, LANG_JP = 1, LANG_ES = 2, LANG_COUNT = 3 };

enum {
	S_WINDOW_TITLE = 0, S_APP_TITLE, S_APP_SUBTITLE,
	S_GRP_SET, S_GRP_LIST,
	S_LBL_LENGTH, S_LBL_COUNT,
	S_OPT_AMBIG, S_OPT_CLEAR, S_TIP_AMBIG, S_TIP_CLEAR,
	S_GENERATE, S_TIP_GENERATE, S_TIP_COPYALL, S_TIP_SAVE,
	S_STRENGTH_FMT, S_STR_WEAK, S_STR_FAIR, S_STR_STRONG, S_STR_VSTRONG,
	S_ST_READY, S_ST_GENERATED, S_ST_GENERATED_ONE, S_ST_COPIED, S_ST_COPIED_ALL,
	S_ST_CLEAR_IN, S_ST_CLEARED, S_ST_SAVED, S_ST_SAVE_FAIL, S_ST_CLIP_FAIL, S_ST_RNG_FAIL,
	S_SAVE_FILTER, S_SAVE_DEFNAME,
	S_MENU_FILE, S_MENU_GENERATE, S_MENU_COPY, S_MENU_COPYALL, S_MENU_SAVE, S_MENU_EXIT,
	S_MENU_LANG, S_MENU_EN, S_MENU_JP, S_MENU_ES,
	S_MENU_HELP, S_MENU_HELPTOPICS, S_MENU_ABOUT,
	S_HELP_TITLE, S_HELP_BODY, S_BTN_OK,
	S_ABOUT_TITLE, S_ABOUT_NAME, S_ABOUT_VERSION, S_ABOUT_CREATED, S_ABOUT_BODY,
	S_ABOUT_AUTHOR, S_ABOUT_LICENSE,
	S_COUNT
};

static const wchar_t* const STR[LANG_COUNT][S_COUNT] = {

/* ------------------------------------------------------------------ English */
{
	[S_WINDOW_TITLE] = L"Random Password Generator",
	[S_APP_TITLE]    = L"Random Password Generator",
	[S_APP_SUBTITLE] = L"Strong passwords for zip, 7z and rar files sent by email",
	[S_GRP_SET]      = L"1.  Settings",
	[S_GRP_LIST]     = L"2.  Passwords",
	[S_LBL_LENGTH]   = L"Length",
	[S_LBL_COUNT]    = L"How many",
	[S_OPT_AMBIG]    = L"Avoid ambiguous characters  (0 O 1 l I)",
	[S_OPT_CLEAR]    = L"Clear the clipboard 30 s after copying",
	[S_TIP_AMBIG]    = L"Leaves out 0, O, 1, l and I.\n"
	                   L"They look alike in many fonts, so a password that is\n"
	                   L"read out or typed by hand goes wrong less often.",
	[S_TIP_CLEAR]    = L"30 seconds after you copy a password it is wiped\n"
	                   L"from the clipboard, unless you have copied something\n"
	                   L"else in between. Copies never go to the clipboard\n"
	                   L"history (Win+V) or to the cloud clipboard.",
	[S_GENERATE]     = L"Generate",
	[S_TIP_GENERATE] = L"A new set of passwords (F5). Nothing is stored.",
	[S_TIP_COPYALL]  = L"Copy all the passwords, one per line (Ctrl+Shift+C)",
	[S_TIP_SAVE]     = L"Save all the passwords to a text file (Ctrl+S)",
	[S_STRENGTH_FMT] = L"%d characters per position  \x00B7  about %d bits  \x00B7  %ls",
	[S_STR_WEAK]     = L"weak",
	[S_STR_FAIR]     = L"fair",
	[S_STR_STRONG]   = L"strong",
	[S_STR_VSTRONG]  = L"very strong",
	[S_ST_READY]     = L"Ready.",
	[S_ST_GENERATED] = L"%d new passwords of %d characters.",
	[S_ST_GENERATED_ONE] = L"1 new password of %d characters.",
	[S_ST_COPIED]    = L"Password %d copied to the clipboard.",
	[S_ST_COPIED_ALL]= L"%d passwords copied to the clipboard.",
	[S_ST_CLEAR_IN]  = L"  It will be cleared in %d s.",
	[S_ST_CLEARED]   = L"The clipboard was cleared.",
	[S_ST_SAVED]     = L"Saved: %ls",
	[S_ST_SAVE_FAIL] = L"Could not write the file.",
	[S_ST_CLIP_FAIL] = L"Could not open the clipboard. Try again.",
	[S_ST_RNG_FAIL]  = L"The Windows random number generator failed. No password was made.",
	[S_SAVE_FILTER]  = L"Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME] = L"passwords.txt",
	[S_MENU_FILE]    = L"&File",
	[S_MENU_GENERATE]= L"&Generate\tF5",
	[S_MENU_COPY]    = L"&Copy selected\tCtrl+C",
	[S_MENU_COPYALL] = L"Copy &all\tCtrl+Shift+C",
	[S_MENU_SAVE]    = L"&Save as...\tCtrl+S",
	[S_MENU_EXIT]    = L"E&xit",
	[S_MENU_LANG]    = L"&Language",
	[S_MENU_EN]      = L"English",
	[S_MENU_JP]      = L"\x65E5\x672C\x8A9E",
	[S_MENU_ES]      = L"Espa\x00F1ol",
	[S_MENU_HELP]    = L"&Help",
	[S_MENU_HELPTOPICS] = L"&Help\tF1",
	[S_MENU_ABOUT]   = L"&About",
	[S_HELP_TITLE]   = L"Random Password Generator \x2014 Help",
	[S_HELP_BODY]    =
		L"WHAT IT DOES\r\n"
		L"Makes strong random passwords, for example to protect a zip, 7z or rar file "
		L"that is sent by email.\r\n\r\n"
		L"HOW TO USE IT\r\n"
		L"1.  Choose the length (4 to 128 characters, 20 by default) and how many "
		L"passwords you want (1 to 50).\r\n"
		L"2.  Press Generate (F5). Every press gives a completely new set. A fresh set "
		L"is also made when the program starts.\r\n"
		L"3.  To copy one password: click the copy icon at the end of its row, "
		L"double-click the row, or select it and press Enter or Ctrl+C.\r\n"
		L"4.  Ctrl+Shift+C copies all of them, one per line. Save as (Ctrl+S) writes "
		L"them to a text file, one per line.\r\n\r\n"
		L"THE RULES\r\n"
		L"\x2022  Every password has at least one lowercase letter, one uppercase letter, "
		L"one digit and one symbol, in random positions.\r\n"
		L"\x2022  The symbols are  ! @ # $ % & * + - = ?  which are accepted almost "
		L"everywhere.\r\n"
		L"\x2022  'Avoid ambiguous characters' leaves out 0 O 1 l I, which are easy to "
		L"mix up when a password is read out or typed by hand.\r\n"
		L"\x2022  The randomness comes from the Windows cryptographic random number "
		L"generator (BCryptGenRandom), never from a predictable one.\r\n"
		L"\x2022  The rules are the same as password_gen.py in the RandomPassGen folder.\r\n\r\n"
		L"COLOURS\r\n"
		L"In the list, digits are blue and symbols are amber, so a password is easier "
		L"to read back or dictate. What is copied is plain text.\r\n\r\n"
		L"STRENGTH\r\n"
		L"The line under the settings shows how many characters each position can take "
		L"and the entropy: length \x00D7 log2(characters). 80 bits or more is very strong for "
		L"a file sent by email; the default 20 characters give about 122 bits.\r\n\r\n"
		L"CLIPBOARD\r\n"
		L"With 'Clear the clipboard 30 s after copying' on, a copied password is wiped "
		L"30 seconds later, or when the program is closed, unless something else was "
		L"copied in between. Copies are always kept out of the Windows clipboard "
		L"history (Win+V) and the cloud clipboard.\r\n\r\n"
		L"PORTABLE\r\n"
		L"Nothing to install: the program is this one file. Its settings (length, how many, "
		L"options, language, window position) are kept in randompassgen.ini next to it, "
		L"when that folder can be written to. Windows only offers to pin a program to the "
		L"taskbar when it runs from a local disk, not from a network folder.\r\n\r\n"
		L"GOOD PRACTICE\r\n"
		L"Send the password through a different channel from the file: the file by "
		L"email, the password in a separate email, a chat or a text message. Nothing is "
		L"stored by this program: once the window is closed, the passwords are gone.\r\n",
	[S_BTN_OK]       = L"OK",
	[S_ABOUT_TITLE]  = L"About",
	[S_ABOUT_NAME]   = L"Random Password Generator",
	[S_ABOUT_VERSION]= L"Version %ls",
	[S_ABOUT_CREATED]= L"Created 2026-11-09",
	[S_ABOUT_BODY]   = L"Makes strong random passwords with the Windows cryptographic random "
	                   L"number generator. Each password has at least one lowercase letter, "
	                   L"one uppercase letter, one digit and one symbol. Nothing is stored or "
	                   L"sent anywhere: the passwords exist only in this window until you copy "
	                   L"or save them.",
	[S_ABOUT_AUTHOR] = L"Author: %ls",
	[S_ABOUT_LICENSE]= L"Free software under the GNU General Public License, version 2 (GPL-2.0-only). "
	                   L"It comes with no warranty. \x00A9 2026 Gabriel Castiglia.",
},

/* ------------------------------------------------------------------ Japanese */
{
	[S_WINDOW_TITLE] = L"ランダムパスワード生成ツール",
	[S_APP_TITLE]    = L"ランダムパスワード生成ツール",
	[S_APP_SUBTITLE] = L"メールで送る zip・7z・rar ファイル用の強力なパスワード",
	[S_GRP_SET]      = L"1.  設定",
	[S_GRP_LIST]     = L"2.  パスワード",
	[S_LBL_LENGTH]   = L"文字数",
	[S_LBL_COUNT]    = L"生成数",
	[S_OPT_AMBIG]    = L"紛らわしい文字を使わない  (0 O 1 l I)",
	[S_OPT_CLEAR]    = L"コピーの 30 秒後にクリップボードを消去する",
	[S_TIP_AMBIG]    = L"0、O、1、l、I を使いません。\n"
	                   L"多くのフォントで見分けにくいため、読み上げや\n"
	                   L"手入力での間違いが減ります。",
	[S_TIP_CLEAR]    = L"パスワードをコピーしてから 30 秒後に\n"
	                   L"クリップボードから消去します（その間に別のものを\n"
	                   L"コピーした場合は消去しません）。クリップボードの\n"
	                   L"履歴 (Win+V) やクラウドには残りません。",
	[S_GENERATE]     = L"生成",
	[S_TIP_GENERATE] = L"新しいパスワードを生成します (F5)。何も保存されません。",
	[S_TIP_COPYALL]  = L"すべてのパスワードを 1 行ずつコピー (Ctrl+Shift+C)",
	[S_TIP_SAVE]     = L"すべてのパスワードをテキストファイルに保存 (Ctrl+S)",
	[S_STRENGTH_FMT] = L"1 文字あたり %d 種類  \x00B7  約 %d ビット  \x00B7  %ls",
	[S_STR_WEAK]     = L"弱い",
	[S_STR_FAIR]     = L"普通",
	[S_STR_STRONG]   = L"強い",
	[S_STR_VSTRONG]  = L"非常に強い",
	[S_ST_READY]     = L"準備完了。",
	[S_ST_GENERATED] = L"パスワードを %d 個生成しました（%d 文字）。",
	[S_ST_GENERATED_ONE] = L"パスワードを 1 個生成しました（%d 文字）。",
	[S_ST_COPIED]    = L"パスワード %d をクリップボードにコピーしました。",
	[S_ST_COPIED_ALL]= L"%d 個のパスワードをクリップボードにコピーしました。",
	[S_ST_CLEAR_IN]  = L"  %d 秒後に消去します。",
	[S_ST_CLEARED]   = L"クリップボードを消去しました。",
	[S_ST_SAVED]     = L"保存しました: %ls",
	[S_ST_SAVE_FAIL] = L"ファイルを書き込めませんでした。",
	[S_ST_CLIP_FAIL] = L"クリップボードを開けませんでした。もう一度お試しください。",
	[S_ST_RNG_FAIL]  = L"Windows の乱数生成器でエラーが発生しました。パスワードは生成されていません。",
	[S_SAVE_FILTER]  = L"テキストファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME] = L"passwords.txt",
	[S_MENU_FILE]    = L"ファイル(&F)",
	[S_MENU_GENERATE]= L"生成(&G)\tF5",
	[S_MENU_COPY]    = L"選択したものをコピー(&C)\tCtrl+C",
	[S_MENU_COPYALL] = L"すべてコピー(&A)\tCtrl+Shift+C",
	[S_MENU_SAVE]    = L"名前を付けて保存(&S)...\tCtrl+S",
	[S_MENU_EXIT]    = L"終了(&X)",
	[S_MENU_LANG]    = L"言語(&L)",
	[S_MENU_EN]      = L"English",
	[S_MENU_JP]      = L"日本語",
	[S_MENU_ES]      = L"Espa\x00F1ol",
	[S_MENU_HELP]    = L"ヘルプ(&H)",
	[S_MENU_HELPTOPICS] = L"ヘルプ(&H)\tF1",
	[S_MENU_ABOUT]   = L"バージョン情報(&A)",
	[S_HELP_TITLE]   = L"ランダムパスワード生成ツール \x2014 ヘルプ",
	[S_HELP_BODY]    =
		L"概要\r\n"
		L"強力なランダムパスワードを生成します。メールで送る zip・7z・rar ファイルの保護などに使います。\r\n\r\n"
		L"使い方\r\n"
		L"1.  文字数（4～128、既定は 20）と生成数（1～50）を指定します。\r\n"
		L"2.  [生成] (F5) を押します。押すたびにまったく新しいパスワードが生成されます。起動時にも生成されます。\r\n"
		L"3.  1 つだけコピーするには、行の右端のコピーアイコンをクリックするか、行をダブルクリックするか、行を選択して Enter または Ctrl+C を押します。\r\n"
		L"4.  Ctrl+Shift+C ですべてを 1 行ずつコピーします。[名前を付けて保存] (Ctrl+S) でテキストファイルに 1 行ずつ保存します。\r\n\r\n"
		L"生成ルール\r\n"
		L"\x2022  各パスワードには小文字・大文字・数字・記号がそれぞれ 1 文字以上、ランダムな位置に含まれます。\r\n"
		L"\x2022  記号は  ! @ # $ % & * + - = ?  で、ほとんどのシステムで使えます。\r\n"
		L"\x2022  [紛らわしい文字を使わない] をオンにすると、読み上げや手入力で間違えやすい 0 O 1 l I を除外します。\r\n"
		L"\x2022  乱数は Windows の暗号論的乱数生成器 (BCryptGenRandom) から取得します。予測可能な乱数は使いません。\r\n"
		L"\x2022  ルールは RandomPassGen フォルダーの password_gen.py と同じです。\r\n\r\n"
		L"色分け\r\n"
		L"一覧では数字が青、記号がオレンジで表示され、読み返しや読み上げがしやすくなっています。コピーされるのはプレーンテキストです。\r\n\r\n"
		L"強度\r\n"
		L"設定欄の下の行に、1 文字あたりの文字の種類とエントロピー（文字数 \x00D7 log2(種類)）を表示します。メールで送るファイルには 80 ビット以上で十分に強力です。既定の 20 文字で約 122 ビットです。\r\n\r\n"
		L"クリップボード\r\n"
		L"[コピーの 30 秒後にクリップボードを消去する] をオンにすると、コピーしたパスワードを 30 秒後またはプログラム終了時に消去します（その間に別のものをコピーした場合は消去しません）。コピーは常に Windows のクリップボード履歴 (Win+V) とクラウドクリップボードの対象外です。\r\n\r\n"
		L"ポータブル\r\n"
		L"インストールは不要で、プログラムはこのファイル 1 つだけです。設定（文字数・生成数・オプション・言語・ウィンドウ位置）は、書き込みできる場合は同じフォルダーの randompassgen.ini に保存されます。タスクバーへのピン留めは、ネットワークフォルダーではなくローカルディスクから起動した場合にのみ Windows が提供します。\r\n\r\n"
		L"注意事項\r\n"
		L"パスワードはファイルとは別の経路で送ってください（ファイルはメール、パスワードは別のメール・チャット・SMS など）。このプログラムは何も保存しません。ウィンドウを閉じるとパスワードは残りません。\r\n",
	[S_BTN_OK]       = L"OK",
	[S_ABOUT_TITLE]  = L"バージョン情報",
	[S_ABOUT_NAME]   = L"ランダムパスワード生成ツール",
	[S_ABOUT_VERSION]= L"バージョン %ls",
	[S_ABOUT_CREATED]= L"作成日 2026-11-09",
	[S_ABOUT_BODY]   = L"Windows の暗号論的乱数生成器で強力なランダムパスワードを生成します。"
	                   L"各パスワードには小文字・大文字・数字・記号がそれぞれ 1 文字以上含まれます。"
	                   L"パスワードは保存も送信もされず、コピーまたは保存するまでこのウィンドウの中にしか存在しません。",
	[S_ABOUT_AUTHOR] = L"作成者: %ls",
	[S_ABOUT_LICENSE]= L"GNU 一般公衆利用許諾書 バージョン 2 (GPL-2.0-only) に基づくフリーソフトウェアです。"
	                   L"無保証です。\x00A9 2026 Gabriel Castiglia",
},

/* ------------------------------------------------------------------ Spanish */
{
	[S_WINDOW_TITLE] = L"Generador de contrase\x00F1" L"as aleatorias",
	[S_APP_TITLE]    = L"Generador de contrase\x00F1" L"as",
	[S_APP_SUBTITLE] = L"Contrase\x00F1" L"as fuertes para archivos zip, 7z y rar enviados por email",
	[S_GRP_SET]      = L"1.  Opciones",
	[S_GRP_LIST]     = L"2.  Contrase\x00F1" L"as",
	[S_LBL_LENGTH]   = L"Largo",
	[S_LBL_COUNT]    = L"Cantidad",
	[S_OPT_AMBIG]    = L"Evitar caracteres ambiguos  (0 O 1 l I)",
	[S_OPT_CLEAR]    = L"Borrar el portapapeles 30 s despu\x00E9s de copiar",
	[S_TIP_AMBIG]    = L"No usa 0, O, 1, l ni I.\n"
	                   L"Se parecen en muchas fuentes: una contrase\x00F1" L"a dictada\n"
	                   L"o tecleada a mano falla menos.",
	[S_TIP_CLEAR]    = L"30 segundos despu\x00E9s de copiar una contrase\x00F1" L"a, se borra\n"
	                   L"del portapapeles, salvo que en el medio hayas\n"
	                   L"copiado otra cosa. Las copias nunca quedan en el\n"
	                   L"historial (Win+V) ni en el portapapeles en la nube.",
	[S_GENERATE]     = L"Generar",
	[S_TIP_GENERATE] = L"Un juego nuevo de contrase\x00F1" L"as (F5). No se guarda nada.",
	[S_TIP_COPYALL]  = L"Copiar todas las contrase\x00F1" L"as, una por l\x00ED" L"nea (Ctrl+Shift+C)",
	[S_TIP_SAVE]     = L"Guardar todas las contrase\x00F1" L"as en un archivo de texto (Ctrl+S)",
	[S_STRENGTH_FMT] = L"%d caracteres por posici\x00F3" L"n  \x00B7  unos %d bits  \x00B7  %ls",
	[S_STR_WEAK]     = L"d\x00E9" L"bil",
	[S_STR_FAIR]     = L"aceptable",
	[S_STR_STRONG]   = L"fuerte",
	[S_STR_VSTRONG]  = L"muy fuerte",
	[S_ST_READY]     = L"Listo.",
	[S_ST_GENERATED] = L"%d contrase\x00F1" L"as nuevas de %d caracteres.",
	[S_ST_GENERATED_ONE] = L"1 contrase\x00F1" L"a nueva de %d caracteres.",
	[S_ST_COPIED]    = L"Contrase\x00F1" L"a %d copiada al portapapeles.",
	[S_ST_COPIED_ALL]= L"%d contrase\x00F1" L"as copiadas al portapapeles.",
	[S_ST_CLEAR_IN]  = L"  Se borra en %d s.",
	[S_ST_CLEARED]   = L"Se borr\x00F3 el portapapeles.",
	[S_ST_SAVED]     = L"Guardado: %ls",
	[S_ST_SAVE_FAIL] = L"No se pudo escribir el archivo.",
	[S_ST_CLIP_FAIL] = L"No se pudo abrir el portapapeles. Prob\x00E1 de nuevo.",
	[S_ST_RNG_FAIL]  = L"Fall\x00F3 el generador aleatorio de Windows. No se gener\x00F3 ninguna contrase\x00F1" L"a.",
	[S_SAVE_FILTER]  = L"Archivos de texto (*.txt)\0*.txt\0Todos los archivos (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME] = L"passwords.txt",
	[S_MENU_FILE]    = L"&Archivo",
	[S_MENU_GENERATE]= L"&Generar\tF5",
	[S_MENU_COPY]    = L"&Copiar la seleccionada\tCtrl+C",
	[S_MENU_COPYALL] = L"Copiar &todas\tCtrl+Shift+C",
	[S_MENU_SAVE]    = L"&Guardar como...\tCtrl+S",
	[S_MENU_EXIT]    = L"&Salir",
	[S_MENU_LANG]    = L"&Idioma",
	[S_MENU_EN]      = L"English",
	[S_MENU_JP]      = L"\x65E5\x672C\x8A9E",
	[S_MENU_ES]      = L"Espa\x00F1ol",
	[S_MENU_HELP]    = L"A&yuda",
	[S_MENU_HELPTOPICS] = L"&Ayuda\tF1",
	[S_MENU_ABOUT]   = L"Acerca &de",
	[S_HELP_TITLE]   = L"Generador de contrase\x00F1" L"as \x2014 Ayuda",
	[S_HELP_BODY]    =
		L"QU\x00C9 HACE\r\n"
		L"Genera contrase\x00F1" L"as aleatorias fuertes, por ejemplo para proteger un zip, 7z o rar "
		L"que se manda por email.\r\n\r\n"
		L"C\x00D3MO SE USA\r\n"
		L"1.  Eleg\x00ED el largo (de 4 a 128 caracteres, 20 por omisi\x00F3n) y cu\x00E1ntas "
		L"contrase\x00F1" L"as quer\x00E9s (de 1 a 50).\r\n"
		L"2.  Apret\x00E1 Generar (F5). Cada vez sale un juego completamente nuevo. Al abrir "
		L"el programa tambi\x00E9n se genera uno.\r\n"
		L"3.  Para copiar una: clic en el \x00ED" L"cono de copiar al final de su fila, doble clic "
		L"en la fila, o seleccionarla y apretar Enter o Ctrl+C.\r\n"
		L"4.  Ctrl+Shift+C copia todas, una por l\x00ED" L"nea. Guardar como (Ctrl+S) las escribe "
		L"en un archivo de texto, una por l\x00ED" L"nea.\r\n\r\n"
		L"LAS REGLAS\r\n"
		L"\x2022  Cada contrase\x00F1" L"a tiene al menos una min\x00FA" L"scula, una may\x00FA" L"scula, un d\x00ED" L"gito "
		L"y un s\x00ED" L"mbolo, en posiciones al azar.\r\n"
		L"\x2022  Los s\x00ED" L"mbolos son  ! @ # $ % & * + - = ?  que se aceptan casi en "
		L"cualquier sistema.\r\n"
		L"\x2022  'Evitar caracteres ambiguos' saca 0 O 1 l I, que se confunden f\x00E1" L"cil al "
		L"dictar o teclear una contrase\x00F1" L"a a mano.\r\n"
		L"\x2022  El azar sale del generador criptogr\x00E1" L"fico de Windows (BCryptGenRandom), "
		L"nunca de uno predecible.\r\n"
		L"\x2022  Las reglas son las mismas de password_gen.py, en la carpeta RandomPassGen.\r\n\r\n"
		L"COLORES\r\n"
		L"En la lista los d\x00ED" L"gitos van en azul y los s\x00ED" L"mbolos en \x00E1" L"mbar, para que una "
		L"contrase\x00F1" L"a se lea o se dicte m\x00E1" L"s f\x00E1" L"cil. Lo que se copia es texto plano.\r\n\r\n"
		L"FUERZA\r\n"
		L"La l\x00ED" L"nea de abajo de las opciones dice cu\x00E1" L"ntos caracteres puede tomar cada "
		L"posici\x00F3" L"n y la entrop\x00ED" L"a: largo \x00D7 log2(caracteres). Con 80 bits o m\x00E1" L"s es muy "
		L"fuerte para un archivo mandado por email; los 20 caracteres por omisi\x00F3" L"n dan "
		L"unos 122 bits.\r\n\r\n"
		L"PORTAPAPELES\r\n"
		L"Con 'Borrar el portapapeles 30 s despu\x00E9s de copiar' activado, una contrase\x00F1" L"a "
		L"copiada se borra a los 30 segundos, o al cerrar el programa, salvo que en el "
		L"medio se haya copiado otra cosa. Las copias nunca entran al historial del "
		L"portapapeles de Windows (Win+V) ni al portapapeles en la nube.\r\n\r\n"
		L"PORT\x00C1" L"TIL\r\n"
		L"No hay nada que instalar: el programa es este \x00FA" L"nico archivo. Su configuraci\x00F3" L"n "
		L"(largo, cantidad, opciones, idioma, posici\x00F3" L"n de la ventana) se guarda en "
		L"randompassgen.ini, al lado, si esa carpeta se puede escribir. Windows s\x00F3" L"lo ofrece "
		L"anclar un programa a la barra de tareas si corre desde un disco local, no desde "
		L"una carpeta de red.\r\n\r\n"
		L"BUENA PR\x00C1" L"CTICA\r\n"
		L"Mand\x00E1 la contrase\x00F1" L"a por un canal distinto del archivo: el archivo por email, la "
		L"contrase\x00F1" L"a en otro email, un chat o un SMS. El programa no guarda nada: al "
		L"cerrar la ventana, las contrase\x00F1" L"as desaparecen.\r\n",
	[S_BTN_OK]       = L"Aceptar",
	[S_ABOUT_TITLE]  = L"Acerca de",
	[S_ABOUT_NAME]   = L"Generador de contrase\x00F1" L"as",
	[S_ABOUT_VERSION]= L"Versi\x00F3" L"n %ls",
	[S_ABOUT_CREATED]= L"Creado el 2026-11-09",
	[S_ABOUT_BODY]   = L"Genera contrase\x00F1" L"as aleatorias fuertes con el generador criptogr\x00E1" L"fico "
	                   L"de Windows. Cada contrase\x00F1" L"a tiene al menos una min\x00FA" L"scula, una "
	                   L"may\x00FA" L"scula, un d\x00ED" L"gito y un s\x00ED" L"mbolo. No se guarda ni se env\x00ED" L"a nada: "
	                   L"las contrase\x00F1" L"as existen s\x00F3" L"lo en esta ventana hasta que las copi\x00E1" L"s o "
	                   L"las guard\x00E1" L"s.",
	[S_ABOUT_AUTHOR] = L"Autor: %ls",
	[S_ABOUT_LICENSE]= L"Software libre bajo la Licencia P\x00FA" L"blica General GNU, versi\x00F3" L"n 2 (GPL-2.0-only). "
	                   L"Se entrega sin ninguna garant\x00ED" L"a. \x00A9 2026 Gabriel Castiglia.",
},
};

#endif
