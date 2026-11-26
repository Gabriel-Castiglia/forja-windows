/**
  settings.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The settings file: where forja.ini lives (beside the executable for a portable copy,
  otherwise %APPDATA%\Forja), reading and writing its values, and the whole set of
  settings the window remembers.
**/

#include "forja.h"

/* ------------------------------------------------------------------ settings */

/* forja.ini beside the executable when one is there (portable use), otherwise
 * %APPDATA%\Forja\forja.ini -- the program folder of an installed copy cannot be
 * written. Writing may still fail; then the program simply starts with the defaults. */

void iniPath(void)
{
	wchar_t* slash;
	wchar_t dir[MAX_PATH];
	DWORD n;

	wchar_t old[MAX_PATH];

	GetModuleFileNameW(NULL, gIni, MAX_PATH);
	slash = wcsrchr(gIni, L'\\');
	if (slash && (size_t)(slash - gIni) + 14 < MAX_PATH)
	{
		/* a portable copy that still has the settings of Wyrmhold (this program's name up to 2.1) */
		wcscpy(slash + 1, L"wyrm" L"hold.ini");
		lstrcpynW(old, gIni, MAX_PATH);
		wcscpy(slash + 1, L"forja.ini");
		if (GetFileAttributesW(gIni) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(old) != INVALID_FILE_ATTRIBUTES)
			CopyFileW(old, gIni, TRUE);
	}
	if (GetFileAttributesW(gIni) != INVALID_FILE_ATTRIBUTES) return;

	n = GetEnvironmentVariableW(L"APPDATA", dir, MAX_PATH);
	if (n == 0 || n + 24 >= MAX_PATH) return;           /* no profile: stay beside the exe */
	_snwprintf(old, MAX_PATH, L"%ls\\Wyrm" L"hold\\wyrm" L"hold.ini", dir);
	old[MAX_PATH - 1] = 0;
	wcscat(dir, L"\\Forja");
	CreateDirectoryW(dir, NULL);
	_snwprintf(gIni, MAX_PATH, L"%ls\\forja.ini", dir);
	gIni[MAX_PATH - 1] = 0;
	/* the first time: the settings of Wyrmhold, this program's name up to 2.1, come over */
	if (GetFileAttributesW(gIni) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(old) != INVALID_FILE_ATTRIBUTES)
		CopyFileW(old, gIni, TRUE);
}

int iniInt(const wchar_t* key, int def)
{
	wchar_t buf[32];

	GetPrivateProfileStringW(INI_SECTION, key, L"", buf, 32, gIni);
	return buf[0] ? _wtoi(buf) : def;
}

void iniSetInt(const wchar_t* key, int v)
{
	wchar_t buf[32];

	if (gNoSave) return;
	_snwprintf(buf, 32, L"%d", v);
	buf[31] = 0;
	WritePrivateProfileStringW(INI_SECTION, key, buf, gIni);
}

void iniStr(const wchar_t* key, wchar_t* out, int cap)
{
	GetPrivateProfileStringW(INI_SECTION, key, L"", out, (DWORD)cap, gIni);
}

/* Character sets are stored between quotes: the .ini reader trims spaces and strips one
 * pair of outer quotes, so the set comes back exactly, spaces and quotes included. */
static void iniSetChars(const wchar_t* key, const char* v)
{
	wchar_t w[PW_SET_MAX + 4];

	if (gNoSave) return;
	_snwprintf(w, PW_SET_MAX + 4, L"\"%hs\"", v);
	w[PW_SET_MAX + 3] = 0;
	WritePrivateProfileStringW(INI_SECTION, key, w, gIni);
}

static void iniChars(const wchar_t* key, char* out)
{
	wchar_t w[PW_SET_MAX + 4];
	int i, n = 0;

	iniStr(key, w, PW_SET_MAX + 4);
	for (i = 0; w[i] && n < PW_SET_MAX - 1; i++)
		if (w[i] > 32 && w[i] < 127) out[n++] = (char)w[i];
	out[n] = 0;
}

void iniSetStr(const wchar_t* key, const wchar_t* v)
{
	if (!gNoSave) WritePrivateProfileStringW(INI_SECTION, key, v, gIni);
}


void saveSettings(void)
{
	WINDOWPLACEMENT wp;

	if (gNoSave) return;

	iniSetInt(L"Mode", gOpt.mode);
	iniSetInt(L"Length", gOpt.length);
	iniSetInt(L"Count", gCount);
	iniSetInt(L"Lower", gOpt.lower);
	iniSetInt(L"Upper", gOpt.upper);
	iniSetInt(L"Digits", gOpt.digits);
	iniSetInt(L"Symbols", gOpt.symbols);
	iniSetInt(L"SymbolSet", gOpt.symbolSet);
	iniSetChars(L"CustomSymbols", gOpt.customSymbols);
	iniSetInt(L"AvoidAmbiguous", gOpt.avoidAmbiguous);
	iniSetInt(L"NoRepeat", gCharNoRepeat);
	iniSetInt(L"StartLetter", gOpt.startLetter);
	iniSetChars(L"Exclude", gCharExclude);
	iniSetInt(L"Words", gOpt.words);
	iniSetInt(L"Separator", gOpt.separator);
	iniSetInt(L"Capitalize", gOpt.capitalize);
	iniSetInt(L"AddNumber", gOpt.addNumber);
	iniSetInt(L"PinLength", gOpt.pinLength);
	iniSetInt(L"PinNoRepeat", gPinNoRepeat);
	iniSetInt(L"PinNoSequence", gOpt.noSequence);
	iniSetChars(L"PinExclude", gPinExclude);
	if (!gClearFromArg)                 /* /clearafter is for this session only */
	{
		iniSetInt(L"ClearClipboard", gClearClip);
		iniSetInt(L"ClearSeconds", gClearSecs);
	}

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(hMain, &wp)) return;
	iniSetInt(L"WndX", wp.rcNormalPosition.left);
	iniSetInt(L"WndY", wp.rcNormalPosition.top);
	iniSetInt(L"WndW", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	iniSetInt(L"WndH", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
	iniSetInt(L"WndMax", wp.showCmd == SW_SHOWMAXIMIZED);
}

void loadSettings(void)
{
	pw_defaults(&gOpt);
	gOpt.mode           = clampInt(iniInt(L"Mode", PW_CHARS), PW_CHARS, PW_PIN);
	gOpt.length         = clampInt(iniInt(L"Length", 20), PW_MIN_LEN, PW_MAX_LEN);
	gCount              = clampInt(iniInt(L"Count", 5), 1, MAX_COUNT);
	gOpt.lower          = iniInt(L"Lower", 1) != 0;
	gOpt.upper          = iniInt(L"Upper", 1) != 0;
	gOpt.digits         = iniInt(L"Digits", 1) != 0;
	gOpt.symbols        = iniInt(L"Symbols", 1) != 0;
	gOpt.symbolSet      = clampInt(iniInt(L"SymbolSet", PW_SYM_BASIC), PW_SYM_BASIC, PW_SYM_CUSTOM);
	iniChars(L"CustomSymbols", gOpt.customSymbols);
	gOpt.avoidAmbiguous = iniInt(L"AvoidAmbiguous", 1) != 0;
	gCharNoRepeat       = iniInt(L"NoRepeat", 0) != 0;
	gOpt.startLetter    = iniInt(L"StartLetter", 0) != 0;
	iniChars(L"Exclude", gCharExclude);
	gOpt.words          = clampInt(iniInt(L"Words", 6), PW_MIN_WORDS, PW_MAX_WORDS);
	gOpt.separator      = clampInt(iniInt(L"Separator", PW_SEP_HYPHEN), 0, PW_SEP_COUNT - 1);
	gOpt.capitalize     = iniInt(L"Capitalize", 1) != 0;
	gOpt.addNumber      = iniInt(L"AddNumber", 1) != 0;
	gOpt.pinLength      = clampInt(iniInt(L"PinLength", 6), PW_MIN_LEN, PW_MAX_LEN);
	gPinNoRepeat        = iniInt(L"PinNoRepeat", 1) != 0;
	gOpt.noSequence     = iniInt(L"PinNoSequence", 1) != 0;
	iniChars(L"PinExclude", gPinExclude);
	gClearClip          = iniInt(L"ClearClipboard", 1) != 0;
	gClearSecs          = clampInt(iniInt(L"ClearSeconds", CLIP_DEFAULT), CLIP_MIN, CLIP_MAX);
	gAutoUpdate         = iniInt(L"CheckUpdates", 1) != 0;
	gTheme              = clampInt(iniInt(L"Theme", 0), 0, 2);
	gAccent             = clampInt(iniInt(L"Accent", 0), 0, ACCENT_COUNT - 1);
	gLang               = clampInt(iniInt(L"Language", LANG_EN), 0, LANG_COUNT - 1);
	iniStr(L"LastFolder", gLastFolder, MAX_PATH);
}
