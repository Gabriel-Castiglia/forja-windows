/**
  draw.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  Drawing primitives (anti-aliased shapes through the GDI+ flat API, text, fills) and the
  small glyphs: copy, check mark, save, chevron, plus and minus.
**/

#include "forja.h"

/* ------------------------------------------------------------------ drawing primitives */

DWORD argb(COLORREF c, int alpha)
{
	return ((DWORD)alpha << 24) | ((DWORD)GetRValue(c) << 16) | ((DWORD)GetGValue(c) << 8) | GetBValue(c);
}

GpGraphics* gfx(HDC dc)
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
void roundRect(HDC dc, const RECT* r, int radius, COLORREF fill, COLORREF line, float width)
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

void circle(HDC dc, float cx, float cy, float rad, COLORREF fill, COLORREF line, float width)
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

void fillSolid(HDC dc, const RECT* r, COLORREF c)
{
	HBRUSH b = CreateSolidBrush(c);
	FillRect(dc, r, b);
	DeleteObject(b);
}

void drawText(HDC dc, const wchar_t* s, int n, RECT* r, HFONT f, COLORREF c, UINT flags)
{
	HGDIOBJ of = SelectObject(dc, f);
	SetBkMode(dc, TRANSPARENT);
	SetTextColor(dc, c);
	DrawTextW(dc, s, n, r, flags | DT_NOPREFIX);
	SelectObject(dc, of);
}

int textWidth(HFONT f, const wchar_t* s, int n)
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
int headLen(const wchar_t* s)
{
	const wchar_t* tab = wcschr(s, L'\t');
	return tab ? (int)(tab - s) : (int)wcslen(s);
}

/* ------------------------------------------------------------------ glyphs */

void glyphCopy(HDC dc, int x, int y, int s, COLORREF ink, COLORREF fill)
{
	RECT a, b;
	int off = s * 5 / 16, rad = SC(3);

	SetRect(&a, x + off, y, x + s, y + s - off);
	SetRect(&b, x, y + off, x + s - off, y + s);
	roundRect(dc, &a, rad, CLR_NONE, ink, (float)SC(1) * 1.4f);
	roundRect(dc, &b, rad, fill, ink, (float)SC(1) * 1.4f);
}

void glyphCheck(HDC dc, int x, int y, int s, COLORREF ink, float w)
{
	float p[6] = { x + s * 0.16f, y + s * 0.54f, x + s * 0.41f, y + s * 0.78f, x + s * 0.86f, y + s * 0.26f };
	strokes(dc, p, 3, ink, w);
}

void glyphSave(HDC dc, int x, int y, int s, COLORREF ink)
{
	float w = (float)SC(1) * 1.5f;
	float tray[8] = { (float)x, y + s * 0.58f, (float)x, (float)(y + s), (float)(x + s), (float)(y + s), (float)(x + s), y + s * 0.58f };
	float stem[4] = { x + s * 0.5f, (float)y, x + s * 0.5f, y + s * 0.70f };
	float head[6] = { x + s * 0.25f, y + s * 0.45f, x + s * 0.5f, y + s * 0.70f, x + s * 0.75f, y + s * 0.45f };
	strokes(dc, tray, 4, ink, w);
	strokes(dc, stem, 2, ink, w);
	strokes(dc, head, 3, ink, w);
}

void glyphChevron(HDC dc, int cx, int cy, int s, COLORREF ink)
{
	float p[6] = { cx - s * 0.5f, cy - s * 0.25f, (float)cx, cy + s * 0.25f, cx + s * 0.5f, cy - s * 0.25f };
	strokes(dc, p, 3, ink, (float)SC(1) * 1.6f);
}

void glyphPlusMinus(HDC dc, int cx, int cy, int s, COLORREF ink, int plus)
{
	float h[4] = { cx - s * 0.5f, (float)cy, cx + s * 0.5f, (float)cy };
	float v[4] = { (float)cx, cy - s * 0.5f, (float)cx, cy + s * 0.5f };
	strokes(dc, h, 2, ink, (float)SC(1) * 1.8f);
	if (plus) strokes(dc, v, 2, ink, (float)SC(1) * 1.8f);
}
