/**
  controls.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The owner-drawn controls: buttons of every kind (segments, chips, switches, drop-downs,
  icons, steppers, theme segments and swatches), the slider class and the owner-drawn menus.
**/

#include "forja.h"
#include <oleacc.h>               /* ROLE_SYSTEM_* and STATE_SYSTEM_*, for screen readers */

/* ------------------------------------------------------------------ owner-drawn buttons */

Ctl* ctlOf(HWND h) { return (Ctl*)GetWindowLongPtrW(h, GWLP_USERDATA); }

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
	if (p.y < gBandTop + gBandH) return PAL.band;
	if (PtInRect(&gSegTrack, p)) return PAL.field;
	if (p.y < gCardOpt.top) return PAL.page;
	if (h == hCntDec || h == hCntInc) return PAL.field;
	return PAL.card;
}

/* "2 min", "45 s"...: one of the drop-down's own texts when the time is one of its choices. */
static void clearTimeText(int secs, wchar_t* buf, int cap)
{
	int i;

	for (i = 0; i < CLEAR_CHOICE_N; i++)
		if (CLEAR_CHOICES[i] == secs) { headCopy(buf, cap, T(S_CLR_10S + i)); return; }
	if (secs % 60 == 0) _snwprintf(buf, cap, T(S_CLR_FMT_M), secs / 60);
	else                _snwprintf(buf, cap, T(S_CLR_FMT_S), secs);
	buf[cap - 1] = 0;
}

/* What a drop-down button shows: the profile, the symbol set, the clipboard time or the
 * separator (without the hint after the tab). */
static void dropText(HWND h, wchar_t* buf, int cap)
{
	if (h == hProfile)
	{
		int p = currentPreset();
		const wchar_t* name = p >= 0 ? T(PRESET_STR[p]) : T(S_PRESET_CUSTOM);
		_snwprintf(buf, cap, T(S_PROFILE_FMT), headLen(name), name);
		buf[cap - 1] = 0;
	}
	else if (h == hSymSet)    headCopy(buf, cap, T(S_SYM_BASIC + gOpt.symbolSet));
	else if (h == hClearTime) clearTimeText(gClearSecs, buf, cap);
	else                      headCopy(buf, cap, T(S_SEP_HYPHEN + gOpt.separator));
}

/* ------------------------------------------------------------------ for screen readers */

/* The name, role and state of an owner-drawn button, told to Windows (access.c) when they
 * change. The name is the button's window text, which Windows reads for any button; only
 * the Generate button draws that text, the others draw from their own data. */
static void accButton(HWND h, Ctl* c)
{
	wchar_t name[200], cur[200], val[160];
	int role = ROLE_SYSTEM_PUSHBUTTON, checked = -1, state;

	switch (c->kind)
	{
	case K_SEG:      role = ROLE_SYSTEM_RADIOBUTTON; checked = gOpt.mode == c->value; break;
	case K_THEMESEG: role = ROLE_SYSTEM_RADIOBUTTON; checked = gTheme == c->value;    break;
	case K_SWATCH:   role = ROLE_SYSTEM_RADIOBUTTON; checked = gAccent == c->value;   break;
	case K_CHIP:
	case K_TOGGLE:   role = ROLE_SYSTEM_CHECKBUTTON; checked = c->bind && *c->bind;   break;
	case K_DROP:     role = ROLE_SYSTEM_BUTTONDROPDOWN; break;
	default:         break;
	}

	name[0] = 0;
	if (c->kind == K_DROP)
	{
		dropText(h, val, 160);
		if (h == hSymSet)         _snwprintf(name, 200, L"%ls: %ls", T(S_LBL_SYMBOLS), val);
		else if (h == hSeparator) _snwprintf(name, 200, L"%ls: %ls", T(S_LBL_SEPARATOR), val);
		else                      lstrcpynW(name, val, 200);
	}
	else if (c->kind == K_ICON) lstrcpynW(name, T(c->glyph == G_SAVE ? S_TIP_SAVE : S_TIP_COPYALL), 200);
	else if (c->kind == K_STEP) lstrcpynW(name, T(c->glyph == G_MINUS ? S_ACC_FEWER : S_ACC_MORE), 200);
	else if (c->kind != K_PRIMARY && c->str >= 0) headCopy(name, 200, T(c->str));
	name[199] = 0;
	if (name[0] && (!GetWindowTextW(h, cur, 200) || wcscmp(cur, name))) SetWindowTextW(h, name);

	if (c->accRole != role) { accSetRole(h, role); c->accRole = role; }
	if (checked < 0) return;    /* a plain button: Windows knows its state */
	state = accStateOf(h, checked);
	if (state != c->accState)
	{
		accSetState(h, state);
		if (c->accState) accNotify(h, EVENT_OBJECT_STATECHANGE);
		c->accState = state;
	}
}

static BOOL CALLBACK accChild(HWND h, LPARAM lp)
{
	(void)lp;
	if ((GetWindowLongPtrW(h, GWL_STYLE) & BS_TYPEMASK) == BS_OWNERDRAW && ctlOf(h))
	{
		wchar_t cls[16];
		if (GetClassNameW(h, cls, 16) && !lstrcmpiW(cls, L"Button")) accButton(h, ctlOf(h));
	}
	return TRUE;
}

/* After the layout or the options changed: every owner-drawn button of a window, the
 * slider, the number and text fields (whose labels are painted, not windows) and the list. */
void accRefresh(HWND parent)
{
	wchar_t v[16];
	int label = gOpt.mode == PW_WORDS ? S_LBL_WORDS : gOpt.mode == PW_PIN ? S_LBL_DIGITS : S_LBL_LENGTH;

	if (!parent) return;
	EnumChildWindows(parent, accChild, 0);
	if (parent != hMain) return;
	if (hSlider)
	{
		accSetRole(hSlider, ROLE_SYSTEM_SLIDER);
		accSetName(hSlider, T(label));
		_snwprintf(v, 16, L"%d", (int)SendMessageW(hSlider, SLM_GETPOS, 0, 0));
		v[15] = 0;
		accSetValue(hSlider, v);
		accSetState(hSlider, accStateOf(hSlider, -1));
	}
	accSetName(hLen, T(label));
	accSetName(hCnt, T(S_LBL_COUNT));
	accSetName(hExclude, T(S_LBL_EXCLUDE));
	accSetName(hCustom, T(S_LBL_CUSTOM));
	accSetName(hList, T(S_ACC_LIST));
}

void drawButton(const DRAWITEMSTRUCT* d)
{
	Ctl* c = ctlOf(d->hwndItem);
	RECT r = d->rcItem, t;
	int down = (d->itemState & ODS_SELECTED) != 0, dis = (d->itemState & ODS_DISABLED) != 0;
	int focus = (d->itemState & ODS_FOCUS) && !(d->itemState & ODS_NOFOCUSRECT);
	int hot = c && c->hot;
	COLORREF under = underColour(d->hwndItem);
	BackBuf bb;
	HDC mem;
	int w = r.right - r.left, h = r.bottom - r.top;
	wchar_t buf[160];

	if (!c) return;
	accButton(d->hwndItem, c);

	/* drawn off screen, then copied in one go: no flicker on hover */
	mem = bufBegin(&bb, d->hDC, r.left, r.top, w, h);
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
	case K_SEG:          /* the kind of password (main window) and the theme (Theme window) */
	case K_THEMESEG:
	{
		int on = c->kind == K_SEG ? gOpt.mode == c->value : gTheme == c->value;
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
		/* the clipboard time is greyed while clearing is off */
		drawText(mem, buf, -1, &t, fUi, (d->hwndItem == hClearTime && !gClearClip) ? PAL.hint : PAL.text,
		         DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
		glyphChevron(mem, r.right - SC(16), h / 2, SC(9), PAL.hint);
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

	bufEnd(&bb);
}

/* The subclass of every owner-drawn button: hover tracking, the hand cursor, and double
 * clicks fed back as single ones. It frees the button's Ctl when the button goes. */
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

HWND mkButton(HWND parent, int id, int kind, int str, int* bind, int value, int glyph)
{
	HWND b = CreateWindowExW(0, L"BUTTON", str >= 0 ? T(str) : L"", WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
	                         0, 0, 10, 10, parent, (HMENU)(INT_PTR)id, gInst, NULL);
	Ctl* c = (Ctl*)calloc(1, sizeof(Ctl));

	if (c) { c->kind = kind; c->str = str; c->bind = bind; c->value = value; c->glyph = glyph; }
	SetWindowLongPtrW(b, GWLP_USERDATA, (LONG_PTR)c);
	SetWindowSubclass(b, ctlProc, 1, 0);
	SendMessageW(b, WM_SETFONT, (WPARAM)fUi, FALSE);
	return b;
}

/* The default push button of a window (Generate, OK): pressed by Enter, in the bold font. */
HWND mkDefaultButton(HWND parent, int id, int str)
{
	HWND b = mkButton(parent, id, K_PRIMARY, str, NULL, 0, G_NONE);

	SetWindowLongPtrW(b, GWL_STYLE, GetWindowLongPtrW(b, GWL_STYLE) | BS_DEFPUSHBUTTON);
	SendMessageW(b, WM_SETFONT, (WPARAM)fBold, FALSE);
	return b;
}

/* Registers a window class of this program. A second call for the same class fails quietly,
 * which is harmless: the class is already there. */
void registerClass(const wchar_t* name, WNDPROC proc, UINT style)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.style = style;
	wc.lpfnWndProc = proc;
	wc.hInstance = gInst;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = name;
	RegisterClassW(&wc);
}

/* ------------------------------------------------------------------ the slider */

/* A slider of our own: the trackbar cannot be themed dark, nor drawn anti-aliased. It
 * tells its parent about every change with WM_COMMAND(id, SLN_CHANGE). */

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
	{
		wchar_t v[16];
		_snwprintf(v, 16, L"%d", pos);
		v[15] = 0;
		accSetValue(h, v);
		accNotify(h, EVENT_OBJECT_VALUECHANGE);
	}
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

LRESULT CALLBACK sliderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
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
	case WM_KILLFOCUS:
		InvalidateRect(hwnd, NULL, FALSE);
		accSetState(hwnd, accStateOf(hwnd, -1));
		if (msg == WM_SETFOCUS) accNotify(hwnd, EVENT_OBJECT_FOCUS);
		return 0;
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
		BackBuf bb;
		HDC dc = BeginPaint(hwnd, &ps), mem;
		RECT r, tr;
		int x, cy, th = SC(4), kr;

		GetClientRect(hwnd, &r);
		mem = bufBegin(&bb, dc, 0, 0, r.right, r.bottom);
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
		bufEnd(&bb);
		EndPaint(hwnd, &ps);
		return 0;
	}
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

/* ------------------------------------------------------------------ menus */

/* Owner-drawn: their text changes with the language, and Windows draws a menu bar white
 * however dark the window is. Each item carries its string id in the item data, plus flags. */

/* MENU_SEP, MENU_BAR and MENU_ID are in forja.h (access.c names the items too). */
#define MENU_ISBAR(d) (((d) & MENU_BAR) != 0)

void measureMenuItem(MEASUREITEMSTRUCT* mi)
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
	w = textWidth(fUi, t, headLen(t));

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

void drawMenuItem(const DRAWITEMSTRUCT* d)
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

/* An owner-drawn item: its string id travels as the item data (see drawMenuItem). */
static void addItem(HMENU m, UINT id, int str, int checked)
{
	AppendMenuW(m, MF_OWNERDRAW | (checked ? MF_CHECKED : 0), id, (LPCWSTR)(ULONG_PTR)str);
}

static void addSep(HMENU m) { AppendMenuW(m, MF_OWNERDRAW | MF_SEPARATOR, 0, (LPCWSTR)MENU_SEP); }

/* The background of a menu and its submenus, in the theme's menu colour. The brush comes from
 * the cache (draw.c): opening a drop-down list no longer deletes the brush the menu bar still
 * paints with. */
static void setMenuBrush(HMENU m)
{
	MENUINFO mi;

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;
	mi.hbrBack = solidBrush(PAL.menuBg);
	SetMenuInfo(m, &mi);
}

/* The three kinds of password and the profiles: the Passwords menu and the Profile button. */
static void fillPresetMenu(HMENU m)
{
	int i, cur = currentPreset();

	addItem(m, IDM_MODE_CHARS, S_MENU_MODE_CHARS, gOpt.mode == PW_CHARS);
	addItem(m, IDM_MODE_WORDS, S_MENU_MODE_WORDS, gOpt.mode == PW_WORDS);
	addItem(m, IDM_MODE_PIN, S_MENU_MODE_PIN, gOpt.mode == PW_PIN);
	addSep(m);
	for (i = 0; i < P_COUNT; i++) addItem(m, IDM_PRESET_0 + i, PRESET_STR[i], cur == i);
}

/* The menu bar, made again whenever a check mark, the language, the theme or the scale
 * changes. The old bar is destroyed once the new one is in place. */
void buildMenu(HWND hwnd)
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
	addItem(help, IDM_UPDATE, S_MENU_UPDATE, 0);
	addItem(help, IDM_UPDATE_AUTO, S_MENU_UPDATE_AUTO, gAutoUpdate);
	addSep(help);
	addItem(help, IDM_ABOUT, S_MENU_ABOUT, 0);

	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)file, (LPCWSTR)(ULONG_PTR)(S_MENU_FILE | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)kind, (LPCWSTR)(ULONG_PTR)(S_MENU_KIND | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)view, (LPCWSTR)(ULONG_PTR)(S_MENU_VIEW | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)lang, (LPCWSTR)(ULONG_PTR)(S_MENU_LANG | MENU_BAR));
	AppendMenuW(bar, MF_POPUP | MF_OWNERDRAW, (UINT_PTR)help, (LPCWSTR)(ULONG_PTR)(S_MENU_HELP | MENU_BAR));

	setMenuBrush(bar);
	accNameMenu(bar);
	SetMenu(hwnd, bar);
	if (hMenu) DestroyMenu(hMenu);
	hMenu = bar;
	DrawMenuBar(hwnd);
}

/* A drop-down button's list, under the button. Returns the command picked, or 0. */
UINT dropMenu(HWND button)
{
	HMENU m = CreatePopupMenu();
	RECT r;
	UINT cmd;
	int i;

	if (button == hProfile) fillPresetMenu(m);
	else if (button == hSymSet)
		for (i = 0; i <= PW_SYM_CUSTOM; i++) addItem(m, IDM_SYM_0 + i, S_SYM_BASIC + i, gOpt.symbolSet == i);
	else if (button == hClearTime)
		for (i = 0; i < CLEAR_CHOICE_N; i++) addItem(m, IDM_CLR_0 + i, S_CLR_10S + i, gClearSecs == CLEAR_CHOICES[i]);
	else
		for (i = 0; i < PW_SEP_COUNT; i++) addItem(m, IDM_SEP_0 + i, S_SEP_HYPHEN + i, gOpt.separator == i);

	setMenuBrush(m);
	accNameMenu(m);
	GetWindowRect(button, &r);
	cmd = (UINT)TrackPopupMenu(m, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY,
	                           r.left, r.bottom + SC(4), 0, hMain, NULL);
	DestroyMenu(m);
	return cmd;
}
