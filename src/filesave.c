/**
  filesave.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  Saving the list to a text file: written whole to a temporary file, then put in place.
**/

#include "forja.h"

/* ------------------------------------------------------------------ saving */

/* The list is written to a temporary file in the same folder and only then put in place of
 * `path`: a failed write (full disk, a pulled USB stick) never leaves a half-written file of
 * passwords, nor spoils a file that was there before. The temporary file goes if anything fails. */
void writePasswords(const wchar_t* path, int* ok)
{
	wchar_t dir[MAX_PATH], tmp[MAX_PATH];
	wchar_t* slash;
	HANDLE f;
	DWORD done;
	int i;

	*ok = 0;
	lstrcpynW(dir, path, MAX_PATH);
	slash = wcsrchr(dir, L'\\');
	if (slash) *slash = 0; else lstrcpyW(dir, L".");
	if (!GetTempFileNameW(dir, L"frj", 0, tmp)) return;

	f = CreateFileW(tmp, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (f == INVALID_HANDLE_VALUE) { DeleteFileW(tmp); return; }

	*ok = 1;
	for (i = 0; i < gPwN && *ok; i++)
	{
		DWORD len = (DWORD)strlen(gPw[i]);
		if (!WriteFile(f, gPw[i], len, &done, NULL) || done != len) *ok = 0;
		if (*ok && (!WriteFile(f, "\r\n", 2, &done, NULL) || done != 2)) *ok = 0;
	}
	if (*ok && !FlushFileBuffers(f)) *ok = 0;
	if (!CloseHandle(f)) *ok = 0;
	if (*ok && !MoveFileExW(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) *ok = 0;
	if (!*ok) DeleteFileW(tmp);
}

static void rememberFolderOf(const wchar_t* file)
{
	wchar_t* slash;

	lstrcpynW(gLastFolder, file, MAX_PATH);
	slash = wcsrchr(gLastFolder, L'\\');
	if (slash) *slash = 0;
	iniSetStr(L"LastFolder", gLastFolder);
}

void saveAs(HWND hwnd)
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
