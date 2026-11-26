/**
  updui.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 25, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The window side of the self-installing update: the daily check, the question, the download
  and its verification on a thread of its own, running the setup (update.c has the rest).
**/

#include "forja.h"

/* ------------------------------------------------------------------ updates */

/* Help -> Check for updates (and once a day by itself), as in the sister program Gryphmark:
 * when GitHub has a newer release, one question, and then everything happens alone. The
 * release's SHA256SUMS.txt must carry the author's signature (release.sig, update.c
 * upd_signature_valid; without it nothing is downloaded), the setup is downloaded and checked
 * against that list (a file that does not match is deleted, never run); the window closes (a copy waiting to be wiped goes, as on any exit);
 * the setup runs in silence keeping the install options, waits for this process to be gone
 * (/WHWAITPID) and starts the program again (/WHRELAUNCH, see installer/Forja.iss). A
 * portable copy cannot be installed over: it is offered the download page. The network work
 * runs on a thread of its own, which reports through WM_APP_UPDATE and WM_APP_PROGRESS. */

#define UPD_AGENT     L"Forja/" APP_VERSION
#define UPD_PAGE      L"https://" APP_REPO L"/releases/latest"

typedef struct UpdJob {
	int stage;                          /* 0 the check, 1 the download */
	int interactive;                    /* asked for from the menu: say so when nothing is new */
	DWORD err;                          /* Win32 / WinHTTP error, or HTTP status; 0 = none */
	int errStr;                         /* or a string saying what went wrong; 0 = none */
	UpdRelease rel;
	wchar_t file[MAX_PATH];             /* the verified setup (stage 1) */
	HANDLE lock;                        /* read handle to it, denying writes and deletes: from
	                                     * the hash until the setup has started, nobody can swap it */
} UpdJob;

static int gUpdBusy = 0;                /* a check or a download is running */
static UpdJob* gUpdCheck = NULL;        /* the check running, if any (only the window's thread uses it) */
static int gUpdDownloading = 0;
static wchar_t gUpdTag[64];             /* the version being downloaded, for the status line */

typedef struct { char* p; size_t n, cap; } MemSink;
typedef struct { HANDLE f; int lastPct; ULONGLONG lastMb; } FileSink;

static int sinkMem(void* ctx, const BYTE* d, DWORD n, ULONGLONG done, ULONGLONG total)
{
	MemSink* m = (MemSink*)ctx;

	(void)done; (void)total;
	if (m->n + n + 1 > m->cap) return 0;        /* larger than any answer we expect */
	memcpy(m->p + m->n, d, n);
	m->n += n;
	m->p[m->n] = 0;
	return 1;
}

static int sinkFile(void* ctx, const BYTE* d, DWORD n, ULONGLONG done, ULONGLONG total)
{
	FileSink* s = (FileSink*)ctx;
	DWORD w;

	if (!WriteFile(s->f, d, n, &w, NULL) || w != n) return 0;
	if (total)
	{
		int pct = (int)(done * 100 / total);
		if (pct != s->lastPct) { s->lastPct = pct; PostMessageW(hMain, WM_APP_PROGRESS, (WPARAM)pct, (LPARAM)done); }
	}
	else if ((done >> 20) != s->lastMb) { s->lastMb = done >> 20; PostMessageW(hMain, WM_APP_PROGRESS, (WPARAM)-1, (LPARAM)done); }
	return 1;
}

static void finishJob(UpdJob* j)
{
	if (!PostMessageW(hMain, WM_APP_UPDATE, 0, (LPARAM)j))              /* the window is gone */
	{
		if (j->stage && j->lock && j->lock != INVALID_HANDLE_VALUE) CloseHandle(j->lock);
		free(j);
	}
}

/* The thread of the check: the latest release, as the GitHub API describes it. */
static DWORD WINAPI checkThread(void* arg)
{
	UpdJob* j = (UpdJob*)arg;
	MemSink m;

	m.cap = 1 << 20;
	m.n = 0;
	m.p = (char*)malloc(m.cap);
	if (!m.p) j->err = ERROR_OUTOFMEMORY;
	else
	{
		m.p[0] = 0;
		j->err = upd_http_get(UPD_API_URL, UPD_AGENT, L"application/vnd.github+json", 10000, sinkMem, &m);
		if (!j->err && !upd_parse_release(m.p, &j->rel)) j->errStr = S_UPD_BADREPLY;
		free(m.p);
	}
	finishJob(j);
	return 0;
}

/* Downloads go to %TEMP%\ForjaUpdate-<16 random hex digits>, a new folder each time, so no
 * other program can prepare a file there in advance. Returns 0 if no folder could be made. */
int newUpdateFolder(wchar_t* dir)
{
	wchar_t base[MAX_PATH];
	BYTE r[8];
	int tries, i;

	if (!GetTempPathW(MAX_PATH, base)) return 0;
	for (tries = 0; tries < 8; tries++)
	{
		wchar_t* p;
		if (BCryptGenRandom(NULL, r, sizeof(r), BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0) return 0;
		_snwprintf(dir, MAX_PATH, L"%lsForjaUpdate-", base);
		dir[MAX_PATH - 1] = 0;
		p = dir + wcslen(dir);
		if (p + 17 > dir + MAX_PATH) return 0;
		for (i = 0; i < 8; i++) { _snwprintf(p, 3, L"%02x", r[i]); p += 2; }
		*p = 0;
		if (CreateDirectoryW(dir, NULL)) return 1;
		if (GetLastError() != ERROR_ALREADY_EXISTS) return 0;
	}
	return 0;
}

/* Removes the setups of earlier updates (run, failed or cut short when the program closed):
 * %TEMP%\ForjaUpdate (3.0.0 and earlier) and every %TEMP%\ForjaUpdate-*. A file still in use
 * (a setup still running, another window downloading) refuses and stays for the next start. */
void removeUpdateLeftovers(void)
{
	wchar_t base[MAX_PATH], pat[MAX_PATH], dir[MAX_PATH], f[MAX_PATH];
	WIN32_FIND_DATAW d, e;
	HANDLE h, k;

	if (!GetTempPathW(MAX_PATH, base)) return;
	_snwprintf(pat, MAX_PATH, L"%lsForjaUpdate*", base);
	pat[MAX_PATH - 1] = 0;
	h = FindFirstFileW(pat, &d);
	if (h == INVALID_HANDLE_VALUE) return;
	do
	{
		if (!(d.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (d.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) continue;
		if (wcscmp(d.cFileName, L"ForjaUpdate") && wcsncmp(d.cFileName, L"ForjaUpdate-", 12)) continue;
		_snwprintf(dir, MAX_PATH, L"%ls%ls", base, d.cFileName);
		dir[MAX_PATH - 1] = 0;
		_snwprintf(pat, MAX_PATH, L"%ls\\*", dir);
		pat[MAX_PATH - 1] = 0;
		k = FindFirstFileW(pat, &e);
		if (k != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (e.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
				_snwprintf(f, MAX_PATH, L"%ls\\%ls", dir, e.cFileName);
				f[MAX_PATH - 1] = 0;
				DeleteFileW(f);
			} while (FindNextFileW(k, &e));
			FindClose(k);
		}
		RemoveDirectoryW(dir);
	} while (FindNextFileW(h, &d));
	FindClose(h);
}

/* The thread of the download: SHA256SUMS.txt, then the setup, then its hash. The hash is taken
 * through a handle that denies writing and deleting to everyone else and stays open until the
 * setup has been started (onUpdateDone), so the file that runs is the file that was checked.
 * A setup that is incomplete or does not match is deleted here, so it can never be run. */
static DWORD WINAPI downloadThread(void* arg)
{
	UpdJob* j = (UpdJob*)arg;
	wchar_t dir[MAX_PATH], name[128], url[1024];
	char want[65], got[65];
	MemSink m;

	j->lock = INVALID_HANDLE_VALUE;
	if (!newUpdateFolder(dir)) { j->err = GetLastError() ? GetLastError() : ERROR_PATH_NOT_FOUND; finishJob(j); return 0; }
	MultiByteToWideChar(CP_UTF8, 0, j->rel.setupName, -1, name, 128);
	_snwprintf(j->file, MAX_PATH, L"%ls\\%ls", dir, name);
	j->file[MAX_PATH - 1] = 0;

	m.cap = 1 << 16;
	m.n = 0;
	m.p = (char*)malloc(m.cap);
	if (!m.p) { j->err = ERROR_OUTOFMEMORY; finishJob(j); return 0; }
	m.p[0] = 0;
	MultiByteToWideChar(CP_UTF8, 0, j->rel.sumsUrl, -1, url, 1024);
	j->err = upd_http_get(url, UPD_AGENT, NULL, 30000, sinkMem, &m);
	/* the author's signature of the list first (release.sig): without a valid one nothing is
	 * downloaded or run, so a release someone else put on the page never installs itself */
	if (!j->err)
	{
		MemSink sg;
		sg.cap = 1024;
		sg.n = 0;
		sg.p = (char*)malloc(sg.cap);
		if (!sg.p) j->err = ERROR_OUTOFMEMORY;
		else
		{
			sg.p[0] = 0;
			MultiByteToWideChar(CP_UTF8, 0, j->rel.sigUrl, -1, url, 1024);
			j->err = upd_http_get(url, UPD_AGENT, NULL, 30000, sinkMem, &sg);
			if (!j->err && !upd_signature_valid(m.p, m.n, sg.p)) j->errStr = S_UPD_BADSIG;
			free(sg.p);
		}
	}
	if (!j->err && !j->errStr && !upd_hash_for(m.p, j->rel.setupName, want)) j->errStr = S_UPD_NOHASH;
	free(m.p);

	if (!j->err && !j->errStr)
	{
		FileSink s;
		s.lastPct = -1;
		s.lastMb = 0;
		s.f = CreateFileW(j->file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (s.f == INVALID_HANDLE_VALUE) j->err = GetLastError();
		else
		{
			MultiByteToWideChar(CP_UTF8, 0, j->rel.setupUrl, -1, url, 1024);
			j->err = upd_http_get(url, UPD_AGENT, NULL, 60000, sinkFile, &s);
			if (!FlushFileBuffers(s.f) && !j->err) j->err = GetLastError();
			CloseHandle(s.f);
			if (!j->err)
			{
				j->lock = CreateFileW(j->file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (j->lock == INVALID_HANDLE_VALUE) j->err = GetLastError();
				else if (!upd_sha256_handle(j->lock, got) || strcmp(got, want)) j->errStr = S_UPD_BADHASH;
			}
			if (j->err || j->errStr)
			{
				if (j->lock != INVALID_HANDLE_VALUE) { CloseHandle(j->lock); j->lock = INVALID_HANDLE_VALUE; }
				DeleteFileW(j->file);
				RemoveDirectoryW(dir);
			}
		}
	}
	else RemoveDirectoryW(dir);
	finishJob(j);
	return 0;
}

static void startUpdateJob(int stage, int interactive, const UpdRelease* rel)
{
	UpdJob* j = (UpdJob*)calloc(1, sizeof(UpdJob));
	HANDLE t;

	if (!j) return;
	j->stage = stage;
	j->interactive = interactive;
	if (rel) j->rel = *rel;
	gUpdBusy = 1;
	if (stage == 0) gUpdCheck = j;
	t = CreateThread(NULL, 0, stage ? downloadThread : checkThread, j, 0, NULL);
	if (t) CloseHandle(t);
	else { free(j); gUpdBusy = 0; gUpdDownloading = 0; gUpdCheck = NULL; }
}

/* One daily check for the automatic ones, stamped in the .ini. */
static int dailyCheckDue(void)
{
	wchar_t buf[32];
	long long last, now = (long long)_time64(NULL);

	iniStr(L"UpdateCheck", buf, 32);
	last = _wtoi64(buf);
	return now - last >= 24 * 3600 || now < last;
}

void checkUpdates(int interactive)
{
	wchar_t buf[32];

	if (gUpdBusy)
	{
		/* the daily check is under way: asked for from the menu, it now answers as such */
		if (interactive && gUpdCheck) { gUpdCheck->interactive = 1; setStatus(ST_PLAIN, T(S_UPD_CHECKING)); }
		return;
	}
	if (!interactive && (!gAutoUpdate || !dailyCheckDue())) return;
	_snwprintf(buf, 32, L"%lu", (unsigned long)_time64(NULL));    /* even if it fails (no network): */
	buf[31] = 0;                                              /* at most one try a day */
	iniSetStr(L"UpdateCheck", buf);
	if (interactive) setStatus(ST_PLAIN, T(S_UPD_CHECKING));
	startUpdateJob(0, interactive, NULL);
}

static void updateMessage(int fmt, const UpdJob* j)
{
	wchar_t why[256], msg[640];

	if (j->errStr) lstrcpynW(why, T(j->errStr), 256);
	else { _snwprintf(why, 256, T(S_UPD_NET), (unsigned long)j->err); why[255] = 0; }
	_snwprintf(msg, 640, T(fmt), why);
	msg[639] = 0;
	MessageBoxW(hMain, msg, L"Forja", MB_OK | MB_ICONWARNING);
}

/* The one question: 2 = Update now, 1 = What's new, 0 = Later (or the dialog closed). */
static int askUpdate(const wchar_t* tag)
{
	TASKDIALOGCONFIG tc;
	TASKDIALOG_BUTTON b[3];
	wchar_t head[256], body[1024];
	int r = 0;

	_snwprintf(head, 256, T(S_UPD_ASK_HEAD), tag);
	head[255] = 0;
	_snwprintf(body, 1024, T(S_UPD_ASK), APP_VERSION);
	body[1023] = 0;
	b[0].nButtonID = 102; b[0].pszButtonText = T(S_UPD_NOW);
	b[1].nButtonID = 101; b[1].pszButtonText = T(S_UPD_NOTES);
	b[2].nButtonID = 100; b[2].pszButtonText = T(S_UPD_LATER);

	ZeroMemory(&tc, sizeof(tc));
	tc.cbSize = sizeof(tc);
	tc.hwndParent = hMain;
	tc.hInstance = gInst;
	tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tc.pszWindowTitle = L"Forja";
	tc.pszMainIcon = TD_INFORMATION_ICON;
	tc.pszMainInstruction = head;
	tc.pszContent = body;
	tc.cButtons = 3;
	tc.pButtons = b;
	tc.nDefaultButton = 102;
	if (FAILED(TaskDialogIndirect(&tc, &r, NULL, NULL))) return 0;
	return r == 102 ? 2 : r == 101 ? 1 : 0;
}

/* The first start, with no CheckUpdates in the .ini yet (a new install, or settings from before
 * 3.1.0): Forja asks once whether it may ask GitHub for a new version every day, instead of
 * doing it unasked. The answer is saved and can be changed in the Help menu; a dialog closed
 * without an answer saves nothing, checks nothing this time, and the question comes back on
 * the next start. A start by the setup after an update means the check was on: it stays on. */
void askAutoUpdateOnce(int updated)
{
	TASKDIALOGCONFIG tc;
	TASKDIALOG_BUTTON b[2];
	int r = 0;

	if (gNoSave || iniInt(L"CheckUpdates", -1) != -1) return;
	if (updated) { gAutoUpdate = 1; iniSetInt(L"CheckUpdates", 1); return; }

	b[0].nButtonID = 101; b[0].pszButtonText = T(S_FIRST_UPD_YES);
	b[1].nButtonID = 100; b[1].pszButtonText = T(S_FIRST_UPD_NO);
	ZeroMemory(&tc, sizeof(tc));
	tc.cbSize = sizeof(tc);
	tc.hwndParent = hMain;
	tc.hInstance = gInst;
	tc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_POSITION_RELATIVE_TO_WINDOW;
	tc.pszWindowTitle = L"Forja";
	tc.pszMainIcon = TD_INFORMATION_ICON;
	tc.pszMainInstruction = T(S_FIRST_UPD_HEAD);
	tc.pszContent = T(S_FIRST_UPD_BODY);
	tc.cButtons = 2;
	tc.pButtons = b;
	tc.nDefaultButton = 101;
	if (FAILED(TaskDialogIndirect(&tc, &r, NULL, NULL))) r = 0;
	gAutoUpdate = (r == 101);
	if (r == 101 || r == 100) iniSetInt(L"CheckUpdates", gAutoUpdate);
	buildMenu(hMain);
}

static void offerPage(int fmt, const wchar_t* tag)
{
	wchar_t msg[640];

	_snwprintf(msg, 640, T(fmt), tag, APP_VERSION);
	msg[639] = 0;
	if (MessageBoxW(hMain, msg, L"Forja", MB_YESNO | MB_ICONINFORMATION) == IDYES)
		ShellExecuteW(hMain, L"open", UPD_PAGE, NULL, NULL, SW_SHOWNORMAL);
}

void onUpdateProgress(int pct, ULONGLONG done)
{
	wchar_t amount[32], buf[256];

	if (!gUpdDownloading) return;
	if (pct >= 0) _snwprintf(amount, 32, L"%d %%", pct);
	else          _snwprintf(amount, 32, L"%u MB", (unsigned)(done >> 20));
	amount[31] = 0;
	_snwprintf(buf, 256, T(S_UPD_DOWNLOADING), gUpdTag, amount);
	buf[255] = 0;
	setStatus(ST_PLAIN, buf);
}

void onUpdateDone(UpdJob* j)
{
	gUpdBusy = 0;
	gUpdCheck = NULL;

	if (j->stage == 0)
	{
		wchar_t tag[64];
		const wchar_t* bare = tag;

		if (j->interactive) setStatus(ST_PLAIN, T(S_ST_READY));
		if (j->err || j->errStr)
		{
			if (j->interactive) updateMessage(S_UPD_ERROR, j);
			free(j);
			return;
		}
		MultiByteToWideChar(CP_UTF8, 0, j->rel.tag, -1, tag, 64);
		tag[63] = 0;
		while (*bare == L'v' || *bare == L'V') bare++;
		if (!upd_is_newer(tag, APP_VERSION))
		{
			if (j->interactive)
			{
				wchar_t msg[256];
				_snwprintf(msg, 256, T(S_UPD_UPTODATE), APP_VERSION);
				msg[255] = 0;
				MessageBoxW(hMain, msg, L"Forja", MB_OK | MB_ICONINFORMATION);
			}
		}
		else if (!upd_installed_copy()) offerPage(S_UPD_PORTABLE, bare);
		else if (!j->rel.setupUrl[0] || !j->rel.sumsUrl[0] || !j->rel.sigUrl[0]) offerPage(S_UPD_AVAILABLE, bare);   /* no setup, hashes or signature */
		else
		{
			int r = askUpdate(bare);
			if (r == 1) ShellExecuteW(hMain, L"open", UPD_PAGE, NULL, NULL, SW_SHOWNORMAL);
			else if (r == 2)
			{
				lstrcpynW(gUpdTag, bare, 64);
				gUpdDownloading = 1;
				onUpdateProgress(0, 0);
				startUpdateJob(1, 1, &j->rel);
			}
		}
		free(j);
		return;
	}

	/* the download is over: run the verified setup, then close so that it can replace us */
	gUpdDownloading = 0;
	if (!j->err && !j->errStr)
	{
		SHELLEXECUTEINFOW sei;
		wchar_t args[160];

		_snwprintf(args, 160, L"/VERYSILENT /SUPPRESSMSGBOXES /NORESTART /WHWAITPID=%lu /WHRELAUNCH=1",
		           (unsigned long)GetCurrentProcessId());
		args[159] = 0;
		ZeroMemory(&sei, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_NOASYNC;
		sei.hwnd = hMain;
		sei.lpVerb = L"open";
		sei.lpFile = j->file;
		sei.lpParameters = args;
		sei.nShow = SW_SHOWNORMAL;
		if (ShellExecuteExW(&sei))
		{
			/* the setup has started (SEE_MASK_NOASYNC): the file can be let go */
			if (j->lock && j->lock != INVALID_HANDLE_VALUE) CloseHandle(j->lock);
			free(j);
			DestroyWindow(hMain);
			return;
		}
		j->err = GetLastError();                /* refused, e.g. the elevation prompt cancelled */
	}
	if (j->lock && j->lock != INVALID_HANDLE_VALUE) { CloseHandle(j->lock); j->lock = INVALID_HANDLE_VALUE; }
	setStatus(ST_PLAIN, T(S_ST_READY));
	updateMessage(S_UPD_FAILED, j);
	free(j);
}
