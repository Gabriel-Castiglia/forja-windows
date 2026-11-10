/**
  Wyrmhold -- the window.
  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 9, 2026
  SPDX-License-Identifier: GPL-2.0-only

  A Win32 front end for the engine in pwgen.h. One portable, statically linked executable:
  installed with its setup, or run portable. The settings it remembers go to
  %APPDATA%\Wyrmhold\wyrmhold.ini -- or, when a wyrmhold.ini sits beside the executable,
  to that one (portable use).

  The name: "wyrm" is the Old English word for dragon, "hold" a stronghold. A wyrmhold is
  the dragon's keep -- where a dragon guards its hoard, as the program guards passwords.

  What the window does:
    - three kinds of password -- characters, passphrase, PIN -- chosen with a segmented
      control; ready-made profiles for common cases (sites that refuse symbols, sites with
      a short maximum, Wi-Fi, PIN...) in a drop-down and in the menu;
    - characters: length slider, which classes to use (chips), which symbol set, leave out
      look-alikes, no repeated neighbours, start with a letter, characters to leave out,
      the user's own symbols; passphrase: words, separator, capitals, a digit; PIN: digits,
      no repeats, no runs;
    - a strength line with the exact entropy, a level and how long guessing would take;
    - an owner-drawn list of passwords: digits blue, symbols amber, a copy icon on every row
      that turns into a check mark once the row has been copied;
    - copy one (icon, double-click, Enter, Ctrl+C), copy all (Ctrl+Shift+C), save (Ctrl+S);
    - copies are kept out of the clipboard history and the cloud clipboard, and are wiped
      30 s later (or on exit) unless something else has been copied since;
    - light, dark or follow-Windows theme and twelve accent colours (View -> Theme and
      colour), which also tint the icon;
    - English, Japanese and Spanish; Help, About and Theme windows.

  How it is drawn: the page, the cards, the labels, the field frames and the strength line
  are painted by the main window into a back buffer; the interactive parts are owner-drawn
  BUTTONs (so Tab, Space, Enter and focus work as usual) plus a small slider class of our
  own; text input stays in plain EDIT controls. Shapes are anti-aliased with GDI+ (its flat
  C API, declared below); text stays GDI with ClearType.

  Implementation notes:
    - fonts and brushes are released only after nothing points at them any more;
    - icons are loaded without LR_SHARED, which ignores the size asked for;
    - an owner-drawn button turns a quick second click into BN_DOUBLECLICKED, so double
      clicks are fed back as single ones;
    - Esc closes Help, About and Theme, and the accelerators are quiet while a modal one is up;
    - a window cannot be driven from another process reliably (UIPI filters WM_GETTEXT),
      so the command line does everything a test needs:

        wyrmhold.exe [options] [/out:<file> | /entropy:<file> | /shot:<file.bmp>]
          options: /mode:chars|words|pin  /len:N  /words:N  /count:N  /classes:[l][u][d][s]
                   /symbols:basic|all|min|custom:<chars>  /exclude:<chars>  /ambiguous
                   /norepeat  /startletter  /sep:hyphen|space|dot|underscore|none|digit
                   /nocap  /nonumber  /allowruns  /preset:N
          window:  /theme:light|dark  /accent:N  /lang:en|jp|es  /open:help|about|theme

      /out writes the passwords (count up to 100000) and exits without a window; without
      options they use the standard profile. /entropy writes "bits pool check" and exits.
      /shot draws the window into a bitmap and exits. None of them writes the .ini.
**/

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "pwgen.h"
#include "resource.h"
#include "ui_strings.h"
#include "version.h"

#define WIDEN_(s)     L##s
#define WIDEN(s)      WIDEN_(s)
#define APP_VERSION   WIDEN(WH_VERSION_STR)
#define APP_DATE      WIDEN(WH_DATE_STR)
#define APP_REPO      WIDEN(WH_REPO_STR)
#define APP_AUTHOR    L"Gabriel Castiglia"
#define APP_AUMID     L"Castiglia.Wyrmhold"
#define INI_SECTION   L"settings"

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif
#ifndef PW_RENDERFULLCONTENT
#define PW_RENDERFULLCONTENT 0x00000002
#endif

/* ------------------------------------------------------------------ GDI+ flat API */

/* The handful of GDI+ calls used for anti-aliased shapes. gdiplus.dll is part of every
 * Windows since XP; its C++ headers are not usable from C, so the flat functions are
 * declared here, exactly as gdiplus.dll exports them. */
typedef void GpGraphics;
typedef void GpBrush;
typedef void GpPen;
typedef void GpPath;
typedef struct { float X, Y; } GpPointF;
typedef struct { UINT32 GdiplusVersion; void* DebugEventCallback; BOOL SuppressBackgroundThread; BOOL SuppressExternalCodecs; } GpStartupInput;

int  WINAPI GdiplusStartup(ULONG_PTR*, const GpStartupInput*, void*);
void WINAPI GdiplusShutdown(ULONG_PTR);
int  WINAPI GdipCreateFromHDC(HDC, GpGraphics**);
int  WINAPI GdipDeleteGraphics(GpGraphics*);
int  WINAPI GdipSetSmoothingMode(GpGraphics*, int);
int  WINAPI GdipSetPixelOffsetMode(GpGraphics*, int);
int  WINAPI GdipCreateSolidFill(DWORD, GpBrush**);
int  WINAPI GdipCreateLineBrush(const GpPointF*, const GpPointF*, DWORD, DWORD, int, GpBrush**);
int  WINAPI GdipDeleteBrush(GpBrush*);
int  WINAPI GdipCreatePen1(DWORD, float, int, GpPen**);
int  WINAPI GdipDeletePen(GpPen*);
int  WINAPI GdipSetPenStartCap(GpPen*, int);
int  WINAPI GdipSetPenEndCap(GpPen*, int);
int  WINAPI GdipSetPenLineJoin(GpPen*, int);
int  WINAPI GdipCreatePath(int, GpPath**);
int  WINAPI GdipDeletePath(GpPath*);
int  WINAPI GdipAddPathArc(GpPath*, float, float, float, float, float, float);
int  WINAPI GdipAddPathLine(GpPath*, float, float, float, float);
int  WINAPI GdipClosePathFigure(GpPath*);
int  WINAPI GdipFillPath(GpGraphics*, GpBrush*, GpPath*);
int  WINAPI GdipDrawPath(GpGraphics*, GpPen*, GpPath*);
int  WINAPI GdipFillEllipse(GpGraphics*, GpBrush*, float, float, float, float);
int  WINAPI GdipDrawEllipse(GpGraphics*, GpPen*, float, float, float, float);
int  WINAPI GdipDrawLine(GpGraphics*, GpPen*, float, float, float, float);
int  WINAPI GdipFillRectangle(GpGraphics*, GpBrush*, float, float, float, float);

#define GP_ANTIALIAS   4
#define GP_HALFPIXEL   4
#define GP_UNITPIXEL   2
#define GP_CAPROUND    2
#define GP_JOINROUND   2

static ULONG_PTR gGdipToken;

/* ------------------------------------------------------------------ palette */

typedef struct {
	COLORREF page, card, border, text, hint, field, fieldBorder, track,
	         ok, warn, err, digit, symbol,
	         accent, onAccent, accentSoft, accentHover, accentDown,
	         band, band2, bandText, bandSub, menuBg;
} Palette;

/* Twelve accent colours: a deeper tone for the light theme, a brighter one for the dark,
 * and the colour of the icon's tile. The first one, black, is the default. */
typedef struct { int name; COLORREF light, dark, tile; } Accent;

static const Accent ACCENTS[] = {
	{ S_ACCENT_BLACK,  RGB(0x1F,0x29,0x37), RGB(0xD1,0xD5,0xDB), RGB(0x22,0x24,0x2C) },
	{ S_ACCENT_BLUE,   RGB(0x25,0x63,0xEB), RGB(0x60,0xA5,0xFA), RGB(0x2F,0x6F,0xE0) },
	{ S_ACCENT_GOLD,   RGB(0x9A,0x6F,0x00), RGB(0xE0,0xB0,0x3C), RGB(0xB8,0x86,0x0B) },
	{ S_ACCENT_PURPLE, RGB(0x6D,0x28,0xD9), RGB(0xA7,0x8B,0xFA), RGB(0x6D,0x28,0xD9) },
	{ S_ACCENT_SKY,    RGB(0x02,0x84,0xC7), RGB(0x38,0xBD,0xF8), RGB(0x0E,0xA5,0xE9) },
	{ S_ACCENT_EMBER,  RGB(0xC2,0x41,0x0C), RGB(0xFB,0x92,0x3C), RGB(0xEA,0x58,0x0C) },
	{ S_ACCENT_RED,    RGB(0xB9,0x1C,0x1C), RGB(0xF8,0x71,0x71), RGB(0xDC,0x26,0x26) },
	{ S_ACCENT_YELLOW, RGB(0xA1,0x62,0x07), RGB(0xFA,0xCC,0x15), RGB(0xEA,0xB3,0x08) },
	{ S_ACCENT_GREEN,  RGB(0x15,0x80,0x3D), RGB(0x4A,0xDE,0x80), RGB(0x16,0xA3,0x4A) },
	{ S_ACCENT_TEAL,   RGB(0x0F,0x76,0x6E), RGB(0x2D,0xD4,0xBF), RGB(0x0D,0x94,0x88) },
	{ S_ACCENT_PINK,   RGB(0xBE,0x18,0x5D), RGB(0xF4,0x72,0xB6), RGB(0xDB,0x27,0x77) },
	{ S_ACCENT_WHITE,  RGB(0x47,0x55,0x69), RGB(0xF1,0xF5,0xF9), RGB(0xF1,0xF5,0xF9) },
};
#define ACCENT_COUNT ((int)(sizeof(ACCENTS) / sizeof(ACCENTS[0])))

static Palette PAL;
static int gDark = 0;
static int gTheme = 0;                  /* 0 follow Windows, 1 light, 2 dark */
static int gAccent = 0;

static COLORREF mix(COLORREF a, COLORREF b, double t)
{
	return RGB((int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t + 0.5),
	           (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t + 0.5),
	           (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t + 0.5));
}

static double luminance(COLORREF c)
{
	return (0.2126 * GetRValue(c) + 0.7152 * GetGValue(c) + 0.0722 * GetBValue(c)) / 255.0;
}

static void makePalette(void)
{
	COLORREF a = gDark ? ACCENTS[gAccent].dark : ACCENTS[gAccent].light;
	/* the band takes the icon's tile colour, so the icon sits on its own colour; on the dark
	 * theme the accent of black is a light grey, which would make the band a washed grey.
	 * A white tile would make the band grey too: then the deeper accent tone. */
	COLORREF t = luminance(ACCENTS[gAccent].tile) > 0.8 ? ACCENTS[gAccent].light : ACCENTS[gAccent].tile;

	if (gDark)
	{
		PAL.page = RGB(0x15,0x18,0x1D);  PAL.card = RGB(0x1E,0x22,0x28);  PAL.border = RGB(0x2D,0x33,0x3B);
		PAL.text = RGB(0xE6,0xE8,0xEB);  PAL.hint = RGB(0x98,0xA2,0xAE);  PAL.field = RGB(0x26,0x2B,0x32);
		PAL.fieldBorder = RGB(0x3A,0x41,0x4B);  PAL.track = RGB(0x3A,0x41,0x4B);
		PAL.ok = RGB(0x4A,0xDE,0x80);  PAL.warn = RGB(0xF0,0xB4,0x4C);  PAL.err = RGB(0xFF,0x7B,0x72);
		PAL.digit = RGB(0x79,0xB8,0xFF);  PAL.symbol = RGB(0xF0,0xB4,0x4C);
		PAL.menuBg = RGB(0x26,0x2B,0x32);
		PAL.band = mix(t, RGB(0x0B,0x0E,0x12), 0.62);
	}
	else
	{
		PAL.page = RGB(0xF3,0xF5,0xF8);  PAL.card = RGB(0xFF,0xFF,0xFF);  PAL.border = RGB(0xE1,0xE5,0xEB);
		PAL.text = RGB(0x1F,0x23,0x28);  PAL.hint = RGB(0x5F,0x69,0x75);  PAL.field = RGB(0xF7,0xF8,0xFA);
		PAL.fieldBorder = RGB(0xCB,0xD2,0xDA);  PAL.track = RGB(0xD9,0xDE,0xE5);
		PAL.ok = RGB(0x1A,0x7F,0x37);  PAL.warn = RGB(0xB5,0x47,0x08);  PAL.err = RGB(0xB4,0x23,0x18);
		PAL.digit = RGB(0x1F,0x5F,0xBF);  PAL.symbol = RGB(0xB5,0x47,0x08);
		PAL.menuBg = RGB(0xFF,0xFF,0xFF);
		PAL.band = mix(t, RGB(0x0E,0x15,0x20), 0.50);
	}
	PAL.accent = a;
	PAL.onAccent = luminance(a) > 0.55 ? RGB(0x11,0x14,0x18) : RGB(0xFF,0xFF,0xFF);
	PAL.accentSoft = mix(PAL.card, a, gDark ? 0.20 : 0.11);
	PAL.accentHover = mix(a, gDark ? RGB(255,255,255) : RGB(0,0,0), 0.12);
	PAL.accentDown = mix(a, gDark ? RGB(255,255,255) : RGB(0,0,0), 0.24);
	PAL.band2 = mix(PAL.band, t, 0.35);
	PAL.bandText = RGB(0xFF,0xFF,0xFF);
	PAL.bandSub = mix(RGB(0xFF,0xFF,0xFF), PAL.band, 0.32);
}

/* ------------------------------------------------------------------ state */

enum {
	IDC_SEG_CHARS = 1001, IDC_SEG_WORDS, IDC_SEG_PIN, IDC_PROFILE,
	IDC_SLIDER, IDC_LEN, IDC_CNT, IDC_CNT_DEC, IDC_CNT_INC,
	IDC_CHIP_LOWER, IDC_CHIP_UPPER, IDC_CHIP_DIGITS, IDC_CHIP_SYMBOLS, IDC_SYMSET,
	IDC_TG_AMBIG, IDC_TG_NOREPEAT, IDC_TG_START, IDC_EXCLUDE, IDC_CUSTOM,
	IDC_SEPARATOR, IDC_TG_CAP, IDC_TG_NUMBER, IDC_TG_PINREPEAT, IDC_TG_NOSEQ,
	IDC_GENERATE, IDC_TG_CLEAR, IDC_SAVE, IDC_COPYALL, IDC_LIST, IDC_STATUS
};

enum { TMR_CLIP = 1, TMR_MARK = 2, TMR_REGEN = 3 };
enum { ST_PLAIN, ST_OK, ST_WARN, ST_ERR };

/* owner-drawn button kinds */
enum { K_PRIMARY, K_SEG, K_CHIP, K_TOGGLE, K_DROP, K_ICON, K_STEP, K_THEMESEG, K_SWATCH };
enum { G_NONE, G_COPY, G_SAVE, G_MINUS, G_PLUS };

typedef struct {
	int kind, str, value, glyph, hot;
	int* bind;                          /* the option a chip or toggle shows and flips */
} Ctl;

#define MAX_COUNT     50
#define AUTO_MAX      100000
#define CLIP_SECONDS  30

static int gDpi = 96;
#define SC(v)  MulDiv((v), gDpi, 96)

/* Logical units, at 96 dpi. SC() turns each one into pixels. */
#define DESIGN_W   800
#define MIN_W      760
#define BAND_H     72
#define ICON_SZ    44
#define MARGIN     20
#define SEG_H      40
#define CARD_OPT_H 266
#define ROW_LH     36      /* one password row */
#define ICON_ZONE  44      /* the copy icon at the right of a row */
#define LIST_HEAD  52
#define STATUS_H   40
#define FIELD_H    32

static HINSTANCE gInst;
static HWND hMain, hTip, hList;
static HWND hSeg[3], hProfile, hSlider, hLen, hCnt, hCntDec, hCntInc;
static HWND hChip[4], hSymSet, hTgAmbig, hTgNoRepeat, hTgStart, hExclude, hCustom;
static HWND hSeparator, hTgCap, hTgNumber, hTgPinRepeat, hTgNoSeq;
static HWND hGenerate, hTgClear, hSave, hCopyAll;
static HWND hHelpWnd, hAboutWnd, hThemeWnd;
static HICON gSwatchIcon[16];
static HFONT fUi, fBold, fTitle, fSub, fSmall, fPw, fPwSmall;
static HICON icBand, icAbout, icBig, icSmall;
static HMENU hMenu;

static int gLang = LANG_EN;
static PwOptions gOpt;                  /* what the controls show */
static int gCount = 5, gClearClip = 1;
static int gCharNoRepeat = 0, gPinNoRepeat = 1;
static char gCharExclude[PW_SET_MAX], gPinExclude[PW_SET_MAX];
static int gNoSave = 0;
static int gSyncing = 0;                /* controls are being set from the options: no feedback */

static char gPw[MAX_COUNT][PW_OUT_MAX];
static int gPwN = 0, gPwMode = PW_CHARS;

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

/* where the main window paints what is not a control (filled by layout) */
typedef struct { int str; RECT r; int style; } Label;
enum { L_PLAIN, L_BOLD, L_HINT, L_TITLE };
static Label gLabels[16];
static int gLabelN = 0;
static RECT gSegTrack, gCardOpt, gCardList, gStrength, gStatusR, gVersionR;
static HWND gFieldEdit[4];
static RECT gFieldBox[4];
static int gFieldN = 0;

#define T(id)  (STR[gLang][(id)])

static void onOptionsChanged(int now);
static void syncControls(void);
static void layout(void);

/* ------------------------------------------------------------------ settings */

/* wyrmhold.ini beside the executable when one is there (portable use), otherwise
 * %APPDATA%\Wyrmhold\wyrmhold.ini -- the program folder of an installed copy cannot be
 * written. Writing may still fail; then the program simply starts with the defaults. */

static void iniPath(void)
{
	wchar_t* slash;
	wchar_t dir[MAX_PATH];
	DWORD n;

	GetModuleFileNameW(NULL, gIni, MAX_PATH);
	slash = wcsrchr(gIni, L'\\');
	if (slash && (size_t)(slash - gIni) + 14 < MAX_PATH) wcscpy(slash + 1, L"wyrmhold.ini");
	if (GetFileAttributesW(gIni) != INVALID_FILE_ATTRIBUTES) return;

	n = GetEnvironmentVariableW(L"APPDATA", dir, MAX_PATH);
	if (n == 0 || n + 24 >= MAX_PATH) return;           /* no profile: stay beside the exe */
	wcscat(dir, L"\\Wyrmhold");
	CreateDirectoryW(dir, NULL);
	_snwprintf(gIni, MAX_PATH, L"%ls\\wyrmhold.ini", dir);
	gIni[MAX_PATH - 1] = 0;
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

static void iniSetStr(const wchar_t* key, const wchar_t* v)
{
	if (!gNoSave) WritePrivateProfileStringW(INI_SECTION, key, v, gIni);
}

static int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

/* ------------------------------------------------------------------ the options in effect */

/* noRepeat and the exclusions are kept apart for characters and PINs: excluding "1" from a
 * PIN says nothing about a character password. */
static void effectiveOptions(PwOptions* o)
{
	*o = gOpt;
	o->noRepeat = (o->mode == PW_PIN) ? gPinNoRepeat : gCharNoRepeat;
	strcpy(o->exclude, (o->mode == PW_PIN) ? gPinExclude : gCharExclude);
}

/* ------------------------------------------------------------------ profiles */

enum { P_STANDARD, P_MAX, P_NOSYM, P_MINSYM, P_SHORT, P_WIFI, P_PHRASE, P_PIN, P_COUNT };

static const int PRESET_STR[P_COUNT] = {
	S_PRESET_STANDARD, S_PRESET_MAX, S_PRESET_NOSYM, S_PRESET_MINSYM,
	S_PRESET_SHORT, S_PRESET_WIFI, S_PRESET_PHRASE, S_PRESET_PIN
};

/* A profile, as the effective options it stands for. */
static void presetOptions(int p, PwOptions* o, int* noRepeat)
{
	pw_defaults(o);
	*noRepeat = 0;
	switch (p)
	{
	case P_MAX:    o->length = 32; o->symbolSet = PW_SYM_ALL; o->avoidAmbiguous = 0; break;
	case P_NOSYM:  o->symbols = 0; break;
	case P_MINSYM: o->symbolSet = PW_SYM_MIN; o->startLetter = 1; break;
	case P_SHORT:  o->length = 12; o->startLetter = 1; *noRepeat = 1; break;
	case P_WIFI:   o->upper = 0; o->symbols = 0; break;
	case P_PHRASE: o->mode = PW_WORDS; break;
	case P_PIN:    o->mode = PW_PIN; *noRepeat = 1; break;
	}
}

/* Which profile the current settings are, or -1 ("custom"). Only what matters for the
 * current kind of password is compared. */
static int currentPreset(void)
{
	PwOptions e, p;
	int i, nr;

	effectiveOptions(&e);
	for (i = 0; i < P_COUNT; i++)
	{
		presetOptions(i, &p, &nr);
		if (p.mode != e.mode) continue;
		if (e.mode == PW_WORDS)
		{
			if (p.words == e.words && p.separator == e.separator && p.capitalize == e.capitalize && p.addNumber == e.addNumber) return i;
		}
		else if (e.mode == PW_PIN)
		{
			if (p.pinLength == e.pinLength && nr == e.noRepeat && p.noSequence == e.noSequence && !e.exclude[0]) return i;
		}
		else if (p.length == e.length && p.lower == e.lower && p.upper == e.upper && p.digits == e.digits &&
		         p.symbols == e.symbols && (p.symbolSet == e.symbolSet || !e.symbols) &&
		         p.avoidAmbiguous == e.avoidAmbiguous && nr == e.noRepeat && p.startLetter == e.startLetter && !e.exclude[0])
			return i;
	}
	return -1;
}

static void applyPreset(int p)
{
	PwOptions o;
	int nr;

	presetOptions(p, &o, &nr);
	if (o.mode == PW_CHARS)
	{
		gOpt.mode = PW_CHARS;
		gOpt.length = o.length; gOpt.lower = o.lower; gOpt.upper = o.upper; gOpt.digits = o.digits;
		gOpt.symbols = o.symbols; gOpt.symbolSet = o.symbolSet; gOpt.avoidAmbiguous = o.avoidAmbiguous;
		gOpt.startLetter = o.startLetter; gCharNoRepeat = nr; gCharExclude[0] = 0;
	}
	else if (o.mode == PW_WORDS)
	{
		gOpt.mode = PW_WORDS;
		gOpt.words = o.words; gOpt.separator = o.separator; gOpt.capitalize = o.capitalize; gOpt.addNumber = o.addNumber;
	}
	else
	{
		gOpt.mode = PW_PIN;
		gOpt.pinLength = o.pinLength; gOpt.noSequence = o.noSequence; gPinNoRepeat = nr; gPinExclude[0] = 0;
	}
	syncControls();
	layout();
	onOptionsChanged(1);
}

/* ------------------------------------------------------------------ theme */

static int systemPrefersDark(void)
{
	HKEY k;
	DWORD v = 1, cb = sizeof(v), type = 0;
	int dark = 0;

	if (gTheme == 1) return 0;
	if (gTheme == 2) return 1;

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

/* Windows has no documented way to ask for dark scroll bars and edit menus. These uxtheme
 * ordinals are what applications use; they are looked up at run time and skipped when
 * absent. Everything that matters is owner-drawn anyway. */
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
	themeChild(hwnd, (LPARAM)gDark);            /* the window's own scroll bar (Help) */
	EnumChildWindows(hwnd, themeChild, (LPARAM)gDark);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
}

/* ------------------------------------------------------------------ the icon, in the accent */

/* The icon: the engraving (dragon and tower) laid on a rounded tile of the accent's colour.
 * The drawing comes without the tile as resource IDR_ICONLAYER (tools/make_icon.py): 256 x 256
 * pixels of T, K, R, G, B, where the pixel is rgb = RGB + tile * K / 255 with alpha T,
 * premultiplied. Here it is coloured, reduced to the size asked by averaging the area each
 * pixel covers, and turned into an icon. Without the resource, the executable's own icon. */

static const unsigned char* gLayer;
static int gLayerN;

static void loadLayer(void)
{
	HRSRC r = FindResourceW(gInst, MAKEINTRESOURCEW(IDR_ICONLAYER), MAKEINTRESOURCEW(10) /* RT_RCDATA */);
	HGLOBAL h = r ? LoadResource(gInst, r) : NULL;
	const unsigned char* p = h ? (const unsigned char*)LockResource(h) : NULL;
	DWORD len = r ? SizeofResource(gInst, r) : 0;
	int n;

	if (!p || len < 6 || memcmp(p, "WYI1", 4)) return;
	n = p[4] | (p[5] << 8);
	if (n <= 0 || len < 6 + (DWORD)n * n * 5) return;
	gLayer = p + 6;
	gLayerN = n;
}

/* the source span [lo, hi) of output pixel i, and the weights of the source pixels in it */
static void spanOf(int i, int size, int n, int* first, int* last, double* wFirst, double* wLast)
{
	double lo = (double)i * n / size, hi = (double)(i + 1) * n / size;

	*first = (int)lo;
	*last = (int)ceil(hi) - 1;
	if (*last >= n) *last = n - 1;
	if (*last < *first) *last = *first;
	*wFirst = (*first == *last) ? hi - lo : (*first + 1) - lo;
	*wLast = (*first == *last) ? 0 : hi - *last;
}

static HICON tintedIconFor(int accent, int size)
{
	COLORREF c = ACCENTS[accent].tile;
	double tile[3] = { GetBValue(c), GetGValue(c), GetRValue(c) };   /* B, G, R as in the DIB */
	BITMAPINFO bi;
	void* bits = NULL;
	DWORD* px;
	HBITMAP colour, mask;
	ICONINFO ii;
	HICON out;
	int n = gLayerN, x, y;

	if (!gLayer || size <= 0)
		return (HICON)LoadImageW(gInst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, size, size, 0);

	ZeroMemory(&bi, sizeof(bi));
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = size;
	bi.bmiHeader.biHeight = -size;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biCompression = BI_RGB;
	colour = CreateDIBSection(NULL, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!colour || !bits) return NULL;
	px = (DWORD*)bits;

	for (y = 0; y < size; y++)
	{
		int y0, y1;
		double wy0, wy1;
		spanOf(y, size, n, &y0, &y1, &wy0, &wy1);
		for (x = 0; x < size; x++)
		{
			int x0, x1, sy, sx, k;
			double wx0, wx1, acc[4] = { 0, 0, 0, 0 }, wsum = 0, a;
			spanOf(x, size, n, &x0, &x1, &wx0, &wx1);
			for (sy = y0; sy <= y1; sy++)
			{
				double wy = sy == y0 ? wy0 : sy == y1 ? wy1 : 1.0;
				for (sx = x0; sx <= x1; sx++)
				{
					double w = wy * (sx == x0 ? wx0 : sx == x1 ? wx1 : 1.0);
					const unsigned char* q = gLayer + ((size_t)sy * n + sx) * 5;   /* T K R G B */
					double kk = q[1] / 255.0;
					acc[0] += w * (q[4] + tile[0] * kk);
					acc[1] += w * (q[3] + tile[1] * kk);
					acc[2] += w * (q[2] + tile[2] * kk);
					acc[3] += w * q[0];
					wsum += w;
				}
			}
			if (wsum <= 0) { px[y * size + x] = 0; continue; }
			a = acc[3] / wsum;
			if (a < 0.5) { px[y * size + x] = 0; continue; }
			for (k = 0; k < 3; k++)                                 /* straight alpha for the icon */
			{
				acc[k] = acc[k] / wsum * 255.0 / a;
				if (acc[k] > 255) acc[k] = 255;
			}
			px[y * size + x] = ((DWORD)(a + 0.5) << 24) | ((DWORD)(acc[2] + 0.5) << 16) |
			                   ((DWORD)(acc[1] + 0.5) << 8) | (DWORD)(acc[0] + 0.5);
		}
	}

	mask = CreateBitmap(size, size, 1, 1, NULL);
	ii.fIcon = TRUE;
	ii.xHotspot = ii.yHotspot = 0;
	ii.hbmColor = colour;
	ii.hbmMask = mask;
	out = CreateIconIndirect(&ii);
	DeleteObject(colour);
	DeleteObject(mask);
	return out;
}

static void makeIcons(void)
{
	HICON ob = icBand, oa = icAbout, og = icBig, os = icSmall;

	icBand  = tintedIconFor(gAccent, SC(ICON_SZ));
	icAbout = tintedIconFor(gAccent, SC(72));
	icBig   = tintedIconFor(gAccent, GetSystemMetrics(SM_CXICON));
	icSmall = tintedIconFor(gAccent, GetSystemMetrics(SM_CXSMICON));
	if (hMain)
	{
		SendMessageW(hMain, WM_SETICON, ICON_BIG, (LPARAM)icBig);
		SendMessageW(hMain, WM_SETICON, ICON_SMALL, (LPARAM)icSmall);
	}
	if (hHelpWnd)  { SendMessageW(hHelpWnd, WM_SETICON, ICON_BIG, (LPARAM)icBig);  SendMessageW(hHelpWnd, WM_SETICON, ICON_SMALL, (LPARAM)icSmall); }
	if (hAboutWnd) { SendMessageW(hAboutWnd, WM_SETICON, ICON_BIG, (LPARAM)icBig); SendMessageW(hAboutWnd, WM_SETICON, ICON_SMALL, (LPARAM)icSmall); }
	if (hThemeWnd) { SendMessageW(hThemeWnd, WM_SETICON, ICON_BIG, (LPARAM)icBig); SendMessageW(hThemeWnd, WM_SETICON, ICON_SMALL, (LPARAM)icSmall); }
	if (ob) DestroyIcon(ob);
	if (oa) DestroyIcon(oa);
	if (og) DestroyIcon(og);
	if (os) DestroyIcon(os);
}

static void refreshTheme(void)
{
	gDark = systemPrefersDark();
	makePalette();
	enableDarkControls(gDark);
	if (hMain)     applyTheme(hMain);
	if (hHelpWnd)  applyTheme(hHelpWnd);
	if (hAboutWnd) applyTheme(hAboutWnd);
	if (hThemeWnd) applyTheme(hThemeWnd);
	if (hMenu && hMain) DrawMenuBar(hMain);
}

/* ------------------------------------------------------------------ drawing primitives */

static DWORD argb(COLORREF c, int alpha)
{
	return ((DWORD)alpha << 24) | ((DWORD)GetRValue(c) << 16) | ((DWORD)GetGValue(c) << 8) | GetBValue(c);
}

static GpGraphics* gfx(HDC dc)
{
	GpGraphics* g = NULL;

	if (GdipCreateFromHDC(dc, &g) != 0) return NULL;
	GdipSetSmoothingMode(g, GP_ANTIALIAS);
	GdipSetPixelOffsetMode(g, GP_HALFPIXEL);
	return g;
}

static GpPath* roundPath(float x, float y, float w, float h, float r)
{
	GpPath* p = NULL;
	float d;

	if (r * 2 > h) r = h / 2;
	if (r * 2 > w) r = w / 2;
	d = r * 2;
	GdipCreatePath(0, &p);
	if (!p) return NULL;
	if (r <= 0.5f)
	{
		GdipAddPathLine(p, x, y, x + w, y);
		GdipAddPathLine(p, x + w, y, x + w, y + h);
		GdipAddPathLine(p, x + w, y + h, x, y + h);
	}
	else
	{
		GdipAddPathArc(p, x, y, d, d, 180, 90);
		GdipAddPathArc(p, x + w - d, y, d, d, 270, 90);
		GdipAddPathArc(p, x + w - d, y + h - d, d, d, 0, 90);
		GdipAddPathArc(p, x, y + h - d, d, d, 90, 90);
	}
	GdipClosePathFigure(p);
	return p;
}

/* A rounded rectangle, filled and/or outlined (pass CLR_NONE to skip either). */
static void roundRect(HDC dc, const RECT* r, int radius, COLORREF fill, COLORREF line, float width)
{
	GpGraphics* g = gfx(dc);
	GpPath* p;

	if (!g) return;
	if (fill != CLR_NONE)
	{
		GpBrush* b = NULL;
		p = roundPath((float)r->left, (float)r->top, (float)(r->right - r->left), (float)(r->bottom - r->top), (float)radius);
		GdipCreateSolidFill(argb(fill, 255), &b);
		if (p && b) GdipFillPath(g, b, p);
		if (b) GdipDeleteBrush(b);
		if (p) GdipDeletePath(p);
	}
	if (line != CLR_NONE)
	{
		GpPen* pen = NULL;
		float h = width / 2;
		p = roundPath(r->left + h, r->top + h, r->right - r->left - width, r->bottom - r->top - width, radius - h);
		GdipCreatePen1(argb(line, 255), width, GP_UNITPIXEL, &pen);
		if (p && pen) GdipDrawPath(g, pen, p);
		if (pen) GdipDeletePen(pen);
		if (p) GdipDeletePath(p);
	}
	GdipDeleteGraphics(g);
}

static void circle(HDC dc, float cx, float cy, float rad, COLORREF fill, COLORREF line, float width)
{
	GpGraphics* g = gfx(dc);

	if (!g) return;
	if (fill != CLR_NONE)
	{
		GpBrush* b = NULL;
		GdipCreateSolidFill(argb(fill, 255), &b);
		if (b) { GdipFillEllipse(g, b, cx - rad, cy - rad, rad * 2, rad * 2); GdipDeleteBrush(b); }
	}
	if (line != CLR_NONE)
	{
		GpPen* p = NULL;
		GdipCreatePen1(argb(line, 255), width, GP_UNITPIXEL, &p);
		if (p) { GdipDrawEllipse(g, p, cx - rad + width / 2, cy - rad + width / 2, rad * 2 - width, rad * 2 - width); GdipDeletePen(p); }
	}
	GdipDeleteGraphics(g);
}

/* A polyline with round ends and joins: check marks, chevrons, arrows. */
static void strokes(HDC dc, const float* pts, int n, COLORREF c, float width)
{
	GpGraphics* g = gfx(dc);
	GpPen* p = NULL;
	int i;

	if (!g) return;
	GdipCreatePen1(argb(c, 255), width, GP_UNITPIXEL, &p);
	if (p)
	{
		GdipSetPenStartCap(p, GP_CAPROUND);
		GdipSetPenEndCap(p, GP_CAPROUND);
		GdipSetPenLineJoin(p, GP_JOINROUND);
		for (i = 0; i + 1 < n; i++) GdipDrawLine(g, p, pts[2 * i], pts[2 * i + 1], pts[2 * i + 2], pts[2 * i + 3]);
		GdipDeletePen(p);
	}
	GdipDeleteGraphics(g);
}

static void fillSolid(HDC dc, const RECT* r, COLORREF c)
{
	HBRUSH b = CreateSolidBrush(c);
	FillRect(dc, r, b);
	DeleteObject(b);
}

static void drawText(HDC dc, const wchar_t* s, int n, RECT* r, HFONT f, COLORREF c, UINT flags)
{
	HGDIOBJ of = SelectObject(dc, f);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, c);
	DrawTextW(dc, s, n, r, flags | DT_NOPREFIX);
	SelectObject(dc, of);
}

static int textWidth(HFONT f, const wchar_t* s, int n)
{
	HDC dc = GetDC(NULL);
	HGDIOBJ of = SelectObject(dc, f);
	SIZE sz = { 0, 0 };

	GetTextExtentPoint32W(dc, s, n < 0 ? (int)wcslen(s) : n, &sz);
	SelectObject(dc, of);
	ReleaseDC(NULL, dc);
	return sz.cx;
}

/* Menu-style strings carry a hint after a tab ("Basic\t! @ # ..."): the part before it. */
static int headLen(const wchar_t* s)
{
	const wchar_t* tab = wcschr(s, L'\t');
	return tab ? (int)(tab - s) : (int)wcslen(s);
}

/* ------------------------------------------------------------------ glyphs */

static void glyphCopy(HDC dc, int x, int y, int s, COLORREF ink, COLORREF fill)
{
	RECT a, b;
	int off = s * 5 / 16, rad = SC(3);

	SetRect(&a, x + off, y, x + s, y + s - off);
	SetRect(&b, x, y + off, x + s - off, y + s);
	roundRect(dc, &a, rad, CLR_NONE, ink, (float)SC(1) * 1.4f);
	roundRect(dc, &b, rad, fill, ink, (float)SC(1) * 1.4f);
}

static void glyphCheck(HDC dc, int x, int y, int s, COLORREF ink, float w)
{
	float p[6] = { x + s * 0.16f, y + s * 0.54f, x + s * 0.41f, y + s * 0.78f, x + s * 0.86f, y + s * 0.26f };
	strokes(dc, p, 3, ink, w);
}

static void glyphSave(HDC dc, int x, int y, int s, COLORREF ink)
{
	float w = (float)SC(1) * 1.5f;
	float tray[8] = { (float)x, y + s * 0.58f, (float)x, (float)(y + s), (float)(x + s), (float)(y + s), (float)(x + s), y + s * 0.58f };
	float stem[4] = { x + s * 0.5f, (float)y, x + s * 0.5f, y + s * 0.70f };
	float head[6] = { x + s * 0.25f, y + s * 0.45f, x + s * 0.5f, y + s * 0.70f, x + s * 0.75f, y + s * 0.45f };
	strokes(dc, tray, 4, ink, w);
	strokes(dc, stem, 2, ink, w);
	strokes(dc, head, 3, ink, w);
}

static void glyphChevron(HDC dc, int cx, int cy, int s, COLORREF ink)
{
	float p[6] = { cx - s * 0.5f, cy - s * 0.25f, (float)cx, cy + s * 0.25f, cx + s * 0.5f, cy - s * 0.25f };
	strokes(dc, p, 3, ink, (float)SC(1) * 1.6f);
}

static void glyphPlusMinus(HDC dc, int cx, int cy, int s, COLORREF ink, int plus)
{
	float h[4] = { cx - s * 0.5f, (float)cy, cx + s * 0.5f, (float)cy };
	float v[4] = { (float)cx, cy - s * 0.5f, (float)cx, cy + s * 0.5f };
	strokes(dc, h, 2, ink, (float)SC(1) * 1.8f);
	if (plus) strokes(dc, v, 2, ink, (float)SC(1) * 1.8f);
}

/* ------------------------------------------------------------------ status */

static void invalidateStatus(void)
{
	if (hMain) InvalidateRect(hMain, &gStatusR, FALSE);
}

static void setStatus(int kind, const wchar_t* text)
{
	gStatusKind = kind;
	lstrcpynW(gStatusText, text, 512);
	invalidateStatus();
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

/* ------------------------------------------------------------------ generating */

static void wipePasswords(void)
{
	SecureZeroMemory(gPw, sizeof(gPw));
	gPwN = 0;
}

static void sayGenerated(void)
{
	if (gPwN == 1) setStatus(ST_OK, T(S_ST_GENERATED_ONE));
	else           setStatusF(ST_OK, S_ST_GENERATED, gPwN);
}

static void doGenerate(void)
{
	PwOptions o;
	int i;

	KillTimer(hMain, TMR_REGEN);
	effectiveOptions(&o);
	wipePasswords();

	SendMessageW(hList, WM_SETREDRAW, FALSE, 0);
	SendMessageW(hList, LB_RESETCONTENT, 0, 0);

	if (pw_check(&o, NULL) != PW_OK)
	{
		/* the strength line says why; the list stays empty rather than showing stale ones */
		SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(hList, NULL, TRUE);
		setStatus(ST_WARN, T(S_ST_NOTHING));
		return;
	}

	for (i = 0; i < gCount; i++)
	{
		if (!pw_make(&o, gPw[i]))
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
	gPwMode = o.mode;
	gMarkRow = -1;
	SendMessageW(hList, LB_SETCURSEL, 0, 0);
	SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hList, NULL, TRUE);
	sayGenerated();
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
	InvalidateRect(hCopyAll, NULL, FALSE);

	KillTimer(hMain, TMR_CLIP);
	gClipLeft = 0;
	if (gClearClip)
	{
		gClipLeft = CLIP_SECONDS;
		SetTimer(hMain, TMR_CLIP, 1000, NULL);
	}
	showCopied();
}

static void copyRow(int row)
{
	wchar_t w[PW_OUT_MAX];

	if (row < 0 || row >= gPwN) return;

	MultiByteToWideChar(CP_ACP, 0, gPw[row], -1, w, PW_OUT_MAX);
	if (!putClipboard(w)) { SecureZeroMemory(w, sizeof(w)); setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }
	SecureZeroMemory(w, sizeof(w));

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED), row + 1);
	gClipMsg[255] = 0;
	afterCopy(row);
}

static void copyAll(void)
{
	static wchar_t w[MAX_COUNT * (PW_OUT_MAX + 2) + 1];
	int i, n = 0;

	if (gPwN <= 0) return;

	for (i = 0; i < gPwN; i++)
	{
		n += MultiByteToWideChar(CP_ACP, 0, gPw[i], -1, w + n, PW_OUT_MAX) - 1;
		w[n++] = L'\r';
		w[n++] = L'\n';
	}
	w[n] = 0;

	i = putClipboard(w);
	SecureZeroMemory(w, sizeof(w));
	if (!i) { setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED_ALL), gPwN);
	gClipMsg[255] = 0;
	afterCopy(-2);
}

static void copySelected(void)
{
	HWND f = GetFocus();
	wchar_t cls[16];

	/* Ctrl+C in a text field copies its text, as everywhere else */
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

/* ------------------------------------------------------------------ strength */

/* Guessing time for an attacker who has the file and tries 100 billion passwords a second
 * (a rack of GPUs against a fast hash); on average half the space is searched. */
static int guessTimeString(double bits)
{
	double secs = pow(2.0, bits - 1) / 1e11, year = 3.156e7;

	if (secs < 1)            return S_TIME_INSTANT;
	if (secs < 3600)         return S_TIME_MINUTES;
	if (secs < 86400)        return S_TIME_HOURS;
	if (secs < year)         return S_TIME_DAYS;
	if (secs < 100 * year)   return S_TIME_YEARS;
	if (secs < 1e6 * year)   return S_TIME_CENTURIES;
	if (secs < 1.4e10 * year)return S_TIME_MILLIONS;
	return S_TIME_FOREVER;
}

static int strengthLevel(double bits)
{
	return bits < 36 ? 0 : bits < 50 ? 1 : bits < 64 ? 2 : bits < 80 ? 3 : 4;
}

static COLORREF levelColour(int level)
{
	return level <= 1 ? PAL.err : level == 2 ? PAL.warn : PAL.ok;
}

static void paintStrength(HDC dc)
{
	static const int words[5] = { S_STR_VWEAK, S_STR_WEAK, S_STR_FAIR, S_STR_STRONG, S_STR_VSTRONG };
	static const int classNames[4] = { S_CLASS_LOWER, S_CLASS_UPPER, S_CLASS_DIGITS, S_CLASS_SYMBOLS };
	RECT r = gStrength, t;
	PwOptions o;
	int which, check, level, i, x, segW = SC(24), segH = SC(6), gap = SC(4), lineH = (r.bottom - r.top) / 2;
	double bits;
	wchar_t buf[256];
	COLORREF lc;

	effectiveOptions(&o);
	check = pw_check(&o, &which);
	if (check != PW_OK)
	{
		int id = check == PW_ERR_NOCLASS ? S_ERR_NOCLASS : check == PW_ERR_EMPTYCLASS ? S_ERR_EMPTYCLASS :
		         check == PW_ERR_NODIGITS ? S_ERR_NODIGITS : S_ERR_TOOSHORT;
		_snwprintf(buf, 256, T(id), which >= 0 ? T(classNames[which]) : L"");
		buf[255] = 0;
		t = r;
		drawText(dc, buf, -1, &t, fBold, PAL.err, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		return;
	}

	bits = pw_entropy(&o);
	level = strengthLevel(bits);
	lc = levelColour(level);
	/* a PIN is weak by design and safe only where tries are limited: said so, not in red */
	if (o.mode == PW_PIN) lc = PAL.warn;

	x = r.left;
	for (i = 0; i < 5; i++)
	{
		RECT s;
		SetRect(&s, x, r.top + (lineH - segH) / 2, x + segW, r.top + (lineH + segH) / 2);
		roundRect(dc, &s, segH / 2, i <= level ? lc : PAL.track, CLR_NONE, 0);
		x += segW + gap;
	}
	x += SC(8);

	SetRect(&t, x, r.top, r.right, r.top + lineH);
	{
		const wchar_t* word = T(o.mode == PW_PIN ? S_STR_PIN : words[level]);
		drawText(dc, word, -1, &t, fBold, lc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		t.left += textWidth(fBold, word, -1);
	}
	_snwprintf(buf, 256, T(S_STRENGTH_BITS), (int)(bits + 0.5));
	buf[255] = 0;
	drawText(dc, buf, -1, &t, fUi, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	_snwprintf(buf, 256, T(o.mode == PW_WORDS ? S_STRENGTH_DETAIL_W : S_STRENGTH_DETAIL), pw_pool(&o), T(guessTimeString(bits)));
	buf[255] = 0;
	SetRect(&t, r.left, r.top + lineH, r.right, r.bottom);
	drawText(dc, buf, -1, &t, fSmall, PAL.hint, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

/* ------------------------------------------------------------------ owner-drawn buttons */

static Ctl* ctlOf(HWND h) { return (Ctl*)GetWindowLongPtrW(h, GWLP_USERDATA); }

/* The colour under a control, so its corners blend in. */
static COLORREF underColour(HWND h)
{
	RECT r;
	POINT p;

	if (GetParent(h) == hHelpWnd && hHelpWnd) return PAL.page;   /* the Help footer */
	if (ctlOf(h) && ctlOf(h)->kind == K_THEMESEG) return PAL.field;
	if (GetParent(h) == hThemeWnd && hThemeWnd) return ctlOf(h) && ctlOf(h)->kind == K_PRIMARY ? PAL.page : PAL.card;
	if (GetParent(h) != hMain) return PAL.card;
	GetWindowRect(h, &r);
	p.x = r.left; p.y = r.top;
	ScreenToClient(hMain, &p);
	if (p.y < SC(BAND_H)) return PAL.band;
	if (PtInRect(&gSegTrack, p)) return PAL.field;
	if (p.y < gCardOpt.top) return PAL.page;
	if (h == hCntDec || h == hCntInc) return PAL.field;
	return PAL.card;
}

static void dropText(HWND h, wchar_t* buf, int cap)
{
	if (h == hProfile)
	{
		int p = currentPreset();
		const wchar_t* name = p >= 0 ? T(PRESET_STR[p]) : T(S_PRESET_CUSTOM);
		_snwprintf(buf, cap, T(S_PROFILE_FMT), headLen(name), name);
	}
	else if (h == hSymSet)
	{
		const wchar_t* s = T(S_SYM_BASIC + gOpt.symbolSet);
		_snwprintf(buf, cap, L"%.*ls", headLen(s), s);
	}
	else
	{
		const wchar_t* s = T(S_SEP_HYPHEN + gOpt.separator);
		_snwprintf(buf, cap, L"%.*ls", headLen(s), s);
	}
	buf[cap - 1] = 0;
}

static void drawButton(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	Ctl* c = ctlOf(d->hwndItem);
	RECT r = d->rcItem, t;
	int down = (d->itemState & ODS_SELECTED) != 0, dis = (d->itemState & ODS_DISABLED) != 0;
	int focus = (d->itemState & ODS_FOCUS) && !(d->itemState & ODS_NOFOCUSRECT);
	int hot = c && c->hot;
	COLORREF under = underColour(d->hwndItem);
	HDC mem;
	HBITMAP bmp;
	HGDIOBJ ob;
	int w = r.right - r.left, h = r.bottom - r.top;
	wchar_t buf[160];

	if (!c) return;

	/* drawn off screen, then copied in one go: no flicker on hover */
	mem = CreateCompatibleDC(dc);
	bmp = CreateCompatibleBitmap(dc, w, h);
	ob = SelectObject(mem, bmp);
	SetRect(&r, 0, 0, w, h);
	fillSolid(mem, &r, under);

	switch (c->kind)
	{
	case K_PRIMARY:
	{
		COLORREF bg = dis ? PAL.track : down ? PAL.accentDown : hot ? PAL.accentHover : PAL.accent;
		roundRect(mem, &r, SC(8), bg, CLR_NONE, 0);
		if (focus) { RECT f = r; InflateRect(&f, -SC(2), -SC(2)); roundRect(mem, &f, SC(6), CLR_NONE, PAL.onAccent, (float)SC(1)); }
		GetWindowTextW(d->hwndItem, buf, 160);
		t = r;
		drawText(mem, buf, -1, &t, fBold, dis ? PAL.hint : PAL.onAccent, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		break;
	}
	case K_SEG:
	{
		int on = gOpt.mode == c->value;
		if (on) roundRect(mem, &r, SC(7), PAL.accent, CLR_NONE, 0);
		else if (hot) roundRect(mem, &r, SC(7), PAL.accentSoft, CLR_NONE, 0);
		if (focus) { RECT f = r; InflateRect(&f, -SC(2), -SC(2)); roundRect(mem, &f, SC(5), CLR_NONE, on ? PAL.onAccent : PAL.accent, (float)SC(1)); }
		t = r;
		drawText(mem, T(c->str), -1, &t, on ? fBold : fUi, on ? PAL.onAccent : PAL.text, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		break;
	}
	case K_CHIP:
	{
		int on = c->bind && *c->bind;
		COLORREF bg = on ? PAL.accentSoft : (hot ? PAL.field : PAL.card), line = on ? PAL.accent : PAL.fieldBorder;
		int cs = SC(12);
		roundRect(mem, &r, h / 2, bg, focus ? PAL.accent : line, focus ? (float)SC(2) : (float)SC(1));
		t = r;
		if (on)
		{
			glyphCheck(mem, r.left + SC(12), (h - cs) / 2, cs, PAL.accent, (float)SC(2));
			t.left += SC(28);
			t.right -= SC(12);
			drawText(mem, T(c->str), -1, &t, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		}
		else drawText(mem, T(c->str), -1, &t, fUi, PAL.hint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		break;
	}
	case K_TOGGLE:
	{
		int on = c->bind && *c->bind, sw = SC(34), sh = SC(18), sy = (h - sh) / 2, kr = SC(6);
		RECT s;
		SetRect(&s, 0, sy, sw, sy + sh);
		roundRect(mem, &s, sh / 2, on ? (hot ? PAL.accentHover : PAL.accent) : (hot ? mix(PAL.track, PAL.text, 0.12) : PAL.track), CLR_NONE, 0);
		circle(mem, on ? (float)(sw - sh / 2) : (float)(sh / 2), (float)(sy + sh / 2), (float)kr,
		       on ? PAL.onAccent : (gDark ? PAL.hint : RGB(255, 255, 255)), CLR_NONE, 0);
		if (focus) { RECT f = s; InflateRect(&f, SC(2), SC(2)); roundRect(mem, &f, sh / 2 + SC(2), CLR_NONE, PAL.accent, (float)SC(1)); }
		t = r;
		t.left = sw + SC(10);
		drawText(mem, T(c->str), headLen(T(c->str)), &t, fUi, dis ? PAL.hint : PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		break;
	}
	case K_DROP:
	{
		COLORREF bg = hot || down ? PAL.accentSoft : PAL.field;
		roundRect(mem, &r, SC(8), bg, focus ? PAL.accent : PAL.fieldBorder, focus ? (float)SC(2) : (float)SC(1));
		dropText(d->hwndItem, buf, 160);
		t = r;
		t.left += SC(12);
		t.right -= SC(30);
		drawText(mem, buf, -1, &t, fUi, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		glyphChevron(mem, r.right - SC(16), h / 2, SC(9), PAL.hint);
		break;
	}
	case K_THEMESEG:
	{
		int on = gTheme == c->value;
		if (on) roundRect(mem, &r, SC(7), PAL.accent, CLR_NONE, 0);
		else if (hot) roundRect(mem, &r, SC(7), PAL.accentSoft, CLR_NONE, 0);
		if (focus) { RECT f = r; InflateRect(&f, -SC(2), -SC(2)); roundRect(mem, &f, SC(5), CLR_NONE, on ? PAL.onAccent : PAL.accent, (float)SC(1)); }
		t = r;
		drawText(mem, T(c->str), -1, &t, on ? fBold : fUi, on ? PAL.onAccent : PAL.text, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		break;
	}
	case K_SWATCH:
	{
		/* a tile: the icon in that colour and its name; the chosen one is ringed */
		int on = gAccent == c->value, is = SC(44);
		RECT tile = r;
		roundRect(mem, &tile, SC(10), on ? PAL.accentSoft : hot ? PAL.field : PAL.card,
		          on || focus ? PAL.accent : PAL.border, on ? (float)SC(2) : (float)SC(1));
		if (gSwatchIcon[c->value])
			DrawIconEx(mem, (w - is) / 2, SC(10), gSwatchIcon[c->value], is, is, 0, NULL, DI_NORMAL);
		SetRect(&t, SC(2), SC(10) + is + SC(4), w - SC(2), h - SC(4));
		drawText(mem, T(c->str), -1, &t, on ? fBold : fSmall, PAL.text, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		break;
	}
	case K_ICON:
	case K_STEP:
	{
		int s = c->kind == K_ICON ? SC(16) : SC(12), x = (w - s) / 2, y = (h - s) / 2;
		COLORREF ink = dis ? PAL.track : (hot || down) ? PAL.accent : PAL.hint;
		if (hot || down) roundRect(mem, &r, SC(7), c->kind == K_ICON ? PAL.accentSoft : mix(PAL.field, PAL.accent, 0.15), CLR_NONE, 0);
		if (focus) roundRect(mem, &r, SC(7), CLR_NONE, PAL.accent, (float)SC(1));
		if (c->glyph == G_COPY)
		{
			if (gMarkRow == -2) glyphCheck(mem, x, y, s, PAL.ok, (float)SC(2));
			else glyphCopy(mem, x, y, s, ink, (hot || down) ? PAL.accentSoft : under);
		}
		else if (c->glyph == G_SAVE) glyphSave(mem, x, y, s, ink);
		else glyphPlusMinus(mem, w / 2, h / 2, s, ink, c->glyph == G_PLUS);
		break;
	}
	}

	BitBlt(dc, d->rcItem.left, d->rcItem.top, w, h, mem, 0, 0, SRCCOPY);
	SelectObject(mem, ob);
	DeleteObject(bmp);
	DeleteDC(mem);
}

static LRESULT CALLBACK ctlProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref)
{
	Ctl* c = ctlOf(hwnd);

	(void)id; (void)ref;
	switch (msg)
	{
	case WM_MOUSEMOVE:
		if (c && !c->hot)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
			c->hot = 1;
			TrackMouseEvent(&tme);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		break;
	case WM_MOUSELEAVE:
		if (c) { c->hot = 0; InvalidateRect(hwnd, NULL, FALSE); }
		break;
	case WM_SETCURSOR:
		SetCursor(LoadCursorW(NULL, IDC_HAND));
		return TRUE;
	case WM_LBUTTONDBLCLK:
		/* a quick second click is a click: toggles and steppers must not swallow it */
		return DefSubclassProc(hwnd, WM_LBUTTONDOWN, wp, lp);
	case WM_ERASEBKGND:
		return 1;
	case WM_NCDESTROY:
		free(c);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		RemoveWindowSubclass(hwnd, ctlProc, 1);
		break;
	}
	return DefSubclassProc(hwnd, msg, wp, lp);
}

static HWND mkButton(HWND parent, int id, int kind, int str, int* bind, int value, int glyph)
{
	HWND b = CreateWindowExW(0, L"BUTTON", str >= 0 ? T(str) : L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
	                         0, 0, 10, 10, parent, (HMENU)(INT_PTR)id, gInst, NULL);
	Ctl* c = (Ctl*)calloc(1, sizeof(Ctl));

	if (c) { c->kind = kind; c->str = str; c->bind = bind; c->value = value; c->glyph = glyph; }
	SetWindowLongPtrW(b, GWLP_USERDATA, (LONG_PTR)c);
	SetWindowSubclass(b, ctlProc, 1, 0);
	SendMessageW(b, WM_SETFONT, (WPARAM)fUi, FALSE);
	return b;
}

/* ------------------------------------------------------------------ the slider */

/* A slider of our own: the trackbar cannot be themed dark, nor drawn anti-aliased. It
 * tells its parent about every change with WM_COMMAND(id, SLN_CHANGE). */
#define SLM_SETRANGE (WM_USER + 1)
#define SLM_SETPOS   (WM_USER + 2)
#define SLM_GETPOS   (WM_USER + 3)
#define SLN_CHANGE   1

typedef struct { int lo, hi, pos, hot, drag; } Slider;

static int sliderX(HWND h, const Slider* s, int pos)
{
	RECT r;
	int pad = SC(10);

	GetClientRect(h, &r);
	if (s->hi <= s->lo) return pad;
	return pad + MulDiv(clampInt(pos, s->lo, s->hi) - s->lo, r.right - 2 * pad, s->hi - s->lo);
}

static void sliderSet(HWND h, Slider* s, int pos, int notify)
{
	pos = clampInt(pos, s->lo, s->hi);
	if (pos == s->pos) return;
	s->pos = pos;
	InvalidateRect(h, NULL, FALSE);
	if (notify) SendMessageW(GetParent(h), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(h), SLN_CHANGE), (LPARAM)h);
}

static void sliderFromX(HWND h, Slider* s, int x)
{
	RECT r;
	int pad = SC(10), span;

	GetClientRect(h, &r);
	span = r.right - 2 * pad;
	if (span <= 0) return;
	sliderSet(h, s, s->lo + MulDiv(clampInt(x - pad, 0, span), s->hi - s->lo, span), 1);
}

static LRESULT CALLBACK sliderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	Slider* s = (Slider*)GetWindowLongPtrW(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_NCCREATE:
		s = (Slider*)calloc(1, sizeof(Slider));
		if (!s) return FALSE;
		s->lo = 0; s->hi = 100;
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)s);
		break;
	case WM_NCDESTROY:
		free(s);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
		break;
	case SLM_SETRANGE: if (s) { s->lo = (int)wp; s->hi = (int)lp; InvalidateRect(hwnd, NULL, FALSE); } return 0;
	case SLM_SETPOS:   if (s) { s->pos = clampInt((int)wp, s->lo, s->hi); InvalidateRect(hwnd, NULL, FALSE); } return 0;
	case SLM_GETPOS:   return s ? s->pos : 0;
	case WM_GETDLGCODE: return DLGC_WANTARROWS;
	case WM_SETFOCUS:
	case WM_KILLFOCUS: InvalidateRect(hwnd, NULL, FALSE); return 0;
	case WM_ERASEBKGND: return 1;
	case WM_SETCURSOR: SetCursor(LoadCursorW(NULL, IDC_HAND)); return TRUE;
	case WM_MOUSEMOVE:
		if (s && !s->hot)
		{
			TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
			s->hot = 1;
			TrackMouseEvent(&tme);
			InvalidateRect(hwnd, NULL, FALSE);
		}
		if (s && s->drag) sliderFromX(hwnd, s, (short)LOWORD(lp));
		return 0;
	case WM_MOUSELEAVE: if (s) { s->hot = 0; InvalidateRect(hwnd, NULL, FALSE); } return 0;
	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		SetCapture(hwnd);
		if (s) { s->drag = 1; sliderFromX(hwnd, s, (short)LOWORD(lp)); }
		return 0;
	case WM_LBUTTONUP:
	case WM_CAPTURECHANGED:
		if (s && s->drag) { s->drag = 0; if (msg == WM_LBUTTONUP) ReleaseCapture(); InvalidateRect(hwnd, NULL, FALSE); }
		return 0;
	case WM_MOUSEWHEEL:
		if (s) sliderSet(hwnd, s, s->pos + ((short)HIWORD(wp) > 0 ? 1 : -1), 1);
		return 0;
	case WM_KEYDOWN:
		if (!s) break;
		switch (wp)
		{
		case VK_LEFT:  case VK_DOWN: sliderSet(hwnd, s, s->pos - 1, 1); return 0;
		case VK_RIGHT: case VK_UP:   sliderSet(hwnd, s, s->pos + 1, 1); return 0;
		case VK_PRIOR: sliderSet(hwnd, s, s->pos + 4, 1); return 0;
		case VK_NEXT:  sliderSet(hwnd, s, s->pos - 4, 1); return 0;
		case VK_HOME:  sliderSet(hwnd, s, s->lo, 1); return 0;
		case VK_END:   sliderSet(hwnd, s, s->hi, 1); return 0;
		}
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps), mem = CreateCompatibleDC(dc);
		RECT r, tr;
		HBITMAP bmp;
		HGDIOBJ ob;
		int x, cy, th = SC(4), kr;

		GetClientRect(hwnd, &r);
		bmp = CreateCompatibleBitmap(dc, r.right, r.bottom);
		ob = SelectObject(mem, bmp);
		fillSolid(mem, &r, PAL.card);
		if (s)
		{
			x = sliderX(hwnd, s, s->pos);
			cy = r.bottom / 2;
			SetRect(&tr, SC(10), cy - th / 2, r.right - SC(10), cy + th / 2 + (th & 1));
			roundRect(mem, &tr, th / 2, PAL.track, CLR_NONE, 0);
			tr.right = x;
			roundRect(mem, &tr, th / 2, PAL.accent, CLR_NONE, 0);
			kr = SC(s->hot || s->drag ? 9 : 8);
			if (GetFocus() == hwnd) circle(mem, (float)x, (float)cy, (float)(kr + SC(3)), mix(PAL.card, PAL.accent, 0.25), CLR_NONE, 0);
			circle(mem, (float)x, (float)cy, (float)kr, PAL.accent, CLR_NONE, 0);
			circle(mem, (float)x, (float)cy, (float)(kr - SC(3)), gDark ? PAL.card : RGB(255, 255, 255), CLR_NONE, 0);
		}
		BitBlt(dc, 0, 0, r.right, r.bottom, mem, 0, 0, SRCCOPY);
		SelectObject(mem, ob);
		DeleteObject(bmp);
		DeleteDC(mem);
		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

/* ------------------------------------------------------------------ menus */

/* Owner-drawn: their text changes with the language, and Windows draws a menu bar white
 * however dark the window is. Each item carries its string id in the item data, plus flags. */

#define MENU_SEP      ((ULONG_PTR)-1)
#define MENU_BAR      ((ULONG_PTR)0x10000)    /* an item ON the bar */
#define MENU_ID(d)    ((int)((d) & 0xFFFF))
#define MENU_ISBAR(d) (((d) & MENU_BAR) != 0)

static void measureMenuItem(MEASUREITEMSTRUCT* mi)
{
	const wchar_t *t, *tab;
	int w;

	if (mi->itemData == MENU_SEP)
	{
		mi->itemWidth = SC(80);
		mi->itemHeight = SC(9);
		return;
	}

	t = T(MENU_ID(mi->itemData));
	tab = wcschr(t, L'\t');
	w = textWidth(fUi, t, tab ? (int)(tab - t) : (int)wcslen(t));

	if (MENU_ISBAR(mi->itemData))
	{
		mi->itemWidth = (UINT)(w + SC(18));
		mi->itemHeight = (UINT)SC(26);
		return;
	}
	w += SC(36) + SC(28);
	if (tab) w += SC(24) + textWidth(fUi, tab + 1, -1);
	mi->itemWidth = (UINT)w;
	mi->itemHeight = (UINT)SC(30);
}

static void drawMenuItem(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC;
	RECT r = d->rcItem, t;
	int hot = (d->itemState & (ODS_SELECTED | ODS_HOTLIGHT)) != 0;
	int dis = (d->itemState & (ODS_DISABLED | ODS_GRAYED)) != 0;
	const wchar_t *s, *tab;
	COLORREF ink;

	fillSolid(dc, &r, PAL.menuBg);

	if (d->itemData == MENU_SEP)
	{
		RECT l = r;
		l.left += SC(10); l.right -= SC(10);
		l.top = (r.top + r.bottom) / 2; l.bottom = l.top + 1;
		fillSolid(dc, &l, PAL.border);
		return;
	}

	s = T(MENU_ID(d->itemData));
	ink = dis ? PAL.hint : PAL.text;

	if (MENU_ISBAR(d->itemData))
	{
		if (hot) { RECT h = r; InflateRect(&h, -SC(2), -SC(3)); roundRect(dc, &h, SC(5), PAL.accentSoft, CLR_NONE, 0); }
		t = r;
		{
			HGDIOBJ of = SelectObject(dc, fUi);
			SetBkMode(dc, TRANSPARENT);
			SetTextColor(dc, ink);
			DrawTextW(dc, s, -1, &t, DT_CENTER | DT_VCENTER | DT_SINGLELINE | ((d->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0));
			SelectObject(dc, of);
		}
		return;
	}

	if (hot && !dis) { RECT h = r; InflateRect(&h, -SC(4), -SC(1)); roundRect(dc, &h, SC(6), PAL.accentSoft, CLR_NONE, 0); }

	if (d->itemState & ODS_CHECKED)
		glyphCheck(dc, r.left + SC(13), (r.top + r.bottom) / 2 - SC(7), SC(14), PAL.accent, (float)SC(2));

	tab = wcschr(s, L'\t');
	t = r;
	t.left += SC(36);
	t.right -= SC(16);
	{
		HGDIOBJ of = SelectObject(dc, fUi);
		SetBkMode(dc, TRANSPARENT);
		SetTextColor(dc, ink);
		DrawTextW(dc, s, tab ? (int)(tab - s) : -1, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE | ((d->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0));
		if (tab)
		{
			SetTextColor(dc, PAL.hint);
			DrawTextW(dc, tab + 1, -1, &t, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
		}
		SelectObject(dc, of);
	}
}

static void addItem(HMENU m, UINT id, int str, int checked)
{
	AppendMenuW(m, MF_OWNERDRAW | (checked ? MF_CHECKED : 0), id, (LPCWSTR)(ULONG_PTR)str);
}

static void addSep(HMENU m) { AppendMenuW(m, MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCWSTR)MENU_SEP); }

static void setMenuBrush(HMENU m)
{
	MENUINFO mi;
	static HBRUSH br = NULL;
	HBRUSH old = br;

	br = CreateSolidBrush(PAL.menuBg);
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
	mi.hbrBack = br;
	SetMenuInfo(m, &mi);
	if (old) DeleteObject(old);
}

static void fillPresetMenu(HMENU m)
{
	int i, cur = currentPreset();

	addItem(m, IDM_MODE_CHARS, S_MENU_MODE_CHARS, gOpt.mode == PW_CHARS);
	addItem(m, IDM_MODE_WORDS, S_MENU_MODE_WORDS, gOpt.mode == PW_WORDS);
	addItem(m, IDM_MODE_PIN, S_MENU_MODE_PIN, gOpt.mode == PW_PIN);
	addSep(m);
	for (i = 0; i < P_COUNT; i++) addItem(m, IDM_PRESET_0 + i, PRESET_STR[i], cur == i);
}

static void buildMenu(HWND hwnd)
{
	HMENU file = CreatePopupMenu(), kind = CreatePopupMenu(), view = CreatePopupMenu();
	HMENU lang = CreatePopupMenu(), help = CreatePopupMenu(), bar = CreateMenu();

	addItem(file, IDM_GENERATE, S_MENU_GENERATE, 0);
	addSep(file);
	addItem(file, IDM_COPY, S_MENU_COPY, 0);
	addItem(file, IDM_COPYALL, S_MENU_COPYALL, 0);
	addItem(file, IDM_SAVE, S_MENU_SAVE, 0);
	addSep(file);
	addItem(file, IDM_EXIT, S_MENU_EXIT, 0);

	fillPresetMenu(kind);

	addItem(view, IDM_THEME_0 + 0, S_MENU_THEME_AUTO, gTheme == 0);
	addItem(view, IDM_THEME_0 + 1, S_MENU_THEME_LIGHT, gTheme == 1);
	addItem(view, IDM_THEME_0 + 2, S_MENU_THEME_DARK, gTheme == 2);
	addSep(view);
	addItem(view, IDM_THEMEDLG, S_MENU_THEMEDLG, 0);

	addItem(lang, IDM_LANG_EN, S_MENU_EN, gLang == LANG_EN);
	addItem(lang, IDM_LANG_JP, S_MENU_JP, gLang == LANG_JP);
	addItem(lang, IDM_LANG_ES, S_MENU_ES, gLang == LANG_ES);

	addItem(help, IDM_HELP, S_MENU_HELPTOPICS, 0);
	addSep(help);
	addItem(help, IDM_ABOUT, S_MENU_ABOUT, 0);

	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)file, (LPCWSTR)(ULONG_PTR)(S_MENU_FILE | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)kind, (LPCWSTR)(ULONG_PTR)(S_MENU_KIND | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)view, (LPCWSTR)(ULONG_PTR)(S_MENU_VIEW | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)lang, (LPCWSTR)(ULONG_PTR)(S_MENU_LANG | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)help, (LPCWSTR)(ULONG_PTR)(S_MENU_HELP | MENU_BAR));

	setMenuBrush(bar);
	SetMenu(hwnd, bar);
	if (hMenu) DestroyMenu(hMenu);
	hMenu = bar;
	DrawMenuBar(hwnd);
}

/* A drop-down button's list, under the button. Returns the command picked, or 0. */
static UINT dropMenu(HWND button)
{
	HMENU m = CreatePopupMenu();
	RECT r;
	UINT cmd;
	int i;

	if (button == hProfile) fillPresetMenu(m);
	else if (button == hSymSet)
		for (i = 0; i <= PW_SYM_CUSTOM; i++) addItem(m, IDM_SYM_0 + i, S_SYM_BASIC + i, gOpt.symbolSet == i);
	else
		for (i = 0; i < PW_SEP_COUNT; i++) addItem(m, IDM_SEP_0 + i, S_SEP_HYPHEN + i, gOpt.separator == i);

	setMenuBrush(m);
	GetWindowRect(button, &r);
	cmd = (UINT)TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY,
	                           r.left, r.bottom + SC(4), 0, hMain, NULL);
	DestroyMenu(m);
	return cmd;
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

/* The strength line is painted, not a window: its tip is tied to a rectangle. */
static void setStrengthTip(UINT msg)
{
	TOOLINFOW ti;

	if (!hTip) return;
	ZeroMemory(&ti, sizeof(ti));
	ti.cbSize = sizeof(ti);
	ti.uFlags = TTF_SUBCLASS;
	ti.hwnd = hMain;
	ti.uId = 1;
	ti.rect = gStrength;
	ti.lpszText = (LPWSTR)T(S_TIP_STRENGTH);
	SendMessageW(hTip, msg, 0, (LPARAM)&ti);
}

static void allTips(UINT msg)
{
	setTip(hSeg[0], S_TIP_MODE_CHARS, msg);  setTip(hSeg[1], S_TIP_MODE_WORDS, msg);
	setTip(hSeg[2], S_TIP_MODE_PIN, msg);    setTip(hProfile, S_TIP_PROFILE, msg);
	setTip(hChip[0], S_TIP_CHIPS, msg);      setTip(hChip[1], S_TIP_CHIPS, msg);
	setTip(hChip[2], S_TIP_CHIPS, msg);      setTip(hChip[3], S_TIP_CHIPS, msg);
	setTip(hSymSet, S_TIP_SYMSET, msg);      setTip(hTgAmbig, S_TIP_AMBIG, msg);
	setTip(hTgNoRepeat, S_TIP_NOREPEAT, msg); setTip(hTgStart, S_TIP_START, msg);
	setTip(hExclude, S_TIP_EXCLUDE, msg);    setTip(hCustom, S_TIP_CUSTOM, msg);
	setTip(hSeparator, S_TIP_SEPARATOR, msg); setTip(hTgCap, S_TIP_CAP, msg);
	setTip(hTgNumber, S_TIP_NUMBER, msg);    setTip(hTgPinRepeat, S_TIP_NOREPEAT, msg);
	setTip(hTgNoSeq, S_TIP_NOSEQ, msg);      setTip(hGenerate, S_TIP_GENERATE, msg);
	setTip(hTgClear, S_TIP_CLEAR, msg);      setTip(hCopyAll, S_TIP_COPYALL, msg);
	setTip(hSave, S_TIP_SAVE, msg);
	setStrengthTip(msg);
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

/* One password row: number, the password (digits and symbols coloured), the copy icon. */
static void drawRow(const DRAWITEMSTRUCT* d)
{
	HDC dc = d->hDC, mem;
	RECT r = d->rcItem, t, hl;
	int row = (int)d->itemData, sel = (d->itemState & ODS_SELECTED) != 0, hot;
	int w = r.right - r.left, h = r.bottom - r.top, iconSz = SC(16), iconX, x, cw, maxX, i, len;
	HBITMAP bmp;
	HGDIOBJ ob, of;
	TEXTMETRICW tm;
	SIZE m;
	HFONT f = fPw;
	wchar_t num[8];
	COLORREF bg;

	if ((int)d->itemID < 0 || row < 0 || row >= gPwN) { fillSolid(dc, &r, PAL.card); return; }

	mem = CreateCompatibleDC(dc);
	bmp = CreateCompatibleBitmap(dc, w, h);
	ob = SelectObject(mem, bmp);
	SetRect(&r, 0, 0, w, h);
	fillSolid(mem, &r, PAL.card);

	hot = (gHotRow == row);
	bg = sel ? PAL.accentSoft : hot ? PAL.field : PAL.card;
	hl = r;
	InflateRect(&hl, -SC(2), -SC(2));
	if (sel || hot) roundRect(mem, &hl, SC(8), bg, CLR_NONE, 0);
	if (sel)
	{
		RECT a = hl;
		a.left += SC(4); a.right = a.left + SC(3);
		InflateRect(&a, 0, -SC(7));
		roundRect(mem, &a, SC(2), PAL.accent, CLR_NONE, 0);
	}

	/* number */
	_snwprintf(num, 8, L"%d", row + 1);
	num[7] = 0;
	SetRect(&t, SC(8), 0, SC(36), h);
	drawText(mem, num, -1, &t, fSmall, PAL.hint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);

	/* password, one character at a time in a fixed pitch; a long passphrase in a smaller one */
	len = (int)strlen(gPw[row]);
	x = SC(50);
	iconX = w - SC(ICON_ZONE) + (SC(ICON_ZONE) - iconSz) / 2;
	maxX = w - SC(ICON_ZONE) - SC(4);
	of = SelectObject(mem, f);
	GetTextExtentPoint32W(mem, L"M", 1, &m);
	if (x + m.cx * len > maxX)
	{
		f = fPwSmall;
		SelectObject(mem, f);
		GetTextExtentPoint32W(mem, L"M", 1, &m);
	}
	GetTextMetricsW(mem, &tm);
	cw = m.cx;
	SetBkMode(mem, TRANSPARENT);

	for (i = 0; i < len; i++)
	{
		char c = gPw[row][i];
		wchar_t wc = (wchar_t)(unsigned char)c;
		int y = (h - tm.tmHeight) / 2;

		if (x + 2 * cw > maxX && i + 1 < len)
		{
			SetTextColor(mem, PAL.hint);
			TextOutW(mem, x, y, L"\x2026", 1);
			break;
		}

		if (c >= '0' && c <= '9')                           SetTextColor(mem, PAL.digit);
		else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) SetTextColor(mem, PAL.text);
		else if (gPwMode == PW_WORDS)                       SetTextColor(mem, PAL.hint);
		else                                                SetTextColor(mem, PAL.symbol);

		if (c == ' ' && gPwMode == PW_WORDS) { SetTextColor(mem, PAL.track); wc = 0x00B7; }
		TextOutW(mem, x, y, &wc, 1);
		x += cw;
	}
	SelectObject(mem, of);

	/* copy icon, or the check mark right after this row (or all rows) was copied */
	if (gMarkRow == row || gMarkRow == -2)
		glyphCheck(mem, iconX, (h - iconSz) / 2, iconSz, PAL.ok, (float)SC(2));
	else
	{
		int on = hot && gHotIcon;
		if (on) { RECT ib; SetRect(&ib, iconX - SC(7), (h - iconSz) / 2 - SC(7), iconX + iconSz + SC(7), (h + iconSz) / 2 + SC(7)); roundRect(mem, &ib, SC(7), PAL.accentSoft, CLR_NONE, 0); }
		glyphCopy(mem, iconX, (h - iconSz) / 2, iconSz, on ? PAL.accent : PAL.hint, on ? PAL.accentSoft : (sel || hot) ? bg : PAL.card);
	}

	BitBlt(dc, d->rcItem.left, d->rcItem.top, w, h, mem, 0, 0, SRCCOPY);
	SelectObject(mem, ob);
	DeleteObject(bmp);
	DeleteDC(mem);
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

	case WM_ERASEBKGND:
	{
		/* below the last row */
		RECT rc;
		GetClientRect(hwnd, &rc);
		fillSolid((HDC)wp, &rc, PAL.card);
		return 1;
	}

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

static HWND mkEdit(int id, int limit, DWORD style)
{
	HWND e = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | style,
	                         0, 0, 10, 10, hMain, (HMENU)(INT_PTR)id, gInst, NULL);
	SendMessageW(e, WM_SETFONT, (WPARAM)fUi, FALSE);
	SendMessageW(e, EM_SETLIMITTEXT, (WPARAM)limit, 0);
	return e;
}

static void buildControls(void)
{
	WNDCLASSW wc;
	int i;

	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;          /* the track is drawn across the whole width */
	wc.lpfnWndProc = sliderProc;
	wc.hInstance = gInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"WyrmholdSlider";
	RegisterClassW(&wc);

	for (i = 0; i < 3; i++)
		hSeg[i] = mkButton(hMain, IDC_SEG_CHARS + i, K_SEG, S_MODE_CHARS + i, NULL, PW_CHARS + i, G_NONE);
	hProfile = mkButton(hMain, IDC_PROFILE, K_DROP, -1, NULL, 0, G_NONE);

	hSlider = CreateWindowExW(0, L"WyrmholdSlider", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
	                          0, 0, 10, 10, hMain, (HMENU)(INT_PTR)IDC_SLIDER, gInst, NULL);
	hLen = mkEdit(IDC_LEN, 3, ES_NUMBER | ES_CENTER);
	hCntDec = mkButton(hMain, IDC_CNT_DEC, K_STEP, -1, NULL, 0, G_MINUS);
	hCnt = mkEdit(IDC_CNT, 2, ES_NUMBER | ES_CENTER);
	hCntInc = mkButton(hMain, IDC_CNT_INC, K_STEP, -1, NULL, 0, G_PLUS);

	hChip[0] = mkButton(hMain, IDC_CHIP_LOWER, K_CHIP, S_CHIP_LOWER, &gOpt.lower, 0, G_NONE);
	hChip[1] = mkButton(hMain, IDC_CHIP_UPPER, K_CHIP, S_CHIP_UPPER, &gOpt.upper, 0, G_NONE);
	hChip[2] = mkButton(hMain, IDC_CHIP_DIGITS, K_CHIP, S_CHIP_DIGITS, &gOpt.digits, 0, G_NONE);
	hChip[3] = mkButton(hMain, IDC_CHIP_SYMBOLS, K_CHIP, S_CHIP_SYMBOLS, &gOpt.symbols, 0, G_NONE);
	hSymSet = mkButton(hMain, IDC_SYMSET, K_DROP, -1, NULL, 0, G_NONE);

	hTgAmbig    = mkButton(hMain, IDC_TG_AMBIG, K_TOGGLE, S_OPT_AMBIG, &gOpt.avoidAmbiguous, 0, G_NONE);
	hTgNoRepeat = mkButton(hMain, IDC_TG_NOREPEAT, K_TOGGLE, S_OPT_NOREPEAT, &gCharNoRepeat, 0, G_NONE);
	hTgStart    = mkButton(hMain, IDC_TG_START, K_TOGGLE, S_OPT_START, &gOpt.startLetter, 0, G_NONE);
	hExclude = mkEdit(IDC_EXCLUDE, PW_SET_MAX - 1, 0);
	hCustom  = mkEdit(IDC_CUSTOM, PW_SET_MAX - 1, 0);

	hSeparator   = mkButton(hMain, IDC_SEPARATOR, K_DROP, -1, NULL, 0, G_NONE);
	hTgCap       = mkButton(hMain, IDC_TG_CAP, K_TOGGLE, S_OPT_CAP, &gOpt.capitalize, 0, G_NONE);
	hTgNumber    = mkButton(hMain, IDC_TG_NUMBER, K_TOGGLE, S_OPT_NUMBER, &gOpt.addNumber, 0, G_NONE);
	hTgPinRepeat = mkButton(hMain, IDC_TG_PINREPEAT, K_TOGGLE, S_OPT_NOREPEAT, &gPinNoRepeat, 0, G_NONE);
	hTgNoSeq     = mkButton(hMain, IDC_TG_NOSEQ, K_TOGGLE, S_OPT_NOSEQ, &gOpt.noSequence, 0, G_NONE);

	hGenerate = mkButton(hMain, IDC_GENERATE, K_PRIMARY, S_GENERATE, NULL, 0, G_NONE);
	SetWindowLongPtrW(hGenerate, GWL_STYLE, GetWindowLongPtrW(hGenerate, GWL_STYLE) | BS_DEFPUSHBUTTON);
	SendMessageW(hGenerate, WM_SETFONT, (WPARAM)fBold, FALSE);

	hTgClear = mkButton(hMain, IDC_TG_CLEAR, K_TOGGLE, S_OPT_CLEAR, &gClearClip, 0, G_NONE);
	hSave    = mkButton(hMain, IDC_SAVE, K_ICON, -1, NULL, 0, G_SAVE);
	hCopyAll = mkButton(hMain, IDC_COPYALL, K_ICON, -1, NULL, 0, G_COPY);

	hList = CreateWindowExW(0, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
	                        LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
	                        0, 0, 10, 10, hMain, (HMENU)(INT_PTR)IDC_LIST, gInst, NULL);
	SendMessageW(hList, WM_SETFONT, (WPARAM)fUi, FALSE);
	SetWindowSubclass(hList, listProc, 1, 0);

	hTip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
	                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hMain, NULL, gInst, NULL);
	SendMessageW(hTip, TTM_SETMAXTIPWIDTH, 0, SC(420));
	SendMessageW(hTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);
	SendMessageW(hTip, WM_SETFONT, (WPARAM)fUi, TRUE);
	allTips(TTM_ADDTOOLW);
}

/* Puts the options into the controls (numbers, character sets, the slider range). */
static void syncControls(void)
{
	wchar_t w[PW_SET_MAX];
	int lo, hi, v, i;

	gSyncing = 1;
	if (gOpt.mode == PW_WORDS)    { lo = PW_MIN_WORDS; hi = PW_MAX_WORDS; v = gOpt.words; }
	else if (gOpt.mode == PW_PIN) { lo = PW_MIN_LEN; hi = 20; v = gOpt.pinLength; }
	else                          { lo = PW_MIN_LEN; hi = 64; v = gOpt.length; }
	SendMessageW(hSlider, SLM_SETRANGE, (WPARAM)lo, (LPARAM)hi);
	SendMessageW(hSlider, SLM_SETPOS, (WPARAM)v, 0);
	_snwprintf(w, 16, L"%d", v); w[15] = 0;
	SetWindowTextW(hLen, w);
	_snwprintf(w, 16, L"%d", gCount); w[15] = 0;
	SetWindowTextW(hCnt, w);
	_snwprintf(w, PW_SET_MAX, L"%hs", gOpt.mode == PW_PIN ? gPinExclude : gCharExclude); w[PW_SET_MAX - 1] = 0;
	SetWindowTextW(hExclude, w);
	_snwprintf(w, PW_SET_MAX, L"%hs", gOpt.customSymbols); w[PW_SET_MAX - 1] = 0;
	SetWindowTextW(hCustom, w);
	gSyncing = 0;

	for (i = 0; i < 3; i++) InvalidateRect(hSeg[i], NULL, FALSE);
	InvalidateRect(hMain, NULL, FALSE);
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

/* ------------------------------------------------------------------ layout */

static void place(HWND h, int x, int y, int w, int hh, int show)
{
	SetWindowPos(h, NULL, x, y, w, hh, SWP_NOZORDER | SWP_NOACTIVATE | (show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
}

static void addLabel(int str, int x, int y, int w, int h, int style)
{
	if (gLabelN >= 16) return;
	gLabels[gLabelN].str = str;
	SetRect(&gLabels[gLabelN].r, x, y, x + w, y + h);
	gLabels[gLabelN].style = style;
	gLabelN++;
}

/* An EDIT inside a painted, rounded field frame of FIELD_H. */
static void placeField(HWND e, int x, int y, int w, int show)
{
	TEXTMETRICW tm;
	HDC dc = GetDC(e);
	HGDIOBJ of = SelectObject(dc, fUi);
	int eh;

	GetTextMetricsW(dc, &tm);
	SelectObject(dc, of);
	ReleaseDC(e, dc);
	eh = tm.tmHeight + SC(2);
	place(e, x + SC(8), y + (SC(FIELD_H) - eh) / 2, w - SC(16), eh, show);
	if (show && gFieldN < 4)
	{
		gFieldEdit[gFieldN] = e;
		SetRect(&gFieldBox[gFieldN], x, y, x + w, y + SC(FIELD_H));
		gFieldN++;
	}
}

static int toggleWidth(int str) { return SC(34 + 10 + 10) + textWidth(fUi, T(str), headLen(T(str))); }

static void layout(void)
{
	RECT rc;
	int W, H, m = SC(MARGIN), y, x, i, segW, cx, cr, labelW, r1, r2, r3, r4, rb, sliderR, cntX, chipX;
	int mode = gOpt.mode, chars = mode == PW_CHARS, words = mode == PW_WORDS, pin = mode == PW_PIN;
	static const int rowLabels[] = { S_LBL_LENGTH, S_LBL_WORDS, S_LBL_DIGITS, S_LBL_INCLUDE, S_LBL_SEPARATOR, S_LBL_EXCLUDE };

	if (!hList) return;
	GetClientRect(hMain, &rc);
	W = rc.right;
	H = rc.bottom;
	gLabelN = 0;
	gFieldN = 0;

	/* the kind of password, and the profile on the right */
	y = SC(BAND_H) + SC(16);
	segW = SC(120);
	for (i = 0; i < 3; i++)
	{
		int tw = textWidth(fBold, T(S_MODE_CHARS + i), -1) + SC(36);
		if (tw > segW) segW = tw;
	}
	SetRect(&gSegTrack, m, y, m + 3 * segW + SC(4) * 4, y + SC(SEG_H));
	for (i = 0; i < 3; i++)
		place(hSeg[i], m + SC(4) + i * (segW + SC(4)), y + SC(4), segW, SC(SEG_H - 8), 1);
	{
		int pw = W - gSegTrack.right - SC(16) - m;
		if (pw > SC(330)) pw = SC(330);
		place(hProfile, W - m - pw, y + SC(4), pw, SC(SEG_H - 8), 1);
	}

	/* the options card */
	y += SC(SEG_H) + SC(14);
	SetRect(&gCardOpt, m, y, W - m, y + SC(CARD_OPT_H));
	cx = m + SC(20);
	cr = W - m - SC(20);
	r1 = y + SC(18);
	r2 = r1 + SC(48);
	r3 = r2 + SC(46);
	r4 = r3 + SC(44);
	rb = gCardOpt.bottom - SC(18) - SC(44);

	labelW = 0;
	for (i = 0; i < 6; i++)
	{
		int tw = textWidth(fUi, T(rowLabels[i]), -1);
		if (tw > labelW) labelW = tw;
	}
	labelW += SC(18);

	/* row 1: slider + number, and how many on the right */
	{
		int cntLbl = textWidth(fUi, T(S_LBL_COUNT), -1) + SC(12), stepW = SC(116);
		cntX = cr - stepW;
		addLabel(chars ? S_LBL_LENGTH : words ? S_LBL_WORDS : S_LBL_DIGITS, cx, r1, labelW, SC(FIELD_H), L_PLAIN);
		addLabel(S_LBL_COUNT, cntX - cntLbl, r1, cntLbl, SC(FIELD_H), L_PLAIN);
		sliderR = cntX - cntLbl - SC(28) - SC(64);
		place(hSlider, cx + labelW - SC(10), r1, sliderR - (cx + labelW - SC(10)), SC(FIELD_H), 1);
		placeField(hLen, sliderR + SC(10), r1, SC(60), 1);
		place(hCntDec, cntX + SC(3), r1 + SC(3), SC(28), SC(FIELD_H - 6), 1);
		placeField(hCnt, cntX + SC(32), r1, stepW - SC(64), 1);
		place(hCntInc, cntX + stepW - SC(31), r1 + SC(3), SC(28), SC(FIELD_H - 6), 1);
		/* the stepper's frame spans both buttons */
		gFieldBox[gFieldN - 1].left = cntX;
		gFieldBox[gFieldN - 1].right = cntX + stepW;
	}

	/* row 2 */
	chipX = cx + labelW;
	if (chars)
	{
		int symW = SC(220), symLbl = textWidth(fUi, T(S_LBL_SYMBOLS), -1) + SC(12);
		addLabel(S_LBL_INCLUDE, cx, r2, labelW, SC(FIELD_H), L_PLAIN);
		for (i = 0; i < 4; i++)
		{
			int cw = textWidth(fBold, T(S_CHIP_LOWER + i), -1) + SC(42);
			place(hChip[i], chipX, r2 + SC(1), cw, SC(30), 1);
			chipX += cw + SC(8);
		}
		if (chipX + SC(16) + symLbl + symW > cr) symW = cr - chipX - SC(16) - symLbl;
		addLabel(S_LBL_SYMBOLS, cr - symW - symLbl, r2, symLbl, SC(FIELD_H), L_PLAIN);
		place(hSymSet, cr - symW, r2, symW, SC(FIELD_H), 1);
	}
	else for (i = 0; i < 4; i++) place(hChip[i], 0, 0, 0, 0, 0);
	if (!chars) place(hSymSet, 0, 0, 0, 0, 0);

	if (words)
	{
		addLabel(S_LBL_SEPARATOR, cx, r2, labelW, SC(FIELD_H), L_PLAIN);
		place(hSeparator, cx + labelW, r2, SC(230), SC(FIELD_H), 1);
	}
	else place(hSeparator, 0, 0, 0, 0, 0);

	/* row 3: switches */
	x = cx;
	if (chars)
	{
		int w1 = toggleWidth(S_OPT_AMBIG), w2 = toggleWidth(S_OPT_NOREPEAT), w3 = toggleWidth(S_OPT_START);
		place(hTgAmbig, x, r3, w1, SC(30), 1);    x += w1 + SC(24);
		place(hTgNoRepeat, x, r3, w2, SC(30), 1); x += w2 + SC(24);
		place(hTgStart, x, r3, w3, SC(30), 1);
	}
	else { place(hTgAmbig, 0, 0, 0, 0, 0); place(hTgNoRepeat, 0, 0, 0, 0, 0); place(hTgStart, 0, 0, 0, 0, 0); }

	if (words)
	{
		int w1 = toggleWidth(S_OPT_CAP), w2 = toggleWidth(S_OPT_NUMBER);
		place(hTgCap, x, r3, w1, SC(30), 1);  x += w1 + SC(24);
		place(hTgNumber, x, r3, w2, SC(30), 1);
		addLabel(S_WORDS_NOTE, cx, r4, cr - cx, SC(FIELD_H), L_HINT);
	}
	else { place(hTgCap, 0, 0, 0, 0, 0); place(hTgNumber, 0, 0, 0, 0, 0); }

	if (pin)
	{
		int w1 = toggleWidth(S_OPT_NOREPEAT), w2 = toggleWidth(S_OPT_NOSEQ);
		place(hTgPinRepeat, cx, r2, w1, SC(30), 1);
		place(hTgNoSeq, cx + w1 + SC(24), r2, w2, SC(30), 1);
		addLabel(S_PIN_NOTE, cx, r3, cr - cx, SC(30), L_HINT);
	}
	else { place(hTgPinRepeat, 0, 0, 0, 0, 0); place(hTgNoSeq, 0, 0, 0, 0, 0); }

	/* row 4: leave out (characters and PIN), my own symbols (characters, custom set) */
	if (chars || pin)
	{
		addLabel(S_LBL_EXCLUDE, cx, r4, labelW, SC(FIELD_H), L_PLAIN);
		placeField(hExclude, cx + labelW, r4, SC(170), 1);
	}
	else place(hExclude, 0, 0, 0, 0, 0);

	if (chars && gOpt.symbolSet == PW_SYM_CUSTOM)
	{
		int lw = textWidth(fUi, T(S_LBL_CUSTOM), -1) + SC(12);
		x = cx + labelW + SC(170) + SC(24);
		addLabel(S_LBL_CUSTOM, x, r4, lw, SC(FIELD_H), L_PLAIN);
		placeField(hCustom, x + lw, r4, SC(200), 1);
	}
	else place(hCustom, 0, 0, 0, 0, 0);

	/* bottom row: strength and the button */
	place(hGenerate, cr - SC(160), rb, SC(160), SC(44), 1);
	SetRect(&gStrength, cx, rb, cr - SC(180), rb + SC(44));

	/* the list card */
	y = gCardOpt.bottom + SC(14);
	SetRect(&gCardList, m, y, W - m, H - SC(STATUS_H));
	addLabel(S_CARD_LIST, cx, y + SC(10), SC(300), SC(32), L_BOLD);
	place(hCopyAll, cr - SC(34) + SC(8), y + SC(10), SC(34), SC(32), 1);
	place(hSave, cr - SC(72) + SC(8), y + SC(10), SC(34), SC(32), 1);
	{
		int tw = toggleWidth(S_OPT_CLEAR);
		place(hTgClear, cr - SC(72) + SC(8) - SC(16) - tw, y + SC(11), tw, SC(30), 1);
	}
	place(hList, m + SC(10), y + SC(LIST_HEAD), W - 2 * m - SC(20), gCardList.bottom - y - SC(LIST_HEAD) - SC(10), 1);

	/* status line */
	SetRect(&gStatusR, m + SC(4), H - SC(STATUS_H), W - m - SC(80), H);
	SetRect(&gVersionR, W - m - SC(80), H - SC(STATUS_H), W - m, H);

	if (hTip) setStrengthTip(TTM_NEWTOOLRECTW);
	/* Every child was moved or resized: repaint all of them whole. Moving a child only
	 * repaints the part that comes into view, so the owner-drawn controls (slider track,
	 * stepper, list rows with their copy icons at the right edge) would keep stale pixels. */
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

/* ------------------------------------------------------------------ painting */

static void paintBand(HDC dc, int W)
{
	RECT band;
	GpGraphics* g = gfx(dc);
	int tx = SC(20 + ICON_SZ + 16);

	SetRect(&band, 0, 0, W, SC(BAND_H));
	if (g)
	{
		GpPointF a = { 0, 0 }, b = { (float)W, 0 };
		GpBrush* br = NULL;
		GdipCreateLineBrush(&a, &b, argb(PAL.band, 255), argb(PAL.band2, 255), 0, &br);
		if (br) { GdipFillRectangle(g, br, 0, 0, (float)W, (float)SC(BAND_H)); GdipDeleteBrush(br); }
		GdipDeleteGraphics(g);
	}
	else fillSolid(dc, &band, PAL.band);

	if (icBand)
		DrawIconEx(dc, SC(20), (SC(BAND_H) - SC(ICON_SZ)) / 2, icBand, SC(ICON_SZ), SC(ICON_SZ), 0, NULL, DI_NORMAL);

	SetRect(&band, tx, SC(12), W - SC(20), SC(44));
	drawText(dc, T(S_APP_TITLE), -1, &band, fTitle, PAL.bandText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	SetRect(&band, tx + SC(1), SC(42), W - SC(20), SC(62));
	drawText(dc, T(S_APP_SUBTITLE), -1, &band, fSub, PAL.bandSub, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

static void paintMain(HDC dc)
{
	RECT rc, r;
	int i;

	GetClientRect(hMain, &rc);
	fillSolid(dc, &rc, PAL.page);
	paintBand(dc, rc.right);

	roundRect(dc, &gSegTrack, SC(10), PAL.field, PAL.border, (float)SC(1));
	roundRect(dc, &gCardOpt, SC(12), PAL.card, PAL.border, (float)SC(1));
	roundRect(dc, &gCardList, SC(12), PAL.card, PAL.border, (float)SC(1));

	for (i = 0; i < gFieldN; i++)
	{
		int focus = GetFocus() == gFieldEdit[i];
		roundRect(dc, &gFieldBox[i], SC(8), PAL.field, focus ? PAL.accent : PAL.fieldBorder, focus ? (float)SC(2) : (float)SC(1));
	}

	for (i = 0; i < gLabelN; i++)
	{
		const Label* l = &gLabels[i];
		r = l->r;
		drawText(dc, T(l->str), -1, &r, l->style == L_BOLD ? fBold : l->style == L_HINT ? fSmall : fUi,
		         l->style == L_HINT ? PAL.hint : PAL.text,
		         DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	paintStrength(dc);

	r = gStatusR;
	drawText(dc, gStatusText, -1, &r, fUi,
	         gStatusKind == ST_ERR ? PAL.err : gStatusKind == ST_WARN ? PAL.warn : gStatusKind == ST_OK ? PAL.ok : PAL.hint,
	         DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	r = gVersionR;
	drawText(dc, L"v" APP_VERSION, -1, &r, fUi, PAL.hint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
}

/* ------------------------------------------------------------------ help window */

/* The help text is one string per language: "# " starts a heading, "\x2022 " a bullet,
 * an empty line a new paragraph. It is laid out here, so it reads like a document. */

static int gHelpScroll = 0, gHelpHeight = 0;

static int helpLayout(HDC dc, int width, int top, int scroll, int paint)
{
	const wchar_t* s = T(S_HELP_BODY);
	int y = top - scroll;

	while (*s)
	{
		const wchar_t* e = wcschr(s, L'\n');
		int n = e ? (int)(e - s) : (int)wcslen(s), heading = 0, bullet = 0, x = SC(28);
		RECT r;
		HFONT f = fUi;
		COLORREF c = PAL.text;

		if (n >= 2 && s[0] == L'#' && s[1] == L' ') { heading = 1; s += 2; n -= 2; f = fBold; c = PAL.accent; y += SC(10); }
		else if (n >= 2 && s[0] == 0x2022 && s[1] == L' ') { bullet = 1; s += 2; n -= 2; x += SC(16); }

		if (n == 0) y += SC(6);
		else
		{
			HGDIOBJ of = SelectObject(dc, f);
			SetRect(&r, x, y, width - SC(28), y);
			DrawTextW(dc, s, n, &r, DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
			if (paint)
			{
				SetBkMode(dc, TRANSPARENT);
				SetTextColor(dc, c);
				r.right = width - SC(28);
				DrawTextW(dc, s, n, &r, DT_WORDBREAK | DT_NOPREFIX);
				if (bullet) circle(dc, (float)(x - SC(10)), (float)(y + SC(10)), (float)SC(2) + 0.5f, PAL.accent, CLR_NONE, 0);
			}
			SelectObject(dc, of);
			y = r.bottom + (heading ? SC(4) : SC(3));
		}
		s += n;
		if (*s == L'\n') s++;
	}
	return y + scroll - top + SC(20);
}

static void helpScrollTo(HWND hwnd, int pos)
{
	RECT rc;
	SCROLLINFO si;
	int view;

	GetClientRect(hwnd, &rc);
	view = rc.bottom - SC(64);
	pos = clampInt(pos, 0, gHelpHeight > view ? gHelpHeight - view : 0);
	gHelpScroll = pos;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMax = gHelpHeight;
	si.nPage = (UINT)(view > 0 ? view : 1);
	si.nPos = pos;
	SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
	InvalidateRect(hwnd, NULL, FALSE);
}

static void helpMeasure(HWND hwnd)
{
	RECT rc;
	HDC dc = GetDC(hwnd);

	GetClientRect(hwnd, &rc);
	gHelpHeight = helpLayout(dc, rc.right, SC(16), 0, 0);
	ReleaseDC(hwnd, dc);
	helpScrollTo(hwnd, gHelpScroll);
}

static LRESULT CALLBACK helpProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps), mem = CreateCompatibleDC(dc);
		RECT rc, bar;
		HBITMAP bmp;
		HGDIOBJ ob;

		GetClientRect(hwnd, &rc);
		bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
		ob = SelectObject(mem, bmp);
		fillSolid(mem, &rc, PAL.card);
		helpLayout(mem, rc.right, SC(16), gHelpScroll, 1);
		SetRect(&bar, 0, rc.bottom - SC(64), rc.right, rc.bottom);
		fillSolid(mem, &bar, PAL.page);
		bar.bottom = bar.top + 1;
		fillSolid(mem, &bar, PAL.border);
		BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
		SelectObject(mem, ob);
		DeleteObject(bmp);
		DeleteDC(mem);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_DRAWITEM:
		drawButton((const DRAWITEMSTRUCT*)lp);
		return TRUE;
	case WM_SIZE:
	{
		HWND ok = GetDlgItem(hwnd, IDOK);
		RECT rc;
		GetClientRect(hwnd, &rc);
		if (ok) SetWindowPos(ok, NULL, rc.right - SC(130), rc.bottom - SC(50), SC(110), SC(36), SWP_NOZORDER);
		helpMeasure(hwnd);
		return 0;
	}
	case WM_VSCROLL:
	{
		SCROLLINFO si;
		int pos = gHelpScroll;
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		switch (LOWORD(wp))
		{
		case SB_LINEUP:        pos -= SC(40); break;
		case SB_LINEDOWN:      pos += SC(40); break;
		case SB_PAGEUP:        pos -= (int)si.nPage; break;
		case SB_PAGEDOWN:      pos += (int)si.nPage; break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION: pos = si.nTrackPos; break;
		case SB_TOP:           pos = 0; break;
		case SB_BOTTOM:        pos = gHelpHeight; break;
		}
		helpScrollTo(hwnd, pos);
		return 0;
	}
	case WM_MOUSEWHEEL:
		helpScrollTo(hwnd, gHelpScroll - (short)HIWORD(wp) * SC(60) / WHEEL_DELTA);
		return 0;
	case WM_KEYDOWN:
		if (wp == VK_DOWN)  { helpScrollTo(hwnd, gHelpScroll + SC(40)); return 0; }
		if (wp == VK_UP)    { helpScrollTo(hwnd, gHelpScroll - SC(40)); return 0; }
		if (wp == VK_NEXT)  { helpScrollTo(hwnd, gHelpScroll + SC(300)); return 0; }
		if (wp == VK_PRIOR) { helpScrollTo(hwnd, gHelpScroll - SC(300)); return 0; }
		break;
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
	HWND ok;
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
		wc.lpszClassName = L"WyrmholdHelp";
		RegisterClassW(&wc);
		registered = 1;
	}

	gHelpScroll = 0;
	GetWindowRect(owner, &r);
	hHelpWnd = CreateWindowExW(0, L"WyrmholdHelp", T(S_HELP_TITLE), WS_OVERLAPPEDWINDOW | WS_VSCROLL,
	                           r.left + SC(40), r.top + SC(30), SC(700), SC(660), owner, NULL, gInst, NULL);
	SendMessageW(hHelpWnd, WM_SETICON, ICON_BIG, (LPARAM)icBig);
	SendMessageW(hHelpWnd, WM_SETICON, ICON_SMALL, (LPARAM)icSmall);
	ok = mkButton(hHelpWnd, IDOK, K_PRIMARY, S_BTN_OK, NULL, 0, G_NONE);
	SetWindowLongPtrW(ok, GWL_STYLE, GetWindowLongPtrW(ok, GWL_STYLE) | BS_DEFPUSHBUTTON);
	SendMessageW(ok, WM_SETFONT, (WPARAM)fBold, FALSE);
	applyTheme(hHelpWnd);
	ShowWindow(hHelpWnd, SW_SHOW);
	SendMessageW(hHelpWnd, WM_SIZE, 0, 0);
	SetFocus(ok);
}

/* ------------------------------------------------------------------ about window */

/* Icon, name, version and date (from version.h), author, project page, what the name means, what the
 * program does, the credits and the licence. The height is measured from the text, so it fits in all
 * three languages. */

#define AB_W     540
#define AB_M     28
#define AB_ICON  72
#define AB_TEXT  146
#define AB_GAP   12

static int textHeight(HDC dc, const wchar_t* s, int width)
{
	RECT r = { 0, 0, width, 0 };

	DrawTextW(dc, s, -1, &r, DT_WORDBREAK | DT_NOPREFIX | DT_CALCRECT);
	return r.bottom - r.top;
}

static int aboutBodyHeight(int width)
{
	HDC dc = GetDC(NULL);
	HGDIOBJ old = SelectObject(dc, fUi);
	int h = textHeight(dc, T(S_ABOUT_MEANING), width) + SC(AB_GAP) + textHeight(dc, T(S_ABOUT_BODY), width) + SC(AB_GAP);

	SelectObject(dc, fSmall);
	h += textHeight(dc, T(S_ABOUT_CREDITS), width) + SC(AB_GAP) + textHeight(dc, T(S_ABOUT_LICENSE), width);
	SelectObject(dc, old);
	ReleaseDC(NULL, dc);
	return h;
}

static LRESULT CALLBACK aboutProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps), mem = CreateCompatibleDC(dc);
		RECT rc, t;
		HBITMAP bmp;
		HGDIOBJ ob;
		wchar_t line[128];
		int tx = SC(AB_M + AB_ICON + 20), wdt;

		GetClientRect(hwnd, &rc);
		bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
		ob = SelectObject(mem, bmp);
		fillSolid(mem, &rc, PAL.card);
		t = rc;
		t.bottom = SC(4);
		fillSolid(mem, &t, PAL.accent);
		if (icAbout) DrawIconEx(mem, SC(AB_M), SC(AB_M), icAbout, SC(AB_ICON), SC(AB_ICON), 0, NULL, DI_NORMAL);

		SetRect(&t, tx, SC(AB_M - 4), rc.right - SC(AB_M), SC(AB_M + 32));
		drawText(mem, T(S_ABOUT_NAME), -1, &t, fTitle, PAL.text, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
		/* built here, so it can never disagree with the executable that prints it */
		_snwprintf(line, 128, T(S_ABOUT_VERSION), APP_VERSION, APP_DATE); line[127] = 0;
		SetRect(&t, tx + SC(2), SC(AB_M + 36), rc.right, SC(AB_M + 58));
		drawText(mem, line, -1, &t, fUi, PAL.hint, DT_LEFT | DT_SINGLELINE);
		_snwprintf(line, 128, T(S_ABOUT_AUTHOR), APP_AUTHOR); line[127] = 0;
		SetRect(&t, tx + SC(2), SC(AB_M + 60), rc.right, SC(AB_M + 84));
		drawText(mem, line, -1, &t, fBold, PAL.text, DT_LEFT | DT_SINGLELINE);
		SetRect(&t, tx + SC(2), SC(AB_M + 88), rc.right - SC(AB_M), SC(AB_M + 110));
		drawText(mem, APP_REPO, -1, &t, fSmall, PAL.hint, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);

		wdt = rc.right - 2 * SC(AB_M);
		SetRect(&t, SC(AB_M), SC(AB_TEXT), rc.right - SC(AB_M), rc.bottom);
		{
			HGDIOBJ of = SelectObject(mem, fUi);
			drawText(mem, T(S_ABOUT_MEANING), -1, &t, fUi, PAL.hint, DT_WORDBREAK);
			t.top += textHeight(mem, T(S_ABOUT_MEANING), wdt) + SC(AB_GAP);
			drawText(mem, T(S_ABOUT_BODY), -1, &t, fUi, PAL.text, DT_WORDBREAK);
			t.top += textHeight(mem, T(S_ABOUT_BODY), wdt) + SC(AB_GAP);
			SelectObject(mem, fSmall);
			drawText(mem, T(S_ABOUT_CREDITS), -1, &t, fSmall, PAL.hint, DT_WORDBREAK);
			t.top += textHeight(mem, T(S_ABOUT_CREDITS), wdt) + SC(AB_GAP);
			drawText(mem, T(S_ABOUT_LICENSE), -1, &t, fSmall, PAL.hint, DT_WORDBREAK);
			SelectObject(mem, of);
		}

		BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
		SelectObject(mem, ob);
		DeleteObject(bmp);
		DeleteDC(mem);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_DRAWITEM:
		drawButton((const DRAWITEMSTRUCT*)lp);
		return TRUE;
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
	int ch;

	if (hAboutWnd) { SetForegroundWindow(hAboutWnd); return; }

	if (!registered)
	{
		WNDCLASSW wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = aboutProc;
		wc.hInstance = gInst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = L"WyrmholdAbout";
		RegisterClassW(&wc);
		registered = 1;
	}

	ch = SC(AB_TEXT) + aboutBodyHeight(SC(AB_W - 2 * AB_M)) + SC(24) + SC(56);
	SetRect(&want, 0, 0, SC(AB_W), ch);
	AdjustWindowRectEx(&want, WS_POPUPWINDOW | WS_CAPTION, FALSE, WS_EX_DLGMODALFRAME);

	GetWindowRect(owner, &r);
	hAboutWnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"WyrmholdAbout", T(S_ABOUT_TITLE),
	                            WS_POPUPWINDOW | WS_CAPTION,
	                            r.left + ((r.right - r.left) - (want.right - want.left)) / 2,
	                            r.top + ((r.bottom - r.top) - (want.bottom - want.top)) / 3,
	                            want.right - want.left, want.bottom - want.top, owner, NULL, gInst, NULL);

	GetClientRect(hAboutWnd, &c);
	ok = mkButton(hAboutWnd, IDOK, K_PRIMARY, S_BTN_OK, NULL, 0, G_NONE);
	SetWindowLongPtrW(ok, GWL_STYLE, GetWindowLongPtrW(ok, GWL_STYLE) | BS_DEFPUSHBUTTON);
	SendMessageW(ok, WM_SETFONT, (WPARAM)fBold, FALSE);
	SetWindowPos(ok, NULL, c.right - SC(AB_M) - SC(110), c.bottom - SC(56), SC(110), SC(36), SWP_NOZORDER);

	applyTheme(hAboutWnd);
	ShowWindow(hAboutWnd, SW_SHOW);
	EnableWindow(owner, FALSE);
	SetFocus(ok);
}

/* ------------------------------------------------------------------ theme window */

/* View -> Theme and colour: light, dark or as Windows, and the colour catalogue as tiles
 * showing the icon in each colour. Every click applies at once, to every open window and to
 * the icon, and is remembered. */

#define TH_COLS  6
#define TH_TILE  92
#define TH_TILEH 86
#define TH_GAP   10
#define TH_M     24

static void setThemeChoice(int theme)
{
	gTheme = theme;
	iniSetInt(L"Theme", gTheme);
	refreshTheme();
	buildMenu(hMain);
	if (hThemeWnd) RedrawWindow(hThemeWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
}

static void setAccent(int accent)
{
	gAccent = clampInt(accent, 0, ACCENT_COUNT - 1);
	iniSetInt(L"Accent", gAccent);
	makePalette();
	makeIcons();
	buildMenu(hMain);
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
	if (hHelpWnd)  RedrawWindow(hHelpWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	if (hAboutWnd) RedrawWindow(hAboutWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
	if (hThemeWnd) RedrawWindow(hThemeWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

static int themeY(int part)
{
	/* 0 appearance label, 1 segments, 2 colour label, 3 tiles */
	static const int y[4] = { 16, 44, 104, 132 };
	return SC(y[part]);
}

static LRESULT CALLBACK themeProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps), mem = CreateCompatibleDC(dc);
		RECT rc, t, track;
		HBITMAP bmp;
		HGDIOBJ ob;

		GetClientRect(hwnd, &rc);
		bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
		ob = SelectObject(mem, bmp);
		fillSolid(mem, &rc, PAL.card);
		SetRect(&t, SC(TH_M), themeY(0), rc.right - SC(TH_M), themeY(0) + SC(24));
		drawText(mem, T(S_THEME_APPEARANCE), -1, &t, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		SetRect(&track, SC(TH_M), themeY(1), SC(TH_M) + 3 * SC(140) + 4 * SC(4), themeY(1) + SC(40));
		roundRect(mem, &track, SC(10), PAL.field, PAL.border, (float)SC(1));
		SetRect(&t, SC(TH_M), themeY(2), rc.right - SC(TH_M), themeY(2) + SC(24));
		drawText(mem, T(S_THEME_COLOUR), -1, &t, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		SetRect(&t, 0, rc.bottom - SC(64), rc.right, rc.bottom);
		fillSolid(mem, &t, PAL.page);
		t.bottom = t.top + 1;
		fillSolid(mem, &t, PAL.border);
		BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
		SelectObject(mem, ob);
		DeleteObject(bmp);
		DeleteDC(mem);
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_DRAWITEM:
		drawButton((const DRAWITEMSTRUCT*)lp);
		return TRUE;
	case WM_COMMAND:
	{
		int id = LOWORD(wp);
		if (id == IDOK || id == IDCANCEL) { DestroyWindow(hwnd); return 0; }
		if (id >= IDM_THEME_0 && id < IDM_THEME_0 + 3) { setThemeChoice(id - IDM_THEME_0); return 0; }
		if (id >= IDM_ACCENT_0 && id < IDM_ACCENT_0 + ACCENT_COUNT) { setAccent(id - IDM_ACCENT_0); return 0; }
		break;
	}
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
	{
		int i;
		hThemeWnd = NULL;
		for (i = 0; i < ACCENT_COUNT; i++) if (gSwatchIcon[i]) { DestroyIcon(gSwatchIcon[i]); gSwatchIcon[i] = NULL; }
		EnableWindow(hMain, TRUE);
		SetForegroundWindow(hMain);
		return 0;
	}
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static void showTheme(HWND owner)
{
	static int registered = 0;
	static const int names[3] = { S_THEME_AUTO, S_THEME_LIGHT, S_THEME_DARK };
	HWND ok;
	RECT r, want, c;
	int i, rows = (ACCENT_COUNT + TH_COLS - 1) / TH_COLS;
	int cw = 2 * TH_M + TH_COLS * TH_TILE + (TH_COLS - 1) * TH_GAP;
	int ch = 132 + rows * TH_TILEH + (rows - 1) * TH_GAP + 24 + 64;

	if (hThemeWnd) { SetForegroundWindow(hThemeWnd); return; }

	if (!registered)
	{
		WNDCLASSW wc;
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = themeProc;
		wc.hInstance = gInst;
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName = L"WyrmholdTheme";
		RegisterClassW(&wc);
		registered = 1;
	}

	for (i = 0; i < ACCENT_COUNT; i++) gSwatchIcon[i] = tintedIconFor(i, SC(44));

	SetRect(&want, 0, 0, SC(cw), SC(ch));
	AdjustWindowRectEx(&want, WS_POPUPWINDOW | WS_CAPTION, FALSE, WS_EX_DLGMODALFRAME);
	GetWindowRect(owner, &r);
	hThemeWnd = CreateWindowExW(WS_EX_DLGMODALFRAME, L"WyrmholdTheme", T(S_THEME_TITLE),
	                            WS_POPUPWINDOW | WS_CAPTION,
	                            r.left + ((r.right - r.left) - (want.right - want.left)) / 2,
	                            r.top + ((r.bottom - r.top) - (want.bottom - want.top)) / 3,
	                            want.right - want.left, want.bottom - want.top, owner, NULL, gInst, NULL);
	SendMessageW(hThemeWnd, WM_SETICON, ICON_BIG, (LPARAM)icBig);
	SendMessageW(hThemeWnd, WM_SETICON, ICON_SMALL, (LPARAM)icSmall);

	for (i = 0; i < 3; i++)
	{
		HWND b = mkButton(hThemeWnd, IDM_THEME_0 + i, K_THEMESEG, names[i], NULL, i, G_NONE);
		SetWindowPos(b, NULL, SC(TH_M) + SC(4) + i * (SC(140) + SC(4)), themeY(1) + SC(4), SC(140), SC(32), SWP_NOZORDER);
	}
	for (i = 0; i < ACCENT_COUNT; i++)
	{
		HWND b = mkButton(hThemeWnd, IDM_ACCENT_0 + i, K_SWATCH, ACCENTS[i].name, NULL, i, G_NONE);
		SetWindowPos(b, NULL, SC(TH_M + (i % TH_COLS) * (TH_TILE + TH_GAP)), themeY(3) + SC((i / TH_COLS) * (TH_TILEH + TH_GAP)),
		             SC(TH_TILE), SC(TH_TILEH), SWP_NOZORDER);
	}

	GetClientRect(hThemeWnd, &c);
	ok = mkButton(hThemeWnd, IDOK, K_PRIMARY, S_BTN_OK, NULL, 0, G_NONE);
	SetWindowLongPtrW(ok, GWL_STYLE, GetWindowLongPtrW(ok, GWL_STYLE) | BS_DEFPUSHBUTTON);
	SendMessageW(ok, WM_SETFONT, (WPARAM)fBold, FALSE);
	SetWindowPos(ok, NULL, c.right - SC(TH_M) - SC(110), c.bottom - SC(50), SC(110), SC(36), SWP_NOZORDER);

	applyTheme(hThemeWnd);
	ShowWindow(hThemeWnd, SW_SHOW);
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

static HFONT monoFont(int height)
{
	return CreateFontW(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
	                   OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");
}

/* The fonts a control still holds are deleted only after it has been given the new ones. */
static HFONT fOld[5];

static void makeFonts(void)
{
	fOld[0] = fUi; fOld[1] = fBold; fOld[2] = fTitle; fOld[3] = fSub; fOld[4] = fSmall;

	fUi    = uiFont(-SC(14), FW_NORMAL);
	fBold  = uiFont(-SC(14), FW_SEMIBOLD);
	fTitle = uiFont(-SC(23), FW_SEMIBOLD);
	fSub   = uiFont(-SC(13), FW_NORMAL);
	fSmall = uiFont(-SC(12), FW_NORMAL);
	if (!fPw)      fPw = monoFont(-SC(17));
	if (!fPwSmall) fPwSmall = monoFont(-SC(14));
}

static void dropOldFonts(void)
{
	int i;
	for (i = 0; i < 5; i++) { if (fOld[i]) DeleteObject(fOld[i]); fOld[i] = NULL; }
}

static BOOL CALLBACK setFontChild(HWND h, LPARAM lp)
{
	(void)lp;
	SendMessageW(h, WM_SETFONT, (WPARAM)(h == hGenerate ? fBold : fUi), FALSE);
	return TRUE;
}

static void applyLanguage(void)
{
	makeFonts();

	SetWindowTextW(hMain, T(S_WINDOW_TITLE));
	SetWindowTextW(hGenerate, T(S_GENERATE));
	EnumChildWindows(hMain, setFontChild, 0);
	SendMessageW(hTip, WM_SETFONT, (WPARAM)fUi, FALSE);
	allTips(TTM_UPDATETIPTEXTW);
	buildMenu(hMain);
	layout();

	/* the status line is re-said in the new language when there is something to say */
	if (gPwN > 0) sayGenerated();
	else          setStatus(ST_PLAIN, T(S_ST_READY));

	if (hHelpWnd)  { DestroyWindow(hHelpWnd); showHelp(hMain); }
	if (hAboutWnd) { DestroyWindow(hAboutWnd); showAbout(hMain); }
	if (hThemeWnd) { DestroyWindow(hThemeWnd); showTheme(hMain); }

	dropOldFonts();
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

static void setLanguage(int lang)
{
	if (lang == gLang) return;
	gLang = lang;
	iniSetInt(L"Language", lang);
	applyLanguage();
}

/* ------------------------------------------------------------------ reacting to the controls */

/* A setting changed: the strength and the profile name follow at once, and a new set is
 * made -- right away for a click, shortly after the last keystroke or drag step otherwise,
 * so "20" is not generated as "2" first. */
static void onOptionsChanged(int now)
{
	PwOptions o;

	effectiveOptions(&o);
	EnableWindow(hGenerate, pw_check(&o, NULL) == PW_OK);
	InvalidateRect(hMain, &gStrength, FALSE);
	InvalidateRect(hProfile, NULL, FALSE);
	InvalidateRect(hSymSet, NULL, FALSE);
	InvalidateRect(hSeparator, NULL, FALSE);
	if (now) doGenerate();
	else SetTimer(hMain, TMR_REGEN, 300, NULL);
}

static void setMode(int mode)
{
	if (mode == gOpt.mode) return;
	gOpt.mode = mode;
	syncControls();
	layout();
	onOptionsChanged(1);
}

static void setSliderValue(int v)
{
	if (gOpt.mode == PW_WORDS)    gOpt.words = v;
	else if (gOpt.mode == PW_PIN) gOpt.pinLength = v;
	else                          gOpt.length = v;
}

static void readCharsField(HWND e, char* out)
{
	wchar_t w[PW_SET_MAX];
	int i, n = 0;

	GetWindowTextW(e, w, PW_SET_MAX);
	for (i = 0; w[i] && n < PW_SET_MAX - 1; i++)
		if (w[i] > 32 && w[i] < 127 && !memchr(out, (char)w[i], (size_t)n)) out[n++] = (char)w[i];
	out[n] = 0;
}

static void fieldNumber(HWND e, int lo, int hi, int* v)
{
	wchar_t w[16];

	GetWindowTextW(e, w, 16);
	if (w[0]) *v = clampInt(_wtoi(w), lo, hi);
}

static void stepCount(int delta)
{
	wchar_t w[16];

	gCount = clampInt(gCount + delta, 1, MAX_COUNT);
	_snwprintf(w, 16, L"%d", gCount); w[15] = 0;
	gSyncing = 1;
	SetWindowTextW(hCnt, w);
	gSyncing = 0;
	onOptionsChanged(1);
}

static void onCommand(HWND hwnd, int id, int code, HWND ctl)
{
	Ctl* c = ctl ? ctlOf(ctl) : NULL;

	/* owner-drawn buttons: BN_CLICKED, and BN_DOUBLECLICKED for the second of two quick clicks */
	if (c && (code == BN_CLICKED || code == BN_DOUBLECLICKED))
	{
		if (c->kind == K_CHIP || c->kind == K_TOGGLE)
		{
			if (c->bind) *c->bind = !*c->bind;
			InvalidateRect(ctl, NULL, FALSE);
			if (id != IDC_TG_CLEAR) onOptionsChanged(1);
			return;
		}
		if (c->kind == K_SEG) { setMode(c->value); return; }
		if (c->kind == K_DROP)
		{
			UINT cmd = dropMenu(ctl);
			if (cmd) SendMessageW(hwnd, WM_COMMAND, cmd, 0);
			return;
		}
	}

	switch (id)
	{
	case IDC_SLIDER:
		if (code == SLN_CHANGE)
		{
			wchar_t w[16];
			int v = (int)SendMessageW(hSlider, SLM_GETPOS, 0, 0);
			setSliderValue(v);
			_snwprintf(w, 16, L"%d", v); w[15] = 0;
			gSyncing = 1;
			SetWindowTextW(hLen, w);
			gSyncing = 0;
			onOptionsChanged(0);
		}
		return;
	case IDC_LEN:
		if (code == EN_CHANGE && !gSyncing)
		{
			int v = gOpt.mode == PW_WORDS ? gOpt.words : gOpt.mode == PW_PIN ? gOpt.pinLength : gOpt.length;
			if (gOpt.mode == PW_WORDS) fieldNumber(hLen, PW_MIN_WORDS, PW_MAX_WORDS, &v);
			else                       fieldNumber(hLen, PW_MIN_LEN, PW_MAX_LEN, &v);
			setSliderValue(v);
			SendMessageW(hSlider, SLM_SETPOS, (WPARAM)v, 0);
			onOptionsChanged(0);
		}
		if (code == EN_KILLFOCUS) syncControls();      /* what is shown is what is used */
		if (code == EN_SETFOCUS || code == EN_KILLFOCUS) InvalidateRect(hMain, NULL, FALSE);
		return;
	case IDC_CNT:
		if (code == EN_CHANGE && !gSyncing) { fieldNumber(hCnt, 1, MAX_COUNT, &gCount); onOptionsChanged(0); }
		if (code == EN_KILLFOCUS) syncControls();
		if (code == EN_SETFOCUS || code == EN_KILLFOCUS) InvalidateRect(hMain, NULL, FALSE);
		return;
	case IDC_EXCLUDE:
	case IDC_CUSTOM:
		if (code == EN_CHANGE && !gSyncing)
		{
			if (id == IDC_CUSTOM) readCharsField(hCustom, gOpt.customSymbols);
			else readCharsField(hExclude, gOpt.mode == PW_PIN ? gPinExclude : gCharExclude);
			onOptionsChanged(0);
		}
		if (code == EN_SETFOCUS || code == EN_KILLFOCUS) InvalidateRect(hMain, NULL, FALSE);
		return;
	case IDC_CNT_DEC: stepCount(-1); return;
	case IDC_CNT_INC: stepCount(+1); return;
	case IDC_COPYALL: copyAll(); return;
	case IDC_SAVE:    saveAs(hwnd); return;
	case IDC_LIST:
		if (code == LBN_DBLCLK) copyRow((int)SendMessageW(hList, LB_GETCURSEL, 0, 0));
		return;
	case IDOK:               /* Enter outside the list: the default button */
	case IDC_GENERATE:
	case IDM_GENERATE: doGenerate();          return;
	case IDM_COPY:     copySelected();        return;
	case IDM_COPYALL:  copyAll();             return;
	case IDM_SAVE:     saveAs(hwnd);          return;
	case IDM_HELP:     showHelp(hwnd);        return;
	case IDM_ABOUT:    showAbout(hwnd);       return;
	case IDM_LANG_EN:  setLanguage(LANG_EN);  return;
	case IDM_LANG_JP:  setLanguage(LANG_JP);  return;
	case IDM_LANG_ES:  setLanguage(LANG_ES);  return;
	case IDM_EXIT:     DestroyWindow(hwnd);   return;
	}

	if (id >= IDM_MODE_CHARS && id <= IDM_MODE_PIN) { setMode(id - IDM_MODE_CHARS); buildMenu(hwnd); return; }
	if (id >= IDM_PRESET_0 && id < IDM_PRESET_0 + P_COUNT) { applyPreset(id - IDM_PRESET_0); buildMenu(hwnd); return; }
	if (id >= IDM_THEME_0 && id < IDM_THEME_0 + 3) { setThemeChoice(id - IDM_THEME_0); return; }
	if (id >= IDM_ACCENT_0 && id < IDM_ACCENT_0 + ACCENT_COUNT) { setAccent(id - IDM_ACCENT_0); return; }
	if (id == IDM_THEMEDLG) { showTheme(hwnd); return; }
	if (id >= IDM_SYM_0 && id <= IDM_SYM_0 + PW_SYM_CUSTOM)
	{
		gOpt.symbolSet = id - IDM_SYM_0;
		if (gOpt.symbolSet != PW_SYM_CUSTOM || gOpt.symbols == 0) gOpt.symbols = 1;
		layout();
		RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
		if (gOpt.symbolSet == PW_SYM_CUSTOM) SetFocus(hCustom);
		onOptionsChanged(1);
		return;
	}
	if (id >= IDM_SEP_0 && id < IDM_SEP_0 + PW_SEP_COUNT)
	{
		gOpt.separator = id - IDM_SEP_0;
		onOptionsChanged(1);
		return;
	}
}

/* ------------------------------------------------------------------ window */

static void saveSettings(void)
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
	iniSetInt(L"ClearClipboard", gClearClip);

	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	if (!GetWindowPlacement(hMain, &wp)) return;
	iniSetInt(L"WndX", wp.rcNormalPosition.left);
	iniSetInt(L"WndY", wp.rcNormalPosition.top);
	iniSetInt(L"WndW", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
	iniSetInt(L"WndH", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);
	iniSetInt(L"WndMax", wp.showCmd == SW_SHOWMAXIMIZED);
}

static void loadSettings(void)
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
	gTheme              = clampInt(iniInt(L"Theme", 0), 0, 2);
	gAccent             = clampInt(iniInt(L"Accent", 0), 0, ACCENT_COUNT - 1);
	gLang               = clampInt(iniInt(L"Language", LANG_EN), 0, LANG_COUNT - 1);
	iniStr(L"LastFolder", gLastFolder, MAX_PATH);
}

static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hwnd, &ps), mem = CreateCompatibleDC(dc);
		RECT rc;
		HBITMAP bmp;
		HGDIOBJ ob;

		GetClientRect(hwnd, &rc);
		bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
		ob = SelectObject(mem, bmp);
		paintMain(mem);
		BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
		SelectObject(mem, ob);
		DeleteObject(bmp);
		DeleteDC(mem);
		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_ERASEBKGND:
		return 1;

	/* Windows draws a one-pixel light line under the menu bar, which shows on the dark
	 * theme; it is painted over in the menu colour right after the frame. */
	case WM_NCPAINT:
	case WM_NCACTIVATE:
	{
		LRESULT r = DefWindowProcW(hwnd, msg, wp, lp);
		MENUBARINFO mbi;
		RECT wr, line;
		HDC dc;

		mbi.cbSize = sizeof(mbi);
		if (GetMenuBarInfo(hwnd, OBJID_MENU, 0, &mbi))
		{
			GetWindowRect(hwnd, &wr);
			line = mbi.rcBar;
			OffsetRect(&line, -wr.left, -wr.top);
			line.top = line.bottom;
			line.bottom = line.top + 1;
			dc = GetWindowDC(hwnd);
			fillSolid(dc, &line, PAL.menuBg);
			ReleaseDC(hwnd, dc);
		}
		return r;
	}

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
		if (d->CtlType == ODT_MENU)    { drawMenuItem(d); return TRUE; }
		if (d->CtlType == ODT_LISTBOX) { drawRow(d);      return TRUE; }
		if (d->CtlType == ODT_BUTTON)  { drawButton(d);   return TRUE; }
		return 0;
	}

	case WM_CTLCOLOREDIT:
		SetBkColor((HDC)wp, PAL.field);
		SetTextColor((HDC)wp, PAL.text);
		{
			static HBRUSH br = NULL;
			static COLORREF brc = CLR_NONE;
			if (brc != PAL.field) { if (br) DeleteObject(br); br = CreateSolidBrush(PAL.field); brc = PAL.field; }
			return (LRESULT)br;
		}

	case WM_CTLCOLORLISTBOX:
		SetBkColor((HDC)wp, PAL.card);
		{
			static HBRUSH br = NULL;
			static COLORREF brc = CLR_NONE;
			if (brc != PAL.card) { if (br) DeleteObject(br); br = CreateSolidBrush(PAL.card); brc = PAL.card; }
			return (LRESULT)br;
		}

	case WM_SETTINGCHANGE:
		if (lp && 0 == lstrcmpiW((const wchar_t*)lp, L"ImmersiveColorSet") && gTheme == 0) { refreshTheme(); buildMenu(hwnd); }
		return 0;

	case WM_SIZE:
		layout();
		return 0;

	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lp)->ptMinTrackSize.x = SC(MIN_W);
		((MINMAXINFO*)lp)->ptMinTrackSize.y = SC(640);
		return 0;

	case WM_TIMER:
		if (wp == TMR_CLIP) { clipTick(); return 0; }
		if (wp == TMR_MARK)
		{
			KillTimer(hwnd, TMR_MARK);
			gMarkRow = -1;
			InvalidateRect(hList, NULL, FALSE);
			InvalidateRect(hCopyAll, NULL, FALSE);
			return 0;
		}
		if (wp == TMR_REGEN) { doGenerate(); return 0; }
		return 0;

	case WM_COMMAND:
		onCommand(hwnd, LOWORD(wp), HIWORD(wp), (HWND)lp);
		return 0;

	case WM_DESTROY:
		/* a password still waiting to be wiped goes now */
		if (gClipLeft > 0) wipeOurClipboard();
		saveSettings();
		wipePasswords();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wp, lp);
}

/* ------------------------------------------------------------------ command line */

static int writeAuto(const wchar_t* path, const PwOptions* o, int count)
{
	FILE* f = path ? _wfopen(path, L"wb") : NULL;
	char pw[PW_OUT_MAX];
	int i;

	if (!f) return 2;
	if (pw_check(o, NULL) != PW_OK) { fclose(f); return 5; }
	for (i = 0; i < count; i++)
	{
		if (!pw_make(o, pw)) { SecureZeroMemory(pw, sizeof(pw)); fclose(f); return 3; }
		fputs(pw, f);
		fputs("\r\n", f);
	}
	SecureZeroMemory(pw, sizeof(pw));
	return fclose(f) == 0 ? 0 : 2;
}

static int writeEntropy(const wchar_t* path, const PwOptions* o)
{
	FILE* f = path ? _wfopen(path, L"wb") : NULL;

	if (!f) return 2;
	fprintf(f, "%.6f %d %d\r\n", pw_entropy(o), pw_pool(o), pw_check(o, NULL));
	return fclose(f) == 0 ? 0 : 2;
}

/* Draws the window (or the Help / About box in front of it) into a 32-bit BMP. */
static int saveShot(HWND w, const wchar_t* path)
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

/* The text after "/key:" in the wide command line, or NULL: file names keep every character,
 * whatever the ANSI code page (the narrow __argv would turn the rest into question marks). */
static const wchar_t* widePathArg(const wchar_t* key)
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
static int parseOptions(PwOptions* o, int* noRepeat, char* exclude, int* count, int* preset)
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
	GpStartupInput gsi;
	RECT want;
	const char *outPath = NULL, *shotPath = NULL, *openWhat = NULL, *entPath = NULL;
	int i, langArg = -1, ww, wh, wx, wy, rc = 0;

	(void)prev; (void)cmdLine;

	gInst = inst;
	loadLayer();

	for (i = 1; i < __argc; i++)
	{
		const char* a = __argv[i];

		if (a[0] != '/' && a[0] != '-') continue;
		a++;
		if      (!_strnicmp(a, "out:", 4))     outPath = a + 4;
		else if (!_strnicmp(a, "entropy:", 8)) entPath = a + 8;
		else if (!_strnicmp(a, "shot:", 5))    shotPath = a + 5;
		else if (!_strnicmp(a, "open:", 5))    openWhat = a + 5;
		else if (!_stricmp(a, "theme:dark"))   gTheme = 2;
		else if (!_stricmp(a, "theme:light"))  gTheme = 1;
		else if (!_strnicmp(a, "accent:", 7))  gAccent = clampInt(atoi(a + 7), 0, ACCENT_COUNT - 1);
		else if (!_stricmp(a, "lang:en"))      langArg = LANG_EN;
		else if (!_stricmp(a, "lang:jp"))      langArg = LANG_JP;
		else if (!_stricmp(a, "lang:es"))      langArg = LANG_ES;
	}

	/* passwords (or their entropy) to a file, no window at all; without options they
	 * use the standard profile, whatever the .ini says */
	if (outPath || entPath)
	{
		PwOptions o;
		int nr = 0, count = 5, preset = -1;
		char ex[PW_SET_MAX] = "";

		pw_defaults(&o);
		parseOptions(&o, &nr, ex, &count, &preset);
		if (preset >= 0 && preset < P_COUNT) presetOptions(preset, &o, &nr);
		o.noRepeat = nr;
		strcpy(o.exclude, ex);
		if (entPath) return writeEntropy(widePathArg(L"entropy:"), &o);
		return writeAuto(widePathArg(L"out:"), &o, clampInt(count, 1, AUTO_MAX));
	}

	/* only the window remembers settings: the command line above leaves no folder behind */
	iniPath();
	if (shotPath) gNoSave = 1;

	setAppId();
	readDpi();
	{
		int t = gTheme, a = gAccent;
		loadSettings();
		if (t) gTheme = t;              /* /theme and /accent win over the .ini */
		if (a) gAccent = a;
	}
	if (langArg >= 0) gLang = langArg;

	/* a screenshot may ask for a kind of password or a profile */
	if (shotPath)
	{
		PwOptions o = gOpt;
		int nr = 0, count = gCount, preset = -1;
		char ex[PW_SET_MAX] = "";

		pw_defaults(&o);
		parseOptions(&o, &nr, ex, &count, &preset);
		gOpt = o;
		gCharNoRepeat = nr;
		strcpy(gCharExclude, ex);
		gCount = clampInt(count, 1, MAX_COUNT);
		if (preset >= 0 && preset < P_COUNT)
		{
			PwOptions p;
			int pnr;
			presetOptions(preset, &p, &pnr);
			gOpt = p;
			if (p.mode == PW_PIN) gPinNoRepeat = pnr; else gCharNoRepeat = pnr;
		}
	}

	ZeroMemory(&gsi, sizeof(gsi));
	gsi.GdiplusVersion = 1;
	GdiplusStartup(&gGdipToken, &gsi, NULL);

	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_BAR_CLASSES;
	InitCommonControlsEx(&icc);

	makeFonts();
	refreshTheme();
	makeIcons();

	ZeroMemory(&wc, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = wndProc;
	wc.hInstance = inst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = L"Wyrmhold";
	wc.hIcon = icBig;
	RegisterClassW(&wc);

	/* the default size is the client the design needs: five rows in the list */
	SetRect(&want, 0, 0, SC(DESIGN_W), SC(BAND_H + 16 + SEG_H + 14 + CARD_OPT_H + 14 + LIST_HEAD + 5 * ROW_LH + 14 + STATUS_H));
	AdjustWindowRectEx(&want, WS_OVERLAPPEDWINDOW, TRUE, 0);
	ww = iniInt(L"WndW", want.right - want.left);
	wh = iniInt(L"WndH", want.bottom - want.top);
	wx = iniInt(L"WndX", CW_USEDEFAULT);
	wy = iniInt(L"WndY", CW_USEDEFAULT);
	if (ww < SC(MIN_W) || wh < SC(640) || shotPath)
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

	hMain = CreateWindowExW(0, L"Wyrmhold", T(S_WINDOW_TITLE), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	                        wx, wy, ww, wh, NULL, NULL, inst, NULL);

	buildControls();
	buildMenu(hMain);
	makeIcons();
	refreshTheme();                 /* again, now that the window and the menu exist */
	syncControls();
	layout();
	lstrcpynW(gStatusText, T(S_ST_READY), 512);
	doGenerate();                   /* a fresh set on every start */

	accel = LoadAcceleratorsW(inst, MAKEINTRESOURCEW(IDI_APP));

	if (shotPath)
	{
		HWND target = hMain;

		ShowWindow(hMain, SW_SHOWNOACTIVATE);
		UpdateWindow(hMain);
		if (openWhat && !_stricmp(openWhat, "help"))  { showHelp(hMain);  target = hHelpWnd; }
		if (openWhat && !_stricmp(openWhat, "about")) { showAbout(hMain); target = hAboutWnd; }
		if (openWhat && !_stricmp(openWhat, "theme")) { showTheme(hMain); target = hThemeWnd; }
		pump(700);
		rc = saveShot(target, widePathArg(L"shot:"));
		DestroyWindow(hMain);
		GdiplusShutdown(gGdipToken);
		return rc;
	}

	ShowWindow(hMain, (iniInt(L"WndMax", 0) && show != SW_SHOWMINNOACTIVE) ? SW_SHOWMAXIMIZED : show);
	UpdateWindow(hMain);

	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		/* while About or Theme is up the main window is disabled, so its accelerators are not
		 * what the user is aiming at. Help, About and Theme get IsDialogMessage for Tab and Esc. */
		if (!hAboutWnd && !hThemeWnd && (msg.hwnd == hMain || IsChild(hMain, msg.hwnd)) &&
		    TranslateAcceleratorW(hMain, accel, &msg)) continue;
		if (hAboutWnd && IsDialogMessageW(hAboutWnd, &msg)) continue;
		if (hThemeWnd && IsDialogMessageW(hThemeWnd, &msg)) continue;
		if (hHelpWnd && IsDialogMessageW(hHelpWnd, &msg)) continue;
		if (IsDialogMessageW(hMain, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	GdiplusShutdown(gGdipToken);
	return (int)msg.wParam;
}
