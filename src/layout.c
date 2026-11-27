/**
  layout.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The main window's content: building the controls, laying them out for any width and
  height, the strength line, tooltips, the password list and the painting.
**/

#include "forja.h"

static LRESULT CALLBACK statusProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

/* ------------------------------------------------------------------ strength */

/* Guessing time for an attacker who has the file and tries 100 billion passwords a second
 * (a rack of GPUs against a fast hash); on average half the space is searched. */
int guessTimeString(double bits)
{
	double secs = pow(2.0, bits - 1) / 1e11, year = 3.156e7;

	if (secs < 1)            return S_TIME_INSTANT;
	if (secs < 60)           return S_TIME_SECONDS;
	if (secs < 3600)         return S_TIME_MINUTES;
	if (secs < 86400)        return S_TIME_HOURS;
	if (secs < year)         return S_TIME_DAYS;
	if (secs < 100 * year)   return S_TIME_YEARS;
	if (secs < 1e3 * year)   return S_TIME_CENTURIES;
	if (secs < 1e6 * year)   return S_TIME_THOUSANDS;
	if (secs < 1e9 * year)   return S_TIME_MILLIONS;
	if (secs < 1e12 * year)  return S_TIME_BILLIONS;   /* the universe is 1.4e10 years old */
	if (secs < 1e15 * year)  return S_TIME_TRILLIONS;
	return S_TIME_FOREVER;
}

/* 0 very weak (below 36 bits), 1 weak (below 50), 2 fair (below 64), 3 strong (below 80),
 * 4 very strong. */
static int strengthLevel(double bits)
{
	return bits < 36 ? 0 : bits < 50 ? 1 : bits < 64 ? 2 : bits < 80 ? 3 : 4;
}

static COLORREF levelColour(int level)
{
	return level <= 1 ? PAL.err : level == 2 ? PAL.warn : PAL.ok;
}

/* The strength line: five segments, the level and the bits, and below them the pool and the
 * guessing time -- or, when the options cannot make a password, why not. */
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
		         check == PW_ERR_NODIGITS ? S_ERR_NODIGITS : check == PW_ERR_TOOTIGHT ? S_ERR_TOOTIGHT : S_ERR_TOOSHORT;
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

/* ------------------------------------------------------------------ tooltips */

/* Adds (TTM_ADDTOOLW) or updates (TTM_UPDATETIPTEXTW) the tooltip of a control. */
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

void allTips(UINT msg)
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
	setTip(hClearTime, S_TIP_CLEARTIME, msg);
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

/* The password row under a mouse position of the list, or -1; onIcon says whether the
 * pointer is on the copy icon at the right of the row. */
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
void drawRow(const DRAWITEMSTRUCT* d)
{
	HDC mem;
	BackBuf bb;
	RECT r = d->rcItem, t, hl;
	int row = (int)d->itemData, sel = (d->itemState & ODS_SELECTED) != 0, hot;
	int w = r.right - r.left, h = r.bottom - r.top, iconSz = SC(16), iconX, x, cw, maxX, i, len;
	HGDIOBJ of;
	TEXTMETRICW tm;
	SIZE m;
	HFONT f = fPw;
	wchar_t num[8];
	COLORREF bg;

	if ((int)d->itemID < 0 || row < 0 || row >= gPwN) { fillSolid(d->hDC, &r, PAL.card); return; }

	mem = bufBegin(&bb, d->hDC, r.left, r.top, w, h);
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

	bufEnd(&bb);
}

/* The subclass of the list: the row and icon under the pointer, a click on the copy icon,
 * Enter to copy, and the text of a row for screen readers. */
static LRESULT CALLBACK listProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref)
{
	int row, onIcon;

	(void)id; (void)ref;

	switch (msg)
	{
	/* the text of a row, asked for by screen readers: the list holds empty strings, so the
	 * password is converted here, when asked, and the copy on the stack is wiped */
	case LB_GETTEXTLEN:
	case LB_GETTEXT:
	{
		wchar_t buf[PW_OUT_MAX];
		int n;

		row = (int)SendMessageW(hwnd, LB_GETITEMDATA, wp, 0);
		n = passwordText(row, buf, PW_OUT_MAX);
		if (n < 0) break;
		if (msg == LB_GETTEXT && lp) memcpy((wchar_t*)lp, buf, (size_t)(n + 1) * sizeof(wchar_t));
		SecureZeroMemory(buf, sizeof(buf));
		return n;
	}
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

/* A text field of the main window, holding at most `limit` characters. */
static HWND mkEdit(int id, int limit, DWORD style)
{
	HWND e = CreateWindowExW(0, L"EDIT", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | style,
	                         0, 0, 10, 10, hMain, (HMENU)(INT_PTR)id, gInst, NULL);
	SendMessageW(e, WM_SETFONT, (WPARAM)fUi, FALSE);
	SendMessageW(e, EM_SETLIMITTEXT, (WPARAM)limit, 0);
	return e;
}

/* Every control of the main window, made once; layout() places them. */
void buildControls(void)
{
	int i;

	/* the slider's track is drawn across the whole width: redrawn whole on a resize */
	registerClass(L"ForjaSlider", sliderProc, CS_HREDRAW | CS_VREDRAW);
	registerClass(L"ForjaStatus", statusProc, 0);

	for (i = 0; i < 3; i++)
		hSeg[i] = mkButton(hMain, IDC_SEG_CHARS + i, K_SEG, S_MODE_CHARS + i, NULL, PW_CHARS + i, G_NONE);
	hProfile = mkButton(hMain, IDC_PROFILE, K_DROP, -1, NULL, 0, G_NONE);

	hSlider = CreateWindowExW(0, L"ForjaSlider", L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,
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

	hGenerate = mkDefaultButton(hMain, IDC_GENERATE, S_GENERATE);

	hTgClear = mkButton(hMain, IDC_TG_CLEAR, K_TOGGLE, S_OPT_CLEAR, &gClearClip, 0, G_NONE);
	hClearTime = mkButton(hMain, IDC_CLEARTIME, K_DROP, -1, NULL, 0, G_NONE);
	hSave    = mkButton(hMain, IDC_SAVE, K_ICON, -1, NULL, 0, G_SAVE);
	hCopyAll = mkButton(hMain, IDC_COPYALL, K_ICON, -1, NULL, 0, G_COPY);

	hList = CreateWindowExW(0, L"LISTBOX", NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL |
	                        LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
	                        0, 0, 10, 10, hMain, (HMENU)(INT_PTR)IDC_LIST, gInst, NULL);
	SendMessageW(hList, WM_SETFONT, (WPARAM)fUi, FALSE);
	SetWindowSubclass(hList, listProc, 1, 0);
	accSetName(hList, T(S_ACC_LIST));

	hStatus = CreateWindowExW(0, L"ForjaStatus", L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
	                          0, 0, 10, 10, hMain, (HMENU)(INT_PTR)IDC_STATUS, gInst, NULL);
	accStatusInit(hStatus);

	hTip = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
	                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hMain, NULL, gInst, NULL);
	SendMessageW(hTip, TTM_SETMAXTIPWIDTH, 0, SC(420));
	SendMessageW(hTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 30000);
	SendMessageW(hTip, WM_SETFONT, (WPARAM)fUi, TRUE);
	allTips(TTM_ADDTOOLW);
}

/* A number into a text field, without the field reporting it back as a change. */
void setNumberText(HWND h, int v)
{
	wchar_t w[16];
	int was = gSyncing;

	_snwprintf(w, 16, L"%d", v);
	w[15] = 0;
	gSyncing = 1;
	SetWindowTextW(h, w);
	gSyncing = was;
}

/* Puts the options into the controls (numbers, character sets, the slider range). The slider
 * reaches 64 characters or 20 PIN digits; the field next to it takes up to 128. */
void syncControls(void)
{
	wchar_t w[PW_SET_MAX];
	int lo, hi, v, i;

	gSyncing = 1;
	if (gOpt.mode == PW_WORDS)    { lo = PW_MIN_WORDS; hi = PW_MAX_WORDS; v = gOpt.words; }
	else if (gOpt.mode == PW_PIN) { lo = PW_MIN_LEN; hi = 20; v = gOpt.pinLength; }
	else                          { lo = PW_MIN_LEN; hi = 64; v = gOpt.length; }
	SendMessageW(hSlider, SLM_SETRANGE, (WPARAM)lo, (LPARAM)hi);
	SendMessageW(hSlider, SLM_SETPOS, (WPARAM)v, 0);
	setNumberText(hLen, v);
	setNumberText(hCnt, gCount);
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

/* Moves a control (or hides it, show = 0). While gMeasure is set, nothing is touched. */
static void place(HWND h, int x, int y, int w, int hh, int show)
{
	if (gMeasure) return;
	SetWindowPos(h, NULL, x, y, w, hh, SWP_NOZORDER | SWP_NOACTIVATE | (show ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	/* scrolled under the list card, or partly: only the part above shows. A control wholly
	 * under it keeps an empty region rather than being hidden, so Tab still reaches it (and
	 * keepFocusInView scrolls it back into view). */
	SetWindowRgn(h, show && y + hh > gClipBottom ? CreateRectRgn(0, 0, w, maxInt(0, gClipBottom - y)) : NULL, FALSE);
}

/* A text the main window paints itself (paintMain), recorded for the next paint. */
static void addLabel(int str, int x, int y, int w, int h, int style)
{
	if (gMeasure || gLabelN >= 16) return;
	gLabels[gLabelN].str = str;
	SetRect(&gLabels[gLabelN].r, x, y, x + w, y + h);
	gLabels[gLabelN].style = style;
	gLabelN++;
}

/* An EDIT inside a painted, rounded field frame of FIELD_H. */
static void placeField(HWND e, int x, int y, int w, int show)
{
	TEXTMETRICW tm;
	HDC dc;
	HGDIOBJ of;
	int eh;

	if (gMeasure) return;
	dc = GetDC(e);
	of = SelectObject(dc, fUi);
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

/* The width a switch needs: the switch itself, the gap and its text. */
static int toggleWidth(int str) { return SC(34 + 10 + 10) + textWidth(fUi, T(str), headLen(T(str))); }

/* Switches side by side, going on to the next row when the next one does not fit. Returns
 * the top of the row after them. */
static int flowToggles(HWND* hs, const int* strs, int n, int cx, int cr, int row)
{
	int x = cx, i;

	for (i = 0; i < n; i++)
	{
		int w = minInt(toggleWidth(strs[i]), cr - cx);
		if (x > cx && x + w > cr) { row += STEP_PX; x = cx; }
		place(hs[i], x, row, w, SC(30), 1);
		x += w + SC(24);
	}
	return row + STEP_PX;
}

/* The rows of the options card for one kind of password, from row down. Whatever does not
 * fit the width goes on to a row of its own, so a narrow window wraps instead of cutting.
 * The strength line and Generate come last, at bottomY when it is not negative (the foot of
 * a card sized for the tallest kind). Returns the bottom of that last row. */
static int optionRows(int mode, int cx, int cr, int row, int bottomY)
{
	static const int rowLabels[] = { S_LBL_LENGTH, S_LBL_WORDS, S_LBL_DIGITS, S_LBL_INCLUDE, S_LBL_SEPARATOR, S_LBL_EXCLUDE };
	int chars = mode == PW_CHARS, words = mode == PW_WORDS, pin = mode == PW_PIN;
	int inner = cr - cx, labelW = 0, i, x, step = STEP_PX, hb = SC(gCompact ? 40 : 44);

	for (i = 0; i < 6; i++) labelW = maxInt(labelW, textWidth(fUi, T(rowLabels[i]), -1));
	labelW += SC(18);

	/* the slider and its number, and how many: on one row, or the count on the next */
	{
		int cntLbl = textWidth(fUi, T(S_LBL_COUNT), -1) + SC(12), stepW = SC(116);
		int sliderL = cx + labelW - SC(10), sliderR, cntX, cntLblX, cntRow;
		if (labelW + SC(100 + 70 + 24) + cntLbl + stepW <= inner)
		{
			cntX = cr - stepW;
			cntLblX = cntX - cntLbl;
			sliderR = cntLblX - SC(24) - SC(70);
			cntRow = row;
		}
		else
		{
			sliderR = cr - SC(70);
			cntLblX = cx;
			cntX = cx + maxInt(labelW, cntLbl);
			cntRow = row + step;
		}
		addLabel(chars ? S_LBL_LENGTH : words ? S_LBL_WORDS : S_LBL_DIGITS, cx, row, labelW, SC(FIELD_H), L_PLAIN);
		place(hSlider, sliderL, row, sliderR - sliderL, SC(FIELD_H), 1);
		placeField(hLen, sliderR + SC(10), row, SC(60), 1);
		addLabel(S_LBL_COUNT, cntLblX, cntRow, cntLbl, SC(FIELD_H), L_PLAIN);
		place(hCntDec, cntX + SC(3), cntRow + SC(3), SC(28), SC(FIELD_H - 6), 1);
		placeField(hCnt, cntX + SC(32), cntRow, stepW - SC(64), 1);
		place(hCntInc, cntX + stepW - SC(31), cntRow + SC(3), SC(28), SC(FIELD_H - 6), 1);
		if (!gMeasure && gFieldN > 0)
		{
			/* the stepper's frame spans both buttons */
			gFieldBox[gFieldN - 1].left = cntX;
			gFieldBox[gFieldN - 1].right = cntX + stepW;
		}
		row = cntRow + step;
	}

	if (chars)
	{
		/* the chips, then the symbol set on the same row when it fits */
		int symW = SC(220), symLbl = textWidth(fUi, T(S_LBL_SYMBOLS), -1) + SC(12);
		addLabel(S_LBL_INCLUDE, cx, row, labelW, SC(FIELD_H), L_PLAIN);
		x = cx + labelW;
		for (i = 0; i < 4; i++)
		{
			int cw = textWidth(fBold, T(S_CHIP_LOWER + i), -1) + SC(42);
			if (x > cx + labelW && x + cw > cr) { row += step; x = cx + labelW; }
			place(hChip[i], x, row + SC(1), cw, SC(30), 1);
			x += cw + SC(8);
		}
		if (x + SC(16) + symLbl + SC(150) <= cr)
		{
			symW = minInt(symW, cr - x - SC(16) - symLbl);
			addLabel(S_LBL_SYMBOLS, cr - symW - symLbl, row, symLbl, SC(FIELD_H), L_PLAIN);
			place(hSymSet, cr - symW, row, symW, SC(FIELD_H), 1);
		}
		else
		{
			int lw = maxInt(labelW, symLbl);
			row += step;
			symW = minInt(symW, cr - cx - lw);
			addLabel(S_LBL_SYMBOLS, cx, row, lw, SC(FIELD_H), L_PLAIN);
			place(hSymSet, cx + lw, row, symW, SC(FIELD_H), 1);
		}
		row += step;
	}
	else
	{
		for (i = 0; i < 4; i++) place(hChip[i], 0, 0, 0, 0, 0);
		place(hSymSet, 0, 0, 0, 0, 0);
	}

	if (words)
	{
		addLabel(S_LBL_SEPARATOR, cx, row, labelW, SC(FIELD_H), L_PLAIN);
		place(hSeparator, cx + labelW, row, minInt(SC(230), cr - cx - labelW), SC(FIELD_H), 1);
		row += step;
	}
	else place(hSeparator, 0, 0, 0, 0, 0);

	/* switches */
	if (chars)
	{
		HWND hs[3] = { hTgAmbig, hTgNoRepeat, hTgStart };
		static const int ss[3] = { S_OPT_AMBIG, S_OPT_NOREPEAT, S_OPT_START };
		row = flowToggles(hs, ss, 3, cx, cr, row);
	}
	else { place(hTgAmbig, 0, 0, 0, 0, 0); place(hTgNoRepeat, 0, 0, 0, 0, 0); place(hTgStart, 0, 0, 0, 0, 0); }

	if (words)
	{
		HWND hs[2] = { hTgCap, hTgNumber };
		static const int ss[2] = { S_OPT_CAP, S_OPT_NUMBER };
		row = flowToggles(hs, ss, 2, cx, cr, row);
		addLabel(S_WORDS_NOTE, cx, row, inner, SC(FIELD_H), L_HINT);
		row += step;
	}
	else { place(hTgCap, 0, 0, 0, 0, 0); place(hTgNumber, 0, 0, 0, 0, 0); }

	if (pin)
	{
		HWND hs[2] = { hTgPinRepeat, hTgNoSeq };
		static const int ss[2] = { S_OPT_NOREPEAT, S_OPT_NOSEQ };
		row = flowToggles(hs, ss, 2, cx, cr, row);
		addLabel(S_PIN_NOTE, cx, row, inner, SC(30), L_HINT);
		row += step;
	}
	else { place(hTgPinRepeat, 0, 0, 0, 0, 0); place(hTgNoSeq, 0, 0, 0, 0, 0); }

	/* leave out (characters and PIN), my own symbols (characters, custom set) */
	if (chars || pin)
	{
		int fw = minInt(SC(170), cr - cx - labelW);
		addLabel(S_LBL_EXCLUDE, cx, row, labelW, SC(FIELD_H), L_PLAIN);
		placeField(hExclude, cx + labelW, row, fw, 1);
		if (chars && gOpt.symbolSet == PW_SYM_CUSTOM)
		{
			int lw = textWidth(fUi, T(S_LBL_CUSTOM), -1) + SC(12);
			x = cx + labelW + fw + SC(24);
			if (x + lw + SC(120) > cr) { row += step; x = cx; lw = maxInt(lw, labelW); }
			addLabel(S_LBL_CUSTOM, x, row, lw, SC(FIELD_H), L_PLAIN);
			placeField(hCustom, x + lw, row, minInt(SC(200), cr - x - lw), 1);
		}
		else place(hCustom, 0, 0, 0, 0, 0);
		row += step;
	}
	else { place(hExclude, 0, 0, 0, 0, 0); place(hCustom, 0, 0, 0, 0, 0); }

	/* the strength line and Generate: side by side, or one above the other when narrow */
	if (bottomY >= 0) row = maxInt(row, bottomY);
	if (inner >= SC(560))
	{
		place(hGenerate, cr - SC(160), row, SC(160), hb, 1);
		if (!gMeasure) SetRect(&gStrength, cx, row, cr - SC(180), row + hb);
		row += hb;
	}
	else
	{
		if (!gMeasure) SetRect(&gStrength, cx, row, cr, row + hb);
		row += hb + SC(6);
		place(hGenerate, cx, row, inner, SC(40), 1);
		row += SC(40);
	}
	return row;
}

/* Everything above the password rows, from y0 down, with a band of bandH (0 = none). The
 * list card follows the options card, or starts at listY when that is not negative (a low
 * window, where the list stays put and the part above it scrolls). Returns the top of the
 * password rows; gUpperBottom and gListHeadH say where the options end and how high the
 * list's head is. With gMeasure set it only measures. */
int layoutTop(int W, int y0, int bandH, int listY)
{
	int m = SC(MARGIN), inner = W - 2 * m, y, i, segW = SC(110), track, cx, cr, cardTop, cardBottom, saved, k;

	if (!gMeasure) { gBandTop = y0; gBandH = bandH; }
	y = y0 + bandH + GAP_PX;

	/* the kind of password, and the profile on the right -- or under it when narrow */
	for (i = 0; i < 3; i++) segW = maxInt(segW, textWidth(fBold, T(S_MODE_CHARS + i), -1) + SC(32));
	if (3 * segW + SC(16) > inner) segW = (inner - SC(16)) / 3;
	track = 3 * segW + SC(16);
	if (!gMeasure) SetRect(&gSegTrack, m, y, m + track, y + SC(SEG_H));
	for (i = 0; i < 3; i++)
		place(hSeg[i], m + SC(4) + i * (segW + SC(4)), y + SC(4), segW, SC(SEG_H - 8), 1);
	if (track + SC(12) + SC(220) <= inner)
	{
		int pw = minInt(SC(330), inner - track - SC(12));
		place(hProfile, W - m - pw, y + SC(4), pw, SC(SEG_H - 8), 1);
		y += SC(SEG_H);
	}
	else
	{
		y += SC(SEG_H) + SC(8);
		place(hProfile, m, y, inner, SC(SEG_H - 8), 1);
		y += SC(SEG_H - 8);
	}
	y += GAP_PX;

	/* the options card: as high as the tallest of the three kinds needs at this width, so the
	 * list does not jump when the kind changes */
	cardTop = y;
	cx = m + SC(18);
	cr = W - m - SC(18);
	saved = gMeasure;
	gMeasure = 1;
	cardBottom = 0;
	for (k = PW_CHARS; k <= PW_PIN; k++) cardBottom = maxInt(cardBottom, optionRows(k, cx, cr, cardTop + PAD_PX, -1));
	gMeasure = saved;
	if (!gMeasure)
	{
		/* the strength line and Generate stay at the foot of the card, whatever the kind */
		int foot = cr - cx >= SC(560) ? SC(gCompact ? 40 : 44) : SC(gCompact ? 40 : 44) + SC(6 + 40);
		optionRows(gOpt.mode, cx, cr, cardTop + PAD_PX, cardBottom - foot);
	}
	cardBottom += PAD_PX;
	if (!gMeasure) SetRect(&gCardOpt, m, cardTop, W - m, cardBottom);
	gUpperBottom = cardBottom;

	/* the list card's head: the title and the icons, and "Clear clipboard after [2 min v]"
	 * beside them, or on a line of its own when narrow. It is never cut off. */
	y = listY >= 0 ? listY : cardBottom + GAP_PX;
	saved = gClipBottom;
	gClipBottom = 0x7FFFFFFF;
	{
		int tw = toggleWidth(S_OPT_CLEAR), dw = 0, titleW = textWidth(fBold, T(S_CARD_LIST), -1) + SC(16);
		int icons = SC(72), xr = cr - icons - SC(8), head = SC(gCompact ? LIST_HEAD - 6 : LIST_HEAD), hy = y + SC(gCompact ? 5 : 8);
		for (k = 0; k < CLEAR_CHOICE_N; k++)
			dw = maxInt(dw, textWidth(fUi, T(S_CLR_10S + k), headLen(T(S_CLR_10S + k))));
		dw += SC(12 + 30);
		if (!gMeasure) SetRect(&gCardList, m, y, W - m, y);           /* the bottom comes later */
		addLabel(S_CARD_LIST, cx, hy, minInt(SC(300), titleW), SC(32), L_LIST);
		place(hCopyAll, cr - SC(34) + SC(8), hy, SC(34), SC(32), 1);
		place(hSave, cr - SC(72) + SC(8), hy, SC(34), SC(32), 1);
		if (cx + titleW + tw + SC(6) + dw <= xr)
		{
			place(hClearTime, xr - dw, hy, dw, SC(32), 1);
			place(hTgClear, xr - dw - SC(6) - tw, hy + SC(1), tw, SC(30), 1);
		}
		else
		{
			tw = minInt(tw, cr - cx - SC(6) - dw);
			place(hTgClear, cx, hy + SC(38), tw, SC(30), 1);
			place(hClearTime, cx + tw + SC(6), hy + SC(37), dw, SC(32), 1);
			head += SC(38);
		}
		gListHeadH = head;
		y += head;
	}
	gClipBottom = saved;
	return y;
}

/* Lays the main window out for its current size: the band, the options, the list, the status
 * line and the scroll bar. */
void layout(void)
{
	static int busy = 0, again = 0;
	RECT rc;
	int W, H, avail, top, listH, band, contentH, m = SC(MARGIN), tries = 0;

	if (!hList) return;
	if (busy) { again = 1; return; }      /* the scroll bar came or went: lay out again after */
	busy = 1;
	do
	{
		SCROLLINFO si;

		again = 0;
		GetClientRect(hMain, &rc);
		W = rc.right;
		H = rc.bottom;
		avail = H - SC(STATUS_H);

		/* the band: whole while the list still shows five rows, slim for four, otherwise none;
		 * then, still too low, tighter rows and gaps; then the part above the list scrolls */
		gMeasure = 1;
		gCompact = 0;
		band = SC(BAND_FULL);
		if (layoutTop(W, 0, band, -1) + SC(5 * ROW_LH + 8) > avail)
		{
			band = SC(BAND_SLIM);
			if (layoutTop(W, 0, band, -1) + SC(4 * ROW_LH + 8) > avail) band = 0;
			if (!band && layoutTop(W, 0, band, -1) + SC(4 * ROW_LH + 8) > avail) gCompact = 1;
		}
		top = layoutTop(W, 0, band, -1);
		gMeasure = 0;

		gLabelN = 0;
		gFieldN = 0;
		if (top + SC(MIN_ROWS * ROW_LH + 8) <= avail)
		{
			/* everything fits: the list takes what is left */
			gScrollY = gScrollMax = 0;
			gClipBottom = 0x7FFFFFFF;
			listH = avail - top - SC(8);
			contentH = avail;
			top = layoutTop(W, 0, band, -1);
		}
		else
		{
			/* too low: the list card keeps MIN_ROWS rows at the bottom, and the part above it
			 * scrolls in what is left, cut off just above the card */
			int listY = avail - gListHeadH - SC(MIN_ROWS * ROW_LH + 8);
			listH = SC(MIN_ROWS * ROW_LH);
			gClipBottom = maxInt(0, listY - SC(10));
			contentH = gUpperBottom + SC(10);
			gScrollMax = maxInt(0, contentH - gClipBottom);
			gScrollY = maxInt(0, minInt(gScrollY, gScrollMax));
			contentH = gClipBottom + gScrollMax;
			top = layoutTop(W, -gScrollY, band, listY);
		}
		gCardList.bottom = top + listH + SC(8);
		{
			int clip = gClipBottom;           /* the list itself is never cut off */
			gClipBottom = 0x7FFFFFFF;
			place(hList, m + SC(10), top, W - 2 * m - SC(20), listH, 1);
			gClipBottom = clip;
		}

		/* status line, fixed at the bottom and on top of everything else */
		SetRect(&gStatusR, 0, H - SC(STATUS_H), W, H);
		if (hStatus) SetWindowPos(hStatus, HWND_TOP, 0, gStatusR.top, W, SC(STATUS_H), SWP_NOACTIVATE | SWP_SHOWWINDOW);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMax = contentH - 1;
		si.nPage = (UINT)maxInt(gScrollMax ? gClipBottom : avail, 1);
		si.nPos = gScrollY;
		SetScrollInfo(hMain, SB_VERT, &si, TRUE);
	} while (again && ++tries < 3);
	busy = 0;

	if (hTip) setStrengthTip(TTM_NEWTOOLRECTW);
	/* Every child was moved or resized: repaint all of them whole. Moving a child only
	 * repaints the part that comes into view, so the owner-drawn controls (slider track,
	 * stepper, list rows with their copy icons at the right edge) would keep stale pixels. */
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
	accRefresh(hMain);              /* shown, hidden or renamed: screen readers are told */
}

/* Scrolls the part above the list (low windows only). */
void scrollTo(int y)
{
	y = maxInt(0, minInt(y, gScrollMax));
	if (y == gScrollY) return;
	gScrollY = y;
	layout();
}

/* Tab moved the focus to a control scrolled out of sight: scroll it back into view. */
void keepFocusInView(void)
{
	static HWND last = NULL;
	HWND f = GetFocus();
	RECT r;

	if (f == last) return;
	last = f;
	if (!f || gScrollMax <= 0 || GetParent(f) != hMain) return;
	if (f == hList || f == hTgClear || f == hClearTime || f == hSave || f == hCopyAll) return;   /* never scrolled */
	GetWindowRect(f, &r);
	MapWindowPoints(NULL, hMain, (POINT*)&r, 2);
	if (r.top < 0) scrollTo(gScrollY + r.top - SC(8));
	else if (r.bottom > gClipBottom) scrollTo(gScrollY + r.bottom - gClipBottom + SC(8));
}

/* ------------------------------------------------------------------ painting */

/* The band at the top: a gradient from the band colour to a lighter tone of the accent. */
static void paintBand(HDC dc, int W)
{
	RECT band;
	GpGraphics* g;
	int top = gBandTop, h = gBandH, slim = gBandH < SC(BAND_FULL), icon = slim ? SC(28) : SC(ICON_SZ);
	int tx = SC(MARGIN) + icon + SC(14);

	/* whole: the icon, the title and the subtitle; slim: the title only; low windows: none */
	if (h <= 0 || top + h <= 0) return;
	SetRect(&band, 0, top, W, top + h);
	g = gfx(dc);
	if (g)
	{
		GpPointF a = { 0, 0 }, b = { (float)W, 0 };
		GpBrush* br = NULL;
		GdipCreateLineBrush(&a, &b, argb(PAL.band, 255), argb(PAL.band2, 255), 0, &br);
		if (br) { GdipFillRectangle(g, br, 0, (float)top, (float)W, (float)h); GdipDeleteBrush(br); }
		GdipDeleteGraphics(g);
	}
	else fillSolid(dc, &band, PAL.band);

	if (icBand)
		DrawIconEx(dc, SC(MARGIN), top + (h - icon) / 2, icBand, icon, icon, 0, NULL, DI_NORMAL);

	if (slim)
	{
		SetRect(&band, tx, top, W - SC(20), top + h);
		drawText(dc, T(S_APP_TITLE), -1, &band, fTitle, PAL.bandText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		return;
	}
	SetRect(&band, tx, top + SC(5), W - SC(20), top + SC(34));
	drawText(dc, T(S_APP_TITLE), -1, &band, fTitle, PAL.bandText, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	SetRect(&band, tx + SC(1), top + SC(33), W - SC(20), top + SC(53));
	drawText(dc, T(S_APP_SUBTITLE), -1, &band, fSub, PAL.bandSub, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

/* Everything of the main window that is not a control: page, band, cards, field frames,
 * labels and the strength line. */
void paintMain(HDC dc)
{
	RECT rc, r;
	int i;

	GetClientRect(hMain, &rc);
	fillSolid(dc, &rc, PAL.page);

	/* the part above the list, cut off above the list card when it scrolls */
	SaveDC(dc);
	if (gClipBottom < rc.bottom) IntersectClipRect(dc, 0, 0, rc.right, gClipBottom);
	paintBand(dc, rc.right);
	roundRect(dc, &gSegTrack, SC(10), PAL.field, PAL.border, (float)SC(1));
	roundRect(dc, &gCardOpt, SC(12), PAL.card, PAL.border, (float)SC(1));

	for (i = 0; i < gFieldN; i++)
	{
		int focus = GetFocus() == gFieldEdit[i];
		roundRect(dc, &gFieldBox[i], SC(8), PAL.field, focus ? PAL.accent : PAL.fieldBorder, focus ? (float)SC(2) : (float)SC(1));
	}

	for (i = 0; i < gLabelN; i++)
	{
		const Label* l = &gLabels[i];
		if (l->style == L_LIST) continue;                   /* the list's title: below */
		r = l->r;
		drawText(dc, T(l->str), -1, &r, l->style == L_BOLD ? fBold : l->style == L_HINT ? fSmall : fUi,
		         l->style == L_HINT ? PAL.hint : PAL.text,
		         DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}
	paintStrength(dc);
	RestoreDC(dc, -1);

	/* the list card, never cut off */
	roundRect(dc, &gCardList, SC(12), PAL.card, PAL.border, (float)SC(1));
	for (i = 0; i < gLabelN; i++)
		if (gLabels[i].style == L_LIST)
		{
			r = gLabels[i].r;
			drawText(dc, T(gLabels[i].str), -1, &r, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		}
}

/* The status line is a child window of its own, on top of the others: in a low window the
 * content scrolls under it, and every control clips its siblings, so nothing paints over it. */
static LRESULT CALLBACK statusProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_PAINT)
	{
		PAINTSTRUCT ps;
		BackBuf bb;
		HDC dc = BeginPaint(hwnd, &ps), mem;
		RECT rc, r;
		int m = SC(MARGIN);

		GetClientRect(hwnd, &rc);
		mem = bufBegin(&bb, dc, 0, 0, rc.right, rc.bottom);
		fillSolid(mem, &rc, PAL.page);
		SetRect(&r, m + SC(4), 0, rc.right - m - SC(80), rc.bottom);
		drawText(mem, gStatusText, -1, &r, fUi,
		         gStatusKind == ST_ERR ? PAL.err : gStatusKind == ST_WARN ? PAL.warn : gStatusKind == ST_OK ? PAL.ok : PAL.hint,
		         DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		SetRect(&r, rc.right - m - SC(80), 0, rc.right - m, rc.bottom);
		drawText(mem, L"v" APP_VERSION, -1, &r, fUi, PAL.hint, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		bufEnd(&bb);
		EndPaint(hwnd, &ps);
		return 0;
	}
	if (msg == WM_ERASEBKGND) return 1;
	return DefWindowProcW(hwnd, msg, wp, lp);
}
