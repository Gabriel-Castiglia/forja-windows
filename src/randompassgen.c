/**
  Random Password Generator -- the window.
  Author: Gabriel Castiglia - 2026-11-09

  A Win32 front end for the rules of password_gen.py (see pwgen.h). One portable,
  statically linked executable: no installer, no registry. The few settings it remembers
  go to randompassgen.ini beside the executable, and only if that folder can be written.

  What the window does:
    - length 4..128 and count 1..50, "avoid ambiguous characters", and a strength line
      (characters per position, entropy, and a four-step bar);
    - a list of passwords drawn by us: digits blue, symbols amber, a copy icon on every row
      that turns into a check mark once the row has been copied;
    - copy one (icon, double-click, Enter, Ctrl+C), copy all (Ctrl+Shift+C), save (Ctrl+S);
    - copies are kept out of the clipboard history and the cloud clipboard, and are wiped
      30 s later (or on exit) unless something else has been copied since;
    - light and dark theme, following Windows; English, Japanese and Spanish;
    - Help and About windows. The version shown is APP_VERSION, read at run time.

  Lessons carried over from the author's earlier Win32 tool, each one a defect seen there:
    - decorative group boxes are owner-drawn STATICs with WS_CLIPSIBLINGS, never owner-
      drawn BUTTONs: a button takes the focus on a click and paints over its siblings;
    - the frame line of a group box runs through the middle of its title, so the badges on
      the list frame are centred on that line, and the frame steps around them;
    - a badge at the right of a scrolled list keeps clear of the scrollbar width;
    - menu bar items and drop-down items are measured separately;
    - fonts and brushes are released only after nothing points at them any more;
    - icons are loaded without LR_SHARED, which ignores the size asked for;
    - Esc closes Help and About, and the accelerators are quiet while About is modal;
    - a window cannot be driven from another process reliably (UIPI filters WM_GETTEXT),
      so the command line does everything a test needs:

        randompassgen.exe [/len:N] [/count:N] [/ambiguous] [/out:<file>]
                          [/shot:<file.bmp>] [/theme:light|dark] [/lang:en|jp|es]
                          [/open:help|about]

      /out writes the passwords (count up to 100000) and exits without a window;
      /shot draws the window into a bitmap and exits. Neither writes the .ini.
**/

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "pwgen.h"
#include "resource.h"
#include "ui_strings.h"

#define APP_VERSION   L"1.0.0"
#define APP_AUTHOR    L"Gabriel Castiglia"
#define APP_AUMID     L"Castiglia.RandomPassGen"
#define INI_SECTION   L"settings"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT 0x00000002
#endif

/* ------------------------------------------------------------------ palette */

/* Neutral slate and blue, in two complete palettes. */
typedef struct {
	COLORREF band, bandSub, body, text, hint, group, frame, editBg, editText,
	         ok, err, warn, rule, menuBg, menuText, menuHot, menuHotText, digit, sel;
} Palette;

static const Palette PAL_LIGHT = {
	RGB(0x24,0x2F,0x3D), RGB(0xA9,0xB8,0xC9), RGB(0xFF,0xFF,0xFF),
	RGB(0x1F,0x23,0x28), RGB(0x5F,0x6B,0x78), RGB(0x2B,0x4C,0x7E),
	RGB(0xCD,0xD3,0xDA), RGB(0xFF,0xFF,0xFF), RGB(0x1F,0x23,0x28),
	RGB(0x1A,0x7F,0x37), RGB(0xB4,0x23,0x18), RGB(0xB5,0x47,0x08),
	RGB(0x4F,0x8E,0xF7), RGB(0xF6,0xF7,0xF9), RGB(0x1F,0x23,0x28),
	RGB(0x2B,0x4C,0x7E), RGB(0xFF,0xFF,0xFF), RGB(0x1F,0x5F,0xBF),
	RGB(0xE8,0xEE,0xF8)
};

static const Palette PAL_DARK = {
	RGB(0x16,0x1B,0x22), RGB(0x8B,0x98,0xA9), RGB(0x1C,0x1F,0x24),
	RGB(0xE6,0xE8,0xEB), RGB(0x98,0xA2,0xAE), RGB(0x8A,0xB4,0xF8),
	RGB(0x3A,0x41,0x4B), RGB(0x24,0x28,0x2E), RGB(0xE6,0xE8,0xEB),
	RGB(0x4A,0xDE,0x80), RGB(0xFF,0x7B,0x72), RGB(0xF0,0xB4,0x4C),
	RGB(0x4F,0x8E,0xF7), RGB(0x24,0x28,0x2E), RGB(0xE6,0xE8,0xEB),
	RGB(0x2F,0x4A,0x73), RGB(0xFF,0xFF,0xFF), RGB(0x79,0xB8,0xFF),
	RGB(0x2A,0x34,0x42)
};

static Palette PAL;
static int gDark = 0;
static int gForceTheme = -1;            /* /theme: -1 follow Windows, 0 light, 1 dark */

/* ------------------------------------------------------------------ state */

enum {
	IDC_GRP_SET = 1001, IDC_GRP_LIST,
	IDC_LBL_LEN, IDC_LEN, IDC_LEN_UD, IDC_LBL_CNT, IDC_CNT, IDC_CNT_UD,
	IDC_AMBIG, IDC_AMBIG_LBL, IDC_INFO_AMBIG,
	IDC_CLEAR, IDC_CLEAR_LBL, IDC_INFO_CLEAR,
	IDC_STRENGTH, IDC_GENERATE, IDC_LIST, IDC_COPYALL, IDC_SAVE,
	IDC_STATUS, IDC_VERSION, IDC_HELP_TEXT
};

enum { TMR_CLIP = 1, TMR_MARK = 2, TMR_REGEN = 3 };

enum { ST_PLAIN, ST_OK, ST_WARN, ST_ERR };

#define MAX_COUNT     50
#define AUTO_MAX      100000
#define CLIP_SECONDS  30

static int gDpi = 96;
#define SC(v)  MulDiv((v), gDpi, 96)

/* Logical units, at 96 dpi. SC() turns each one into pixels. */
#define DESIGN_W  660
#define BAND_H    86
#define ICON_SZ   56
#define PAD       16
#define GAP       16
#define BTN_W     132
#define BTN_H     46
#define Y_SET     (BAND_H + 16)
#define SET_H     170
#define Y_ROW1    (Y_SET + 32)
#define Y_ROW2    (Y_SET + 74)
#define Y_ROW3    (Y_SET + 102)
#define Y_ROW4    (Y_SET + 136)
#define Y_LIST    (Y_SET + SET_H + 16)
#define ROW_LH    34      /* one password row */
#define ICON_ZONE 44      /* the copy icon at the right of a row */
#define VER_W     64
#define BADGE_W   20
#define BADGE_GAP 10
#define SCROLL_W  17      /* kept clear on the right: the list's vertical scrollbar */
#define FIELD_W   78
#define LBL1_X    (PAD + GAP)
#define LBL1_W    76
#define EDIT1_X   (LBL1_X + LBL1_W)
#define LBL2_X    (EDIT1_X + FIELD_W + 30)
#define LBL2_W    86
#define EDIT2_X   (LBL2_X + LBL2_W)

static HINSTANCE gInst;
static HWND hMain, hTip, hStatus, hVersion;
static HWND hGrpSet, hGrpList, hLblLen, hLen, hLenUd, hLblCnt, hCnt, hCntUd;
static HWND hAmbig, hAmbigLbl, hInfoAmbig, hClear, hClearLbl, hInfoClear;
static HWND hStrength, hGenerate, hList, hCopyAll, hSave;
static HWND hHelpWnd, hAboutWnd;
static HFONT fUi, fBold, fTitle, fSub, fBadge, fPw;
static HBRUSH brBody, brBand, brEdit, brMenu;
static HICON icBand, icAbout;
static HMENU hMenu;

static int gLang = LANG_EN;
static int gLen = 20, gCount = 5, gAvoid = 1, gClearClip = 1;
static int gNoSave = 0;

static char gPw[MAX_COUNT][PW_MAX_LEN + 1];
static int gPwN = 0, gPwLen = 0;

static int gHotBadge = 0;               /* IDC_COPYALL or IDC_SAVE under the pointer */
static int gHotRow = -1, gHotIcon = 0;  /* list row under the pointer, and is it on the icon */
static int gMarkRow = -1;               /* row showing the check mark; -2 = all of them */
static int gTrackingList = 0;

static DWORD gClipSeq = 0;              /* clipboard sequence number right after our copy */
static int gClipLeft = 0;               /* seconds until we wipe it; 0 = nothing pending */
static wchar_t gClipMsg[256];

static int gStatusKind = ST_PLAIN;
static wchar_t gStatusText[512];
static wchar_t gIni[MAX_PATH];
static wchar_t gLastFolder[MAX_PATH];

#define T(id)  (STR[gLang][(id)])

/* ------------------------------------------------------------------ settings */

/* randompassgen.ini beside the executable. Writing it may fail (a read-only share, a
 * protected folder); then the program simply starts with the defaults next time. */

static void iniPath(void)
{
	wchar_t* dot;

	GetModuleFileNameW(NULL, gIni, MAX_PATH);
	dot = wcsrchr(gIni, L'.');
	if (dot && (size_t)(dot - gIni) + 5 < MAX_PATH) wcscpy(dot, L".ini");
}

static int iniInt(const wchar_t* key, int def)
{
	wchar_t buf[32];

	GetPrivateProfileStringW(INI_SECTION, key, L"", buf, 32, gIni);
	return buf[0] ? _wtoi(buf) : def;
}

static void iniSetInt(const wchar_t* key, int v)
{
	wchar_t buf[32];

	if (gNoSave) return;
	_snwprintf(buf, 32, L"%d", v);
	buf[31] = 0;
	WritePrivateProfileStringW(INI_SECTION, key, buf, gIni);
}

static void iniStr(const wchar_t* key, wchar_t* out, int cap)
{
	GetPrivateProfileStringW(INI_SECTION, key, L"", out, (DWORD)cap, gIni);
}

static void iniSetStr(const wchar_t* key, const wchar_t* v)
{
	if (!gNoSave) WritePrivateProfileStringW(INI_SECTION, key, v, gIni);
}

static int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* ------------------------------------------------------------------ theme */

static int systemPrefersDark(void)
{
	HKEY k;
	DWORD v = 1, cb = sizeof(v), type = 0;
	int dark = 0;

	if (gForceTheme >= 0) return gForceTheme;

	if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_CURRENT_USER,
	        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
	        0, KEY_READ, &k))
	{
		if (ERROR_SUCCESS == RegQueryValueExW(k, L"AppsUseLightTheme", NULL, &type, (LPBYTE)&v, &cb))
			dark = (v == 0);
		RegCloseKey(k);
	}
	return dark;
}

/* Windows has no documented way to ask for dark controls. These uxtheme ordinals are what
 * applications use; they are looked up at run time and skipped when absent. Everything
 * that matters -- group boxes, labels, the menu, the list -- is drawn by us anyway. */
static void enableDarkControls(int dark)
{
	typedef int  (WINAPI *PSetPreferredAppMode)(int);
	typedef BOOL (WINAPI *PAllowDarkModeForWindow)(HWND, BOOL);
	typedef void (WINAPI *PFlushMenuThemes)(void);
	static PSetPreferredAppMode setMode = NULL;
	static PAllowDarkModeForWindow allowFor = NULL;
	static PFlushMenuThemes flush = NULL;
	static int tried = 0;

	if (!tried)
	{
		HMODULE ux = LoadLibraryW(L"uxtheme.dll");
		tried = 1;
		if (ux)
		{
			allowFor = (PAllowDarkModeForWindow)(void*)GetProcAddress(ux, MAKEINTRESOURCEA(133));
			setMode  = (PSetPreferredAppMode)(void*)GetProcAddress(ux, MAKEINTRESOURCEA(135));
			flush    = (PFlushMenuThemes)(void*)GetProcAddress(ux, MAKEINTRESOURCEA(136));
		}
	}

	if (setMode) setMode(dark ? 2 : 3);        /* 2 = ForceDark, 3 = ForceLight */
	if (allowFor && hMain) allowFor(hMain, dark ? TRUE : FALSE);
	if (flush) flush();
}

static BOOL CALLBACK themeChild(HWND child, LPARAM dark)
{
	SetWindowTheme(child, dark ? L"DarkMode_Explorer" : L"Explorer", NULL);
	return TRUE;
}

static void applyTheme(HWND hwnd)
{
	BOOL dark = gDark ? TRUE : FALSE;

	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
	EnumChildWindows(hwnd, themeChild, (LPARAM)gDark);

	/* RDW_ALLCHILDREN: the owner-drawn children keep the old palette otherwise */
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

static void refreshTheme(void)
{
	HBRUSH oldBody = brBody, oldBand = brBand, oldEdit = brEdit, oldMenu = brMenu;
	MENUINFO mi;

	gDark = systemPrefersDark();
	PAL = gDark ? PAL_DARK : PAL_LIGHT;

	brBody = CreateSolidBrush(PAL.body);
	brBand = CreateSolidBrush(PAL.band);
	brEdit = CreateSolidBrush(PAL.editBg);
	brMenu = CreateSolidBrush(PAL.menuBg);

	enableDarkControls(gDark);

	/* the strip of menu bar no item covers is painted by Windows with this brush */
	if (hMenu)
	{
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
		mi.hbrBack = brMenu;
		SetMenuInfo(hMenu, &mi);
	}

	if (hMain)     applyTheme(hMain);
	if (hHelpWnd)  applyTheme(hHelpWnd);
	if (hAboutWnd) applyTheme(hAboutWnd);

	/* only now: until here the old brushes were still in use */
	if (oldBody) DeleteObject(oldBody);
	if (oldBand) DeleteObject(oldBand);
	if (oldEdit) DeleteObject(oldEdit);
	if (oldMenu) DeleteObject(oldMenu);
}

/* ------------------------------------------------------------------ status */

static void setStatus(int kind, const wchar_t* text)
{
	gStatusKind = kind;
	lstrcpynW(gStatusText, text, 512);
	if (hStatus)
	{
		SetWindowTextW(hStatus, gStatusText);
		InvalidateRect(hStatus, NULL, TRUE);
	}
}

static void setStatusF(int kind, int id, ...)
{
	wchar_t buf[512];
	va_list ap;

	va_start(ap, id);
	_vsnwprintf(buf, 512, T(id), ap);
	va_end(ap);
	buf[511] = 0;
	setStatus(kind, buf);
}

/* ------------------------------------------------------------------ reading the fields */

static int fieldInt(HWND edit, int def)
{
	wchar_t w[16];

	GetWindowTextW(edit, w, 16);
	return w[0] ? _wtoi(w) : def;
}

static void readSettings(void)
{
	gLen = clampInt(fieldInt(hLen, gLen), PW_MIN_LEN, PW_MAX_LEN);
	gCount = clampInt(fieldInt(hCnt, gCount), 1, MAX_COUNT);
	gAvoid = (IsDlgButtonChecked(hMain, IDC_AMBIG) == BST_CHECKED);
	gClearClip = (IsDlgButtonChecked(hMain, IDC_CLEAR) == BST_CHECKED);
}

/* Puts the clamped value back in the field once the user leaves it, so what is shown is
 * what is used. Not while typing: "2" on the way to "20" must not become "4". */
static void normaliseField(HWND edit, int v)
{
	wchar_t w[16];

	_snwprintf(w, 16, L"%d", v);
	w[15] = 0;
	SetWindowTextW(edit, w);
}

/* ------------------------------------------------------------------ generating */

static void wipePasswords(void)
{
	SecureZeroMemory(gPw, sizeof(gPw));
	gPwN = 0;
}

static void doGenerate(void)
{
	int i;

	readSettings();
	wipePasswords();

	SendMessageW(hList, WM_SETREDRAW, FALSE, 0);
	SendMessageW(hList, LB_RESETCONTENT, 0, 0);

	for (i = 0; i < gCount; i++)
	{
		if (!pw_generate(gLen, gAvoid, gPw[i]))
		{
			wipePasswords();
			SendMessageW(hList, LB_RESETCONTENT, 0, 0);
			SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
			setStatus(ST_ERR, T(S_ST_RNG_FAIL));
			return;
		}
		/* no LBS_HASSTRINGS: the list keeps only the row number, never the password */
		SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)i);
	}

	gPwN = gCount;
	gPwLen = gLen;
	gMarkRow = -1;
	SendMessageW(hList, LB_SETCURSEL, 0, 0);
	SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hList, NULL, TRUE);
	InvalidateRect(hStrength, NULL, TRUE);

	if (gPwN == 1) setStatusF(ST_OK, S_ST_GENERATED_ONE, gPwLen);
	else           setStatusF(ST_OK, S_ST_GENERATED, gPwN, gPwLen);
}

/* ------------------------------------------------------------------ clipboard */

/* A clipboard format whose presence tells Windows what to do with the copy. */
static void putDwordFormat(const wchar_t* name, DWORD value)
{
	UINT fmt = RegisterClipboardFormatW(name);
	HGLOBAL h;

	if (!fmt) return;
	h = GlobalAlloc(GMEM_MOVEABLE, sizeof(DWORD));
	if (!h) return;
	*(DWORD*)GlobalLock(h) = value;
	GlobalUnlock(h);
	if (!SetClipboardData(fmt, h)) GlobalFree(h);
}

static int openClipboardRetry(void)
{
	int i;

	/* another program may hold it for a moment */
	for (i = 0; i < 10; i++)
	{
		if (OpenClipboard(hMain)) return 1;
		Sleep(20);
	}
	return 0;
}

static int putClipboard(const wchar_t* text)
{
	size_t bytes = (wcslen(text) + 1) * sizeof(wchar_t);
	HGLOBAL h;

	if (!openClipboardRetry()) return 0;

	EmptyClipboard();

	h = GlobalAlloc(GMEM_MOVEABLE, bytes);
	if (!h) { CloseClipboard(); return 0; }
	memcpy(GlobalLock(h), text, bytes);
	GlobalUnlock(h);
	if (!SetClipboardData(CF_UNICODETEXT, h)) { GlobalFree(h); CloseClipboard(); return 0; }

	/* keep it out of Win+V history, the cloud clipboard and clipboard monitors */
	putDwordFormat(L"ExcludeClipboardContentFromMonitorProcessing", 0);
	putDwordFormat(L"CanIncludeInClipboardHistory", 0);
	putDwordFormat(L"CanUploadToCloudClipboard", 0);

	CloseClipboard();
	gClipSeq = GetClipboardSequenceNumber();
	return 1;
}

/* Wipes the clipboard only if it still holds what we put there. */
static int wipeOurClipboard(void)
{
	if (GetClipboardSequenceNumber() != gClipSeq) return 0;
	if (!openClipboardRetry()) return 0;
	EmptyClipboard();
	CloseClipboard();
	return 1;
}

static void showCopied(void)
{
	wchar_t buf[512];

	if (gClipLeft > 0)
	{
		wchar_t tail[128];
		_snwprintf(tail, 128, T(S_ST_CLEAR_IN), gClipLeft);
		tail[127] = 0;
		_snwprintf(buf, 512, L"%ls%ls", gClipMsg, tail);
	}
	else lstrcpynW(buf, gClipMsg, 512);
	buf[511] = 0;
	setStatus(ST_OK, buf);
}

static void afterCopy(int row)
{
	gMarkRow = row;
	SetTimer(hMain, TMR_MARK, 1500, NULL);
	InvalidateRect(hList, NULL, FALSE);

	KillTimer(hMain, TMR_CLIP);
	gClipLeft = 0;
	readSettings();
	if (gClearClip)
	{
		gClipLeft = CLIP_SECONDS;
		SetTimer(hMain, TMR_CLIP, 1000, NULL);
	}
	showCopied();
}

static void copyRow(int row)
{
	wchar_t w[PW_MAX_LEN + 1];

	if (row < 0 || row >= gPwN) return;

	MultiByteToWideChar(CP_ACP, 0, gPw[row], -1, w, PW_MAX_LEN + 1);
	if (!putClipboard(w)) { SecureZeroMemory(w, sizeof(w)); setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }
	SecureZeroMemory(w, sizeof(w));

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED), row + 1);
	gClipMsg[255] = 0;
	afterCopy(row);
}

static void copyAll(void)
{
	wchar_t w[MAX_COUNT * (PW_MAX_LEN + 2) + 1];
	int i, n = 0;

	if (gPwN <= 0) return;

	for (i = 0; i < gPwN; i++)
	{
		n += MultiByteToWideChar(CP_ACP, 0, gPw[i], -1, w + n, PW_MAX_LEN + 1) - 1;
		w[n++] = L'\r';
		w[n++] = L'\n';
	}
	w[n] = 0;

	if (!putClipboard(w)) { SecureZeroMemory(w, sizeof(w)); setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }
	SecureZeroMemory(w, sizeof(w));

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED_ALL), gPwN);
	gClipMsg[255] = 0;
	afterCopy(-2);
}

static void copySelected(void)
{
	HWND f = GetFocus();
	wchar_t cls[16];

	/* Ctrl+C in one of the number fields copies the number, as everywhere else */
	if (f && GetClassNameW(f, cls, 16) && !lstrcmpiW(cls, L"Edit")) { SendMessageW(f, WM_COPY, 0, 0); return; }

	copyRow((int)SendMessageW(hList, LB_GETCURSEL, 0, 0));
}

static void clipTick(void)
{
	if (GetClipboardSequenceNumber() != gClipSeq)
	{
		/* something else was copied: it is not ours to wipe */
		KillTimer(hMain, TMR_CLIP);
		gClipLeft = 0;
		showCopied();
		return;
	}

	if (--gClipLeft <= 0)
	{
		KillTimer(hMain, TMR_CLIP);
		gClipLeft = 0;
		if (wipeOurClipboard()) setStatus(ST_PLAIN, T(S_ST_CLEARED));
		return;
	}
	showCopied();
}

/* ------------------------------------------------------------------ saving */

static void writePasswords(const wchar_t* path, int* ok)
{
	HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD done;
	int i;

	*ok = 0;
	if (f == INVALID_HANDLE_VALUE) return;

	*ok = 1;
	for (i = 0; i < gPwN && *ok; i++)
	{
		DWORD len = (DWORD)strlen(gPw[i]);
		if (!WriteFile(f, gPw[i], len, &done, NULL) || done != len) *ok = 0;
		if (*ok && (!WriteFile(f, "\r\n", 2, &done, NULL) || done != 2)) *ok = 0;
	}
	CloseHandle(f);
}

static void rememberFolderOf(const wchar_t* file)
{
	wchar_t* slash;

	lstrcpynW(gLastFolder, file, MAX_PATH);
	slash = wcsrchr(gLastFolder, L'\\');
	if (slash) *slash = 0;
	iniSetStr(L"LastFolder", gLastFolder);
}

static void saveAs(HWND hwnd)
{
	OPENFILENAMEW ofn;
	wchar_t path[MAX_PATH];
	wchar_t msg[MAX_PATH + 64];
	int ok;

	if (gPwN <= 0) return;

	lstrcpynW(path, T(S_SAVE_DEFNAME), MAX_PATH);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = T(S_SAVE_FILTER);
	ofn.lpstrFile = path;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = L"txt";
	ofn.lpstrInitialDir = gLastFolder[0] ? gLastFolder : NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

	if (!GetSaveFileNameW(&ofn)) return;

	writePasswords(path, &ok);
	if (!ok) { setStatus(ST_ERR, T(S_ST_SAVE_FAIL)); return; }

	rememberFolderOf(path);
	_snwprintf(msg, MAX_PATH + 64, T(S_ST_SAVED), path);
	msg[MAX_PATH + 63] = 0;
	setStatus(ST_OK, msg);
}

/* ------------------------------------------------------------------ drawing helpers */

static HPEN penOf(COLORREF c, int w) { return CreatePen(PS_SOLID, w, c); }

/* Two overlapping sheets: "copy". Outline only -- detail inside a 16-20 px icon reads as
 * noise. The front sheet is filled first, which puts it in front of the other one. */
static void drawCopyGlyph(HDC dc, int x, int y, int size, COLORREF ink, HBRUSH fill)
{
	int off = size / 4;
	HPEN pen = penOf(ink, 1);
	HGDIOBJ op = SelectObject(dc, pen), ob = SelectObject(dc, fill);

	Rectangle(dc, x + off, y, x + size, y + size - off);
	Rectangle(dc, x, y + off, x + size - off, y + size);

	SelectObject(dc, ob);
	SelectObject(dc, op);
	DeleteObject(pen);
}

static void drawCheckGlyph(HDC dc, int x, int y, int size, COLORREF ink)
{
	HPEN pen = penOf(ink, SC(2));
	HGDIOBJ op = SelectObject(dc, pen);

	MoveToEx(dc, x + size / 8, y + size / 2, NULL);
	LineTo(dc, x + size * 3 / 8, y + size * 6 / 8);
	LineTo(dc, x + size * 7 / 8, y + size / 4);

	SelectObject(dc, op);
	DeleteObject(pen);
}

/* An arrow into a tray: "save". Straight lines only. */
static void drawSaveGlyph(HDC dc, int x, int y, int size, COLORREF ink)
{
	HPEN pen = penOf(ink, 1);
	HGDIOBJ op = SelectObject(dc, pen);
	int cx = x + size / 2, tip = y + size * 11 / 16, q = size / 4;

	MoveToEx(dc, x, y + size * 9 / 16, NULL);          /* the tray */
	LineTo(dc, x, y + size - 1);
	LineTo(dc, x + size - 1, y + size - 1);
	LineTo(dc, x + size - 1, y + size * 9 / 16 - 1);

	MoveToEx(dc, cx, y, NULL);                         /* the arrow */
	LineTo(dc, cx, tip + 1);
	MoveToEx(dc, cx - q, tip - q, NULL);
	LineTo(dc, cx, tip);
	LineTo(dc, cx + q + 1, tip - q - 1);

	SelectObject(dc, op);
	DeleteObject(pen);
}

/* A small circled "i". Drawn, not loaded: a system icon comes at one size and blurs. */
static void drawInfoBadge(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	int w = d->rcItem.right - d->rcItem.left, h = d->rcItem.bottom - d->rcItem.top;
	int size = (w < h ? w : h);
	int x = d->rcItem.left + (w - size) / 2, y = d->rcItem.top + (h - size) / 2;
	HPEN pen = penOf(PAL.hint, 1);
	HGDIOBJ op, ob, of;
	RECT t;

	FillRect(dc, &d->rcItem, brBody);
	op = SelectObject(dc, pen);
	ob = SelectObject(dc, GetStockObject(NULL_BRUSH));
	Ellipse(dc, x, y, x + size, y + size);
	SelectObject(dc, ob);
	SelectObject(dc, op);
	DeleteObject(pen);

	of = SelectObject(dc, fBadge);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, PAL.hint);
	SetRect(&t, x, y, x + size, y + size);
	DrawTextW(dc, L"i", 1, &t, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	SelectObject(dc, of);
}

static void drawFrameBadge(const DRAWITEMSTRUCT* d)
{
	int w = d->rcItem.right - d->rcItem.left, h = d->rcItem.bottom - d->rcItem.top;
	int size = (w < h ? w : h) - SC(2);
	int x = d->rcItem.left + (w - size) / 2, y = d->rcItem.top + (h - size) / 2;
	COLORREF ink = (gHotBadge == (int)d->CtlID) ? PAL.text : PAL.hint;

	FillRect(d->hDC, &d->rcItem, brBody);
	if (d->CtlID == IDC_COPYALL)
	{
		if (gMarkRow == -2) drawCheckGlyph(d->hDC, x, y, size, PAL.ok);
		else drawCopyGlyph(d->hDC, x, y, size, ink, brBody);
	}
	else drawSaveGlyph(d->hDC, x, y, size, ink);
}

/* Where the frame of the list group has to step around the two badges, read from the
 * badges themselves so the two can never drift apart. */
static void badgeGap(HWND group, int* left, int* right)
{
	RECT a, b;

	*left = *right = 0;
	if (!hSave || !hCopyAll) return;

	GetWindowRect(hSave, &a);
	GetWindowRect(hCopyAll, &b);
	MapWindowPoints(NULL, group, (POINT*)&a, 2);
	MapWindowPoints(NULL, group, (POINT*)&b, 2);
	*left = a.left - SC(6);
	*right = b.right + SC(6);
}

static void frameAround(HDC dc, HWND parent, HWND a, HWND b);

/* A group box the theme cannot spoil: the frame runs through the middle of the title and
 * is interrupted for it (and, on the list group, for the badges). */
static void drawGroupBox(const DRAWITEMSTRUCT* d, const wchar_t* title)
{
	HDC dc = d->hDC;
	RECT r = d->rcItem;
	HPEN pen = penOf(PAL.frame, 1);
	HGDIOBJ op, ob, of;
	SIZE ts;
	int tx = r.left + SC(14), mid;

	FillRect(dc, &r, brBody);
	of = SelectObject(dc, fBold);
	GetTextExtentPoint32W(dc, title, (int)wcslen(title), &ts);
	mid = r.top + ts.cy / 2;

	op = SelectObject(dc, pen);
	ob = SelectObject(dc, GetStockObject(NULL_BRUSH));

	MoveToEx(dc, tx - SC(6), mid, NULL);
	LineTo(dc, r.left, mid);
	LineTo(dc, r.left, r.bottom - 1);
	LineTo(dc, r.right - 1, r.bottom - 1);
	LineTo(dc, r.right - 1, mid);

	if (d->CtlID == IDC_GRP_LIST)
	{
		int gl = 0, gr = 0;
		badgeGap(d->hwndItem, &gl, &gr);
		if (gl > tx + ts.cx && gr < r.right - 1)
		{
			LineTo(dc, gr, mid);
			MoveToEx(dc, gl, mid, NULL);
		}
	}
	LineTo(dc, tx + ts.cx + SC(6), mid);

	SelectObject(dc, ob);
	SelectObject(dc, op);
	DeleteObject(pen);

	/* The one-pixel borders of the controls inside. The group draws them, not the main
	 * window: with WS_CLIPCHILDREN the main window cannot paint where the group lies, and
	 * on the light theme a white field without a border disappears into the page. */
	if (d->CtlID == IDC_GRP_SET)
	{
		frameAround(dc, d->hwndItem, hLen, hLenUd);
		frameAround(dc, d->hwndItem, hCnt, hCntUd);
	}
	else frameAround(dc, d->hwndItem, hList, NULL);

	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, PAL.group);
	TextOutW(dc, tx, r.top, title, (int)wcslen(title));
	SelectObject(dc, of);
}

/* Four steps, coloured by level, then the figures and the level in words. */
static void drawStrength(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	RECT r = d->rcItem, t;
	int len, avoid, bits, level, i, x, segW = SC(22), segH = SC(6), segGap = SC(3);
	COLORREF lc;
	HGDIOBJ of;
	wchar_t buf[256];
	SIZE ts;
	static const int words[4] = { S_STR_WEAK, S_STR_FAIR, S_STR_STRONG, S_STR_VSTRONG };

	len = clampInt(fieldInt(hLen, gLen), PW_MIN_LEN, PW_MAX_LEN);
	avoid = (IsDlgButtonChecked(hMain, IDC_AMBIG) == BST_CHECKED);
	bits = (int)(pw_entropy_bits(len, avoid) + 0.5);
	level = bits < 50 ? 0 : bits < 70 ? 1 : bits < 90 ? 2 : 3;
	lc = level == 0 ? PAL.err : level == 1 ? PAL.warn : PAL.ok;

	FillRect(dc, &r, brBody);

	x = r.left;
	for (i = 0; i < 4; i++)
	{
		RECT s;
		HBRUSH b = CreateSolidBrush(i <= level ? lc : PAL.frame);
		SetRect(&s, x, (r.top + r.bottom - segH) / 2, x + segW, (r.top + r.bottom + segH) / 2);
		FillRect(dc, &s, b);
		DeleteObject(b);
		x += segW + segGap;
	}
	x += SC(10);

	of = SelectObject(dc, fUi);
	SetBkMode(dc, TRANSPARENT);
	_snwprintf(buf, 256, T(S_STRENGTH_FMT), pw_pool_size(avoid), bits, L"");
	buf[255] = 0;
	SetTextColor(dc, PAL.hint);
	SetRect(&t, x, r.top, r.right, r.bottom);
	DrawTextW(dc, buf, -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	GetTextExtentPoint32W(dc, buf, (int)wcslen(buf), &ts);

	SelectObject(dc, fBold);
	SetTextColor(dc, lc);
	t.left = x + ts.cx;
	DrawTextW(dc, T(words[level]), -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
	SelectObject(dc, of);
}

/* One password row: number, the password (digits and symbols coloured), the copy icon. */
static void drawRow(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	RECT r = d->rcItem, t;
	int row = (int)d->itemData, sel = (d->itemState & ODS_SELECTED) != 0;
	int iconSz = SC(16), iconX, iconY, x, cw, maxX, i;
	HBRUSH bg;
	HGDIOBJ of;
	TEXTMETRICW tm;
	SIZE m;
	wchar_t num[8];

	if ((int)d->itemID < 0 || row < 0 || row >= gPwN) { FillRect(dc, &r, brEdit); return; }

	bg = sel ? CreateSolidBrush(PAL.sel) : brEdit;
	FillRect(dc, &r, bg);
	if (sel)
	{
		RECT a = r;
		HBRUSH acc = CreateSolidBrush(PAL.rule);
		a.right = a.left + SC(3);
		FillRect(dc, &a, acc);
		DeleteObject(acc);
	}

	SetBkMode(dc, TRANSPARENT);

	/* number */
	of = SelectObject(dc, fUi);
	SetTextColor(dc, PAL.hint);
	_snwprintf(num, 8, L"%d", row + 1);
	num[7] = 0;
	SetRect(&t, r.left + SC(6), r.top, r.left + SC(34), r.bottom);
	DrawTextW(dc, num, -1, &t, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	/* password, one character at a time in a fixed pitch */
	SelectObject(dc, fPw);
	GetTextMetricsW(dc, &tm);
	GetTextExtentPoint32W(dc, L"M", 1, &m);
	cw = m.cx;
	x = r.left + SC(48);
	iconX = r.right - SC(ICON_ZONE) + (SC(ICON_ZONE) - iconSz) / 2;
	maxX = r.right - SC(ICON_ZONE) - SC(4);

	for (i = 0; gPw[row][i]; i++)
	{
		char c = gPw[row][i];
		wchar_t wc = (wchar_t)(unsigned char)c;
		int y = r.top + ((r.bottom - r.top) - tm.tmHeight) / 2;

		if (x + 2 * cw > maxX && gPw[row][i + 1])
		{
			SetTextColor(dc, PAL.hint);
			TextOutW(dc, x, y, L"\x2026", 1);
			break;
		}

		if (c >= '0' && c <= '9')                     SetTextColor(dc, PAL.digit);
		else if (strchr(PW_SYMBOLS, c))               SetTextColor(dc, PAL.warn);
		else                                          SetTextColor(dc, PAL.editText);

		TextOutW(dc, x, y, &wc, 1);
		x += cw;
	}

	/* copy icon, or the check mark right after this row (or all rows) was copied */
	iconY = r.top + ((r.bottom - r.top) - iconSz) / 2;
	if (gMarkRow == row || gMarkRow == -2)
		drawCheckGlyph(dc, iconX, iconY, iconSz, PAL.ok);
	else
		drawCopyGlyph(dc, iconX, iconY, iconSz,
		              (gHotRow == row && gHotIcon) ? PAL.text : PAL.hint, bg);

	/* a faint rule between rows */
	{
		HPEN pen = penOf(PAL.frame, 1);
		HGDIOBJ op = SelectObject(dc, pen);
		MoveToEx(dc, r.left + SC(12), r.bottom - 1, NULL);
		LineTo(dc, r.right - SC(12), r.bottom - 1);
		SelectObject(dc, op);
		DeleteObject(pen);
	}

	SelectObject(dc, of);
	if (sel) DeleteObject(bg);
}

static void paintBand(HWND hwnd, HDC dc)
{
	RECT rc, band;
	HGDIOBJ of;
	HBRUSH rule;

	GetClientRect(hwnd, &rc);
	band = rc;
	band.bottom = SC(BAND_H);
	FillRect(dc, &band, brBand);

	band.top = SC(BAND_H) - SC(3);
	rule = CreateSolidBrush(PAL.rule);
	FillRect(dc, &band, rule);
	DeleteObject(rule);

	if (icBand)
		DrawIconEx(dc, SC(20), (SC(BAND_H) - SC(3) - SC(ICON_SZ)) / 2, icBand,
		           SC(ICON_SZ), SC(ICON_SZ), 0, NULL, DI_NORMAL);

	SetBkMode(dc, TRANSPARENT);
	of = SelectObject(dc, fTitle);
	SetTextColor(dc, RGB(255, 255, 255));
	TextOutW(dc, SC(20 + ICON_SZ + 18), SC(16), T(S_APP_TITLE), (int)wcslen(T(S_APP_TITLE)));
	SelectObject(dc, fSub);
	SetTextColor(dc, PAL.bandSub);
	TextOutW(dc, SC(20 + ICON_SZ + 20), SC(50), T(S_APP_SUBTITLE), (int)wcslen(T(S_APP_SUBTITLE)));
	SelectObject(dc, of);
}

/* A one pixel border around a control (or two, side by side), instead of the 3D one. */
static void frameAround(HDC dc, HWND parent, HWND a, HWND b)
{
	RECT r, r2;
	HPEN pen;
	HGDIOBJ op, ob;

	if (!a) return;
	GetWindowRect(a, &r);
	if (b) { GetWindowRect(b, &r2); UnionRect(&r, &r, &r2); }
	MapWindowPoints(NULL, parent, (POINT*)&r, 2);
	InflateRect(&r, 1, 1);

	pen = penOf(PAL.frame, 1);
	op = SelectObject(dc, pen);
	ob = SelectObject(dc, GetStockObject(NULL_BRUSH));
	Rectangle(dc, r.left, r.top, r.right, r.bottom);
	SelectObject(dc, ob);
	SelectObject(dc, op);
	DeleteObject(pen);
}

/* ------------------------------------------------------------------ menu */

/* Owner-drawn: its text changes with the language, and Windows draws a menu bar white
 * however dark the window is. Each item carries its string id in the item data. */

#define MENU_SEP      ((ULONG_PTR)-1)
#define MENU_BAR      ((ULONG_PTR)0x10000)   /* or-ed into the items ON the bar */
#define MENU_ID(d)    ((int)((d) & 0xFFFF))
#define MENU_ISBAR(d) (((d) & MENU_BAR) != 0)

/* A bar item only needs a little padding around its text. A drop-down item needs room
 * for the check mark and for the accelerator, measured apart from the text. */
static void measureMenuItem(MEASUREITEMSTRUCT* mi)
{
	HDC dc = GetDC(hMain);
	HGDIOBJ old = SelectObject(dc, fUi);
	const wchar_t *t, *tab;
	SIZE s, a;

	if (mi->itemData == MENU_SEP)
	{
		mi->itemWidth = SC(80);
		mi->itemHeight = SC(7);
	}
	else
	{
		t = T(MENU_ID(mi->itemData));
		tab = wcschr(t, L'\t');
		GetTextExtentPoint32W(dc, t, tab ? (int)(tab - t) : (int)wcslen(t), &s);

		if (MENU_ISBAR(mi->itemData))
		{
			mi->itemWidth = (UINT)(s.cx + SC(20));
			mi->itemHeight = (UINT)(s.cy + SC(8));
		}
		else
		{
			int w = SC(28) + s.cx + SC(24) + SC(16);
			if (tab) { GetTextExtentPoint32W(dc, tab + 1, (int)wcslen(tab + 1), &a); w += a.cx; }
			mi->itemWidth = (UINT)w;
			mi->itemHeight = (UINT)(s.cy + SC(6));
		}
	}

	SelectObject(dc, old);
	ReleaseDC(hMain, dc);
}

static void drawMenuItem(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	RECT r = d->rcItem;
	int hot = (d->itemState & (ODS_SELECTED | ODS_HOTLIGHT)) != 0;
	HBRUSH bg;
	HGDIOBJ of;
	const wchar_t *t, *tab;

	if (d->itemData == MENU_SEP)
	{
		HPEN pen = penOf(PAL.frame, 1);
		HGDIOBJ op;
		FillRect(dc, &r, brMenu);
		op = SelectObject(dc, pen);
		MoveToEx(dc, r.left + SC(8), (r.top + r.bottom) / 2, NULL);
		LineTo(dc, r.right - SC(8), (r.top + r.bottom) / 2);
		SelectObject(dc, op);
		DeleteObject(pen);
		return;
	}

	bg = hot ? CreateSolidBrush(PAL.menuHot) : brMenu;
	FillRect(dc, &r, bg);
	if (hot) DeleteObject(bg);

	t = T(MENU_ID(d->itemData));
	of = SelectObject(dc, fUi);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, hot ? PAL.menuHotText : PAL.menuText);

	if (MENU_ISBAR(d->itemData))
	{
		DrawTextW(dc, t, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		SelectObject(dc, of);
		return;
	}

	if (d->itemState & ODS_CHECKED)
	{
		RECT c = r;
		c.left += SC(8);
		c.right = c.left + SC(14);
		DrawTextW(dc, L"\x2713", 1, &c, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}

	tab = wcschr(t, L'\t');
	if (tab)
	{
		RECT left = r, right = r;
		left.left += SC(28);
		right.right -= SC(16);
		DrawTextW(dc, t, (int)(tab - t), &left, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		SetTextColor(dc, hot ? PAL.menuHotText : PAL.hint);
		DrawTextW(dc, tab + 1, -1, &right, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
	}
	else
	{
		RECT c = r;
		c.left += SC(28);
		DrawTextW(dc, t, -1, &c, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}
	SelectObject(dc, of);
}

static void buildMenu(HWND hwnd)
{
	HMENU file = CreatePopupMenu(), lang = CreatePopupMenu(), help = CreatePopupMenu();
	HMENU bar = CreateMenu();
	MENUINFO mi;

	AppendMenuW(file, MF_OWNERDRAW, IDM_GENERATE, (LPCWSTR)(ULONG_PTR)S_MENU_GENERATE);
	AppendMenuW(file, MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCWSTR)MENU_SEP);
	AppendMenuW(file, MF_OWNERDRAW, IDM_COPY, (LPCWSTR)(ULONG_PTR)S_MENU_COPY);
	AppendMenuW(file, MF_OWNERDRAW, IDM_COPYALL, (LPCWSTR)(ULONG_PTR)S_MENU_COPYALL);
	AppendMenuW(file, MF_OWNERDRAW, IDM_SAVE, (LPCWSTR)(ULONG_PTR)S_MENU_SAVE);
	AppendMenuW(file, MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCWSTR)MENU_SEP);
	AppendMenuW(file, MF_OWNERDRAW, IDM_EXIT, (LPCWSTR)(ULONG_PTR)S_MENU_EXIT);

	AppendMenuW(lang, MF_OWNERDRAW | (gLang == LANG_EN ? MF_CHECKED : 0), IDM_LANG_EN, (LPCWSTR)(ULONG_PTR)S_MENU_EN);
	AppendMenuW(lang, MF_OWNERDRAW | (gLang == LANG_JP ? MF_CHECKED : 0), IDM_LANG_JP, (LPCWSTR)(ULONG_PTR)S_MENU_JP);
	AppendMenuW(lang, MF_OWNERDRAW | (gLang == LANG_ES ? MF_CHECKED : 0), IDM_LANG_ES, (LPCWSTR)(ULONG_PTR)S_MENU_ES);

	AppendMenuW(help, MF_OWNERDRAW, IDM_HELP, (LPCWSTR)(ULONG_PTR)S_MENU_HELPTOPICS);
	AppendMenuW(help, MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCWSTR)MENU_SEP);
	AppendMenuW(help, MF_OWNERDRAW, IDM_ABOUT, (LPCWSTR)(ULONG_PTR)S_MENU_ABOUT);

	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)file, (LPCWSTR)(ULONG_PTR)(S_MENU_FILE | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)lang, (LPCWSTR)(ULONG_PTR)(S_MENU_LANG | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)help, (LPCWSTR)(ULONG_PTR)(S_MENU_HELP | MENU_BAR));

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
	mi.hbrBack = brMenu;
	SetMenuInfo(bar, &mi);

	SetMenu(hwnd, bar);
	if (hMenu) DestroyMenu(hMenu);
	hMenu = bar;
	DrawMenuBar(hwnd);
}

/* ------------------------------------------------------------------ tooltips */

static void setTip(HWND tool, int stringId, UINT msg)
{
	TOOLINFOW ti;

	if (!hTip || !tool) return;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.hwnd = GetParent(tool);
	ti.uId = (UINT_PTR)tool;
	ti.lpszText = (LPWSTR)T(stringId);
	SendMessageW(hTip, msg, 0, (LPARAM)&ti);
}

static void allTips(UINT msg)
{
	setTip(hAmbig, S_TIP_AMBIG, msg);     setTip(hAmbigLbl, S_TIP_AMBIG, msg);
	setTip(hInfoAmbig, S_TIP_AMBIG, msg);
	setTip(hClear, S_TIP_CLEAR, msg);     setTip(hClearLbl, S_TIP_CLEAR, msg);
	setTip(hInfoClear, S_TIP_CLEAR, msg);
	setTip(hGenerate, S_TIP_GENERATE, msg);
	setTip(hCopyAll, S_TIP_COPYALL, msg);
	setTip(hSave, S_TIP_SAVE, msg);
}

/* ------------------------------------------------------------------ the list */

static void invalidateRow(int row)
{
	RECT r;

	if (row < 0) return;
	if (SendMessageW(hList, LB_GETITEMRECT, (WPARAM)row, (LPARAM)&r) != LB_ERR)
		InvalidateRect(hList, &r, FALSE);
}

static int rowAt(LPARAM lp, int* onIcon)
{
	RECT rc;
	DWORD hit = (DWORD)SendMessageW(hList, LB_ITEMFROMPOINT, 0, lp);
	int row = (int)LOWORD(hit);

	*onIcon = 0;
	if (HIWORD(hit) || row >= gPwN) return -1;
	GetClientRect(hList, &rc);
	*onIcon = (int)(short)LOWORD(lp) >= rc.right - SC(ICON_ZONE);
	return row;
}

static LRESULT CALLBACK listProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref)
{
	int row, onIcon;

	(void)id; (void)ref;

	switch (msg)
	{
	case WM_MOUSEMOVE:
		row = rowAt(lp, &onIcon);
		if (row != gHotRow || onIcon != gHotIcon)
		{
			int was = gHotRow;
			gHotRow = row;
			gHotIcon = onIcon;
			invalidateRow(was);
			invalidateRow(row);
		}
		if (!gTrackingList)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
			TrackMouseEvent(&tme);
			gTrackingList = 1;
		}
		break;

	case WM_MOUSELEAVE:
		gTrackingList = 0;
		row = gHotRow;
		gHotRow = -1;
		gHotIcon = 0;
		invalidateRow(row);
		break;

	case WM_SETCURSOR:
		if (gHotIcon && gHotRow >= 0) { SetCursor(LoadCursorW(NULL, IDC_HAND)); return TRUE; }
		break;

	case WM_LBUTTONDOWN:
		row = rowAt(lp, &onIcon);
		if (row >= 0 && onIcon)
		{
			SendMessageW(hwnd, LB_SETCURSEL, (WPARAM)row, 0);
			SetFocus(hwnd);
			copyRow(row);
			return 0;
		}
		break;

	case WM_GETDLGCODE:
		/* Enter copies the selected row instead of pressing the default button */
		if (lp && ((MSG*)lp)->message == WM_KEYDOWN && ((MSG*)lp)->wParam == VK_RETURN)
			return DLGC_WANTMESSAGE | DefSubclassProc(hwnd, msg, wp, lp);
		break;

	case WM_KEYDOWN:
		if (wp == VK_RETURN) { copyRow((int)SendMessageW(hwnd, LB_GETCURSEL, 0, 0)); return 0; }
		break;

	case WM_NCDESTROY:
		RemoveWindowSubclass(hwnd, listProc, 1);
		break;
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}

/* ------------------------------------------------------------------ building the window */

static HWND mk(const wchar_t* cls, const wchar_t* text, DWORD style, DWORD ex,
               int x, int y, int w, int h, int id, HFONT font)
{
	HWND c = CreateWindowExW(ex, cls, text, WS_CHILD | WS_VISIBLE | style,
	                         SC(x), SC(y), SC(w), SC(h), hMain, (HMENU)(INT_PTR)id, gInst, NULL);
	SendMessageW(c, WM_SETFONT, (WPARAM)(font ? font : fUi), TRUE);
	return c;
}

static HWND mkSpin(HWND buddy, int id, int lo, int hi, int pos)
{
	HWND ud = CreateWindowExW(0, UPDOWN_CLASSW, NULL,
	                          WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT |
	                          UDS_ARROWKEYS | UDS_NOTHOUSANDS | UDS_HOTTRACK,
	                          0, 0, 0, 0, hMain, (HMENU)(INT_PTR)id, gInst, NULL);
	SendMessageW(ud, UDM_SETBUDDY, (WPARAM)buddy, 0);
	SendMessageW(ud, UDM_SETRANGE32, (WPARAM)lo, (LPARAM)hi);
	SendMessageW(ud, UDM_SETPOS32, 0, (LPARAM)pos);
	return ud;
}

/* The option labels are separate STATICs, not the check boxes' own text: a themed check
 * box draws its text with the theme and may ignore the colour asked for in
 * WM_CTLCOLORSTATIC; a STATIC always honours it. Clicking a label toggles its box. Each
 * label is as wide as its text, and the info badge follows it. */
static void placeOptionLabels(void)
{
	HDC dc = GetDC(hMain);
	HGDIOBJ of = SelectObject(dc, fUi);
	HWND lbl[2] = { hAmbigLbl, hClearLbl }, info[2] = { hInfoAmbig, hInfoClear };
	int ys[2] = { Y_ROW2, Y_ROW3 }, i;

	for (i = 0; i < 2; i++)
	{
		wchar_t t[256];
		SIZE s;
		int x = SC(LBL1_X + 24);

		GetWindowTextW(lbl[i], t, 256);
		GetTextExtentPoint32W(dc, t, (int)wcslen(t), &s);
		SetWindowPos(lbl[i], NULL, x, SC(ys[i]), s.cx + SC(4), SC(22), SWP_NOZORDER);
		SetWindowPos(info[i], NULL, x + s.cx + SC(10), SC(ys[i] + 3), SC(16), SC(16), SWP_NOZORDER);
	}

	SelectObject(dc, of);
	ReleaseDC(hMain, dc);
}

static void buildControls(void)
{
	const int bx = DESIGN_W - PAD - GAP - BTN_W;
	wchar_t num[16];

	hGrpSet  = mk(L"STATIC", L"", SS_OWNERDRAW | WS_CLIPSIBLINGS, 0, PAD, Y_SET, DESIGN_W - 2 * PAD, SET_H, IDC_GRP_SET, fBold);

	hLblLen = mk(L"STATIC", T(S_LBL_LENGTH), SS_CENTERIMAGE, 0, LBL1_X, Y_ROW1, LBL1_W, 28, IDC_LBL_LEN, fUi);
	_snwprintf(num, 16, L"%d", gLen); num[15] = 0;
	hLen = mk(L"EDIT", num, ES_NUMBER | ES_CENTER | WS_TABSTOP, 0, EDIT1_X, Y_ROW1, FIELD_W, 28, IDC_LEN, fUi);
	SendMessageW(hLen, EM_SETLIMITTEXT, 3, 0);
	hLenUd = mkSpin(hLen, IDC_LEN_UD, PW_MIN_LEN, PW_MAX_LEN, gLen);

	hLblCnt = mk(L"STATIC", T(S_LBL_COUNT), SS_CENTERIMAGE, 0, LBL2_X, Y_ROW1, LBL2_W, 28, IDC_LBL_CNT, fUi);
	_snwprintf(num, 16, L"%d", gCount); num[15] = 0;
	hCnt = mk(L"EDIT", num, ES_NUMBER | ES_CENTER | WS_TABSTOP, 0, EDIT2_X, Y_ROW1, FIELD_W, 28, IDC_CNT, fUi);
	SendMessageW(hCnt, EM_SETLIMITTEXT, 2, 0);
	hCntUd = mkSpin(hCnt, IDC_CNT_UD, 1, MAX_COUNT, gCount);

	hAmbig = mk(L"BUTTON", L"", BS_AUTOCHECKBOX | WS_TABSTOP, 0, LBL1_X, Y_ROW2, 18, 22, IDC_AMBIG, fUi);
	hAmbigLbl = mk(L"STATIC", T(S_OPT_AMBIG), SS_NOTIFY | SS_CENTERIMAGE, 0, LBL1_X + 24, Y_ROW2, 300, 22, IDC_AMBIG_LBL, fUi);
	hInfoAmbig = mk(L"STATIC", L"", SS_OWNERDRAW | SS_NOTIFY, 0, 0, 0, 16, 16, IDC_INFO_AMBIG, fUi);

	hClear = mk(L"BUTTON", L"", BS_AUTOCHECKBOX | WS_TABSTOP, 0, LBL1_X, Y_ROW3, 18, 22, IDC_CLEAR, fUi);
	hClearLbl = mk(L"STATIC", T(S_OPT_CLEAR), SS_NOTIFY | SS_CENTERIMAGE, 0, LBL1_X + 24, Y_ROW3, 300, 22, IDC_CLEAR_LBL, fUi);
	hInfoClear = mk(L"STATIC", L"", SS_OWNERDRAW | SS_NOTIFY, 0, 0, 0, 16, 16, IDC_INFO_CLEAR, fUi);

	hStrength = mk(L"STATIC", L"", SS_OWNERDRAW, 0, LBL1_X, Y_ROW4, DESIGN_W - 2 * (PAD + GAP), 22, IDC_STRENGTH, fUi);

	hGenerate = mk(L"BUTTON", T(S_GENERATE), WS_TABSTOP | BS_DEFPUSHBUTTON, 0,
	               bx, Y_SET + 40, BTN_W, BTN_H, IDC_GENERATE, fBold);

	hGrpList = mk(L"STATIC", L"", SS_OWNERDRAW | WS_CLIPSIBLINGS, 0, PAD, Y_LIST, DESIGN_W - 2 * PAD, 220, IDC_GRP_LIST, fBold);
	hSave    = mk(L"STATIC", L"", SS_OWNERDRAW | SS_NOTIFY, 0, 0, 0, BADGE_W, BADGE_W, IDC_SAVE, fUi);
	hCopyAll = mk(L"STATIC", L"", SS_OWNERDRAW | SS_NOTIFY, 0, 0, 0, BADGE_W, BADGE_W, IDC_COPYALL, fUi);

	hList = mk(L"LISTBOX", NULL, LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
	           WS_VSCROLL | WS_TABSTOP, 0, PAD + GAP, Y_LIST + 30, DESIGN_W - 2 * (PAD + GAP), 174, IDC_LIST, fUi);
	SetWindowSubclass(hList, listProc, 1, 0);

	hStatus  = mk(L"STATIC", T(S_ST_READY), SS_LEFTNOWORDWRAP | SS_ENDELLIPSIS | SS_CENTERIMAGE, 0,
	              PAD + 2, 0, 300, 22, IDC_STATUS, fUi);
	hVersion = mk(L"STATIC", L"v" APP_VERSION, SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, VER_W, 22, IDC_VERSION, fUi);
	lstrcpynW(gStatusText, T(S_ST_READY), 512);

	CheckDlgButton(hMain, IDC_AMBIG, gAvoid ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(hMain, IDC_CLEAR, gClearClip ? BST_CHECKED : BST_UNCHECKED);
	placeOptionLabels();

	hTip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
	                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hMain, NULL, gInst, NULL);
	SendMessageW(hTip, TTM_SETMAXTIPWIDTH, 0, SC(420));
	SendMessageW(hTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);
	SendMessageW(hTip, WM_SETFONT, (WPARAM)fUi, TRUE);
	allTips(TTM_ADDTOOLW);
}

static void layout(HWND hwnd)
{
	RECT rc;
	int w, h, pad, gap, top, gh, by;

	if (!hList || !hStatus) return;

	GetClientRect(hwnd, &rc);
	w = rc.right;
	h = rc.bottom;
	pad = SC(PAD);
	gap = SC(GAP);
	top = SC(Y_LIST);
	if (w < SC(520) || h < top + SC(140)) return;

	SetWindowPos(hGrpSet, NULL, 0, 0, w - 2 * pad, SC(SET_H), SWP_NOMOVE | SWP_NOZORDER);
	SetWindowPos(hGenerate, NULL, w - pad - gap - SC(BTN_W), SC(Y_SET + 40), SC(BTN_W), SC(BTN_H), SWP_NOZORDER);
	SetWindowPos(hStrength, NULL, 0, 0, w - 2 * (pad + gap), SC(22), SWP_NOMOVE | SWP_NOZORDER);

	gh = h - top - SC(46);
	SetWindowPos(hGrpList, NULL, pad, top, w - 2 * pad, gh, SWP_NOZORDER);

	/* centred on the frame line, which drawGroupBox puts half a title height below the
	 * top of the group, and clear of the list's scrollbar on the right */
	by = top + SC(9) - SC(BADGE_W) / 2;
	SetWindowPos(hCopyAll, NULL, w - pad - gap - SC(SCROLL_W) - SC(BADGE_W), by, SC(BADGE_W), SC(BADGE_W), SWP_NOZORDER);
	SetWindowPos(hSave, NULL, w - pad - gap - SC(SCROLL_W) - 2 * SC(BADGE_W) - SC(BADGE_GAP), by,
	             SC(BADGE_W), SC(BADGE_W), SWP_NOZORDER);

	SetWindowPos(hList, NULL, pad + gap, top + SC(30), w - 2 * (pad + gap), gh - SC(46), SWP_NOZORDER);
	SetWindowPos(hStatus, NULL, pad + SC(2), h - SC(32), w - 2 * pad - SC(VER_W + 10), SC(24), SWP_NOZORDER);
	SetWindowPos(hVersion, NULL, w - pad - SC(VER_W), h - SC(32), SC(VER_W), SC(24), SWP_NOZORDER);

	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

/* ------------------------------------------------------------------ help window */

static LRESULT CALLBACK helpProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_ERASEBKGND:
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect((HDC)wp, &rc, brBody);
		return 1;
	}
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wp, PAL.editBg);
		SetTextColor((HDC)wp, PAL.editText);
		return (LRESULT)brEdit;
	case WM_SIZE:
	{
		HWND e = GetDlgItem(hwnd, IDC_HELP_TEXT), ok = GetDlgItem(hwnd, IDOK);
		RECT rc;
		GetClientRect(hwnd, &rc);
		if (e)  SetWindowPos(e, NULL, SC(14), SC(14), rc.right - SC(28), rc.bottom - SC(70), SWP_NOZORDER);
		if (ok) SetWindowPos(ok, NULL, rc.right - SC(118), rc.bottom - SC(46), SC(104), SC(32), SWP_NOZORDER);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}
	case WM_COMMAND:
		/* IDCANCEL is what Esc becomes through IsDialogMessage */
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) { DestroyWindow(hwnd); return 0; }
		break;
	case WM_CLOSE:   DestroyWindow(hwnd); return 0;
	case WM_DESTROY: hHelpWnd = NULL;     return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void showHelp(HWND owner)
{
	static int registered = 0;
	HWND e, ok;
	RECT r;

	if (hHelpWnd) { SetForegroundWindow(hHelpWnd); return; }

	if (!registered)
	{
		WNDCLASSW wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = helpProc;
		wc.hInstance = gInst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = L"RandomPassGenHelp";
		wc.hIcon = LoadIconW(gInst, MAKEINTRESOURCEW(IDI_APP));
		RegisterClassW(&wc);
		registered = 1;
	}

	GetWindowRect(owner, &r);
	hHelpWnd = CreateWindowExW(0, L"RandomPassGenHelp", T(S_HELP_TITLE), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	                           r.left + SC(40), r.top + SC(30), SC(680), SC(620), owner, NULL, gInst, NULL);

	e = CreateWindowExW(0, L"EDIT", T(S_HELP_BODY),
	                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
	                    0, 0, 10, 10, hHelpWnd, (HMENU)(INT_PTR)IDC_HELP_TEXT, gInst, NULL);
	SendMessageW(e, WM_SETFONT, (WPARAM)fUi, TRUE);
	SendMessageW(e, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(SC(8), SC(8)));

	ok = CreateWindowW(L"BUTTON", T(S_BTN_OK), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
	                   0, 0, 10, 10, hHelpWnd, (HMENU)(INT_PTR)IDOK, gInst, NULL);
	SendMessageW(ok, WM_SETFONT, (WPARAM)fUi, TRUE);

	SendMessageW(hHelpWnd, WM_SIZE, 0, 0);
	applyTheme(hHelpWnd);
	SetFocus(ok);
}

/* ------------------------------------------------------------------ about window */

/* Icon, name, version (from APP_VERSION), date, author, and what the program does.
 * The height is measured from the text, so it fits in all three languages. */

#define AB_M     24
#define AB_ICON  72
#define AB_TEXT  138

static int aboutBodyHeight(int width)
{
	HDC dc = GetDC(NULL);
	HGDIOBJ old = SelectObject(dc, fUi);
	RECT r = { 0, 0, width, 0 };

	DrawTextW(dc, T(S_ABOUT_BODY), -1, &r, DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
	SelectObject(dc, old);
	ReleaseDC(NULL, dc);
	return r.bottom - r.top;
}

static LRESULT CALLBACK aboutProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps);
		RECT rc, t;
		HGDIOBJ of;
		wchar_t line[128];
		int tx = SC(AB_M + AB_ICON + 18);

		GetClientRect(hwnd, &rc);
		FillRect(dc, &rc, brBody);
		if (icAbout) DrawIconEx(dc, SC(AB_M), SC(AB_M), icAbout, SC(AB_ICON), SC(AB_ICON), 0, NULL, DI_NORMAL);

		SetBkMode(dc, TRANSPARENT);
		of = SelectObject(dc, fTitle);
		SetTextColor(dc, PAL.group);
		TextOutW(dc, tx, SC(AB_M - 2), T(S_ABOUT_NAME), (int)wcslen(T(S_ABOUT_NAME)));

		SelectObject(dc, fUi);
		SetTextColor(dc, PAL.hint);
		/* built here, so it can never disagree with the executable that prints it */
		_snwprintf(line, 128, T(S_ABOUT_VERSION), APP_VERSION); line[127] = 0;
		TextOutW(dc, tx + SC(2), SC(AB_M + 36), line, (int)wcslen(line));
		TextOutW(dc, tx + SC(2), SC(AB_M + 58), T(S_ABOUT_CREATED), (int)wcslen(T(S_ABOUT_CREATED)));

		SelectObject(dc, fBold);
		SetTextColor(dc, PAL.text);
		_snwprintf(line, 128, T(S_ABOUT_AUTHOR), APP_AUTHOR); line[127] = 0;
		TextOutW(dc, tx + SC(2), SC(AB_M + 84), line, (int)wcslen(line));

		SelectObject(dc, fUi);
		SetTextColor(dc, PAL.text);
		SetRect(&t, SC(AB_M), SC(AB_TEXT), rc.right - SC(AB_M), rc.bottom);
		DrawTextW(dc, T(S_ABOUT_BODY), -1, &t, DT_WORDBREAK | DT_NOPREFIX);

		SelectObject(dc, of);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wp, TRANSPARENT);
		return (LRESULT)brBody;
	case WM_COMMAND:
		if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) { DestroyWindow(hwnd); return 0; }
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		hAboutWnd = NULL;
		EnableWindow(hMain, TRUE);
		SetForegroundWindow(hMain);
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void showAbout(HWND owner)
{
	static int registered = 0;
	HWND ok;
	RECT r, want, c;
	int cw = 520, bodyH, ch;

	if (hAboutWnd) { SetForegroundWindow(hAboutWnd); return; }

	if (!registered)
	{
		WNDCLASSW wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = aboutProc;
		wc.hInstance = gInst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = L"RandomPassGenAbout";
		wc.hIcon = LoadIconW(gInst, MAKEINTRESOURCEW(IDI_APP));
		RegisterClassW(&wc);
		registered = 1;
	}

	bodyH = aboutBodyHeight(SC(cw - 2 * AB_M));
	ch = SC(AB_TEXT) + bodyH + SC(20) + SC(52);

	SetRect(&want, 0, 0, SC(cw), ch);
	AdjustWindowRectEx(&want, WS_POPUPWINDOW | WS_CAPTION, FALSE, WS_EX_DLGMODALFRAME);

	GetWindowRect(owner, &r);
	hAboutWnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"RandomPassGenAbout", T(S_ABOUT_TITLE),
	                            WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE,
	                            r.left + ((r.right - r.left) - (want.right - want.left)) / 2,
	                            r.top + ((r.bottom - r.top) - (want.bottom - want.top)) / 3,
	                            want.right - want.left, want.bottom - want.top, owner, NULL, gInst, NULL);

	GetClientRect(hAboutWnd, &c);
	ok = CreateWindowW(L"BUTTON", T(S_BTN_OK), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
	                   c.right - SC(128), c.bottom - SC(46), SC(104), SC(32),
	                   hAboutWnd, (HMENU)(INT_PTR)IDOK, gInst, NULL);
	SendMessageW(ok, WM_SETFONT, (WPARAM)fUi, TRUE);

	applyTheme(hAboutWnd);
	EnableWindow(owner, FALSE);
	SetFocus(ok);
}

/* ------------------------------------------------------------------ fonts and language */

static HFONT uiFont(int height, int weight)
{
	return CreateFontW(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
	                   OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS,
	                   (gLang == LANG_JP) ? L"Yu Gothic UI" : L"Segoe UI");
}

/* The fonts a control still holds are deleted only after it has been given the new ones. */
static HFONT fOldUi, fOldBold, fOldTitle, fOldSub, fOldBadge;

static void makeFonts(void)
{
	fOldUi = fUi; fOldBold = fBold; fOldTitle = fTitle; fOldSub = fSub; fOldBadge = fBadge;

	fUi    = uiFont(-SC(14), FW_NORMAL);
	fBold  = uiFont(-SC(14), FW_SEMIBOLD);
	fTitle = uiFont(-SC(25), FW_SEMIBOLD);
	fSub   = uiFont(-SC(14), FW_NORMAL);
	fBadge = uiFont(-SC(11), FW_BOLD);

	if (!fPw)
		fPw = CreateFontW(-SC(17), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
		                  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN,
		                  L"Consolas");
}

static void dropOldFonts(void)
{
	if (fOldUi)    DeleteObject(fOldUi);
	if (fOldBold)  DeleteObject(fOldBold);
	if (fOldTitle) DeleteObject(fOldTitle);
	if (fOldSub)   DeleteObject(fOldSub);
	if (fOldBadge) DeleteObject(fOldBadge);
	fOldUi = fOldBold = fOldTitle = fOldSub = fOldBadge = NULL;
}

static void setFontOf(HWND h, HFONT f) { if (h) SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE); }

static void applyLanguage(HWND hwnd)
{
	makeFonts();

	SetWindowTextW(hwnd, T(S_WINDOW_TITLE));
	SetWindowTextW(hLblLen, T(S_LBL_LENGTH));
	SetWindowTextW(hLblCnt, T(S_LBL_COUNT));
	SetWindowTextW(hAmbigLbl, T(S_OPT_AMBIG));
	SetWindowTextW(hClearLbl, T(S_OPT_CLEAR));
	SetWindowTextW(hGenerate, T(S_GENERATE));

	setFontOf(hLblLen, fUi);   setFontOf(hLen, fUi);
	setFontOf(hLblCnt, fUi);   setFontOf(hCnt, fUi);
	setFontOf(hAmbigLbl, fUi); setFontOf(hClearLbl, fUi);
	setFontOf(hGenerate, fBold);
	setFontOf(hStatus, fUi);   setFontOf(hVersion, fUi);
	setFontOf(hList, fUi);     setFontOf(hTip, fUi);

	placeOptionLabels();
	allTips(TTM_UPDATETIPTEXTW);
	buildMenu(hwnd);

	/* the status line is re-said in the new language when there is something to say */
	if (gPwN > 0)
	{
		if (gPwN == 1) setStatusF(ST_OK, S_ST_GENERATED_ONE, gPwLen);
		else           setStatusF(ST_OK, S_ST_GENERATED, gPwN, gPwLen);
	}
	else setStatus(ST_PLAIN, T(S_ST_READY));

	if (hHelpWnd)  { DestroyWindow(hHelpWnd); showHelp(hwnd); }
	if (hAboutWnd) { DestroyWindow(hAboutWnd); showAbout(hwnd); }

	dropOldFonts();
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

static void setLanguage(HWND hwnd, int lang)
{
	if (lang == gLang) return;
	gLang = lang;
	iniSetInt(L"Language", lang);
	applyLanguage(hwnd);
}

/* ------------------------------------------------------------------ window */

static void saveSettings(HWND hwnd)
{
	WINDOWPLACEMENT wp;

	if (gNoSave) return;

	readSettings();
	iniSetInt(L"Length", gLen);
	iniSetInt(L"Count", gCount);
	iniSetInt(L"AvoidAmbiguous", gAvoid);
	iniSetInt(L"ClearClipboard", gClearClip);

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(hwnd, &wp)) return;
	iniSetInt(L"WndX", wp.rcNormalPosition.left);
	iniSetInt(L"WndY", wp.rcNormalPosition.top);
	iniSetInt(L"WndW", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	iniSetInt(L"WndH", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
	iniSetInt(L"WndMax", wp.showCmd == SW_SHOWMAXIMIZED);
}

static void toggleCheck(int id)
{
	CheckDlgButton(hMain, id, IsDlgButtonChecked(hMain, id) == BST_CHECKED ? BST_UNCHECKED : BST_CHECKED);
	SendMessageW(hMain, WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)GetDlgItem(hMain, id));
}

static void setHotBadge(int id)
{
	if (gHotBadge == id) return;
	if (gHotBadge) InvalidateRect(GetDlgItem(hMain, gHotBadge), NULL, TRUE);
	gHotBadge = id;
	if (id) InvalidateRect(GetDlgItem(hMain, id), NULL, TRUE);
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps);
		RECT rc;

		GetClientRect(hwnd, &rc);
		rc.top = SC(BAND_H);
		FillRect(dc, &rc, brBody);
		paintBand(hwnd, dc);
		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_ERASEBKGND:
		return 1;

	case WM_MEASUREITEM:
	{
		MEASUREITEMSTRUCT* mi = (MEASUREITEMSTRUCT*)lp;
		if (mi->CtlType == ODT_MENU)    { measureMenuItem(mi); return TRUE; }
		if (mi->CtlType == ODT_LISTBOX) { mi->itemHeight = (UINT)SC(ROW_LH); return TRUE; }
		return 0;
	}

	case WM_DRAWITEM:
	{
		const DRAWITEMSTRUCT* d = (const DRAWITEMSTRUCT*)lp;

		if (d->CtlType == ODT_MENU) { drawMenuItem(d); return TRUE; }
		switch (d->CtlID)
		{
		case IDC_LIST:       drawRow(d);                      return TRUE;
		case IDC_GRP_SET:    drawGroupBox(d, T(S_GRP_SET));   return TRUE;
		case IDC_GRP_LIST:   drawGroupBox(d, T(S_GRP_LIST));  return TRUE;
		case IDC_STRENGTH:   drawStrength(d);                 return TRUE;
		case IDC_INFO_AMBIG:
		case IDC_INFO_CLEAR: drawInfoBadge(d);                return TRUE;
		case IDC_COPYALL:
		case IDC_SAVE:       drawFrameBadge(d);               return TRUE;
		}
		return 0;
	}

	case WM_CTLCOLORSTATIC:
	{
		HDC dc = (HDC)wp;
		int id = GetDlgCtrlID((HWND)lp);

		SetBkMode(dc, TRANSPARENT);
		if (id == IDC_VERSION) SetTextColor(dc, PAL.hint);
		else if (id == IDC_STATUS)
			SetTextColor(dc, gStatusKind == ST_ERR ? PAL.err : gStatusKind == ST_WARN ? PAL.warn :
			                 gStatusKind == ST_OK ? PAL.ok : PAL.hint);
		else SetTextColor(dc, PAL.text);
		return (LRESULT)brBody;
	}

	case WM_CTLCOLORBTN:
		SetBkMode((HDC)wp, TRANSPARENT);
		return (LRESULT)brBody;

	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wp, PAL.editBg);
		SetTextColor((HDC)wp, PAL.editText);
		return (LRESULT)brEdit;

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wp, PAL.editBg);
		return (LRESULT)brEdit;

	case WM_SETTINGCHANGE:
		if (lp && 0 == lstrcmpiW((const wchar_t*)lp, L"ImmersiveColorSet")) refreshTheme();
		return 0;

	case WM_SIZE:
		layout(hwnd);
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lp)->ptMinTrackSize.x = SC(600);
		((MINMAXINFO*)lp)->ptMinTrackSize.y = SC(560);
		return 0;

	case WM_TIMER:
		if (wp == TMR_CLIP) { clipTick(); return 0; }
		if (wp == TMR_MARK)
		{
			KillTimer(hwnd, TMR_MARK);
			gMarkRow = -1;
			InvalidateRect(hList, NULL, FALSE);
			InvalidateRect(hCopyAll, NULL, TRUE);
			return 0;
		}
		if (wp == TMR_REGEN) { KillTimer(hwnd, TMR_REGEN); doGenerate(); return 0; }
		return 0;

	/* the frame badges light up and show a hand under the pointer; the parent gets
	 * WM_MOUSEMOVE as soon as the pointer leaves one */
	case WM_SETCURSOR:
		if ((HWND)wp == hCopyAll || (HWND)wp == hSave)
		{
			setHotBadge(GetDlgCtrlID((HWND)wp));
			SetCursor(LoadCursorW(NULL, IDC_HAND));
			return TRUE;
		}
		break;

	case WM_MOUSEMOVE:
		setHotBadge(0);
		break;

	case WM_COMMAND:
	{
		int id = LOWORD(wp), code = HIWORD(wp);

		if (id == IDC_COPYALL && code == STN_CLICKED) { copyAll(); return 0; }
		if (id == IDC_SAVE && code == STN_CLICKED)    { saveAs(hwnd); return 0; }
		if (id == IDC_AMBIG_LBL && code == STN_CLICKED) { toggleCheck(IDC_AMBIG); return 0; }
		if (id == IDC_CLEAR_LBL && code == STN_CLICKED) { toggleCheck(IDC_CLEAR); return 0; }
		if (id == IDC_LIST && code == LBN_DBLCLK)
		{
			copyRow((int)SendMessageW(hList, LB_GETCURSEL, 0, 0));
			return 0;
		}

		/* a new length or count, or the ambiguous option: a new set, shortly after the
		 * last keystroke so "20" is not generated as "2" first */
		if ((id == IDC_LEN || id == IDC_CNT) && code == EN_CHANGE)
		{
			InvalidateRect(hStrength, NULL, TRUE);
			if (hList) SetTimer(hwnd, TMR_REGEN, 400, NULL);
			return 0;
		}
		if ((id == IDC_LEN || id == IDC_CNT) && code == EN_KILLFOCUS)
		{
			readSettings();
			normaliseField(id == IDC_LEN ? hLen : hCnt, id == IDC_LEN ? gLen : gCount);
			return 0;
		}
		if (id == IDC_AMBIG && code == BN_CLICKED)
		{
			InvalidateRect(hStrength, NULL, TRUE);
			doGenerate();
			return 0;
		}
		if (id == IDC_CLEAR && code == BN_CLICKED) { readSettings(); return 0; }

		switch (id)
		{
		case IDOK:               /* Enter outside the list: the default button */
		case IDC_GENERATE:
		case IDM_GENERATE: KillTimer(hwnd, TMR_REGEN); doGenerate(); return 0;
		case IDM_COPY:     copySelected();             return 0;
		case IDM_COPYALL:  copyAll();                  return 0;
		case IDM_SAVE:     saveAs(hwnd);               return 0;
		case IDM_HELP:     showHelp(hwnd);             return 0;
		case IDM_ABOUT:    showAbout(hwnd);            return 0;
		case IDM_LANG_EN:  setLanguage(hwnd, LANG_EN); return 0;
		case IDM_LANG_JP:  setLanguage(hwnd, LANG_JP); return 0;
		case IDM_LANG_ES:  setLanguage(hwnd, LANG_ES); return 0;
		case IDM_EXIT:     DestroyWindow(hwnd);        return 0;
		}
		return 0;
	}

	case WM_DESTROY:
		/* a password still waiting to be wiped goes now */
		if (gClipLeft > 0) wipeOurClipboard();
		saveSettings(hwnd);
		wipePasswords();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wp, lp);
}

/* ------------------------------------------------------------------ command line */

static int writeAuto(const char* path, int len, int count, int avoid)
{
	FILE* f = fopen(path, "wb");
	char pw[PW_MAX_LEN + 1];
	int i;

	if (!f) return 2;
	for (i = 0; i < count; i++)
	{
		if (!pw_generate(len, avoid, pw)) { fclose(f); return 3; }
		fputs(pw, f);
		fputs("\r\n", f);
	}
	SecureZeroMemory(pw, sizeof(pw));
	return fclose(f) == 0 ? 0 : 2;
}

/* Draws the window (or the Help / About box in front of it) into a 32-bit BMP. */
static int saveShot(HWND w, const char* path)
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
		FILE* f = fopen(path, "wb");
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

static void pump(DWORD ms)
{
	DWORD until = GetTickCount() + ms;
	MSG m;

	while ((LONG)(until - GetTickCount()) > 0)
	{
		while (PeekMessageW(&m, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&m); DispatchMessageW(&m); }
		Sleep(10);
	}
}

/* ------------------------------------------------------------------ entry */

static void readDpi(void)
{
	HDC dc = GetDC(NULL);

	if (dc)
	{
		int d = GetDeviceCaps(dc, LOGPIXELSX);
		if (d >= 96) gDpi = d;
		ReleaseDC(NULL, dc);
	}
}

/* One taskbar button for the pinned icon and the running window. Looked up at run time,
 * so the program still starts where the call does not exist. */
static void setAppId(void)
{
	typedef HRESULT (WINAPI *PSetAppId)(PCWSTR);
	HMODULE sh = GetModuleHandleW(L"shell32.dll");
	PSetAppId set = sh ? (PSetAppId)(void*)GetProcAddress(sh, "SetCurrentProcessExplicitAppUserModelID") : NULL;

	if (set) set(APP_AUMID);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prev, LPSTR cmdLine, int show)
{
	WNDCLASSW wc;
	MSG msg;
	HACCEL accel;
	INITCOMMONCONTROLSEX icc;
	RECT want;
	const char *outPath = NULL, *shotPath = NULL, *openWhat = NULL;
	int i, langArg = -1, ww, wh, wx, wy;
	int lenArg = -1, cntArg = -1, ambArg = 0;

	(void)prev; (void)cmdLine;

	gInst = inst;
	iniPath();

	for (i = 1; i < __argc; i++)
	{
		const char* a = __argv[i];

		if (a[0] != '/' && a[0] != '-') continue;
		a++;
		if      (!_strnicmp(a, "len:", 4))    lenArg = atoi(a + 4);
		else if (!_strnicmp(a, "count:", 6))  cntArg = atoi(a + 6);
		else if (!_stricmp(a, "ambiguous"))   ambArg = 1;
		else if (!_strnicmp(a, "out:", 4))    outPath = a + 4;
		else if (!_strnicmp(a, "shot:", 5))   shotPath = a + 5;
		else if (!_strnicmp(a, "open:", 5))   openWhat = a + 5;
		else if (!_stricmp(a, "theme:dark"))  gForceTheme = 1;
		else if (!_stricmp(a, "theme:light")) gForceTheme = 0;
		else if (!_stricmp(a, "lang:en"))     langArg = LANG_EN;
		else if (!_stricmp(a, "lang:jp"))     langArg = LANG_JP;
		else if (!_stricmp(a, "lang:es"))     langArg = LANG_ES;
	}

	/* passwords to a file, no window at all */
	if (outPath)
		return writeAuto(outPath,
		                 clampInt(lenArg > 0 ? lenArg : 20, PW_MIN_LEN, PW_MAX_LEN),
		                 clampInt(cntArg > 0 ? cntArg : 5, 1, AUTO_MAX),
		                 !ambArg);

	if (shotPath) gNoSave = 1;

	setAppId();
	readDpi();

	gLang  = clampInt(iniInt(L"Language", LANG_EN), 0, LANG_COUNT - 1);
	gLen   = clampInt(iniInt(L"Length", 20), PW_MIN_LEN, PW_MAX_LEN);
	gCount = clampInt(iniInt(L"Count", 5), 1, MAX_COUNT);
	gAvoid = iniInt(L"AvoidAmbiguous", 1) != 0;
	gClearClip = iniInt(L"ClearClipboard", 1) != 0;
	iniStr(L"LastFolder", gLastFolder, MAX_PATH);

	if (langArg >= 0) gLang = langArg;
	if (lenArg > 0)   gLen = clampInt(lenArg, PW_MIN_LEN, PW_MAX_LEN);
	if (cntArg > 0)   gCount = clampInt(cntArg, 1, MAX_COUNT);
	if (ambArg)       gAvoid = 0;

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_UPDOWN_CLASS | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	makeFonts();
	refreshTheme();

	/* not LR_SHARED: that returns the shared 32 px icon whatever size is asked for */
	icBand  = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, SC(ICON_SZ), SC(ICON_SZ), 0);
	icAbout = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, SC(72), SC(72), 0);

	ZeroMemory(&wc, sizeof(wc));
	wc.lpfnWndProc = wndProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"RandomPassGen";
	wc.hIcon = LoadIconW(inst, MAKEINTRESOURCEW(IDI_APP));
	RegisterClassW(&wc);

	/* the default size is the client the design needs: five rows in the list */
	SetRect(&want, 0, 0, SC(DESIGN_W), SC(Y_LIST + 30 + 5 * ROW_LH + 4 + 16 + 46));
	AdjustWindowRectEx(&want, WS_OVERLAPPEDWINDOW, TRUE, 0);
	ww = iniInt(L"WndW", want.right - want.left);
	wh = iniInt(L"WndH", want.bottom - want.top);
	wx = iniInt(L"WndX", CW_USEDEFAULT);
	wy = iniInt(L"WndY", CW_USEDEFAULT);
	if (ww < SC(600) || wh < SC(560) || shotPath)
	{
		ww = want.right - want.left;
		wh = want.bottom - want.top;
	}
	if (shotPath) wx = wy = 40;
	else if (wx != CW_USEDEFAULT)
	{
		RECT saved;
		SetRect(&saved, wx, wy, wx + ww, wy + wh);
		if (!MonitorFromRect(&saved, MONITOR_DEFAULTTONULL)) wx = wy = CW_USEDEFAULT;
	}

	hMain = CreateWindowExW(0, L"RandomPassGen", T(S_WINDOW_TITLE), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	                        wx, wy, ww, wh, NULL, NULL, inst, NULL);

	buildControls();
	buildMenu(hMain);
	refreshTheme();                 /* again, now that the window and the menu exist */
	layout(hMain);
	doGenerate();                   /* a fresh set on every start, like password_gen.py */

	accel = LoadAcceleratorsW(inst, MAKEINTRESOURCEW(IDI_APP));

	if (shotPath)
	{
		int rc;
		HWND target = hMain;

		ShowWindow(hMain, SW_SHOWNOACTIVATE);
		UpdateWindow(hMain);
		if (openWhat && !_stricmp(openWhat, "help"))  { showHelp(hMain);  target = hHelpWnd; }
		if (openWhat && !_stricmp(openWhat, "about")) { showAbout(hMain); target = hAboutWnd; }
		pump(600);
		rc = saveShot(target, shotPath);
		DestroyWindow(hMain);
		return rc;
	}

	ShowWindow(hMain, (iniInt(L"WndMax", 0) && show != SW_SHOWMINNOACTIVE) ? SW_SHOWMAXIMIZED : show);
	UpdateWindow(hMain);

	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		/* while About is up the main window is disabled, so its accelerators are not what
		 * the user is aiming at. Help and About get IsDialogMessage for Tab and Esc. */
		/* ... and only for the main window: Ctrl+C in the Help text copies that text */
		if (!hAboutWnd && (msg.hwnd == hMain || IsChild(hMain, msg.hwnd)) &&
		    TranslateAcceleratorW(hMain, accel, &msg)) continue;
		if (hAboutWnd && IsDialogMessageW(hAboutWnd, &msg)) continue;
		if (hHelpWnd && IsDialogMessageW(hHelpWnd, &msg)) continue;
		if (IsDialogMessageW(hMain, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}
