/**
  dialogs.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The Help, About and Theme windows.
**/

#include "forja.h"

/* The light strip at the foot of the Help and Theme windows, where the OK button sits. */
static void paintFooter(HDC dc, const RECT* rc)
{
	RECT bar;

	SetRect(&bar, 0, rc->bottom - SC(64), rc->right, rc->bottom);
	fillSolid(dc, &bar, PAL.page);
	bar.bottom = bar.top + 1;
	fillSolid(dc, &bar, PAL.border);
}

/* A modal window of the given client size, centred across its owner and a third of the way
 * down, with the icons of the main window. NULL if it could not be made. */
static HWND createModal(HWND owner, const wchar_t* cls, const wchar_t* title, int cw, int ch)
{
	RECT r, want;
	HWND w;

	SetRect(&want, 0, 0, cw, ch);
	AdjustWindowRectEx(&want, WS_POPUPWINDOW | WS_CAPTION, FALSE, WS_EX_DLGMODALFRAME);
	GetWindowRect(owner, &r);
	w = CreateWindowExW(WS_EX_DLGMODALFRAME, cls, title, WS_POPUPWINDOW | WS_CAPTION,
	                    r.left + ((r.right - r.left) - (want.right - want.left)) / 2,
	                    r.top + ((r.bottom - r.top) - (want.bottom - want.top)) / 3,
	                    want.right - want.left, want.bottom - want.top, owner, NULL, gInst, NULL);
	setWindowIcons(w);
	return w;
}

/* ------------------------------------------------------------------ help window */

/* The help text is one string per language: "# " starts a heading, "\x2022 " a bullet,
 * an empty line a new paragraph. It is laid out here, so it reads like a document. */

static int gHelpScroll = 0, gHelpHeight = 0;

/* Lays the help text out from `top`, scrolled by `scroll`, and paints it when `paint` is set.
 * Returns the height of the whole text. */
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

/* Scrolls the help text to pos (clamped), and updates the scroll bar. */
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
		BackBuf bb;
		HDC dc = BeginPaint(hwnd, &ps), mem;
		RECT rc;

		GetClientRect(hwnd, &rc);
		mem = bufBegin(&bb, dc, 0, 0, rc.right, rc.bottom);
		fillSolid(mem, &rc, PAL.card);
		helpLayout(mem, rc.right, SC(16), gHelpScroll, 1);
		paintFooter(mem, &rc);
		bufEnd(&bb);
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

/* The Help window: not modal, so it can stay open beside the main window. */
void showHelp(HWND owner)
{
	HWND ok;
	RECT r;

	if (hHelpWnd) { SetForegroundWindow(hHelpWnd); return; }

	registerClass(L"ForjaHelp", helpProc, CS_HREDRAW | CS_VREDRAW);
	gHelpScroll = 0;
	GetWindowRect(owner, &r);
	hHelpWnd = CreateWindowExW(0, L"ForjaHelp", T(S_HELP_TITLE), WS_OVERLAPPEDWINDOW | WS_VSCROLL,
	                           r.left + SC(40), r.top + SC(30), SC(700), SC(660), owner, NULL, gInst, NULL);
	if (!hHelpWnd) return;
	setWindowIcons(hHelpWnd);
	ok = mkDefaultButton(hHelpWnd, IDOK, S_BTN_OK);
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

/* The height of s wrapped to `width` in the font selected into dc. */
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
		BackBuf bb;
		HDC dc = BeginPaint(hwnd, &ps), mem;
		RECT rc, t;
		wchar_t line[128];
		int tx = SC(AB_M + AB_ICON + 20), wdt;

		GetClientRect(hwnd, &rc);
		mem = bufBegin(&bb, dc, 0, 0, rc.right, rc.bottom);
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

		bufEnd(&bb);
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

/* The About window: modal, it disables the main window while it is up. */
void showAbout(HWND owner)
{
	HWND ok;
	RECT c;

	if (hAboutWnd) { SetForegroundWindow(hAboutWnd); return; }

	registerClass(L"ForjaAbout", aboutProc, 0);
	hAboutWnd = createModal(owner, L"ForjaAbout", T(S_ABOUT_TITLE), SC(AB_W),
	                        SC(AB_TEXT) + aboutBodyHeight(SC(AB_W - 2 * AB_M)) + SC(24) + SC(56));
	if (!hAboutWnd) return;

	GetClientRect(hAboutWnd, &c);
	ok = mkDefaultButton(hAboutWnd, IDOK, S_BTN_OK);
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

/* Follow Windows (0), light (1) or dark (2): applied at once and remembered. */
void setThemeChoice(int theme)
{
	gTheme = theme;
	iniSetInt(L"Theme", gTheme);
	refreshTheme();
	buildMenu(hMain);
	if (hThemeWnd) RedrawWindow(hThemeWnd, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN | RDW_FRAME);
}

/* One of the twelve colours: applied at once to every window and the icon, and remembered. */
void setAccent(int accent)
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
		BackBuf bb;
		HDC dc = BeginPaint(hwnd, &ps), mem;
		RECT rc, t, track;

		GetClientRect(hwnd, &rc);
		mem = bufBegin(&bb, dc, 0, 0, rc.right, rc.bottom);
		fillSolid(mem, &rc, PAL.card);
		SetRect(&t, SC(TH_M), themeY(0), rc.right - SC(TH_M), themeY(0) + SC(24));
		drawText(mem, T(S_THEME_APPEARANCE), -1, &t, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		SetRect(&track, SC(TH_M), themeY(1), SC(TH_M) + 3 * SC(140) + 4 * SC(4), themeY(1) + SC(40));
		roundRect(mem, &track, SC(10), PAL.field, PAL.border, (float)SC(1));
		SetRect(&t, SC(TH_M), themeY(2), rc.right - SC(TH_M), themeY(2) + SC(24));
		drawText(mem, T(S_THEME_COLOUR), -1, &t, fBold, PAL.text, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		paintFooter(mem, &rc);
		bufEnd(&bb);
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

/* The Theme window: modal like About. The swatch icons are made here and destroyed with it. */
void showTheme(HWND owner)
{
	static const int names[3] = { S_THEME_AUTO, S_THEME_LIGHT, S_THEME_DARK };
	HWND ok;
	RECT c;
	int i, rows = (ACCENT_COUNT + TH_COLS - 1) / TH_COLS;
	int cw = 2 * TH_M + TH_COLS * TH_TILE + (TH_COLS - 1) * TH_GAP;
	int ch = 132 + rows * TH_TILEH + (rows - 1) * TH_GAP + 24 + 64;     /* 132: themeY(3), the tiles */

	if (hThemeWnd) { SetForegroundWindow(hThemeWnd); return; }

	registerClass(L"ForjaTheme", themeProc, 0);
	hThemeWnd = createModal(owner, L"ForjaTheme", T(S_THEME_TITLE), SC(cw), SC(ch));
	if (!hThemeWnd) return;
	for (i = 0; i < ACCENT_COUNT; i++) gSwatchIcon[i] = tintedIconFor(i, SC(44));

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
	ok = mkDefaultButton(hThemeWnd, IDOK, S_BTN_OK);
	SetWindowPos(ok, NULL, c.right - SC(TH_M) - SC(110), c.bottom - SC(50), SC(110), SC(36), SWP_NOZORDER);

	applyTheme(hThemeWnd);
	ShowWindow(hThemeWnd, SW_SHOW);
	EnableWindow(owner, FALSE);
	SetFocus(ok);
}
