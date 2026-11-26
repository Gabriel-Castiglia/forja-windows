/**
  Forja -- the window.
  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 9, 2026
  SPDX-License-Identifier: GPL-2.0-only

  A Win32 front end for the engine in pwgen.h. One portable, statically linked executable:
  installed with its setup, or run portable. The settings it remembers go to
  %APPDATA%\Forja\forja.ini -- or, when a forja.ini sits beside the executable,
  to that one (portable use).

  The name: a forja (Spanish for forge) is where metal takes shape under the hammer, on the anvil,
  as each password here takes shape from pure chance. Until version 2.1 it was called Wyrmhold.

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
      when the chosen time is up (2 minutes by default; 10 s to 10 min in the window, 5 s
      to 60 min in the .ini or on the command line; or never) or on exit,
      whichever comes first -- only if the clipboard still holds exactly what we put there;
    - Help -> Check for updates (and once a day by itself, if the first start's question was
      answered yes): one question, then the setup is
      downloaded (only when the release's SHA256SUMS.txt carries the author's signature,
      release.sig), checked against that SHA256SUMS.txt, run in silence, and the
      program comes back (see "updates" below and update.c);
    - light, dark or follow-Windows theme and twelve accent colours (View -> Theme and
      colour), which also tint the icon;
    - English, Japanese and Spanish; Help, About and Theme windows;
    - read by Narrator and other screen readers (access.c): names, roles and states of what
      is drawn here, the passwords of the list, the menus, the status line read out;
    - per-monitor scaling (PerMonitorV2): on a screen with another scale everything measured
      is made again at it (theme.c applyDpi);
    - a layout that fits small screens: rows that do not fit the width go on to the next row
      (down to 560 logical pixels), the band slims down or goes in a low window, then rows and
      gaps tighten, and below that the part above the list scrolls while at least three
      password rows stay in view. A new window is sized to its screen's work area.

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

        forja.exe [options] [/out:<file> | /entropy:<file> | /shot:<file.bmp>]
          options: /mode:chars|words|pin  /len:N  /words:N  /count:N  /classes:[l][u][d][s]
                   /symbols:basic|all|min|custom:<chars>  /exclude:<chars>  /ambiguous
                   /norepeat  /startletter  /sep:hyphen|space|dot|underscore|none|digit
                   /nocap  /nonumber  /allowruns  /preset:N
          window:  /theme:light|dark  /accent:N  /lang:en|jp|es  /open:help|about|theme
                   /clearafter:N[s|m]|off
        forja.exe /selftest:<log>

      /out writes the passwords (count up to 100000) and exits without a window; without
      options they use the standard profile. Its exit code: 0 done, 2 the file could not be
      written, 3 the random number generator failed, 5 the options cannot make a password
      (pw_check), 6 no draw followed the rules within the tries allowed (and the file is
      left empty in every failure). /entropy writes "bits pool check" and exits.
      /shot draws the window into a bitmap and exits; with it, /shotdpi:N draws at another
      scale and /shotarea:WxH stands in for the screen's work area (in pixels), to see the
      window as on a small screen. None of them writes the .ini.
      /clearafter sets how long a copy stays on the clipboard for this session only (seconds,
      or minutes with "m"; 0 or "off" keeps copies); the .ini keeps the window's own choice.
      /selftest checks the clipboard wiping, its settings and the update's pure functions
      with a throw-away .ini, writes the report to <log> and exits with the number of
      failures; the user's clipboard is saved first and put back afterwards.
**/

#define UI_STRINGS_TABLE           /* the string table is compiled here, once */
#include "forja.h"

/* ------------------------------------------------------------------ state */

static void onOptionsChanged(int now);

const int CLEAR_CHOICES[CLEAR_CHOICE_N] = { 10, 30, 60, 120, 300, 600 };

ULONG_PTR gGdipToken;
Palette PAL;
int gDark = 0;
int gTheme = 0;                  /* 0 follow Windows, 1 light, 2 dark */
int gAccent = 0;
int gDpi = 96;
HINSTANCE gInst;
HWND hMain, hTip, hList, hStatus;
HWND hSeg[3], hProfile, hSlider, hLen, hCnt, hCntDec, hCntInc;
HWND hChip[4], hSymSet, hTgAmbig, hTgNoRepeat, hTgStart, hExclude, hCustom;
HWND hSeparator, hTgCap, hTgNumber, hTgPinRepeat, hTgNoSeq;
HWND hGenerate, hTgClear, hClearTime, hSave, hCopyAll;
HWND hHelpWnd, hAboutWnd, hThemeWnd;
HICON gSwatchIcon[16];
HFONT fUi, fBold, fTitle, fSub, fSmall, fPw, fPwSmall;
HICON icBand, icAbout, icBig, icSmall;
HMENU hMenu;
int gLang = LANG_EN;
PwOptions gOpt;                  /* what the controls show */
int gCount = 5, gClearClip = 1, gClearSecs = CLIP_DEFAULT;
int gClearFromArg = 0;           /* /clearafter set them: not saved unless changed in the window */
int gAutoUpdate = 1;             /* check for updates once a day */
int gCharNoRepeat = 0, gPinNoRepeat = 1;
char gCharExclude[PW_SET_MAX], gPinExclude[PW_SET_MAX];
int gNoSave = 0;
int gSyncing = 0;                /* controls are being set from the options: no feedback */
char gPw[MAX_COUNT][PW_OUT_MAX];
int gPwN = 0, gPwMode = PW_CHARS;
int gHotRow = -1, gHotIcon = 0;  /* list row under the pointer, and is it on the icon */
int gMarkRow = -1;               /* row showing the check mark; -2 = all of them */
int gTrackingList = 0;
DWORD gClipSeq = 0;              /* clipboard sequence number right after our copy */
BYTE gClipHash[32];              /* SHA-256 of the text we copied (never the text itself) */
int gClipHave = 0;               /* gClipHash is set */
int gClipLeft = 0;               /* seconds until we wipe it; 0 = nothing pending */
wchar_t gClipMsg[256];
int gStatusKind = ST_PLAIN;
wchar_t gStatusText[512];
wchar_t gIni[MAX_PATH];
wchar_t gLastFolder[MAX_PATH];
Label gLabels[16];
int gLabelN = 0;
RECT gSegTrack, gCardOpt, gCardList, gStrength, gStatusR;
int gBandTop = 0, gBandH = 0;    /* the band as laid out: scrolled, slim or gone (0) */
int gScrollY = 0, gScrollMax = 0; /* in low windows the part above the list scrolls */
int gClipBottom = 0x7FFFFFFF;    /* ...and is cut off here, above the list card */
int gUpperBottom = 0, gListHeadH = 0;   /* bottom of the options card, height of the list's head */
int gMeasure = 0;                /* layout only measures: nothing is moved or recorded */
int gCompact = 0;                /* low window: tighter rows and gaps */
HWND gFieldEdit[4];
RECT gFieldBox[4];
int gFieldN = 0;

/* ------------------------------------------------------------------ the options in effect */

/* noRepeat and the exclusions are kept apart for characters and PINs: excluding "1" from a
 * PIN says nothing about a character password. */
void effectiveOptions(PwOptions* o)
{
	*o = gOpt;
	o->noRepeat = (o->mode == PW_PIN) ? gPinNoRepeat : gCharNoRepeat;
	strcpy(o->exclude, (o->mode == PW_PIN) ? gPinExclude : gCharExclude);
}

/* ------------------------------------------------------------------ profiles */


const int PRESET_STR[P_COUNT] = {
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
int currentPreset(void)
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

/* ------------------------------------------------------------------ status */

static void invalidateStatus(void)
{
	if (hStatus) InvalidateRect(hStatus, NULL, FALSE);
}

void setStatus(int kind, const wchar_t* text)
{
	gStatusKind = kind;
	lstrcpynW(gStatusText, text, 512);
	invalidateStatus();
	accStatusSays(hStatus, gStatusText);   /* read out by Narrator, the focus stays */
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

void wipePasswords(void)
{
	SecureZeroMemory(gPw, sizeof(gPw));
	gPwN = 0;
}

void sayGenerated(void)
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
		int why;
		if (!pw_make_ex(&o, gPw[i], &why))
		{
			wipePasswords();
			SendMessageW(hList, LB_RESETCONTENT, 0, 0);
			SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
			/* the random number generator is blamed only when it really failed */
			setStatus(ST_ERR, T(why == PW_FAIL_RNG ? S_ST_RNG_FAIL : S_ST_TRIES_FAIL));
			return;
		}
		/* the list keeps an empty string and the row number, never the password: a screen
		 * reader that asks for the text of a row gets it from listProc (layout.c) right then */
		{
			LRESULT at = SendMessageW(hList, LB_ADDSTRING, 0, (LPARAM)L"");
			if (at >= 0) SendMessageW(hList, LB_SETITEMDATA, (WPARAM)at, (LPARAM)i);
		}
	}

	gPwN = gCount;
	gPwMode = o.mode;
	gMarkRow = -1;
	SendMessageW(hList, LB_SETCURSEL, 0, 0);
	SendMessageW(hList, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(hList, NULL, TRUE);
	sayGenerated();
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

void setMode(int mode)
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
			else { gClearFromArg = 0; InvalidateRect(hClearTime, NULL, FALSE); clearSwitched(); }
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
	case IDM_UPDATE:   checkUpdates(1);       return;
	case IDM_UPDATE_AUTO:
		gAutoUpdate = !gAutoUpdate;
		iniSetInt(L"CheckUpdates", gAutoUpdate);
		buildMenu(hwnd);
		return;
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
	if (id >= IDM_CLR_0 && id < IDM_CLR_0 + CLEAR_CHOICE_N)
	{
		/* a time picked: clearing is on, and the choice is the window's own (saved) */
		gClearSecs = CLEAR_CHOICES[id - IDM_CLR_0];
		gClearClip = 1;
		gClearFromArg = 0;
		InvalidateRect(hTgClear, NULL, FALSE);
		InvalidateRect(hClearTime, NULL, FALSE);
		return;
	}
}

/* ------------------------------------------------------------------ window */

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

	case WM_DPICHANGED:   /* moved to a screen with another scale (theme.c applyDpi) */
		applyDpi(HIWORD(wp), (const RECT*)lp);
		return 0;

	case WM_GETMINMAXINFO:
	{
		/* the smallest client the layout handles (it wraps and scrolls), as a window size */
		RECT r;
		SetRect(&r, 0, 0, SC(MIN_W), SC(MIN_H));
		AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, TRUE, 0);
		((MINMAXINFO*)lp)->ptMinTrackSize.x = r.right - r.left;
		((MINMAXINFO*)lp)->ptMinTrackSize.y = r.bottom - r.top;
		return 0;
	}

	case WM_VSCROLL:
	{
		SCROLLINFO si;
		int page = maxInt(SC(40), gClipBottom - SC(40));
		si.cbSize = sizeof(si);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		switch (LOWORD(wp))
		{
		case SB_LINEUP:        scrollTo(gScrollY - SC(40)); break;
		case SB_LINEDOWN:      scrollTo(gScrollY + SC(40)); break;
		case SB_PAGEUP:        scrollTo(gScrollY - page); break;
		case SB_PAGEDOWN:      scrollTo(gScrollY + page); break;
		case SB_TOP:           scrollTo(0); break;
		case SB_BOTTOM:        scrollTo(gScrollMax); break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION: scrollTo(si.nTrackPos); break;
		}
		return 0;
	}

	case WM_MOUSEWHEEL:
		/* the wheel over anything but the list (which scrolls itself) moves the part above it */
		if (gScrollMax > 0) { scrollTo(gScrollY - GET_WHEEL_DELTA_WPARAM(wp) * SC(48) / WHEEL_DELTA); return 0; }
		break;

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

	case WM_APP_UPDATE:
		onUpdateDone((UpdJob*)lp);
		return 0;

	case WM_APP_PROGRESS:
		onUpdateProgress((int)wp, (ULONGLONG)lp);
		return 0;

	case WM_ENDSESSION:
		/* Windows is signing out or shutting down: WM_DESTROY may never come */
		if (wp) wipeOnExit();
		return 0;

	case WM_DESTROY:
		/* a password still waiting to be wiped goes now, if it is still ours */
		wipeOnExit();
		saveSettings();
		wipePasswords();
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProcW(hwnd, msg, wp, lp);
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
	const char *testPath = NULL, *clearArg = NULL;
	int i, langArg = -1, ww, wh, wx, wy, rc = 0, updated = 0, shotDpi = 0, areaW = 0, areaH = 0, savedPos, designSize = 0;

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
		else if (!_strnicmp(a, "clearafter:", 11)) clearArg = a + 11;
		else if (!_strnicmp(a, "selftest:", 9))    testPath = a + 9;
		else if (!_stricmp(a, "updated"))      updated = 1;     /* started by the setup after an update */
		else if (!_strnicmp(a, "shotdpi:", 8))  shotDpi = atoi(a + 8);
		else if (!_strnicmp(a, "shotarea:", 9)) sscanf(a + 9, "%dx%d", &areaW, &areaH);
	}

	/* the self-test: no window, a throw-away .ini, the clipboard saved and put back */
	if (testPath) return selfTest(widePathArg(L"selftest:"));

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
	if (shotPath && shotDpi >= 96 && shotDpi <= 480) gDpi = shotDpi;   /* screenshots at another scale */
	{
		int t = gTheme, a = gAccent;
		loadSettings();
		if (t) gTheme = t;              /* /theme and /accent win over the .ini */
		if (a) gAccent = a;
	}
	if (langArg >= 0) gLang = langArg;
	if (clearArg)
	{
		/* for this session only: saveSettings leaves the .ini's choice alone */
		int v = parseDuration(clearArg);
		if (v >= 0)
		{
			gClearClip = v > 0;
			if (v > 0) gClearSecs = clampInt(v, CLIP_MIN, CLIP_MAX);
			gClearFromArg = 1;
		}
	}

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
	wc.lpszClassName = L"Forja";
	wc.hIcon = icBig;
	RegisterClassW(&wc);

	/* the default size is the client the design needs: the whole band and five rows in the
	 * list. It is measured before the controls exist: measuring only needs the fonts. */
	gMeasure = 1;
	SetRect(&want, 0, 0, SC(DESIGN_W), layoutTop(SC(DESIGN_W), 0, SC(BAND_FULL), -1) + SC(5 * ROW_LH + 8 + STATUS_H));
	gMeasure = 0;
	AdjustWindowRectEx(&want, WS_OVERLAPPEDWINDOW, TRUE, 0);
	{
		RECT minR;
		SetRect(&minR, 0, 0, SC(MIN_W), SC(MIN_H));
		AdjustWindowRectEx(&minR, WS_OVERLAPPEDWINDOW, TRUE, 0);
		ww = iniInt(L"WndW", want.right - want.left);
		wh = iniInt(L"WndH", want.bottom - want.top);
		wx = iniInt(L"WndX", CW_USEDEFAULT);
		wy = iniInt(L"WndY", CW_USEDEFAULT);
		if (ww < minR.right - minR.left || wh < minR.bottom - minR.top || shotPath)
		{
			ww = want.right - want.left;
			wh = want.bottom - want.top;
		}
		designSize = (ww == want.right - want.left && wh == want.bottom - want.top);
	}
	if (!shotPath && wx != CW_USEDEFAULT)
	{
		RECT saved;
		SetRect(&saved, wx, wy, wx + ww, wy + wh);
		if (!MonitorFromRect(&saved, MONITOR_DEFAULTTONULL)) wx = wy = CW_USEDEFAULT;
	}
	savedPos = !shotPath && wx != CW_USEDEFAULT;
	{
		/* never larger than the work area of its screen (small screens, high scaling); a new
		 * window is centred in it. /shotarea stands in for a screen when taking screenshots. */
		RECT wa;
		MONITORINFO mi;
		POINT at;
		mi.cbSize = sizeof(mi);
		at.x = wx == CW_USEDEFAULT ? 0 : wx;
		at.y = wx == CW_USEDEFAULT ? 0 : wy;
		if (shotPath && areaW > 0 && areaH > 0) SetRect(&wa, 0, 0, areaW, areaH);
		else if (GetMonitorInfoW(MonitorFromPoint(at, MONITOR_DEFAULTTOPRIMARY), &mi)) wa = mi.rcWork;
		else SystemParametersInfoW(SPI_GETWORKAREA, 0, &wa, 0);
		ww = minInt(ww, wa.right - wa.left);
		wh = minInt(wh, wa.bottom - wa.top);
		if (wx == CW_USEDEFAULT || shotPath)
		{
			wx = wa.left + (wa.right - wa.left - ww) / 2;
			wy = wa.top + (wa.bottom - wa.top - wh) / 2;
		}
		if (shotPath) { wx += 40; wy += 40; }   /* a screenshot is drawn off the work area's corner */
	}

	hMain = CreateWindowExW(0, L"Forja", T(S_WINDOW_TITLE), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	                        wx, wy, ww, wh, NULL, NULL, inst, NULL);

	/* WndX/WndY come from GetWindowPlacement (saveSettings), in workspace coordinates, which
	 * CreateWindowEx reads as screen coordinates: with the taskbar at the top or the left the
	 * window crept by the taskbar's size on every start. SetWindowPlacement takes them as they
	 * were saved -- the .ini keeps the same values and meaning as before. Still hidden here. */
	if (savedPos && hMain)
	{
		WINDOWPLACEMENT wpl;
		ZeroMemory(&wpl, sizeof(wpl));
		wpl.length = sizeof(wpl);
		if (GetWindowPlacement(hMain, &wpl))
		{
			wpl.flags = 0;
			wpl.showCmd = SW_HIDE;
			SetRect(&wpl.rcNormalPosition, wx, wy, wx + ww, wy + wh);
			SetWindowPlacement(hMain, &wpl);
		}
	}

	buildControls();
	buildMenu(hMain);
	makeIcons();
	refreshTheme();                 /* again, now that the window and the menu exist */
	syncControls();
	layout();

	/* Everything above was measured at the main screen's scale. A window that opens on a screen
	 * with another one takes that scale before it is shown; the default size grows or shrinks with
	 * it (a size saved in the .ini is already in that screen's pixels), within the work area.
	 * Screenshots keep the scale they were asked for. */
	if (!shotPath && hMain)
	{
		int d = windowDpi(hMain);
		RECT r;

		if (d >= 96 && d != gDpi && GetWindowRect(hMain, &r))
		{
			if (designSize)
			{
				MONITORINFO mi;
				int w = MulDiv(r.right - r.left, d, gDpi), h = MulDiv(r.bottom - r.top, d, gDpi);
				mi.cbSize = sizeof(mi);
				if (GetMonitorInfoW(MonitorFromWindow(hMain, MONITOR_DEFAULTTONEAREST), &mi))
				{
					w = minInt(w, mi.rcWork.right - mi.rcWork.left);
					h = minInt(h, mi.rcWork.bottom - mi.rcWork.top);
					if (!savedPos)   /* a new window stays centred */
					{
						r.left = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - w) / 2;
						r.top = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - h) / 2;
					}
				}
				r.right = r.left + w;
				r.bottom = r.top + h;
				applyDpi(d, &r);
			}
			else applyDpi(d, NULL);
		}
	}
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
	if (updated) setStatusF(ST_OK, S_UPD_UPDATED, APP_VERSION);
	removeUpdateLeftovers();        /* the setup of the last update, or a download cut short */
	askAutoUpdateOnce(updated);     /* the first start: may Forja look for new versions? */
	checkUpdates(0);                /* once a day, unless switched off */

	while (GetMessageW(&msg, NULL, 0, 0) > 0)
	{
		/* while About or Theme is up the main window is disabled, so its accelerators are not
		 * what the user is aiming at. Help, About and Theme get IsDialogMessage for Tab and Esc. */
		if (!hAboutWnd && !hThemeWnd && (msg.hwnd == hMain || IsChild(hMain, msg.hwnd)) &&
		    TranslateAcceleratorW(hMain, accel, &msg)) continue;
		if (hAboutWnd && IsDialogMessageW(hAboutWnd, &msg)) continue;
		if (hThemeWnd && IsDialogMessageW(hThemeWnd, &msg)) continue;
		if (hHelpWnd && IsDialogMessageW(hHelpWnd, &msg)) continue;
		if (IsDialogMessageW(hMain, &msg)) { keepFocusInView(); continue; }
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	GdiplusShutdown(gGdipToken);
	return (int)msg.wParam;
}
