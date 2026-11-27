/**
  selftest.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  forja.exe /selftest:<log> -- the clipboard wiping, the settings, saving and the update.
**/

#include "forja.h"

/* ------------------------------------------------------------------ self-test */

/* forja.exe /selftest:<log> -- the clipboard wiping on the real clipboard (saved first and
 * put back afterwards; the test copies are kept out of the clipboard history like any other),
 * its settings in a throw-away .ini, and the pure functions of the update. It takes about five
 * seconds and never touches the user's settings. Exit code: the number of failures. */

typedef struct { UINT fmt; HGLOBAL h; } SavedFormat;
static SavedFormat gSaved[64];
static int gSavedN = 0;
static int gTestFails = 0;
static FILE* gTestLog;

/* One line of the report: "ok" or "BAD", and what was checked. */
static void testCheck(int ok, const char* what)
{
	if (!ok) gTestFails++;
	fprintf(gTestLog, "%-4s %s\r\n", ok ? "ok" : "BAD", what);
	fflush(gTestLog);
}

/* formats whose data is a GDI object or private, not a block of memory that can be copied */
static int notMemoryFormat(UINT f)
{
	return f == CF_BITMAP || f == CF_METAFILEPICT || f == CF_PALETTE || f == CF_ENHMETAFILE ||
	       f == CF_OWNERDISPLAY || f == CF_DSPBITMAP || f == CF_DSPMETAFILEPICT || f == CF_DSPENHMETAFILE ||
	       (f >= CF_PRIVATEFIRST && f <= CF_PRIVATELAST) || (f >= CF_GDIOBJFIRST && f <= CF_GDIOBJLAST);
}

/* A copy of every memory format on the clipboard (a bitmap is kept through its CF_DIB). */
static void saveUserClipboard(void)
{
	UINT f = 0;

	gSavedN = 0;
	if (!openClipboardRetry()) return;
	while ((f = EnumClipboardFormats(f)) != 0 && gSavedN < 64)
	{
		HANDLE h;
		HGLOBAL copy;
		SIZE_T n;
		void *src, *dst;

		if (notMemoryFormat(f) || (h = GetClipboardData(f)) == NULL || (n = GlobalSize(h)) == 0) continue;
		if ((src = GlobalLock(h)) == NULL) continue;
		copy = GlobalAlloc(GMEM_MOVEABLE, n);
		if (copy && (dst = GlobalLock(copy)) != NULL)
		{
			memcpy(dst, src, n);
			GlobalUnlock(copy);
			gSaved[gSavedN].fmt = f;
			gSaved[gSavedN++].h = copy;
		}
		else if (copy) GlobalFree(copy);
		GlobalUnlock(h);
	}
	CloseClipboard();
}

/* Puts the saved formats back. The saved content had gone to the clipboard history when it
 * was first copied; unless it carried its own history format, it is kept out this time. */
static int restoreUserClipboard(void)
{
	int i, history = 0;

	if (!openClipboardRetry())
	{
		for (i = 0; i < gSavedN; i++) GlobalFree(gSaved[i].h);
		gSavedN = 0;
		return 0;
	}
	EmptyClipboard();
	for (i = 0; i < gSavedN; i++)
	{
		wchar_t name[64];
		if (GetClipboardFormatNameW(gSaved[i].fmt, name, 64) && !lstrcmpiW(name, L"CanIncludeInClipboardHistory")) history = 1;
		if (!SetClipboardData(gSaved[i].fmt, gSaved[i].h)) GlobalFree(gSaved[i].h);
	}
	if (gSavedN && !history)
	{
		putDwordFormat(L"CanIncludeInClipboardHistory", 0);
		putDwordFormat(L"CanUploadToCloudClipboard", 0);
	}
	CloseClipboard();
	gSavedN = 0;
	return 1;
}

/* A copy made by "another program": a second window of ours, without any marker formats. */
static void otherCopies(HWND other, const wchar_t* text)
{
	HGLOBAL h;
	int i;

	for (i = 0; i < 10 && !OpenClipboard(other); i++) Sleep(20);
	EmptyClipboard();
	h = globalCopy(text, (wcslen(text) + 1) * sizeof(wchar_t));
	if (h && !SetClipboardData(CF_UNICODETEXT, h)) GlobalFree(h);
	CloseClipboard();
}

/* A SHA-256 of the text on the clipboard, so the user's text can be checked when it comes
 * back without keeping a copy of it. 1 = there was text and it was hashed. */
static int clipboardTextHash(BYTE out[32])
{
	HANDLE h;
	const wchar_t* p;
	int ok = 0;

	if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !openClipboardRetry()) return 0;
	h = GetClipboardData(CF_UNICODETEXT);
	p = h ? (const wchar_t*)GlobalLock(h) : NULL;
	if (p)
	{
		ok = upd_sha256(p, wcslen(p) * sizeof(wchar_t), out);
		GlobalUnlock(h);
	}
	CloseClipboard();
	return ok;
}

static int clipboardIs(const wchar_t* text)
{
	HANDLE h;
	const wchar_t* p;
	int same = 0;

	if (!openClipboardRetry()) return 0;
	h = GetClipboardData(CF_UNICODETEXT);
	if (h && (p = (const wchar_t*)GlobalLock(h)) != NULL)
	{
		same = !wcscmp(p, text);
		GlobalUnlock(h);
	}
	CloseClipboard();
	return same;
}

static int clipboardHas(const wchar_t* formatName)
{
	UINT f = RegisterClipboardFormatW(formatName);
	return f && IsClipboardFormatAvailable(f);
}

static LRESULT CALLBACK testProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	if (m == WM_TIMER && w == TMR_CLIP) { clipTick(); return 0; }
	return DefWindowProcW(h, m, w, l);
}

int selfTest(const wchar_t* logPath)
{
	wchar_t tmp[MAX_PATH], dir[MAX_PATH];
	WNDCLASSW wc;
	HWND other;
	BYTE before[32];
	int hadText;

	gTestLog = logPath ? _wfopen(logPath, L"wb") : NULL;
	if (!gTestLog) return 100;
	fprintf(gTestLog, "Forja %ls self-test\r\n", APP_VERSION);

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = testProc;
	wc.hInstance = gInst;
	wc.lpszClassName = L"ForjaTest";
	RegisterClassW(&wc);
	hMain = CreateWindowExW(0, L"ForjaTest", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, gInst, NULL);
	other = CreateWindowExW(0, L"ForjaTest", L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, gInst, NULL);

	/* 1. settings, in a folder of their own that is deleted at the end */
	GetTempPathW(MAX_PATH, tmp);
	_snwprintf(dir, MAX_PATH, L"%lsforja_selftest_%lu_%lu", tmp, (unsigned long)GetCurrentProcessId(), (unsigned long)GetTickCount());
	dir[MAX_PATH - 1] = 0;
	CreateDirectoryW(dir, NULL);
	_snwprintf(gIni, MAX_PATH, L"%ls\\forja.ini", dir);
	gIni[MAX_PATH - 1] = 0;
	gNoSave = 0;

	loadSettings();
	testCheck(gClearClip == 1 && gClearSecs == 120, "settings: by default clearing is on, after 120 s (2 minutes)");
	testCheck(gAutoUpdate == 1, "settings: by default the daily update check is on");
	iniSetInt(L"ClearSeconds", 45);
	iniSetInt(L"ClearClipboard", 0);
	iniSetInt(L"CheckUpdates", 0);
	loadSettings();
	testCheck(gClearClip == 0 && gClearSecs == 45 && gAutoUpdate == 0, "settings: ClearClipboard=0, ClearSeconds=45, CheckUpdates=0 read back");
	iniSetInt(L"ClearSeconds", 1);
	loadSettings();
	testCheck(gClearSecs == CLIP_MIN, "settings: ClearSeconds=1 is raised to 5");
	iniSetInt(L"ClearSeconds", 999999);
	loadSettings();
	testCheck(gClearSecs == CLIP_MAX, "settings: ClearSeconds=999999 is lowered to 3600");

	gClearFromArg = 1; gClearClip = 1; gClearSecs = 30;
	saveSettings();
	loadSettings();
	testCheck(gClearClip == 0 && gClearSecs == CLIP_MAX, "settings: a /clearafter value is not saved");
	gClearFromArg = 0; gClearClip = 1; gClearSecs = 300;
	saveSettings();
	loadSettings();
	testCheck(gClearClip == 1 && gClearSecs == 300, "settings: a choice made in the window is saved");

	testCheck(parseDuration("90") == 90 && parseDuration("45s") == 45 && parseDuration("2m") == 120 &&
	          parseDuration("10M") == 600, "/clearafter: 90, 45s, 2m, 10M");
	testCheck(parseDuration("0") == 0 && parseDuration("off") == 0, "/clearafter: 0 and off mean never");
	testCheck(parseDuration("") == -1 && parseDuration("x") == -1 && parseDuration("5h") == -1 &&
	          parseDuration("-5") == -1, "/clearafter: empty, x, 5h, -5 are refused");

	/* 2. the clipboard: the user's content is saved first and put back at the end */
	hadText = clipboardTextHash(before);
	saveUserClipboard();

	gClearClip = 1;
	gClearSecs = 1;
	testCheck(putClipboard(L"WH-SELFTEST-1") && clipboardIs(L"WH-SELFTEST-1"), "clipboard: copy");
	testCheck(clipboardHas(L"ExcludeClipboardContentFromMonitorProcessing") && clipboardHas(L"CanIncludeInClipboardHistory") &&
	          clipboardHas(L"CanUploadToCloudClipboard"), "clipboard: kept out of the history (Win+V), the cloud and monitors");
	armClipWipe();
	testCheck(gClipLeft == 1, "clipboard: the countdown starts with the time set");
	pump(1700);
	testCheck(CountClipboardFormats() == 0 && gClipLeft == 0, "clipboard: wiped by the timer when the time is up");

	putClipboard(L"WH-SELFTEST-2");
	armClipWipe();
	otherCopies(other, L"WH-SELFTEST-OTHER");
	pump(1700);
	testCheck(clipboardIs(L"WH-SELFTEST-OTHER") && gClipLeft == 0, "clipboard: what another program copied afterwards is left alone");

	putClipboard(L"WH-SELFTEST-3");
	armClipWipe();
	otherCopies(other, L"WH-SELFTEST-3");
	pump(1700);
	testCheck(CountClipboardFormats() == 0, "clipboard: exactly our text, put back by another program, is still wiped");

	gClearSecs = CLIP_DEFAULT;
	putClipboard(L"WH-SELFTEST-4");
	armClipWipe();
	testCheck(gClipLeft == CLIP_DEFAULT, "clipboard: 120 s to go");
	wipeOnExit();
	testCheck(CountClipboardFormats() == 0 && gClipLeft == 0, "clipboard: closing before the time is up wipes it at once");

	putClipboard(L"WH-SELFTEST-5");
	armClipWipe();
	otherCopies(other, L"WH-SELFTEST-OTHER");
	wipeOnExit();
	testCheck(clipboardIs(L"WH-SELFTEST-OTHER"), "clipboard: closing leaves a later copy by another program alone");

	gClearClip = 0;
	putClipboard(L"WH-SELFTEST-6");
	armClipWipe();
	testCheck(gClipLeft == 0, "clipboard: with clearing off no countdown starts");
	wipeOnExit();
	testCheck(clipboardIs(L"WH-SELFTEST-6"), "clipboard: with clearing off a copy stays, also on exit");

	/* the switch acts on the copy already made */
	gClearClip = 1;
	gClearSecs = 1;
	clearSwitched();
	testCheck(gClipLeft == 1, "clipboard: switching clearing on starts the countdown for the copy still there");
	pump(1700);
	testCheck(CountClipboardFormats() == 0, "clipboard: ...and wipes it when the time is up");
	gClearSecs = CLIP_DEFAULT;
	putClipboard(L"WH-SELFTEST-7");
	armClipWipe();
	gClearClip = 0;
	clearSwitched();
	testCheck(gClipLeft == 0, "clipboard: switching clearing off stops the countdown in progress");
	wipeOnExit();
	testCheck(clipboardIs(L"WH-SELFTEST-7"), "clipboard: ...and the copy stays, also on exit");
	putClipboard(L"WH-SELFTEST-8");
	otherCopies(other, L"WH-SELFTEST-OTHER");
	gClearClip = 1;
	clearSwitched();
	testCheck(gClipLeft == 0 && clipboardIs(L"WH-SELFTEST-OTHER"), "clipboard: switching on leaves another program's copy alone");

	testCheck(restoreUserClipboard(), "clipboard: the user's content is put back");
	if (hadText)
	{
		BYTE after[32];
		testCheck(clipboardTextHash(after) && !memcmp(after, before, 32), "clipboard: the user's text is back, unchanged");
	}
	forgetClipboard();

	/* 3. saving a list: the whole file or nothing, and no temporary file left behind */
	{
		wchar_t f[MAX_PATH], pat[MAX_PATH];
		WIN32_FIND_DATAW d;
		HANDLE h;
		char back[64] = { 0 };
		DWORD got = 0;
		int ok, left = 0;

		lstrcpyA(gPw[0], "Abc-1");
		lstrcpyA(gPw[1], "Def-2");
		gPwN = 2;
		_snwprintf(f, MAX_PATH, L"%ls\\list.txt", dir);
		f[MAX_PATH - 1] = 0;
		writePasswords(f, &ok);
		h = CreateFileW(f, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (h != INVALID_HANDLE_VALUE) { ReadFile(h, back, sizeof(back) - 1, &got, NULL); CloseHandle(h); }
		_snwprintf(pat, MAX_PATH, L"%ls\\frj*.tmp", dir);
		pat[MAX_PATH - 1] = 0;
		h = FindFirstFileW(pat, &d);
		if (h != INVALID_HANDLE_VALUE) { left = 1; FindClose(h); }
		testCheck(ok && !strcmp(back, "Abc-1\r\nDef-2\r\n") && !left, "save: the list is written whole, no temporary file stays");
		DeleteFileW(f);
		_snwprintf(f, MAX_PATH, L"%ls\\no such folder\\list.txt", dir);
		f[MAX_PATH - 1] = 0;
		writePasswords(f, &ok);
		testCheck(!ok && GetFileAttributesW(f) == INVALID_FILE_ATTRIBUTES, "save: a failed save leaves no file");
		wipePasswords();
	}

	/* 4. the update's download folder is new each time, and old ones are cleared away */
	{
		wchar_t a[MAX_PATH], b[MAX_PATH], f[MAX_PATH];
		HANDLE h;
		int made = newUpdateFolder(a) && newUpdateFolder(b);
		testCheck(made && lstrcmpiW(a, b) != 0, "update: every download gets a new random folder");
		_snwprintf(f, MAX_PATH, L"%ls\\Forja-Setup-0.0.0.exe", a);
		f[MAX_PATH - 1] = 0;
		h = CreateFileW(f, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
		removeUpdateLeftovers();
		testCheck(GetFileAttributesW(a) == INVALID_FILE_ATTRIBUTES && GetFileAttributesW(b) == INVALID_FILE_ATTRIBUTES,
		          "update: setups and folders of earlier downloads are removed");
	}

	/* 5. the update's pure functions */
	gTestFails += upd_selftest(gTestLog);

	/* 6. the guessing time: every step of the scale, and every text of it in every language */
	{
		static const struct { double bits; int id; } T6[] = {
			{ 30, S_TIME_INSTANT }, { 40, S_TIME_SECONDS }, { 45, S_TIME_MINUTES }, { 52, S_TIME_HOURS },
			{ 60, S_TIME_DAYS }, { 65, S_TIME_YEARS }, { 72, S_TIME_CENTURIES }, { 75, S_TIME_THOUSANDS },
			{ 85, S_TIME_MILLIONS }, { 95, S_TIME_BILLIONS }, { 105, S_TIME_TRILLIONS }, { 122, S_TIME_FOREVER } };
		int k, l, okTime = 1, okText = 1;
		for (k = 0; k < (int)(sizeof(T6) / sizeof(T6[0])); k++) okTime &= guessTimeString(T6[k].bits) == T6[k].id;
		for (l = 0; l < LANG_COUNT; l++)
			for (k = S_TIME_INSTANT; k <= S_TIME_FOREVER; k++) okText &= STR[l][k] && STR[l][k][0];
		testCheck(okTime, "strength: the guessing time goes from seconds up to trillions of years");
		testCheck(okText, "strength: every guessing time has a text in English, Japanese and Spanish");
	}

	DeleteFileW(gIni);
	RemoveDirectoryW(dir);
	testCheck(GetFileAttributesW(dir) == INVALID_FILE_ATTRIBUTES, "the throw-away settings folder is gone");
	DestroyWindow(other);
	DestroyWindow(hMain);
	hMain = NULL;

	fprintf(gTestLog, gTestFails ? "FAILED: %d\r\n" : "ALL SELF-TESTS PASSED\r\n", gTestFails);
	fclose(gTestLog);
	return gTestFails;
}
