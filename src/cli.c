/**
  cli.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The command line: passwords or their entropy to a file, screenshots, and the parsing of
  the options.
**/

#include "forja.h"

/* ------------------------------------------------------------------ command line */

int writeAuto(const wchar_t* path, const PwOptions* o, int count)
{
	FILE* f = path ? _wfopen(path, L"wb") : NULL;
	char pw[PW_OUT_MAX];
	int i;

	if (!f) return 2;
	if (pw_check(o, NULL) != PW_OK) { fclose(f); return 5; }
	for (i = 0; i < count; i++)
	{
		int why;
		if (!pw_make_ex(o, pw, &why))
		{
			/* what was already written goes too: no half list of passwords is left behind */
			SecureZeroMemory(pw, sizeof(pw));
			fclose(f);
			f = _wfopen(path, L"wb");
			if (f) fclose(f);
			return why == PW_FAIL_RNG ? 3 : 6;
		}
		fputs(pw, f);
		fputs("\r\n", f);
	}
	SecureZeroMemory(pw, sizeof(pw));
	return fclose(f) == 0 ? 0 : 2;
}

int writeEntropy(const wchar_t* path, const PwOptions* o)
{
	FILE* f = path ? _wfopen(path, L"wb") : NULL;

	if (!f) return 2;
	fprintf(f, "%.6f %d %d\r\n", pw_entropy(o), pw_pool(o), pw_check(o, NULL));
	return fclose(f) == 0 ? 0 : 2;
}

/* Draws the window (or the Help / About box in front of it) into a 32-bit BMP. */
int saveShot(HWND w, const wchar_t* path)
{
	RECT r;
	int cx, cy, ok = 0;
	HDC screen, mem;
	HBITMAP bmp;
	HGDIOBJ old;
	BITMAPINFO bi;
	void* bits = NULL;

	GetWindowRect(w, &r);
	cx = r.right - r.left;
	cy = r.bottom - r.top;

	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = cx;
	bi.bmiHeader.biHeight = cy;                 /* bottom-up, which is what a BMP file holds */
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;

	screen = GetDC(NULL);
	mem = CreateCompatibleDC(screen);
	bmp = CreateDIBSection(screen, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	old = SelectObject(mem, bmp);

	if (PrintWindow(w, mem, PW_RENDERFULLCONTENT))
	{
		FILE* f = path ? _wfopen(path, L"wb") : NULL;
		if (f)
		{
			BITMAPFILEHEADER fh;
			DWORD size = (DWORD)cx * (DWORD)cy * 4;
			ZeroMemory(&fh, sizeof(fh));
			fh.bfType = 0x4D42;
			fh.bfOffBits = sizeof(fh) + sizeof(bi.bmiHeader);
			fh.bfSize = fh.bfOffBits + size;
			GdiFlush();
			ok = fwrite(&fh, sizeof(fh), 1, f) == 1 &&
			     fwrite(&bi.bmiHeader, sizeof(bi.bmiHeader), 1, f) == 1 &&
			     fwrite(bits, size, 1, f) == 1;
			ok = (fclose(f) == 0) && ok;
		}
	}

	SelectObject(mem, old);
	DeleteObject(bmp);
	DeleteDC(mem);
	ReleaseDC(NULL, screen);
	return ok ? 0 : 4;
}

void pump(DWORD ms)
{
	DWORD until = GetTickCount() + ms;
	MSG m;

	while ((LONG)(until - GetTickCount()) > 0)
	{
		while (PeekMessageW(&m, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessageW(&m); }
		Sleep(10);
	}
}

/* The text after "/key:" in the wide command line, or NULL: file names keep every character,
 * whatever the ANSI code page (the narrow __argv would turn the rest into question marks). */
const wchar_t* widePathArg(const wchar_t* key)
{
	static LPWSTR* argv;
	static int argc;
	size_t n = wcslen(key);
	int i;

	if (!argv) argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	for (i = 1; argv && i < argc; i++)
		if ((argv[i][0] == L'/' || argv[i][0] == L'-') && !_wcsnicmp(argv[i] + 1, key, n)) return argv[i] + 1 + n;
	return NULL;
}

/* Command-line options onto a set of options. Returns 1 if any password option was given. */
int parseOptions(PwOptions* o, int* noRepeat, char* exclude, int* count, int* preset)
{
	int i, any = 0, len = -1;

	for (i = 1; i < __argc; i++)
	{
		const char* a = __argv[i];

		if (a[0] != '/' && a[0] != '-') continue;
		a++;
		any = 1;
		if      (!_strnicmp(a, "mode:", 5))     o->mode = !_stricmp(a + 5, "words") ? PW_WORDS : !_stricmp(a + 5, "pin") ? PW_PIN : PW_CHARS;
		else if (!_strnicmp(a, "len:", 4))      len = atoi(a + 4);
		else if (!_strnicmp(a, "words:", 6))    o->words = atoi(a + 6);
		else if (!_strnicmp(a, "count:", 6))    *count = atoi(a + 6);
		else if (!_strnicmp(a, "classes:", 8))
		{
			o->lower = strchr(a + 8, 'l') != NULL; o->upper = strchr(a + 8, 'u') != NULL;
			o->digits = strchr(a + 8, 'd') != NULL; o->symbols = strchr(a + 8, 's') != NULL;
		}
		else if (!_strnicmp(a, "symbols:custom:", 15)) { o->symbolSet = PW_SYM_CUSTOM; lstrcpynA(o->customSymbols, a + 15, PW_SET_MAX); }
		else if (!_stricmp(a, "symbols:all"))   o->symbolSet = PW_SYM_ALL;
		else if (!_stricmp(a, "symbols:min"))   o->symbolSet = PW_SYM_MIN;
		else if (!_stricmp(a, "symbols:basic")) o->symbolSet = PW_SYM_BASIC;
		else if (!_strnicmp(a, "exclude:", 8))  lstrcpynA(exclude, a + 8, PW_SET_MAX);
		else if (!_stricmp(a, "ambiguous"))     o->avoidAmbiguous = 0;
		else if (!_stricmp(a, "norepeat"))      *noRepeat = 1;
		else if (!_stricmp(a, "startletter"))   o->startLetter = 1;
		else if (!_strnicmp(a, "sep:", 4))
		{
			static const char* names[PW_SEP_COUNT] = { "hyphen", "space", "dot", "underscore", "none", "digit" };
			int k;
			for (k = 0; k < PW_SEP_COUNT; k++) if (!_stricmp(a + 4, names[k])) o->separator = k;
		}
		else if (!_stricmp(a, "nocap"))         o->capitalize = 0;
		else if (!_stricmp(a, "nonumber"))      o->addNumber = 0;
		else if (!_stricmp(a, "allowruns"))     o->noSequence = 0;
		else if (!_strnicmp(a, "preset:", 7))   *preset = atoi(a + 7);
	}
	if (len > 0) { if (o->mode == PW_PIN) o->pinLength = len; else o->length = len; }
	return any;
}

/* The value of /clearafter: "90" or "90s" seconds, "2m" minutes, "0" or "off" never.
 * Returns the seconds (0 = never), or -1 when it is not understood. */
int parseDuration(const char* s)
{
	long v = 0;
	int digits = 0;

	if (!_stricmp(s, "off")) return 0;
	while (*s >= '0' && *s <= '9')
	{
		v = v * 10 + (*s++ - '0');
		if (v > 1000000) return -1;
		digits++;
	}
	if (!digits) return -1;
	if (*s == 'm' || *s == 'M') { v *= 60; s++; }
	else if (*s == 's' || *s == 'S') s++;
	return *s ? -1 : (int)v;
}
