/**
  forja.h
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  What the modules of the window share: the identifiers, the layout units, the state of the
  window and the functions one module calls in another. forja.c has the state and the window
  procedure; each other module says at its top what it does.
**/

#ifndef FORJA_H
#define FORJA_H

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <uxtheme.h>
#include <dwmapi.h>
#include <bcrypt.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

#include "pwgen.h"
#include "resource.h"
#include "ui_strings.h"
#include "update.h"
#include "version.h"

#ifndef BCRYPT_USE_SYSTEM_PREFERRED_RNG
#define BCRYPT_USE_SYSTEM_PREFERRED_RNG 0x00000002
#endif

#define WIDEN_(s)     L##s
#define WIDEN(s)      WIDEN_(s)
#define APP_VERSION   WIDEN(WH_VERSION_STR)
#define APP_DATE      WIDEN(WH_DATE_STR)
#define APP_REPO      WIDEN(WH_REPO_STR)
#define APP_AUTHOR    L"Gabriel Castiglia"
#define APP_AUMID     L"Castiglia.Forja"
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

#define ACCENT_COUNT 12
extern const Accent ACCENTS[ACCENT_COUNT];

/* ------------------------------------------------------------------ state */

enum {
	IDC_SEG_CHARS = 1001, IDC_SEG_WORDS, IDC_SEG_PIN, IDC_PROFILE,
	IDC_SLIDER, IDC_LEN, IDC_CNT, IDC_CNT_DEC, IDC_CNT_INC,
	IDC_CHIP_LOWER, IDC_CHIP_UPPER, IDC_CHIP_DIGITS, IDC_CHIP_SYMBOLS, IDC_SYMSET,
	IDC_TG_AMBIG, IDC_TG_NOREPEAT, IDC_TG_START, IDC_EXCLUDE, IDC_CUSTOM,
	IDC_SEPARATOR, IDC_TG_CAP, IDC_TG_NUMBER, IDC_TG_PINREPEAT, IDC_TG_NOSEQ,
	IDC_GENERATE, IDC_TG_CLEAR, IDC_SAVE, IDC_COPYALL, IDC_LIST, IDC_STATUS, IDC_CLEARTIME
};

enum { TMR_CLIP = 1, TMR_MARK = 2, TMR_REGEN = 3 };

#define WM_APP_UPDATE    (WM_APP + 1)   /* lParam: the finished UpdJob */
#define WM_APP_PROGRESS  (WM_APP + 2)   /* wParam: percent or -1, lParam: bytes so far */
enum { ST_PLAIN, ST_OK, ST_WARN, ST_ERR };

/* owner-drawn button kinds */
enum { K_PRIMARY, K_SEG, K_CHIP, K_TOGGLE, K_DROP, K_ICON, K_STEP, K_THEMESEG, K_SWATCH };
enum { G_NONE, G_COPY, G_SAVE, G_MINUS, G_PLUS };

typedef struct {
	int kind, str, value, glyph, hot;
	int* bind;                          /* the option a chip or toggle shows and flips */
	int accRole, accState;              /* what screen readers were last told (access.c) */
} Ctl;

/* The item data of an owner-drawn menu item: its string id, plus flags. */
#define MENU_SEP      ((ULONG_PTR)-1)
#define MENU_BAR      ((ULONG_PTR)0x10000)    /* an item ON the bar */
#define MENU_ID(d)    ((int)((d) & 0xFFFF))

#define MAX_COUNT     50
#define AUTO_MAX      100000
#define CLIP_DEFAULT  120               /* seconds a copy stays on the clipboard */
#define CLIP_MIN      5
#define CLIP_MAX      3600

/* the times the drop-down next to "Clear clipboard after" offers, in the order of S_CLR_* */
#define CLEAR_CHOICE_N 6
extern const int CLEAR_CHOICES[CLEAR_CHOICE_N];

#define SC(v)  MulDiv((v), gDpi, 96)

/* Logical units, at 96 dpi. SC() turns each one into pixels. */
#define DESIGN_W   780     /* the default client width */
#define MIN_W      560     /* the narrowest client: below the design width, rows wrap */
#define MIN_H      340     /* the lowest client: below what the content needs, the part above the list scrolls */
#define BAND_FULL  60      /* the band with the title and the subtitle */
#define BAND_SLIM  40      /* the title only, when the window is low */
#define ICON_SZ    40
#define MARGIN     16
#define SEG_H      40
#define ROW_LH     36      /* one password row */
#define ICON_ZONE  44      /* the copy icon at the right of a row */
#define LIST_HEAD  48
#define STATUS_H   36
#define FIELD_H    32
#define ROW_STEP   42      /* from one row of the options card to the next */
#define MIN_ROWS   3       /* password rows always in view: in a low window the part above them scrolls */

/* where the main window paints what is not a control (filled by layout) */
typedef struct { int str; RECT r; int style; } Label;
enum { L_PLAIN, L_BOLD, L_HINT, L_TITLE, L_LIST };   /* L_LIST: the list card's title */
#define STEP_PX  SC(gCompact ? 36 : ROW_STEP)
#define GAP_PX   SC(gCompact ? 8 : 12)
#define PAD_PX   SC(gCompact ? 10 : 14)

#define T(id)  (STR[gLang][(id)])

/* profiles: an item's position is its value (IDM_PRESET_0 + p) */
enum { P_STANDARD, P_MAX, P_NOSYM, P_MINSYM, P_SHORT, P_WIFI, P_PHRASE, P_PIN, P_COUNT };

/* the slider (controls.c) tells its parent about every change with WM_COMMAND(id, SLN_CHANGE) */
#define SLM_SETRANGE (WM_USER + 1)
#define SLM_SETPOS   (WM_USER + 2)
#define SLM_GETPOS   (WM_USER + 3)
#define SLN_CHANGE   1

typedef struct UpdJob UpdJob;        /* updui.c */

static inline int clampInt(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
static inline int maxInt(int a, int b) { return a > b ? a : b; }
static inline int minInt(int a, int b) { return a < b ? a : b; }

/* theme.c: the palette */
COLORREF mix(COLORREF a, COLORREF b, double t);
void makePalette(void);

/* settings.c */
void iniPath(void);
int iniInt(const wchar_t* key, int def);
void iniSetInt(const wchar_t* key, int v);
void iniStr(const wchar_t* key, wchar_t* out, int cap);
void iniSetStr(const wchar_t* key, const wchar_t* v);
void saveSettings(void);
void loadSettings(void);

/* forja.c */
extern ULONG_PTR gGdipToken;
extern Palette PAL;
extern int gDark;
extern int gTheme;
extern int gAccent;
extern int gDpi;
extern HINSTANCE gInst;
extern HWND hMain, hTip, hList, hStatus;
extern HWND hSeg[3], hProfile, hSlider, hLen, hCnt, hCntDec, hCntInc;
extern HWND hChip[4], hSymSet, hTgAmbig, hTgNoRepeat, hTgStart, hExclude, hCustom;
extern HWND hSeparator, hTgCap, hTgNumber, hTgPinRepeat, hTgNoSeq;
extern HWND hGenerate, hTgClear, hClearTime, hSave, hCopyAll;
extern HWND hHelpWnd, hAboutWnd, hThemeWnd;
extern HICON gSwatchIcon[16];
extern HFONT fUi, fBold, fTitle, fSub, fSmall, fPw, fPwSmall;
extern HICON icBand, icAbout, icBig, icSmall;
extern HMENU hMenu;
extern int gLang;
extern PwOptions gOpt;
extern int gCount, gClearClip, gClearSecs;
extern int gClearFromArg;
extern int gAutoUpdate;
extern int gCharNoRepeat, gPinNoRepeat;
extern char gCharExclude[PW_SET_MAX], gPinExclude[PW_SET_MAX];
extern int gNoSave;
extern int gSyncing;
extern char gPw[MAX_COUNT][PW_OUT_MAX];
extern int gPwN, gPwMode;
extern int gHotRow, gHotIcon;
extern int gMarkRow;
extern int gTrackingList;
extern DWORD gClipSeq;
extern BYTE gClipHash[32];
extern int gClipHave;
extern int gClipLeft;
extern wchar_t gClipMsg[256];
extern int gStatusKind;
extern wchar_t gStatusText[512];
extern wchar_t gIni[MAX_PATH];
extern wchar_t gLastFolder[MAX_PATH];
extern Label gLabels[16];
extern int gLabelN;
extern RECT gSegTrack, gCardOpt, gCardList, gStrength, gStatusR;
extern int gBandTop, gBandH;
extern int gScrollY, gScrollMax;
extern int gClipBottom;
extern int gUpperBottom, gListHeadH;
extern int gMeasure;
extern int gCompact;
extern HWND gFieldEdit[4];
extern RECT gFieldBox[4];
extern int gFieldN;
extern const int PRESET_STR[P_COUNT];
void effectiveOptions(PwOptions* o);
int currentPreset(void);
void setStatus(int kind, const wchar_t* text);
void wipePasswords(void);
int passwordText(int row, wchar_t* out, int cap);
void sayGenerated(void);
void setMode(int mode);

/* theme.c */
void applyTheme(HWND hwnd);
void loadLayer(void);
HICON tintedIconFor(int accent, int size);
void setWindowIcons(HWND h);
void makeIcons(void);
void refreshTheme(void);
void makeFonts(void);
void setLanguage(int lang);
int windowDpi(HWND h);
void applyDpi(int dpi, const RECT* rect);

/* draw.c */
typedef struct { HDC dst, mem; HBITMAP bmp; HGDIOBJ old; int x, y, w, h; } BackBuf;
HDC bufBegin(BackBuf* b, HDC dst, int x, int y, int w, int h);
void bufEnd(BackBuf* b);
HBRUSH solidBrush(COLORREF c);
DWORD argb(COLORREF c, int alpha);
GpGraphics* gfx(HDC dc);
void roundRect(HDC dc, const RECT* r, int radius, COLORREF fill, COLORREF line, float width);
void circle(HDC dc, float cx, float cy, float rad, COLORREF fill, COLORREF line, float width);
void fillSolid(HDC dc, const RECT* r, COLORREF c);
void drawText(HDC dc, const wchar_t* s, int n, RECT* r, HFONT f, COLORREF c, UINT flags);
int textWidth(HFONT f, const wchar_t* s, int n);
int headLen(const wchar_t* s);
void headCopy(wchar_t* buf, int cap, const wchar_t* s);
void glyphCopy(HDC dc, int x, int y, int s, COLORREF ink, COLORREF fill);
void glyphCheck(HDC dc, int x, int y, int s, COLORREF ink, float w);
void glyphSave(HDC dc, int x, int y, int s, COLORREF ink);
void glyphChevron(HDC dc, int cx, int cy, int s, COLORREF ink);
void glyphPlusMinus(HDC dc, int cx, int cy, int s, COLORREF ink, int plus);

/* clipboard.c */
HGLOBAL globalCopy(const void* data, size_t bytes);
void putDwordFormat(const wchar_t* name, DWORD value);
int openClipboardRetry(void);
int putClipboard(const wchar_t* text);
void forgetClipboard(void);
void wipeOnExit(void);
void armClipWipe(void);
void clearSwitched(void);
void copyRow(int row);
void copyAll(void);
void copySelected(void);
void clipTick(void);

/* filesave.c */
void writePasswords(const wchar_t* path, int* ok);
void saveAs(HWND hwnd);

/* layout.c */
void allTips(UINT msg);
void drawRow(const DRAWITEMSTRUCT* d);
void buildControls(void);
void setNumberText(HWND h, int v);
void syncControls(void);
int layoutTop(int W, int y0, int bandH, int listY);
void layout(void);
void scrollTo(int y);
void keepFocusInView(void);
void paintMain(HDC dc);

/* access.c */
void accSetName(HWND h, const wchar_t* name);
void accSetValue(HWND h, const wchar_t* value);
void accSetRole(HWND h, int role);
int  accStateOf(HWND h, int checked);
void accSetState(HWND h, int state);
void accNotify(HWND h, DWORD event);
void accStatusInit(HWND h);
void accStatusSays(HWND h, const wchar_t* text);
void accNameMenu(HMENU m);

/* controls.c */
Ctl* ctlOf(HWND h);
void drawButton(const DRAWITEMSTRUCT* d);
HWND mkButton(HWND parent, int id, int kind, int str, int* bind, int value, int glyph);
HWND mkDefaultButton(HWND parent, int id, int str);
void registerClass(const wchar_t* name, WNDPROC proc, UINT style);
LRESULT CALLBACK sliderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
void measureMenuItem(MEASUREITEMSTRUCT* mi);
void drawMenuItem(const DRAWITEMSTRUCT* d);
void buildMenu(HWND hwnd);
UINT dropMenu(HWND button);
void accRefresh(HWND parent);

/* dialogs.c */
void showHelp(HWND owner);
void showAbout(HWND owner);
void setThemeChoice(int theme);
void setAccent(int accent);
void showTheme(HWND owner);

/* updui.c */
int newUpdateFolder(wchar_t* dir);
void removeUpdateLeftovers(void);
void checkUpdates(int interactive);
void askAutoUpdateOnce(int updated);
void onUpdateProgress(int pct, ULONGLONG done);
void onUpdateDone(UpdJob* j);

/* cli.c */
int writeAuto(const wchar_t* path, const PwOptions* o, int count);
int writeEntropy(const wchar_t* path, const PwOptions* o);
int saveShot(HWND w, const wchar_t* path);
void pump(DWORD ms);
const wchar_t* widePathArg(const wchar_t* key);
void parseOptions(PwOptions* o, int* noRepeat, char* exclude, int* count, int* preset);
int parseDuration(const char* s);

/* selftest.c */
int selfTest(const wchar_t* logPath);

#endif
