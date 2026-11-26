/**
  clipboard.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  Copying: kept out of the clipboard history and the cloud, wiped when the time is up or on
  exit, and only while the clipboard still holds exactly what was copied.
**/

#include "forja.h"

/* ------------------------------------------------------------------ clipboard */

/* A block of movable memory holding a copy of `bytes` bytes, ready for SetClipboardData,
 * or NULL. */
HGLOBAL globalCopy(const void* data, size_t bytes)
{
	HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, bytes);
	void* p = h ? GlobalLock(h) : NULL;

	if (!p) { if (h) GlobalFree(h); return NULL; }
	memcpy(p, data, bytes);
	GlobalUnlock(h);
	return h;
}

/* A clipboard format whose presence tells Windows what to do with the copy (the clipboard
 * must be open). */
void putDwordFormat(const wchar_t* name, DWORD value)
{
	UINT fmt = RegisterClipboardFormatW(name);
	HGLOBAL h;

	if (!fmt || (h = globalCopy(&value, sizeof(value))) == NULL) return;
	if (!SetClipboardData(fmt, h)) GlobalFree(h);
}

/* Opens the clipboard for the main window; 1 = open (the caller closes it). */
int openClipboardRetry(void)
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

/* Puts text on the clipboard, out of its history, and remembers a hash of it (never the text)
 * to recognise it later. 1 = done. */
int putClipboard(const wchar_t* text)
{
	HGLOBAL h;

	if (!openClipboardRetry()) return 0;

	EmptyClipboard();

	h = globalCopy(text, (wcslen(text) + 1) * sizeof(wchar_t));
	if (!h) { CloseClipboard(); return 0; }
	if (!SetClipboardData(CF_UNICODETEXT, h)) { GlobalFree(h); CloseClipboard(); return 0; }

	/* keep it out of Win+V history, the cloud clipboard and clipboard monitors */
	putDwordFormat(L"ExcludeClipboardContentFromMonitorProcessing", 0);
	putDwordFormat(L"CanIncludeInClipboardHistory", 0);
	putDwordFormat(L"CanUploadToCloudClipboard", 0);

	CloseClipboard();
	gClipSeq = GetClipboardSequenceNumber();
	gClipHave = upd_sha256(text, wcslen(text) * sizeof(wchar_t), gClipHash);
	return 1;
}

/* Nothing on the clipboard is ours any more. */
void forgetClipboard(void)
{
	gClipHave = 0;
	SecureZeroMemory(gClipHash, sizeof(gClipHash));
}

/* With the clipboard open: does its text hash to what we copied? */
static int openClipboardIsOurs(void)
{
	HANDLE h;
	const wchar_t* p;
	SIZE_T cap;
	size_t n = 0;
	BYTE d[32];
	int same = 0;

	if (!gClipHave) return 0;
	if (GetClipboardSequenceNumber() == gClipSeq) return 1;   /* nothing has touched it since */
	h = GetClipboardData(CF_UNICODETEXT);
	if (!h || (p = (const wchar_t*)GlobalLock(h)) == NULL) return 0;
	cap = GlobalSize(h) / sizeof(wchar_t);
	while (n < cap && p[n]) n++;
	if (n < cap && upd_sha256(p, n * sizeof(wchar_t), d)) same = !memcmp(d, gClipHash, 32);
	GlobalUnlock(h);
	SecureZeroMemory(d, sizeof(d));
	return same;
}

/* Something has changed the clipboard since our copy: is it still exactly our text (put
 * back by a clipboard tool, say)? If so it stays ours, under its new sequence number. */
static int clipboardStillOurs(void)
{
	int ours;

	if (!gClipHave) return 0;
	if (GetClipboardSequenceNumber() == gClipSeq) return 1;
	if (!IsClipboardFormatAvailable(CF_UNICODETEXT) || !openClipboardRetry()) return 0;
	ours = openClipboardIsOurs();
	if (ours) gClipSeq = GetClipboardSequenceNumber();
	CloseClipboard();
	return ours;
}

/* Wipes the clipboard only if it still holds exactly what we put there. The check is made
 * with the clipboard open, so nobody can copy something in between. */
static int wipeOurClipboard(void)
{
	int ours;

	if (!gClipHave || !openClipboardRetry()) return 0;
	ours = openClipboardIsOurs();
	if (ours) EmptyClipboard();
	CloseClipboard();
	if (ours) forgetClipboard();
	return ours;
}

/* Closing (or the session ending) before the time is up: a copy waiting to be wiped goes now,
 * if it is still ours. With clearing switched off nothing is waiting, and copies stay. */
void wipeOnExit(void)
{
	if (gClipLeft > 0) wipeOurClipboard();
	gClipLeft = 0;
	if (hMain) KillTimer(hMain, TMR_CLIP);
}

/* Starts the countdown for the copy just made, when clearing is on. */
void armClipWipe(void)
{
	KillTimer(hMain, TMR_CLIP);
	gClipLeft = 0;
	if (gClearClip)
	{
		gClipLeft = clampInt(gClearSecs, 1, CLIP_MAX);
		SetTimer(hMain, TMR_CLIP, 1000, NULL);
	}
}

static void showCopied(void);

/* The "clear the clipboard" switch was flipped. It acts on the copy already made too:
 * switched off, a countdown in progress stops and the copy stays; switched on, a copy that is
 * still ours (and still on the clipboard) starts its countdown now. */
void clearSwitched(void)
{
	if (!gClearClip)
	{
		if (gClipLeft <= 0) return;
		KillTimer(hMain, TMR_CLIP);
		gClipLeft = 0;
		showCopied();
		return;
	}
	if (gClipLeft > 0 || !clipboardStillOurs()) return;
	armClipWipe();
	showCopied();
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

/* After a copy: the check mark on the row (or on every row, -2), the countdown, the status. */
static void afterCopy(int row)
{
	gMarkRow = row;
	SetTimer(hMain, TMR_MARK, 1500, NULL);
	InvalidateRect(hList, NULL, FALSE);
	InvalidateRect(hCopyAll, NULL, FALSE);

	armClipWipe();
	showCopied();
}

void copyRow(int row)
{
	wchar_t w[PW_OUT_MAX];

	if (passwordText(row, w, PW_OUT_MAX) < 0) return;
	if (!putClipboard(w)) { SecureZeroMemory(w, sizeof(w)); setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }
	SecureZeroMemory(w, sizeof(w));

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED), row + 1);
	gClipMsg[255] = 0;
	afterCopy(row);
}

void copyAll(void)
{
	static wchar_t w[MAX_COUNT * (PW_OUT_MAX + 2) + 1];
	int i, n = 0;

	if (gPwN <= 0) return;

	/* one per line, with no line break after the last one: pasted into a single field, a
	 * one-row list is just the password */
	for (i = 0; i < gPwN; i++)
	{
		int k;
		if (i) { w[n++] = L'\r'; w[n++] = L'\n'; }
		k = passwordText(i, w + n, PW_OUT_MAX);
		if (k > 0) n += k;
	}
	w[n] = 0;

	i = putClipboard(w);
	SecureZeroMemory(w, sizeof(w));
	if (!i) { setStatus(ST_ERR, T(S_ST_CLIP_FAIL)); return; }

	_snwprintf(gClipMsg, 256, T(S_ST_COPIED_ALL), gPwN);
	gClipMsg[255] = 0;
	afterCopy(-2);
}

/* Ctrl+C: the text of a focused text field, otherwise the selected password. */
void copySelected(void)
{
	HWND f = GetFocus();
	wchar_t cls[16];

	/* Ctrl+C in a text field copies its text, as everywhere else */
	if (f && GetClassNameW(f, cls, 16) && !lstrcmpiW(cls, L"Edit")) { SendMessageW(f, WM_COPY, 0, 0); return; }

	copyRow((int)SendMessageW(hList, LB_GETCURSEL, 0, 0));
}

/* Once a second while a copy waits to be wiped. */
void clipTick(void)
{
	if (!clipboardStillOurs())
	{
		/* something else was copied: it is not ours to wipe */
		KillTimer(hMain, TMR_CLIP);
		gClipLeft = 0;
		forgetClipboard();
		showCopied();
		return;
	}

	if (--gClipLeft <= 0)
	{
		KillTimer(hMain, TMR_CLIP);
		gClipLeft = 0;
		if (wipeOurClipboard()) setStatus(ST_PLAIN, T(S_ST_CLEARED));
		else { forgetClipboard(); showCopied(); }
		return;
	}
	showCopied();
}
