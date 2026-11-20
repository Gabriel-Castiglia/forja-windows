/**
  Forja -- every piece of text the window shows.
  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 9, 2026
  SPDX-License-Identifier: GPL-2.0-only

  One table, three columns (English, Japanese, Spanish). Nothing user-visible is written
  anywhere else in the program. Each entry is a designated initialiser, so a column cannot
  drift out of step with the enum. The interface uses the wide Win32 API throughout, so
  Japanese renders on any Windows regardless of its code page.

  Conventions:
    - a %d or %ls in one language is in the same order in the others;
    - menu-style texts may carry a hint after a tab ("Basic\t! @ #"): the hint is shown
      right-aligned in menus, and buttons show only the part before the tab;
    - groups used by position (S_MODE_*, S_CHIP_*, S_SYM_*, S_SEP_*, S_CLR_10S..S_CLR_10M)
      stay consecutive;
    - the help text: "# " starts a heading, "\x2022 " a bullet, an empty line a paragraph.
**/

#ifndef UI_STRINGS_H
#define UI_STRINGS_H

enum { LANG_EN = 0, LANG_JP = 1, LANG_ES = 2, LANG_COUNT = 3 };

enum {
	S_WINDOW_TITLE = 0, S_APP_TITLE, S_APP_SUBTITLE,
	S_MODE_CHARS, S_MODE_WORDS, S_MODE_PIN,
	S_TIP_MODE_CHARS, S_TIP_MODE_WORDS, S_TIP_MODE_PIN,
	S_PROFILE_FMT, S_TIP_PROFILE,
	S_PRESET_STANDARD, S_PRESET_MAX, S_PRESET_NOSYM, S_PRESET_MINSYM, S_PRESET_SHORT,
	S_PRESET_WIFI, S_PRESET_PHRASE, S_PRESET_PIN, S_PRESET_CUSTOM,
	S_CARD_LIST,
	S_LBL_LENGTH, S_LBL_WORDS, S_LBL_DIGITS, S_LBL_COUNT, S_LBL_INCLUDE, S_LBL_SYMBOLS,
	S_LBL_SEPARATOR, S_LBL_EXCLUDE, S_LBL_CUSTOM,
	S_CHIP_LOWER, S_CHIP_UPPER, S_CHIP_DIGITS, S_CHIP_SYMBOLS, S_TIP_CHIPS,
	S_SYM_BASIC, S_SYM_ALL, S_SYM_MIN, S_SYM_CUSTOM, S_TIP_SYMSET,
	S_SEP_HYPHEN, S_SEP_SPACE, S_SEP_DOT, S_SEP_UNDERSCORE, S_SEP_NONE, S_SEP_DIGIT, S_TIP_SEPARATOR,
	S_OPT_AMBIG, S_OPT_NOREPEAT, S_OPT_START, S_OPT_CAP, S_OPT_NUMBER, S_OPT_NOSEQ, S_OPT_CLEAR,
	S_TIP_AMBIG, S_TIP_NOREPEAT, S_TIP_START, S_TIP_EXCLUDE, S_TIP_CUSTOM,
	S_TIP_CAP, S_TIP_NUMBER, S_TIP_NOSEQ, S_TIP_CLEAR,
	S_CLR_10S, S_CLR_30S, S_CLR_1M, S_CLR_2M, S_CLR_5M, S_CLR_10M, S_CLR_FMT_S, S_CLR_FMT_M, S_TIP_CLEARTIME,
	S_WORDS_NOTE, S_PIN_NOTE,
	S_GENERATE, S_TIP_GENERATE, S_TIP_COPYALL, S_TIP_SAVE,
	S_STR_VWEAK, S_STR_WEAK, S_STR_FAIR, S_STR_STRONG, S_STR_VSTRONG, S_STR_PIN,
	S_STRENGTH_BITS, S_STRENGTH_DETAIL, S_STRENGTH_DETAIL_W, S_TIP_STRENGTH,
	S_TIME_INSTANT, S_TIME_MINUTES, S_TIME_HOURS, S_TIME_DAYS, S_TIME_YEARS,
	S_TIME_CENTURIES, S_TIME_MILLIONS, S_TIME_FOREVER,
	S_CLASS_LOWER, S_CLASS_UPPER, S_CLASS_DIGITS, S_CLASS_SYMBOLS,
	S_ERR_NOCLASS, S_ERR_EMPTYCLASS, S_ERR_TOOSHORT, S_ERR_NODIGITS,
	S_ST_READY, S_ST_GENERATED, S_ST_GENERATED_ONE, S_ST_NOTHING, S_ST_COPIED, S_ST_COPIED_ALL,
	S_ST_CLEAR_IN, S_ST_CLEARED, S_ST_SAVED, S_ST_SAVE_FAIL, S_ST_CLIP_FAIL, S_ST_RNG_FAIL,
	S_SAVE_FILTER, S_SAVE_DEFNAME,
	S_MENU_FILE, S_MENU_GENERATE, S_MENU_COPY, S_MENU_COPYALL, S_MENU_SAVE, S_MENU_EXIT,
	S_MENU_KIND, S_MENU_MODE_CHARS, S_MENU_MODE_WORDS, S_MENU_MODE_PIN,
	S_MENU_VIEW, S_MENU_THEME_AUTO, S_MENU_THEME_LIGHT, S_MENU_THEME_DARK,
	S_MENU_THEMEDLG, S_THEME_TITLE, S_THEME_APPEARANCE, S_THEME_AUTO, S_THEME_LIGHT, S_THEME_DARK, S_THEME_COLOUR,
	S_ACCENT_BLUE, S_ACCENT_GOLD, S_ACCENT_PURPLE, S_ACCENT_SKY, S_ACCENT_EMBER, S_ACCENT_RED,
	S_ACCENT_YELLOW, S_ACCENT_GREEN, S_ACCENT_TEAL, S_ACCENT_PINK, S_ACCENT_BLACK, S_ACCENT_WHITE,
	S_MENU_LANG, S_MENU_EN, S_MENU_JP, S_MENU_ES,
	S_MENU_HELP, S_MENU_HELPTOPICS, S_MENU_ABOUT, S_MENU_UPDATE, S_MENU_UPDATE_AUTO,
	S_UPD_UPTODATE, S_UPD_AVAILABLE, S_UPD_PORTABLE, S_UPD_ERROR, S_UPD_ASK_HEAD, S_UPD_ASK,
	S_UPD_NOW, S_UPD_NOTES, S_UPD_LATER, S_UPD_CHECKING, S_UPD_DOWNLOADING, S_UPD_NOHASH, S_UPD_BADHASH,
	S_UPD_FAILED, S_UPD_UPDATED, S_UPD_NET, S_UPD_BADREPLY,
	S_HELP_TITLE, S_HELP_BODY, S_BTN_OK,
	S_ABOUT_TITLE, S_ABOUT_NAME, S_ABOUT_MEANING, S_ABOUT_VERSION, S_ABOUT_BODY, S_ABOUT_AUTHOR, S_ABOUT_CREDITS, S_ABOUT_LICENSE,
	S_COUNT
};

static const wchar_t* const STR[LANG_COUNT][S_COUNT] = {

/* ------------------------------------------------------------------ English */
{
	[S_WINDOW_TITLE]   = L"Forja",
	[S_APP_TITLE]      = L"Forja",
	[S_APP_SUBTITLE]   = L"Strong passwords, passphrases and PINs \x2014 made here, stored nowhere",
	[S_MODE_CHARS]     = L"Characters",
	[S_MODE_WORDS]     = L"Passphrase",
	[S_MODE_PIN]       = L"PIN",
	[S_TIP_MODE_CHARS] = L"Random characters: the strongest per character (Ctrl+1).",
	[S_TIP_MODE_WORDS] = L"Random words: long but easy to remember and to type (Ctrl+2).",
	[S_TIP_MODE_PIN]   = L"Digits only, for locks, cards and phones (Ctrl+3).",
	[S_PROFILE_FMT]    = L"Profile: %.*ls",
	[S_TIP_PROFILE]    = L"Ready-made settings for common cases: sites that refuse\n"
	                     L"symbols, sites with a short maximum length, Wi-Fi, PIN\x2026",
	[S_PRESET_STANDARD]= L"Standard\t20 characters, basic symbols",
	[S_PRESET_MAX]     = L"Maximum\t32 characters, every symbol",
	[S_PRESET_NOSYM]   = L"No symbols\tfor sites that refuse them",
	[S_PRESET_MINSYM]  = L"Minimal symbols\tonly - _ .  for strict sites",
	[S_PRESET_SHORT]   = L"Short\t12 characters, for a low maximum",
	[S_PRESET_WIFI]    = L"Wi-Fi\teasy to type on a phone or TV",
	[S_PRESET_PHRASE]  = L"Passphrase\t6 words, easy to remember",
	[S_PRESET_PIN]     = L"PIN\t6 digits, no repeats or runs",
	[S_PRESET_CUSTOM]  = L"Custom",
	[S_CARD_LIST]      = L"Passwords",
	[S_LBL_LENGTH]     = L"Length",
	[S_LBL_WORDS]      = L"Words",
	[S_LBL_DIGITS]     = L"Digits",
	[S_LBL_COUNT]      = L"How many",
	[S_LBL_INCLUDE]    = L"Use",
	[S_LBL_SYMBOLS]    = L"Symbols",
	[S_LBL_SEPARATOR]  = L"Separator",
	[S_LBL_EXCLUDE]    = L"Leave out",
	[S_LBL_CUSTOM]     = L"My symbols",
	[S_CHIP_LOWER]     = L"a\x2013z",
	[S_CHIP_UPPER]     = L"A\x2013Z",
	[S_CHIP_DIGITS]    = L"0\x2013" L"9",
	[S_CHIP_SYMBOLS]   = L"#$%",
	[S_TIP_CHIPS]      = L"Which kinds of character to use. Every kind that is on\n"
	                     L"appears at least once in each password.",
	[S_SYM_BASIC]      = L"Basic\t! @ # $ % & * + - = ?",
	[S_SYM_ALL]        = L"All 32\tevery printable symbol",
	[S_SYM_MIN]        = L"Minimal\t- _ .",
	[S_SYM_CUSTOM]     = L"My own…\ttype them in",
	[S_TIP_SYMSET]     = L"Which symbols. Basic ones are accepted almost everywhere;\n"
	                     L"Minimal (- _ .) suits strict sites; My own lets you type\n"
	                     L"exactly the symbols a site allows.",
	[S_SEP_HYPHEN]     = L"Hyphen\t-",
	[S_SEP_SPACE]      = L"Space",
	[S_SEP_DOT]        = L"Dot\t.",
	[S_SEP_UNDERSCORE] = L"Underscore\t_",
	[S_SEP_NONE]       = L"None",
	[S_SEP_DIGIT]      = L"Random digit\t0\x2013" L"9, adds strength",
	[S_TIP_SEPARATOR]  = L"What goes between the words. A random digit adds about\n"
	                     L"3.3 bits for every gap.",
	[S_OPT_AMBIG]      = L"No look-alikes  0 O 1 l I",
	[S_OPT_NOREPEAT]   = L"No repeats in a row",
	[S_OPT_START]      = L"Start with a letter",
	[S_OPT_CAP]        = L"Capital letters",
	[S_OPT_NUMBER]     = L"Add a digit",
	[S_OPT_NOSEQ]      = L"No runs like 123 or 987",
	[S_OPT_CLEAR]      = L"Clear clipboard after",
	[S_TIP_AMBIG]      = L"Leaves out 0, O, 1, l and I (and | ` ' \" among the symbols).\n"
	                     L"They look alike in many fonts, so a password that is\n"
	                     L"read out or typed by hand goes wrong less often.",
	[S_TIP_NOREPEAT]   = L"Never the same character twice in a row (aa, 11).\n"
	                     L"Some sites and devices refuse them.",
	[S_TIP_START]      = L"The first character is always a letter.\n"
	                     L"Some sites and older systems require it.",
	[S_TIP_EXCLUDE]    = L"Characters that must never appear, for example the ones\n"
	                     L"a site refuses. Type them one after the other.",
	[S_TIP_CUSTOM]     = L"Exactly the symbols to use, for example the list a site\n"
	                     L"says it accepts. Letters and digits typed here are ignored.",
	[S_TIP_CAP]        = L"Each Word Starts With A Capital, for sites that\n"
	                     L"require uppercase letters.",
	[S_TIP_NUMBER]     = L"One digit after one of the words, for sites that\n"
	                     L"require a number.",
	[S_TIP_NOSEQ]      = L"No three digits in a row going up or down by one,\n"
	                     L"such as 123 or 987: the first ones people try.",
	[S_TIP_CLEAR]      = L"A copied password is wiped from the clipboard once the\n"
	                     L"time chosen on the right is up (2 minutes unless you\n"
	                     L"change it), or when Forja is closed \x2014 but only if\n"
	                     L"the clipboard still holds exactly what Forja put\n"
	                     L"there. Switched off, copies stay until you copy\n"
	                     L"something else. Copies never go to the clipboard\n"
	                     L"history (Win+V) or to the cloud clipboard.",
	[S_CLR_10S]        = L"10 s",
	[S_CLR_30S]        = L"30 s",
	[S_CLR_1M]         = L"1 min",
	[S_CLR_2M]         = L"2 min\tdefault",
	[S_CLR_5M]         = L"5 min",
	[S_CLR_10M]        = L"10 min",
	[S_CLR_FMT_S]      = L"%d s",
	[S_CLR_FMT_M]      = L"%d min",
	[S_TIP_CLEARTIME]  = L"How long a copied password stays on the clipboard.\n"
	                     L"Choosing a time also switches clearing on.",
	[S_WORDS_NOTE]     = L"Words from the EFF list of 7776 English words: each adds 12.9 bits.",
	[S_PIN_NOTE]       = L"A PIN is only safe where wrong tries are limited (cards, phones, locks).",
	[S_GENERATE]       = L"Generate",
	[S_TIP_GENERATE]   = L"A new set of passwords (F5). Nothing is stored.",
	[S_TIP_COPYALL]    = L"Copy all the passwords, one per line (Ctrl+Shift+C)",
	[S_TIP_SAVE]       = L"Save all the passwords to a text file (Ctrl+S)",
	[S_STR_VWEAK]      = L"Very weak",
	[S_STR_WEAK]       = L"Weak",
	[S_STR_FAIR]       = L"Fair",
	[S_STR_STRONG]     = L"Strong",
	[S_STR_VSTRONG]    = L"Very strong",
	[S_STR_PIN]        = L"PIN: only where tries are limited",
	[S_STRENGTH_BITS]  = L"  \x00B7  %d bits of entropy",
	[S_STRENGTH_DETAIL]= L"%d possible characters per position  \x00B7  guessing it: %ls",
	[S_STRENGTH_DETAIL_W] = L"%d words in the list  \x00B7  guessing it: %ls",
	[S_TIP_STRENGTH]   = L"The entropy counts exactly how many different passwords\n"
	                     L"these settings can produce (every rule included): n bits\n"
	                     L"means 2^n possibilities. The guessing time assumes an\n"
	                     L"attacker with the file trying 100 billion a second.\n"
	                     L"80 bits or more is very strong.",
	[S_TIME_INSTANT]   = L"instantly",
	[S_TIME_MINUTES]   = L"minutes",
	[S_TIME_HOURS]     = L"hours",
	[S_TIME_DAYS]      = L"days to months",
	[S_TIME_YEARS]     = L"years",
	[S_TIME_CENTURIES] = L"centuries",
	[S_TIME_MILLIONS]  = L"millions of years",
	[S_TIME_FOREVER]   = L"longer than the age of the universe",
	[S_CLASS_LOWER]    = L"the lowercase letters",
	[S_CLASS_UPPER]    = L"the uppercase letters",
	[S_CLASS_DIGITS]   = L"the digits",
	[S_CLASS_SYMBOLS]  = L"the symbols",
	[S_ERR_NOCLASS]    = L"Turn on at least one kind of character.",
	[S_ERR_EMPTYCLASS] = L"Nothing is left of %ls: allow some, or turn that kind off.",
	[S_ERR_TOOSHORT]   = L"No password can follow all these rules. Relax one of them.",
	[S_ERR_NODIGITS]   = L"Every digit is left out.",
	[S_ST_READY]       = L"Ready.",
	[S_ST_GENERATED]   = L"%d new passwords.",
	[S_ST_GENERATED_ONE] = L"1 new password.",
	[S_ST_NOTHING]     = L"No passwords: see the message above the list.",
	[S_ST_COPIED]      = L"Password %d copied to the clipboard.",
	[S_ST_COPIED_ALL]  = L"%d passwords copied to the clipboard.",
	[S_ST_CLEAR_IN]    = L"  It will be cleared in %d s.",
	[S_ST_CLEARED]     = L"The clipboard was cleared.",
	[S_ST_SAVED]       = L"Saved: %ls",
	[S_ST_SAVE_FAIL]   = L"Could not write the file.",
	[S_ST_CLIP_FAIL]   = L"Could not open the clipboard. Try again.",
	[S_ST_RNG_FAIL]    = L"The Windows random number generator failed. No password was made.",
	[S_SAVE_FILTER]    = L"Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME]   = L"passwords.txt",
	[S_MENU_FILE]      = L"&File",
	[S_MENU_GENERATE]  = L"&Generate\tF5",
	[S_MENU_COPY]      = L"&Copy selected\tCtrl+C",
	[S_MENU_COPYALL]   = L"Copy &all\tCtrl+Shift+C",
	[S_MENU_SAVE]      = L"&Save as...\tCtrl+S",
	[S_MENU_EXIT]      = L"E&xit",
	[S_MENU_KIND]      = L"&Passwords",
	[S_MENU_MODE_CHARS]= L"Characters\tCtrl+1",
	[S_MENU_MODE_WORDS]= L"Passphrase\tCtrl+2",
	[S_MENU_MODE_PIN]  = L"PIN\tCtrl+3",
	[S_MENU_VIEW]      = L"&View",
	[S_MENU_THEME_AUTO]= L"Theme: follow Windows",
	[S_MENU_THEME_LIGHT] = L"Theme: light",
	[S_MENU_THEME_DARK]= L"Theme: dark",
	[S_MENU_THEMEDLG]  = L"Theme and colour…",
	[S_THEME_TITLE]    = L"Theme and colour",
	[S_THEME_APPEARANCE] = L"Appearance",
	[S_THEME_AUTO]     = L"As Windows",
	[S_THEME_LIGHT]    = L"Light",
	[S_THEME_DARK]     = L"Dark",
	[S_THEME_COLOUR]   = L"Colour of the window and the icon",
	[S_ACCENT_BLUE]    = L"Blue",
	[S_ACCENT_GOLD]    = L"Gold",
	[S_ACCENT_PURPLE]  = L"Purple",
	[S_ACCENT_SKY]     = L"Sky",
	[S_ACCENT_EMBER]   = L"Ember",
	[S_ACCENT_RED]     = L"Red",
	[S_ACCENT_YELLOW]  = L"Yellow",
	[S_ACCENT_GREEN]   = L"Green",
	[S_ACCENT_TEAL]    = L"Teal",
	[S_ACCENT_PINK]    = L"Pink",
	[S_ACCENT_BLACK]   = L"Black",
	[S_ACCENT_WHITE]   = L"White",
	[S_MENU_LANG]      = L"&Language",
	[S_MENU_EN]        = L"English",
	[S_MENU_JP]        = L"\x65E5\x672C\x8A9E",
	[S_MENU_ES]        = L"Espa\x00F1ol",
	[S_MENU_HELP]      = L"&Help",
	[S_MENU_HELPTOPICS]= L"&Help\tF1",
	[S_MENU_ABOUT]     = L"&About",
	[S_MENU_UPDATE]    = L"Check for &updates\x2026",
	[S_MENU_UPDATE_AUTO] = L"Check for updates &daily",
	[S_UPD_UPTODATE]   = L"Forja %ls is up to date.",
	[S_UPD_AVAILABLE]  = L"Forja %ls is available (you have %ls).\nOpen the download page?",
	[S_UPD_PORTABLE]   = L"Forja %ls is available (you have %ls).\n\nThis copy is portable (it was not "
	                     L"installed with the setup), so it cannot update itself. Open the download page?",
	[S_UPD_ERROR]      = L"Could not check for updates.\n%ls",
	[S_UPD_ASK_HEAD]   = L"Forja %ls is available",
	[S_UPD_ASK]        = L"You have %ls. The update downloads and installs by itself: the setup is checked "
	                     L"against the release's SHA-256 checksums, Forja closes, the new version installs "
	                     L"with the same options and Forja opens again.",
	[S_UPD_NOW]        = L"Update now",
	[S_UPD_NOTES]      = L"What's new",
	[S_UPD_LATER]      = L"Later",
	[S_UPD_CHECKING]   = L"Checking for updates\x2026",
	[S_UPD_DOWNLOADING]= L"Downloading Forja %ls\x2026 %ls",
	[S_UPD_NOHASH]     = L"The release has no checksum for the setup.",
	[S_UPD_BADHASH]    = L"The downloaded setup does not match its checksum; it was deleted.",
	[S_UPD_FAILED]     = L"The update could not be installed.\n%ls",
	[S_UPD_UPDATED]    = L"Forja was updated to %ls.",
	[S_UPD_NET]        = L"The connection to GitHub failed (error %lu).",
	[S_UPD_BADREPLY]   = L"GitHub's answer could not be read.",
	[S_HELP_TITLE]     = L"Forja \x2014 Help",
	[S_HELP_BODY]      =
		L"# What it does\n"
		L"Makes strong random passwords, passphrases and PINs \x2014 for a zip, 7z or rar file sent by "
		L"email, a new account, a Wi-Fi network or a lock. Nothing is stored or sent anywhere.\n"
		L"# How to use it\n"
		L"\x2022 Choose the kind at the top: Characters, Passphrase or PIN (Ctrl+1, Ctrl+2, Ctrl+3).\n"
		L"\x2022 Or pick a ready-made profile from the Profile button (or the Passwords menu).\n"
		L"\x2022 Adjust the settings: every change makes a new set at once. Generate (F5) makes another.\n"
		L"\x2022 To copy one password: click the copy icon at the end of its row, double-click the row, "
		L"or select it and press Enter or Ctrl+C. Ctrl+Shift+C copies them all; Ctrl+S saves them to a text file.\n"
		L"# When a site refuses some characters\n"
		L"\x2022 It refuses symbols: choose the profile \"No symbols\", or switch off the #$% chip.\n"
		L"\x2022 It accepts only some symbols: choose Symbols → Minimal (- _ .), or My own and type exactly "
		L"the symbols the site lists.\n"
		L"\x2022 It refuses particular characters: type them in \"Leave out\".\n"
		L"\x2022 It has a maximum length: lower the length, or choose the profile \"Short\" (12). Make up "
		L"for a short password with every kind of character.\n"
		L"\x2022 It wants the first character to be a letter, or no repeated characters: switch on "
		L"\"Start with a letter\" or \"No repeats in a row\".\n"
		L"# Which kind to choose\n"
		L"\x2022 Characters: the strongest for its length. Best when the password is pasted, not typed.\n"
		L"\x2022 Passphrase: random words such as Copper-Showroom-Wrecking7-Salute. Longer, but far easier to "
		L"remember, dictate or type on a phone. Six words give about 80 bits.\n"
		L"\x2022 PIN: digits only. Safe only where wrong tries are limited, as on cards, phones and locks.\n"
		L"# How the passwords are made\n"
		L"Every character or word comes from the Windows cryptographic random number generator "
		L"(BCryptGenRandom). When a password breaks one of the rules \x2014 a kind of character missing, "
		L"a repeated neighbour, a run like 123 \x2014 it is thrown away and drawn again, never patched. So "
		L"every password that follows the rules is exactly as likely as any other, and no position, kind "
		L"or pattern is favoured.\n"
		L"# Strength\n"
		L"The entropy is counted exactly: the number of different passwords the settings can give, every "
		L"rule included (n bits = 2^n possibilities). Below 50 bits is weak, 64 fair, 80 or more very "
		L"strong. The guessing time assumes an attacker who has the file and tries 100 billion passwords a "
		L"second. The defaults (20 characters) give about 122 bits.\n"
		L"# Colours\n"
		L"In the list, digits are blue and symbols amber, so a password is easier to read back or dictate. "
		L"What is copied is plain text. View → Theme and colour… chooses the theme (light, dark or as "
		L"Windows) and one of twelve colours, which also colour the icon; black is the default.\n"
		L"# Clipboard\n"
		L"With \"Clear clipboard after\" on (the default), a copied password is wiped when the time chosen "
		L"next to it is up \x2014 2 minutes, unless you pick 10 s, 30 s, 1, 5 or 10 minutes \x2014 or as soon "
		L"as the program is closed, whichever comes first. It is wiped only if the clipboard still holds "
		L"exactly what Forja put there: whatever you copied afterwards is left alone. Switched off, "
		L"copies stay until you copy something else. Copies are always kept out of the Windows clipboard "
		L"history (Win+V) and the cloud clipboard.\n"
		L"# Updates\n"
		L"Help \x2192 Check for updates\x2026 asks GitHub whether a newer version has been published; with "
		L"\"Check for updates daily\" on (the default) this also happens by itself once a day. Nothing about "
		L"you or your passwords is sent. When there is one, a single question: Update now, What's new or "
		L"Later. Update now downloads the setup, checks it against the release's SHA-256 checksums (a file "
		L"that does not match is deleted and never run), closes Forja, installs in silence with the same "
		L"options and opens Forja again. A portable copy cannot update itself: it offers the download "
		L"page instead.\n"
		L"# Settings\n"
		L"They are kept in %APPDATA%\\Forja\\forja.ini. To use the program portable, from a "
		L"USB stick or a share, put an empty forja.ini next to forja.exe: then that one is used.\n"
		L"# Good practice\n"
		L"Send the password through a different channel from the file: the file by email, the password in "
		L"a separate message, a chat or a call. Once the window is closed, the passwords are gone.",
	[S_BTN_OK]         = L"OK",
	[S_ABOUT_TITLE]    = L"About",
	[S_ABOUT_NAME]     = L"Forja",
	[S_ABOUT_MEANING]  = L"The name: a forja (Spanish for forge) is where metal takes shape under the hammer, on the anvil, as each password here takes shape from pure chance. Until version 2.1 it was called Wyrmhold.",
	[S_ABOUT_VERSION]  = L"Version %ls  \x00B7  %ls",
	[S_ABOUT_BODY]     = L"Makes strong random passwords, passphrases and PINs with the Windows "
	                     L"cryptographic random number generator, uniformly over every password that "
	                     L"follows the chosen rules. Nothing is stored or sent anywhere: the passwords "
	                     L"exist only in this window until you copy or save them.",
	[S_ABOUT_AUTHOR]   = L"Author: %ls",
	[S_ABOUT_CREDITS]  = L"Passphrase words: the EFF large word list, by the Electronic Frontier "
	                     L"Foundation, under CC BY 3.0 US (eff.org/dice). Icon: the anvil and hammers of the trade card "
	                     L"of Elwell and Taylor, smiths of London (c. 1766), British Museum, public domain.",
	[S_ABOUT_LICENSE]  = L"Free software under the GNU General Public License, version 2 (GPL-2.0-only). "
	                     L"It comes with no warranty. \x00A9 2026 Gabriel Castiglia.",
},

/* ------------------------------------------------------------------ Japanese */
{
	[S_WINDOW_TITLE]   = L"Forja",
	[S_APP_TITLE]      = L"Forja",
	[S_APP_SUBTITLE]   = L"強力なパスワード・パスフレーズ・PIN \x2014 ここで作り、どこにも保存しません",
	[S_MODE_CHARS]     = L"文字",
	[S_MODE_WORDS]     = L"パスフレーズ",
	[S_MODE_PIN]       = L"PIN",
	[S_TIP_MODE_CHARS] = L"ランダムな文字: 1文字あたり最も強力です (Ctrl+1)。",
	[S_TIP_MODE_WORDS] = L"ランダムな単語: 長いが覚えやすく入力しやすい (Ctrl+2)。",
	[S_TIP_MODE_PIN]   = L"数字のみ。錠前・カード・スマートフォン用 (Ctrl+3)。",
	[S_PROFILE_FMT]    = L"プロファイル: %.*ls",
	[S_TIP_PROFILE]    = L"よくある用途向けの設定: 記号を受け付けないサイト、\n"
	                     L"最大文字数が短いサイト、Wi-Fi、PIN など。",
	[S_PRESET_STANDARD]= L"標準\t20文字、基本の記号",
	[S_PRESET_MAX]     = L"最強\t32文字、すべての記号",
	[S_PRESET_NOSYM]   = L"記号なし\t記号を受け付けないサイト用",
	[S_PRESET_MINSYM]  = L"最小限の記号\t- _ . のみ、厳しいサイト用",
	[S_PRESET_SHORT]   = L"短め\t12文字、最大文字数が少ない場合",
	[S_PRESET_WIFI]    = L"Wi-Fi\tスマホやテレビで入力しやすい",
	[S_PRESET_PHRASE]  = L"パスフレーズ\t6単語、覚えやすい",
	[S_PRESET_PIN]     = L"PIN\t6桁、繰り返し・連番なし",
	[S_PRESET_CUSTOM]  = L"カスタム",
	[S_CARD_LIST]      = L"パスワード",
	[S_LBL_LENGTH]     = L"文字数",
	[S_LBL_WORDS]      = L"単語数",
	[S_LBL_DIGITS]     = L"桁数",
	[S_LBL_COUNT]      = L"生成数",
	[S_LBL_INCLUDE]    = L"使う文字",
	[S_LBL_SYMBOLS]    = L"記号",
	[S_LBL_SEPARATOR]  = L"区切り",
	[S_LBL_EXCLUDE]    = L"除外",
	[S_LBL_CUSTOM]     = L"使う記号",
	[S_CHIP_LOWER]     = L"a\x2013z",
	[S_CHIP_UPPER]     = L"A\x2013Z",
	[S_CHIP_DIGITS]    = L"0\x2013" L"9",
	[S_CHIP_SYMBOLS]   = L"#$%",
	[S_TIP_CHIPS]      = L"使う文字の種類。オンにした種類は、\n"
	                     L"どのパスワードにも必ず1文字以上入ります。",
	[S_SYM_BASIC]      = L"基本\t! @ # $ % & * + - = ?",
	[S_SYM_ALL]        = L"全32種\t印字可能なすべての記号",
	[S_SYM_MIN]        = L"最小限\t- _ .",
	[S_SYM_CUSTOM]     = L"自分で指定…\t入力する",
	[S_TIP_SYMSET]     = L"使う記号。基本の記号はほぼどこでも使えます。\n"
	                     L"最小限 (- _ .) は厳しいサイト向け。自分で指定では\n"
	                     L"サイトが許可する記号だけを入力できます。",
	[S_SEP_HYPHEN]     = L"ハイフン\t-",
	[S_SEP_SPACE]      = L"スペース",
	[S_SEP_DOT]        = L"ドット\t.",
	[S_SEP_UNDERSCORE] = L"アンダースコア\t_",
	[S_SEP_NONE]       = L"なし",
	[S_SEP_DIGIT]      = L"ランダムな数字\t0\x2013" L"9、強度が上がる",
	[S_TIP_SEPARATOR]  = L"単語の間に入れるもの。ランダムな数字にすると\n"
	                     L"区切りごとに約3.3ビット強くなります。",
	[S_OPT_AMBIG]      = L"紛らわしい文字なし  0 O 1 l I",
	[S_OPT_NOREPEAT]   = L"同じ文字を連続させない",
	[S_OPT_START]      = L"英字で始める",
	[S_OPT_CAP]        = L"大文字にする",
	[S_OPT_NUMBER]     = L"数字を1つ入れる",
	[S_OPT_NOSEQ]      = L"123 や 987 のような連番なし",
	[S_OPT_CLEAR]      = L"クリップボードを自動消去",
	[S_TIP_AMBIG]      = L"0、O、1、l、I(記号では | ` ' \")を使いません。\n"
	                     L"多くのフォントで見分けにくいため、読み上げたり\n"
	                     L"手で入力したりするときの間違いが減ります。",
	[S_TIP_NOREPEAT]   = L"同じ文字を2回続けません (aa、11)。\n"
	                     L"受け付けないサイトや機器があります。",
	[S_TIP_START]      = L"最初の文字を必ず英字にします。\n"
	                     L"これを求めるサイトや古いシステムがあります。",
	[S_TIP_EXCLUDE]    = L"絶対に使わない文字。たとえばサイトが受け付けない文字を\n"
	                     L"続けて入力してください。",
	[S_TIP_CUSTOM]     = L"使う記号そのもの。たとえばサイトが受け付けると書いている\n"
	                     L"記号の一覧です。ここに入力した英字や数字は無視されます。",
	[S_TIP_CAP]        = L"各単語の先頭を大文字にします。\n"
	                     L"大文字を必須とするサイト用です。",
	[S_TIP_NUMBER]     = L"どれか1つの単語の後に数字を1つ入れます。\n"
	                     L"数字を必須とするサイト用です。",
	[S_TIP_NOSEQ]      = L"123 や 987 のように1ずつ増減する3桁の並びを使いません。\n"
	                     L"真っ先に試される番号だからです。",
	[S_TIP_CLEAR]      = L"コピーしたパスワードは、右で選んだ時間(変更しなければ\n"
	                     L"2分)がたつか Forja を閉じたときに、クリップボード\n"
	                     L"から消去されます。ただし、クリップボードの中身が\n"
	                     L"Forja の入れたものとまったく同じ場合だけです。\n"
	                     L"オフにすると、ほかのものをコピーするまで残ります。\n"
	                     L"コピーがクリップボードの履歴 (Win+V) やクラウドに\n"
	                     L"送られることはありません。",
	[S_CLR_10S]        = L"10秒後",
	[S_CLR_30S]        = L"30秒後",
	[S_CLR_1M]         = L"1分後",
	[S_CLR_2M]         = L"2分後\t既定",
	[S_CLR_5M]         = L"5分後",
	[S_CLR_10M]        = L"10分後",
	[S_CLR_FMT_S]      = L"%d秒後",
	[S_CLR_FMT_M]      = L"%d分後",
	[S_TIP_CLEARTIME]  = L"コピーしたパスワードをクリップボードに残す時間。\n"
	                     L"時間を選ぶと自動消去もオンになります。",
	[S_WORDS_NOTE]     = L"EFF の英単語リスト (7776語) から選びます。1語ごとに 12.9 ビット。",
	[S_PIN_NOTE]       = L"PIN が安全なのは、誤入力の回数が制限される場合だけです (カード、スマホ、錠前)。",
	[S_GENERATE]       = L"生成",
	[S_TIP_GENERATE]   = L"新しいパスワードを生成します (F5)。何も保存されません。",
	[S_TIP_COPYALL]    = L"すべてのパスワードを1行に1つずつコピー (Ctrl+Shift+C)",
	[S_TIP_SAVE]       = L"すべてのパスワードをテキストファイルに保存 (Ctrl+S)",
	[S_STR_VWEAK]      = L"非常に弱い",
	[S_STR_WEAK]       = L"弱い",
	[S_STR_FAIR]       = L"普通",
	[S_STR_STRONG]     = L"強い",
	[S_STR_VSTRONG]    = L"非常に強い",
	[S_STR_PIN]        = L"PIN: 試行回数が制限される場合のみ",
	[S_STRENGTH_BITS]  = L"  \x00B7  エントロピー %d ビット",
	[S_STRENGTH_DETAIL]= L"1文字あたり %d 種類  \x00B7  推測にかかる時間: %ls",
	[S_STRENGTH_DETAIL_W] = L"リストの単語数 %d  \x00B7  推測にかかる時間: %ls",
	[S_TIP_STRENGTH]   = L"エントロピーは、この設定で作れるパスワードの数を\n"
	                     L"すべての規則を含めて正確に数えたものです。n ビットは\n"
	                     L"2^n 通りを意味します。推測時間は、ファイルを持つ攻撃者が\n"
	                     L"毎秒1000億回試す場合の目安です。80ビット以上で非常に強力です。",
	[S_TIME_INSTANT]   = L"一瞬",
	[S_TIME_MINUTES]   = L"数分",
	[S_TIME_HOURS]     = L"数時間",
	[S_TIME_DAYS]      = L"数日〜数か月",
	[S_TIME_YEARS]     = L"数年",
	[S_TIME_CENTURIES] = L"数世紀",
	[S_TIME_MILLIONS]  = L"数百万年",
	[S_TIME_FOREVER]   = L"宇宙の年齢より長い",
	[S_CLASS_LOWER]    = L"小文字",
	[S_CLASS_UPPER]    = L"大文字",
	[S_CLASS_DIGITS]   = L"数字",
	[S_CLASS_SYMBOLS]  = L"記号",
	[S_ERR_NOCLASS]    = L"文字の種類を1つ以上オンにしてください。",
	[S_ERR_EMPTYCLASS] = L"%ls が1文字も残っていません。除外を減らすか、その種類をオフにしてください。",
	[S_ERR_TOOSHORT]   = L"すべての規則を満たすパスワードは作れません。どれかを緩めてください。",
	[S_ERR_NODIGITS]   = L"すべての数字が除外されています。",
	[S_ST_READY]       = L"準備完了。",
	[S_ST_GENERATED]   = L"新しいパスワードを %d 個生成しました。",
	[S_ST_GENERATED_ONE] = L"新しいパスワードを 1 個生成しました。",
	[S_ST_NOTHING]     = L"パスワードはありません。一覧の上のメッセージをご覧ください。",
	[S_ST_COPIED]      = L"パスワード %d をクリップボードにコピーしました。",
	[S_ST_COPIED_ALL]  = L"%d 個のパスワードをクリップボードにコピーしました。",
	[S_ST_CLEAR_IN]    = L"  %d 秒後に消去します。",
	[S_ST_CLEARED]     = L"クリップボードを消去しました。",
	[S_ST_SAVED]       = L"保存しました: %ls",
	[S_ST_SAVE_FAIL]   = L"ファイルに書き込めませんでした。",
	[S_ST_CLIP_FAIL]   = L"クリップボードを開けませんでした。もう一度お試しください。",
	[S_ST_RNG_FAIL]    = L"Windows の乱数生成器でエラーが発生しました。パスワードは生成されていません。",
	[S_SAVE_FILTER]    = L"テキスト ファイル (*.txt)\0*.txt\0すべてのファイル (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME]   = L"passwords.txt",
	[S_MENU_FILE]      = L"ファイル(&F)",
	[S_MENU_GENERATE]  = L"生成(&G)\tF5",
	[S_MENU_COPY]      = L"選択をコピー(&C)\tCtrl+C",
	[S_MENU_COPYALL]   = L"すべてコピー(&A)\tCtrl+Shift+C",
	[S_MENU_SAVE]      = L"名前を付けて保存(&S)...\tCtrl+S",
	[S_MENU_EXIT]      = L"終了(&X)",
	[S_MENU_KIND]      = L"パスワード(&P)",
	[S_MENU_MODE_CHARS]= L"文字\tCtrl+1",
	[S_MENU_MODE_WORDS]= L"パスフレーズ\tCtrl+2",
	[S_MENU_MODE_PIN]  = L"PIN\tCtrl+3",
	[S_MENU_VIEW]      = L"表示(&V)",
	[S_MENU_THEME_AUTO]= L"テーマ: Windows に合わせる",
	[S_MENU_THEME_LIGHT] = L"テーマ: ライト",
	[S_MENU_THEME_DARK]= L"テーマ: ダーク",
	[S_MENU_THEMEDLG]  = L"テーマと色…",
	[S_THEME_TITLE]    = L"テーマと色",
	[S_THEME_APPEARANCE] = L"外観",
	[S_THEME_AUTO]     = L"Windows に合わせる",
	[S_THEME_LIGHT]    = L"ライト",
	[S_THEME_DARK]     = L"ダーク",
	[S_THEME_COLOUR]   = L"ウィンドウとアイコンの色",
	[S_ACCENT_BLUE]    = L"青",
	[S_ACCENT_GOLD]    = L"金",
	[S_ACCENT_PURPLE]  = L"紫",
	[S_ACCENT_SKY]     = L"空色",
	[S_ACCENT_EMBER]   = L"オレンジ",
	[S_ACCENT_RED]     = L"赤",
	[S_ACCENT_YELLOW]  = L"黄",
	[S_ACCENT_GREEN]   = L"緑",
	[S_ACCENT_TEAL]    = L"青緑",
	[S_ACCENT_PINK]    = L"ピンク",
	[S_ACCENT_BLACK]   = L"黒",
	[S_ACCENT_WHITE]   = L"白",
	[S_MENU_LANG]      = L"言語(&L)",
	[S_MENU_EN]        = L"English",
	[S_MENU_JP]        = L"\x65E5\x672C\x8A9E",
	[S_MENU_ES]        = L"Espa\x00F1ol",
	[S_MENU_HELP]      = L"ヘルプ(&H)",
	[S_MENU_HELPTOPICS]= L"ヘルプ(&H)\tF1",
	[S_MENU_ABOUT]     = L"バージョン情報(&A)",
	[S_MENU_UPDATE]    = L"更新の確認(&U)\x2026",
	[S_MENU_UPDATE_AUTO] = L"毎日自動で更新を確認(&D)",
	[S_UPD_UPTODATE]   = L"Forja %ls は最新版です。",
	[S_UPD_AVAILABLE]  = L"Forja %ls が公開されています(現在のバージョン: %ls)。\nダウンロードページを開きますか？",
	[S_UPD_PORTABLE]   = L"Forja %ls が公開されています(現在のバージョン: %ls)。\n\nこのコピーはポータブル版"
	                     L"(セットアップでインストールしたものではありません)のため、自動では更新できません。"
	                     L"ダウンロードページを開きますか？",
	[S_UPD_ERROR]      = L"更新を確認できませんでした。\n%ls",
	[S_UPD_ASK_HEAD]   = L"Forja %ls が公開されています",
	[S_UPD_ASK]        = L"現在のバージョンは %ls です。更新は自動で行われます。セットアップをダウンロードして"
	                     L"リリースの SHA-256 チェックサムと照合し、Forja を閉じ、同じ設定のまま新しい"
	                     L"バージョンをインストールしてから、Forja をもう一度開きます。",
	[S_UPD_NOW]        = L"今すぐ更新",
	[S_UPD_NOTES]      = L"更新内容を見る",
	[S_UPD_LATER]      = L"後で",
	[S_UPD_CHECKING]   = L"更新を確認しています\x2026",
	[S_UPD_DOWNLOADING]= L"Forja %ls をダウンロードしています\x2026 %ls",
	[S_UPD_NOHASH]     = L"このリリースにはセットアップのチェックサムがありません。",
	[S_UPD_BADHASH]    = L"ダウンロードしたセットアップがチェックサムと一致しないため、削除しました。",
	[S_UPD_FAILED]     = L"更新をインストールできませんでした。\n%ls",
	[S_UPD_UPDATED]    = L"Forja を %ls に更新しました。",
	[S_UPD_NET]        = L"GitHub に接続できませんでした (エラー %lu)。",
	[S_UPD_BADREPLY]   = L"GitHub からの応答を読み取れませんでした。",
	[S_HELP_TITLE]     = L"Forja \x2014 ヘルプ",
	[S_HELP_BODY]      =
		L"# できること\n"
		L"強力なランダムパスワード、パスフレーズ、PIN を作ります。メールで送る zip・7z・rar ファイル、"
		L"新しいアカウント、Wi-Fi、錠前などに使えます。何も保存されず、どこにも送信されません。\n"
		L"# 使い方\n"
		L"\x2022 上部で種類を選びます: 文字、パスフレーズ、PIN (Ctrl+1、Ctrl+2、Ctrl+3)。\n"
		L"\x2022 または「プロファイル」ボタン(または「パスワード」メニュー)から用意された設定を選びます。\n"
		L"\x2022 設定を変えると、そのたびに新しいパスワードが作られます。「生成」(F5) でさらに作れます。\n"
		L"\x2022 1つをコピーするには、行の右端のコピーアイコンをクリック、行をダブルクリック、または選択して"
		L" Enter か Ctrl+C を押します。Ctrl+Shift+C ですべてコピー、Ctrl+S でテキストファイルに保存します。\n"
		L"# サイトが一部の文字を受け付けない場合\n"
		L"\x2022 記号を受け付けない: プロファイル「記号なし」を選ぶか、#$% をオフにします。\n"
		L"\x2022 一部の記号しか使えない: 記号 → 最小限 (- _ .) を選ぶか、「自分で指定」でサイトが示す"
		L"記号だけを入力します。\n"
		L"\x2022 特定の文字を受け付けない: その文字を「除外」に入力します。\n"
		L"\x2022 最大文字数がある: 文字数を下げるか、プロファイル「短め」(12文字) を選びます。短い場合は"
		L"すべての種類の文字を使って補ってください。\n"
		L"\x2022 先頭を英字に、または同じ文字の連続を禁止している: 「英字で始める」「同じ文字を連続させない」"
		L"をオンにします。\n"
		L"# どの種類を選ぶか\n"
		L"\x2022 文字: 長さあたり最も強力です。入力せず貼り付けて使う場合に最適です。\n"
		L"\x2022 パスフレーズ: Copper-Showroom-Wrecking7-Salute のようなランダムな単語。長くなりますが、"
		L"覚えたり、読み上げたり、スマホで入力したりするのがずっと楽です。6単語で約80ビットです。\n"
		L"\x2022 PIN: 数字のみ。カード、スマホ、錠前のように誤入力の回数が制限される場合にだけ安全です。\n"
		L"# パスワードの作り方\n"
		L"すべての文字や単語は Windows の暗号用乱数生成器 (BCryptGenRandom) から取ります。規則に合わない"
		L"パスワード(必要な種類がない、同じ文字の連続、123 のような連番など)は修正せず、捨ててもう一度"
		L"引き直します。そのため、規則を満たすどのパスワードもまったく同じ確率で出て、特定の位置、種類、"
		L"パターンが偏ることはありません。\n"
		L"# 強度\n"
		L"エントロピーは、設定で作れるパスワードの数をすべての規則を含めて正確に数えたものです"
		L"(n ビット = 2^n 通り)。50ビット未満は弱く、64ビットで普通、80ビット以上で非常に強力です。"
		L"推測時間は、ファイルを入手した攻撃者が毎秒1000億回試す場合の目安です。初期設定(20文字)で"
		L"約122ビットです。\n"
		L"# 色\n"
		L"一覧では数字が青、記号がオレンジで表示されるので、読み返しや読み上げが楽になります。コピーされるのは"
		L"普通のテキストです。「表示」→「テーマと色…」でテーマ(ライト、ダーク、Windows に合わせる)と"
		L"12 色から色を選べます。アイコンの色も変わります(既定は黒)。\n"
		L"# クリップボード\n"
		L"「クリップボードを自動消去」がオン(既定)のとき、コピーしたパスワードは、その横で選んだ時間"
		L"(既定は2分。10秒、30秒、1分、5分、10分も選べます)がたつか、プログラムを閉じたとき、"
		L"どちらか早いほうで消去されます。消去するのは、クリップボードの中身が Forja の入れたものと"
		L"まったく同じ場合だけです。後からコピーしたものには手を触れません。オフにすると、ほかのものを"
		L"コピーするまで残ります。コピーは常に Windows のクリップボード履歴 (Win+V) とクラウドから"
		L"除外されます。\n"
		L"# 更新\n"
		L"「ヘルプ」\x2192「更新の確認\x2026」で、新しいバージョンが公開されているかを GitHub に問い合わせます。"
		L"「毎日自動で更新を確認」がオン(既定)なら、1日に1回自動でも確認します。あなたやパスワードに"
		L"関する情報は何も送られません。新しいバージョンがあれば、「今すぐ更新」「更新内容を見る」"
		L"「後で」の質問が1回だけ表示されます。「今すぐ更新」を選ぶと、セットアップをダウンロードして"
		L"リリースの SHA-256 チェックサムと照合し(一致しないファイルは削除され、実行されることはありません)、"
		L"Forja を閉じ、同じ設定のまま自動でインストールして、Forja をもう一度開きます。ポータブル版は"
		L"自動では更新できないため、代わりにダウンロードページを開くことを提案します。\n"
		L"# 設定\n"
		L"設定は %APPDATA%\\Forja\\forja.ini に保存されます。USB メモリーや共有フォルダーから"
		L"ポータブルで使うには、forja.exe と同じフォルダーに空の forja.ini を置いてください。\n"
		L"# 安全な使い方\n"
		L"パスワードはファイルとは別の方法で伝えてください。ファイルはメールで、パスワードは別のメール、"
		L"チャット、電話で。ウィンドウを閉じると、パスワードは残りません。",
	[S_BTN_OK]         = L"OK",
	[S_ABOUT_TITLE]    = L"バージョン情報",
	[S_ABOUT_NAME]     = L"Forja",
	[S_ABOUT_MEANING]  = L"名前の由来: Forja はスペイン語で「鍛冶場」。金床の上で槌を打って金属を形づくるように、ここでは一つひとつのパスワードが純粋な偶然から生まれます。バージョン 2.1 までは Wyrmhold という名前でした。",
	[S_ABOUT_VERSION]  = L"バージョン %ls  \x00B7  %ls",
	[S_ABOUT_BODY]     = L"Windows の暗号用乱数生成器を使い、選んだ規則を満たすすべてのパスワードの中から"
	                     L"均等に、強力なパスワード、パスフレーズ、PIN を作ります。何も保存されず、どこにも"
	                     L"送信されません。パスワードは、コピーまたは保存するまでこのウィンドウの中にだけ存在します。",
	[S_ABOUT_AUTHOR]   = L"作成者: %ls",
	[S_ABOUT_CREDITS]  = L"パスフレーズの単語: Electronic Frontier Foundation の EFF large word list"
	                     L"(CC BY 3.0 US、eff.org/dice)。アイコン: ロンドンの鍛冶屋 Elwell and Taylor の商用カード"
	                     L"(1766 年ごろ、大英博物館、パブリックドメイン)の金床と槌。",
	[S_ABOUT_LICENSE]  = L"GNU 一般公衆利用許諾書 第2版 (GPL-2.0-only) のもとで配布されるフリーソフトウェアです。"
	                     L"無保証です。\x00A9 2026 Gabriel Castiglia",
},

/* ------------------------------------------------------------------ Spanish */
{
	[S_WINDOW_TITLE]   = L"Forja",
	[S_APP_TITLE]      = L"Forja",
	[S_APP_SUBTITLE]   = L"Contraseñas, frases y PIN fuertes \x2014 se hacen acá y no se guardan en ningún lado",
	[S_MODE_CHARS]     = L"Caracteres",
	[S_MODE_WORDS]     = L"Frase",
	[S_MODE_PIN]       = L"PIN",
	[S_TIP_MODE_CHARS] = L"Caracteres al azar: lo más fuerte por carácter (Ctrl+1).",
	[S_TIP_MODE_WORDS] = L"Palabras al azar: más largas, pero fáciles de recordar y tipear (Ctrl+2).",
	[S_TIP_MODE_PIN]   = L"Solo dígitos, para cerraduras, tarjetas y teléfonos (Ctrl+3).",
	[S_PROFILE_FMT]    = L"Perfil: %.*ls",
	[S_TIP_PROFILE]    = L"Configuraciones listas para casos comunes: sitios que no\n"
	                     L"aceptan símbolos, sitios con largo máximo corto, Wi-Fi, PIN\x2026",
	[S_PRESET_STANDARD]= L"Estándar\t20 caracteres, símbolos básicos",
	[S_PRESET_MAX]     = L"Máxima\t32 caracteres, todos los símbolos",
	[S_PRESET_NOSYM]   = L"Sin símbolos\tpara sitios que no los aceptan",
	[S_PRESET_MINSYM]  = L"Símbolos mínimos\tsolo - _ .  para sitios estrictos",
	[S_PRESET_SHORT]   = L"Corta\t12 caracteres, para un máximo bajo",
	[S_PRESET_WIFI]    = L"Wi-Fi\tfácil de tipear en un teléfono o TV",
	[S_PRESET_PHRASE]  = L"Frase\t6 palabras, fácil de recordar",
	[S_PRESET_PIN]     = L"PIN\t6 dígitos, sin repetidos ni seguidos",
	[S_PRESET_CUSTOM]  = L"Personalizado",
	[S_CARD_LIST]      = L"Contraseñas",
	[S_LBL_LENGTH]     = L"Largo",
	[S_LBL_WORDS]      = L"Palabras",
	[S_LBL_DIGITS]     = L"Dígitos",
	[S_LBL_COUNT]      = L"Cantidad",
	[S_LBL_INCLUDE]    = L"Usar",
	[S_LBL_SYMBOLS]    = L"Símbolos",
	[S_LBL_SEPARATOR]  = L"Separador",
	[S_LBL_EXCLUDE]    = L"Excluir",
	[S_LBL_CUSTOM]     = L"Mis símbolos",
	[S_CHIP_LOWER]     = L"a\x2013z",
	[S_CHIP_UPPER]     = L"A\x2013Z",
	[S_CHIP_DIGITS]    = L"0\x2013" L"9",
	[S_CHIP_SYMBOLS]   = L"#$%",
	[S_TIP_CHIPS]      = L"Qué tipos de carácter usar. Cada tipo encendido aparece\n"
	                     L"al menos una vez en cada contraseña.",
	[S_SYM_BASIC]      = L"Básicos\t! @ # $ % & * + - = ?",
	[S_SYM_ALL]        = L"Los 32\ttodos los símbolos imprimibles",
	[S_SYM_MIN]        = L"Mínimos\t- _ .",
	[S_SYM_CUSTOM]     = L"Los míos…\tescribirlos",
	[S_TIP_SYMSET]     = L"Qué símbolos. Los básicos se aceptan casi en todos lados;\n"
	                     L"los mínimos (- _ .) sirven para sitios estrictos; con «Los\n"
	                     L"míos» escribís exactamente los que permite el sitio.",
	[S_SEP_HYPHEN]     = L"Guion\t-",
	[S_SEP_SPACE]      = L"Espacio",
	[S_SEP_DOT]        = L"Punto\t.",
	[S_SEP_UNDERSCORE] = L"Guion bajo\t_",
	[S_SEP_NONE]       = L"Ninguno",
	[S_SEP_DIGIT]      = L"Dígito al azar\t0\x2013" L"9, suma fuerza",
	[S_TIP_SEPARATOR]  = L"Qué va entre las palabras. Un dígito al azar suma\n"
	                     L"unos 3,3 bits por cada espacio entre palabras.",
	[S_OPT_AMBIG]      = L"Sin parecidos  0 O 1 l I",
	[S_OPT_NOREPEAT]   = L"Sin repetir seguidos",
	[S_OPT_START]      = L"Empezar con letra",
	[S_OPT_CAP]        = L"Mayúsculas",
	[S_OPT_NUMBER]     = L"Agregar un dígito",
	[S_OPT_NOSEQ]      = L"Sin seguidos tipo 123 o 987",
	[S_OPT_CLEAR]      = L"Borrar portapapeles a los",
	[S_TIP_AMBIG]      = L"Deja afuera 0, O, 1, l e I (y | ` ' \" entre los símbolos).\n"
	                     L"Se parecen en muchas fuentes, así que una contraseña\n"
	                     L"dictada o tipeada a mano falla menos.",
	[S_TIP_NOREPEAT]   = L"Nunca el mismo carácter dos veces seguidas (aa, 11).\n"
	                     L"Algunos sitios y equipos no lo aceptan.",
	[S_TIP_START]      = L"El primer carácter es siempre una letra.\n"
	                     L"Algunos sitios y sistemas viejos lo exigen.",
	[S_TIP_EXCLUDE]    = L"Caracteres que nunca deben aparecer, por ejemplo los que\n"
	                     L"un sitio rechaza. Escribilos uno detrás del otro.",
	[S_TIP_CUSTOM]     = L"Exactamente los símbolos a usar, por ejemplo la lista que\n"
	                     L"el sitio dice aceptar. Letras y dígitos se ignoran acá.",
	[S_TIP_CAP]        = L"Cada Palabra Empieza Con Mayúscula, para sitios que\n"
	                     L"exigen mayúsculas.",
	[S_TIP_NUMBER]     = L"Un dígito después de una de las palabras, para sitios\n"
	                     L"que exigen un número.",
	[S_TIP_NOSEQ]      = L"Ningún trío de dígitos que suba o baje de a uno,\n"
	                     L"como 123 o 987: son los primeros que se prueban.",
	[S_TIP_CLEAR]      = L"Una contraseña copiada se borra del portapapeles cuando\n"
	                     L"pasa el tiempo elegido a la derecha (2 minutos si no lo\n"
	                     L"cambiás) o al cerrar Forja, pero solo si el portapapeles\n"
	                     L"sigue teniendo exactamente lo que puso Forja. Apagado,\n"
	                     L"lo copiado queda hasta que copies otra cosa. Las copias\n"
	                     L"nunca van al historial (Win+V) ni al portapapeles en la nube.",
	[S_CLR_10S]        = L"10 s",
	[S_CLR_30S]        = L"30 s",
	[S_CLR_1M]         = L"1 min",
	[S_CLR_2M]         = L"2 min\tpredeterminado",
	[S_CLR_5M]         = L"5 min",
	[S_CLR_10M]        = L"10 min",
	[S_CLR_FMT_S]      = L"%d s",
	[S_CLR_FMT_M]      = L"%d min",
	[S_TIP_CLEARTIME]  = L"Cuánto tiempo queda en el portapapeles una contraseña copiada.\n"
	                     L"Elegir un tiempo también enciende el borrado.",
	[S_WORDS_NOTE]     = L"Palabras de la lista EFF de 7776 palabras en inglés: cada una suma 12,9 bits.",
	[S_PIN_NOTE]       = L"Un PIN solo es seguro donde los intentos fallidos están limitados (tarjetas, teléfonos, cerraduras).",
	[S_GENERATE]       = L"Generar",
	[S_TIP_GENERATE]   = L"Un juego nuevo de contraseñas (F5). No se guarda nada.",
	[S_TIP_COPYALL]    = L"Copiar todas las contraseñas, una por línea (Ctrl+Mayús+C)",
	[S_TIP_SAVE]       = L"Guardar todas las contraseñas en un archivo de texto (Ctrl+S)",
	[S_STR_VWEAK]      = L"Muy débil",
	[S_STR_WEAK]       = L"Débil",
	[S_STR_FAIR]       = L"Aceptable",
	[S_STR_STRONG]     = L"Fuerte",
	[S_STR_VSTRONG]    = L"Muy fuerte",
	[S_STR_PIN]        = L"PIN: solo con intentos limitados",
	[S_STRENGTH_BITS]  = L"  \x00B7  %d bits de entropía",
	[S_STRENGTH_DETAIL]= L"%d caracteres posibles por posición  \x00B7  adivinarla: %ls",
	[S_STRENGTH_DETAIL_W] = L"%d palabras en la lista  \x00B7  adivinarla: %ls",
	[S_TIP_STRENGTH]   = L"La entropía cuenta exactamente cuántas contraseñas distintas\n"
	                     L"pueden salir con esta configuración (con todas las reglas):\n"
	                     L"n bits son 2^n posibilidades. El tiempo supone un atacante\n"
	                     L"con el archivo que prueba 100 mil millones por segundo.\n"
	                     L"80 bits o más es muy fuerte.",
	[S_TIME_INSTANT]   = L"al instante",
	[S_TIME_MINUTES]   = L"minutos",
	[S_TIME_HOURS]     = L"horas",
	[S_TIME_DAYS]      = L"días a meses",
	[S_TIME_YEARS]     = L"años",
	[S_TIME_CENTURIES] = L"siglos",
	[S_TIME_MILLIONS]  = L"millones de años",
	[S_TIME_FOREVER]   = L"más que la edad del universo",
	[S_CLASS_LOWER]    = L"las minúsculas",
	[S_CLASS_UPPER]    = L"las mayúsculas",
	[S_CLASS_DIGITS]   = L"los dígitos",
	[S_CLASS_SYMBOLS]  = L"los símbolos",
	[S_ERR_NOCLASS]    = L"Encendé al menos un tipo de carácter.",
	[S_ERR_EMPTYCLASS] = L"No queda nada de %ls: permití alguno o apagá ese tipo.",
	[S_ERR_TOOSHORT]   = L"Ninguna contraseña puede cumplir todas estas reglas. Aflojá alguna.",
	[S_ERR_NODIGITS]   = L"Están excluidos todos los dígitos.",
	[S_ST_READY]       = L"Listo.",
	[S_ST_GENERATED]   = L"%d contraseñas nuevas.",
	[S_ST_GENERATED_ONE] = L"1 contraseña nueva.",
	[S_ST_NOTHING]     = L"No hay contraseñas: mirá el mensaje arriba de la lista.",
	[S_ST_COPIED]      = L"Contraseña %d copiada al portapapeles.",
	[S_ST_COPIED_ALL]  = L"%d contraseñas copiadas al portapapeles.",
	[S_ST_CLEAR_IN]    = L"  Se borra en %d s.",
	[S_ST_CLEARED]     = L"Se borró el portapapeles.",
	[S_ST_SAVED]       = L"Guardado: %ls",
	[S_ST_SAVE_FAIL]   = L"No se pudo escribir el archivo.",
	[S_ST_CLIP_FAIL]   = L"No se pudo abrir el portapapeles. Probá de nuevo.",
	[S_ST_RNG_FAIL]    = L"Falló el generador de números aleatorios de Windows. No se generó ninguna contraseña.",
	[S_SAVE_FILTER]    = L"Archivos de texto (*.txt)\0*.txt\0Todos los archivos (*.*)\0*.*\0\0",
	[S_SAVE_DEFNAME]   = L"contraseñas.txt",
	[S_MENU_FILE]      = L"&Archivo",
	[S_MENU_GENERATE]  = L"&Generar\tF5",
	[S_MENU_COPY]      = L"&Copiar la seleccionada\tCtrl+C",
	[S_MENU_COPYALL]   = L"Copiar &todas\tCtrl+Mayús+C",
	[S_MENU_SAVE]      = L"&Guardar como...\tCtrl+S",
	[S_MENU_EXIT]      = L"&Salir",
	[S_MENU_KIND]      = L"&Contraseñas",
	[S_MENU_MODE_CHARS]= L"Caracteres\tCtrl+1",
	[S_MENU_MODE_WORDS]= L"Frase\tCtrl+2",
	[S_MENU_MODE_PIN]  = L"PIN\tCtrl+3",
	[S_MENU_VIEW]      = L"&Ver",
	[S_MENU_THEME_AUTO]= L"Tema: como Windows",
	[S_MENU_THEME_LIGHT] = L"Tema: claro",
	[S_MENU_THEME_DARK]= L"Tema: oscuro",
	[S_MENU_THEMEDLG]  = L"Tema y color…",
	[S_THEME_TITLE]    = L"Tema y color",
	[S_THEME_APPEARANCE] = L"Apariencia",
	[S_THEME_AUTO]     = L"Como Windows",
	[S_THEME_LIGHT]    = L"Claro",
	[S_THEME_DARK]     = L"Oscuro",
	[S_THEME_COLOUR]   = L"Color de la ventana y del ícono",
	[S_ACCENT_BLUE]    = L"Azul",
	[S_ACCENT_GOLD]    = L"Dorado",
	[S_ACCENT_PURPLE]  = L"Morado",
	[S_ACCENT_SKY]     = L"Celeste",
	[S_ACCENT_EMBER]   = L"Naranja",
	[S_ACCENT_RED]     = L"Rojo",
	[S_ACCENT_YELLOW]  = L"Amarillo",
	[S_ACCENT_GREEN]   = L"Verde",
	[S_ACCENT_TEAL]    = L"Verde azulado",
	[S_ACCENT_PINK]    = L"Rosa",
	[S_ACCENT_BLACK]   = L"Negro",
	[S_ACCENT_WHITE]   = L"Blanco",
	[S_MENU_LANG]      = L"&Idioma",
	[S_MENU_EN]        = L"English",
	[S_MENU_JP]        = L"\x65E5\x672C\x8A9E",
	[S_MENU_ES]        = L"Espa\x00F1ol",
	[S_MENU_HELP]      = L"Ay&uda",
	[S_MENU_HELPTOPICS]= L"&Ayuda\tF1",
	[S_MENU_ABOUT]     = L"Acerca &de",
	[S_MENU_UPDATE]    = L"Buscar act&ualizaciones\x2026",
	[S_MENU_UPDATE_AUTO] = L"Buscar actualizaciones &todos los días",
	[S_UPD_UPTODATE]   = L"Forja %ls está actualizado.",
	[S_UPD_AVAILABLE]  = L"Está disponible Forja %ls (tenés la %ls).\n¿Abrir la página de descarga?",
	[S_UPD_PORTABLE]   = L"Está disponible Forja %ls (tenés la %ls).\n\nEsta copia es portable (no se "
	                     L"instaló con el instalador), así que no puede actualizarse sola. ¿Abrir la página de descarga?",
	[S_UPD_ERROR]      = L"No se pudo buscar actualizaciones.\n%ls",
	[S_UPD_ASK_HEAD]   = L"Está disponible Forja %ls",
	[S_UPD_ASK]        = L"Tenés la %ls. La actualización se descarga e instala sola: el instalador se verifica "
	                     L"contra las sumas SHA-256 de la versión publicada, Forja se cierra, la versión nueva "
	                     L"se instala con las mismas opciones y Forja vuelve a abrirse.",
	[S_UPD_NOW]        = L"Actualizar ahora",
	[S_UPD_NOTES]      = L"Ver novedades",
	[S_UPD_LATER]      = L"Más tarde",
	[S_UPD_CHECKING]   = L"Buscando actualizaciones\x2026",
	[S_UPD_DOWNLOADING]= L"Descargando Forja %ls\x2026 %ls",
	[S_UPD_NOHASH]     = L"La versión publicada no trae la suma de verificación del instalador.",
	[S_UPD_BADHASH]    = L"El instalador descargado no coincide con su suma de verificación; se borró.",
	[S_UPD_FAILED]     = L"No se pudo instalar la actualización.\n%ls",
	[S_UPD_UPDATED]    = L"Forja se actualizó a la %ls.",
	[S_UPD_NET]        = L"Falló la conexión con GitHub (error %lu).",
	[S_UPD_BADREPLY]   = L"No se pudo leer la respuesta de GitHub.",
	[S_HELP_TITLE]     = L"Forja \x2014 Ayuda",
	[S_HELP_BODY]      =
		L"# Qué hace\n"
		L"Genera contraseñas, frases de contraseña y PIN fuertes y al azar: para un zip, 7z o rar que se "
		L"manda por email, una cuenta nueva, una red Wi-Fi o una cerradura. No guarda ni envía nada.\n"
		L"# Cómo se usa\n"
		L"\x2022 Elegí el tipo arriba: Caracteres, Frase o PIN (Ctrl+1, Ctrl+2, Ctrl+3).\n"
		L"\x2022 O elegí un perfil listo con el botón Perfil (o el menú Contraseñas).\n"
		L"\x2022 Ajustá las opciones: cada cambio genera un juego nuevo al instante. Generar (F5) hace otro.\n"
		L"\x2022 Para copiar una: clic en el ícono de copiar al final de su fila, doble clic en la fila, o "
		L"seleccionala y apretá Enter o Ctrl+C. Ctrl+Mayús+C copia todas; Ctrl+S las guarda en un archivo de texto.\n"
		L"# Cuando un sitio no acepta algunos caracteres\n"
		L"\x2022 No acepta símbolos: elegí el perfil «Sin símbolos», o apagá el botón #$%.\n"
		L"\x2022 Acepta solo algunos símbolos: elegí Símbolos → Mínimos (- _ .), o «Los míos» y escribí "
		L"exactamente los que lista el sitio.\n"
		L"\x2022 Rechaza caracteres puntuales: escribilos en «Excluir».\n"
		L"\x2022 Tiene un largo máximo: bajá el largo, o elegí el perfil «Corta» (12). Compensá una contraseña "
		L"corta usando todos los tipos de carácter.\n"
		L"\x2022 Exige empezar con letra, o no repetir caracteres seguidos: encendé «Empezar con letra» o "
		L"«Sin repetir seguidos».\n"
		L"# Qué tipo elegir\n"
		L"\x2022 Caracteres: lo más fuerte para su largo. Ideal cuando la contraseña se pega, no se tipea.\n"
		L"\x2022 Frase: palabras al azar como Copper-Showroom-Wrecking7-Salute. Más larga, pero mucho más fácil de "
		L"recordar, dictar o tipear en un teléfono. Seis palabras dan unos 80 bits.\n"
		L"\x2022 PIN: solo dígitos. Seguro solo donde los intentos fallidos están limitados, como en tarjetas, "
		L"teléfonos y cerraduras.\n"
		L"# Cómo se hacen las contraseñas\n"
		L"Cada carácter o palabra sale del generador criptográfico de números aleatorios de Windows "
		L"(BCryptGenRandom). Cuando una contraseña no cumple alguna regla \x2014 falta un tipo de carácter, un "
		L"carácter repetido seguido, una seguidilla como 123 \x2014 se descarta y se sortea de nuevo, nunca se "
		L"parcha. Así, cualquier contraseña que cumpla las reglas es exactamente igual de probable que otra, "
		L"y ninguna posición, tipo o patrón sale favorecido.\n"
		L"# Fuerza\n"
		L"La entropía se cuenta exactamente: cuántas contraseñas distintas pueden salir con la configuración, "
		L"con todas las reglas (n bits = 2^n posibilidades). Menos de 50 bits es débil, 64 aceptable, 80 o más "
		L"muy fuerte. El tiempo para adivinarla supone un atacante que tiene el archivo y prueba 100 mil "
		L"millones de contraseñas por segundo. Con lo predeterminado (20 caracteres) son unos 122 bits.\n"
		L"# Colores\n"
		L"En la lista, los dígitos van en azul y los símbolos en ámbar, para releer o dictar más fácil. Lo que "
		L"se copia es texto común. En Ver → Tema y color… elegís el tema (claro, oscuro o como Windows) y "
		L"uno de doce colores, que también pinta el ícono; el negro es el de fábrica.\n"
		L"# Portapapeles\n"
		L"Con «Borrar portapapeles a los» encendido (viene así), una contraseña copiada se borra cuando pasa el "
		L"tiempo elegido al lado \x2014 2 minutos, salvo que elijas 10 s, 30 s, 1, 5 o 10 minutos \x2014 o apenas "
		L"se cierra el programa, lo que ocurra primero. Se borra solo si el portapapeles sigue teniendo "
		L"exactamente lo que puso Forja: lo que hayas copiado después no se toca. Apagado, lo copiado queda "
		L"hasta que copies otra cosa. Las copias nunca van al historial del portapapeles de Windows (Win+V) ni "
		L"a la nube.\n"
		L"# Actualizaciones\n"
		L"Ayuda \x2192 Buscar actualizaciones\x2026 le pregunta a GitHub si se publicó una versión nueva; con "
		L"«Buscar actualizaciones todos los días» encendido (viene así) eso también se hace solo una vez por "
		L"día. No se envía nada sobre vos ni sobre tus contraseñas. Si hay una, una sola pregunta: Actualizar "
		L"ahora, Ver novedades o Más tarde. Actualizar ahora descarga el instalador, lo verifica contra las "
		L"sumas SHA-256 de la versión publicada (un archivo que no coincide se borra y nunca se ejecuta), "
		L"cierra Forja, instala en silencio con las mismas opciones y vuelve a abrir Forja. Una copia "
		L"portable no puede actualizarse sola: en su lugar ofrece abrir la página de descarga.\n"
		L"# Configuración\n"
		L"Se guarda en %APPDATA%\\Forja\\forja.ini. Para usarlo portable, desde un pendrive o "
		L"una carpeta compartida, poné un forja.ini vacío junto a forja.exe: entonces se usa ese.\n"
		L"# Buenas prácticas\n"
		L"Mandá la contraseña por un canal distinto del archivo: el archivo por email y la contraseña en otro "
		L"mensaje, un chat o una llamada. Al cerrar la ventana, las contraseñas desaparecen.",
	[S_BTN_OK]         = L"Aceptar",
	[S_ABOUT_TITLE]    = L"Acerca de",
	[S_ABOUT_NAME]     = L"Forja",
	[S_ABOUT_MEANING]  = L"El nombre: en la forja el metal toma forma a golpes de martillo sobre el yunque, como acá cada contraseña sale del puro azar. Hasta la versión 2.1 se llamaba Wyrmhold.",
	[S_ABOUT_VERSION]  = L"Versión %ls  \x00B7  %ls",
	[S_ABOUT_BODY]     = L"Genera contraseñas, frases y PIN fuertes con el generador criptográfico de números "
	                     L"aleatorios de Windows, de manera uniforme entre todas las contraseñas que cumplen las "
	                     L"reglas elegidas. No guarda ni envía nada: las contraseñas existen solo en esta ventana "
	                     L"hasta que las copiás o guardás.",
	[S_ABOUT_AUTHOR]   = L"Autor: %ls",
	[S_ABOUT_CREDITS]  = L"Palabras de las frases: la lista grande de EFF, de la Electronic Frontier Foundation, "
	                     L"bajo CC BY 3.0 US (eff.org/dice). Ícono: el yunque y los martillos de la tarjeta de Elwell and "
	                     L"Taylor, herreros de Londres (c. 1766), British Museum, de dominio público.",
	[S_ABOUT_LICENSE]  = L"Software libre bajo la Licencia Pública General de GNU, versión 2 (GPL-2.0-only). "
	                     L"Se ofrece sin garantía. \x00A9 2026 Gabriel Castiglia.",
},
};

#endif
