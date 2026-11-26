/**
  theme.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The colours (palette and the twelve accents), the light / dark theme, the icon tinted in
  the accent, the fonts and the language.
**/

#include "forja.h"

/* ------------------------------------------------------------------ palette */

const Accent ACCENTS[ACCENT_COUNT] = {
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

COLORREF mix(COLORREF a, COLORREF b, double t)
{
	return RGB((int)(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t + 0.5),
	           (int)(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t + 0.5),
	           (int)(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t + 0.5));
}

double luminance(COLORREF c)
{
	return (0.2126 * GetRValue(c) + 0.7152 * GetGValue(c) + 0.0722 * GetBValue(c)) / 255.0;
}

void makePalette(void)
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

void applyTheme(HWND hwnd)
{
	BOOL dark = gDark ? TRUE : FALSE;

	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
	themeChild(hwnd, (LPARAM)gDark);            /* the window's own scroll bar (Help) */
	EnumChildWindows(hwnd, themeChild, (LPARAM)gDark);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_FRAME);
}

/* ------------------------------------------------------------------ the icon, in the accent */

/* The icon: the engraving (anvil and hammers) laid on a rounded tile of the accent's colour.
 * The drawing comes without the tile as resource IDR_ICONLAYER (tools/make_icon.py): 256 x 256
 * pixels of T, K, R, G, B, where the pixel is rgb = RGB + tile * K / 255 with alpha T,
 * premultiplied. Here it is coloured, reduced to the size asked by averaging the area each
 * pixel covers, and turned into an icon. Without the resource, the executable's own icon. */

static const unsigned char* gLayer;
static int gLayerN;

void loadLayer(void)
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

HICON tintedIconFor(int accent, int size)
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

void makeIcons(void)
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

void refreshTheme(void)
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

void makeFonts(void)
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

/* The scale of the screen a window is on (GetDpiForWindow, Windows 10 1607 and later; looked up
 * at run time), or 0 when it cannot be known. */
int windowDpi(HWND h)
{
	typedef UINT (WINAPI *PGetDpiForWindow)(HWND);
	static PGetDpiForWindow get = NULL;
	static int tried = 0;

	if (!tried)
	{
		tried = 1;
		get = (PGetDpiForWindow)(void*)GetProcAddress(GetModuleHandleW(L"user32.dll"), "GetDpiForWindow");
	}
	return (get && h) ? (int)get(h) : 0;
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

/* The manifest asks for per-monitor DPI (PerMonitorV2), so Windows no longer stretches the window
 * as a picture when it goes to a screen with another scale: it says so (WM_DPICHANGED, with the
 * rectangle the window should take there) and everything measured with SC() is made again at the
 * new scale -- fonts, icons, the height of the list's rows, the tips, the menu and the layout. The
 * help, About and theme windows open again at it, as on a change of language. */
void applyDpi(int dpi, const RECT* rect)
{
	HFONT oldPw = fPw, oldPwSmall = fPwSmall;

	if (dpi < 96 || dpi > 960) return;
	if (dpi != gDpi)
	{
		gDpi = dpi;
		fPw = fPwSmall = NULL;
		makeFonts();
		makeIcons();
		EnumChildWindows(hMain, setFontChild, 0);
		SendMessageW(hTip, WM_SETFONT, (WPARAM)fUi, FALSE);
		SendMessageW(hTip, TTM_SETMAXTIPWIDTH, 0, SC(420));
		if (hList) SendMessageW(hList, LB_SETITEMHEIGHT, 0, SC(ROW_LH));
		buildMenu(hMain);
	}
	if (rect) SetWindowPos(hMain, NULL, rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top,
	                       SWP_NOZORDER | SWP_NOACTIVATE);
	if (oldPw == fPw) return;   /* the same scale: only moved */

	layout();
	if (hHelpWnd)  { DestroyWindow(hHelpWnd); showHelp(hMain); }
	if (hAboutWnd) { DestroyWindow(hAboutWnd); showAbout(hMain); }
	if (hThemeWnd) { DestroyWindow(hThemeWnd); showTheme(hMain); }
	dropOldFonts();
	if (oldPw) DeleteObject(oldPw);
	if (oldPwSmall) DeleteObject(oldPwSmall);
	RedrawWindow(hMain, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
}

void setLanguage(int lang)
{
	if (lang == gLang) return;
	gLang = lang;
	iniSetInt(L"Language", lang);
	applyLanguage();
}
