/**
  update.c
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 20, 2026
  SPDX-License-Identifier: GPL-2.0-only

  The parts of the self-installing update that do not touch the window (see update.h).
  upd_is_newer and upd_hash_for follow IsNewer and HashFor of the sister program Gryphmark
  line by line, so both programs decide the same way.
**/

#include "update.h"

#include <winhttp.h>
#include <bcrypt.h>
#include <string.h>
#include <wchar.h>

#ifndef WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY
#define WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY 4
#endif

/* the AppId of installer/Forja.iss, as the setup writes it under Uninstall */
#define UPD_UNINSTALL_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{AA7E8B36-E890-4F08-8A5E-C161BE196A52}_is1"

/* ------------------------------------------------------------------ versions */

/* "2.1.0" -> {2,1,0,-1}, like System.Version.TryParse: 2 to 4 parts, digits only. */
static int parseVersion(const wchar_t* s, long v[4])
{
	int n = 0;

	v[0] = v[1] = v[2] = v[3] = -1;
	for (;;)
	{
		long x = 0;
		int digits = 0;
		while (*s >= L'0' && *s <= L'9')
		{
			if (x > (0x7FFFFFFF - 9) / 10) return 0;     /* checked before it can overflow */
			x = x * 10 + (*s++ - L'0');
			digits++;
		}
		if (!digits || n == 4) return 0;
		v[n++] = x;
		if (*s == 0) break;
		if (*s++ != L'.') return 0;
	}
	return n >= 2;
}

int upd_is_newer(const wchar_t* tag, const wchar_t* mine)
{
	long a[4], b[4];
	int i;

	if (!tag || !mine) return 0;
	while (*tag == L'v' || *tag == L'V') tag++;
	if (!parseVersion(tag, a) || !parseVersion(mine, b)) return 0;
	for (i = 0; i < 4; i++)
		if (a[i] != b[i]) return a[i] > b[i];
	return 0;
}

/* ------------------------------------------------------------------ SHA256SUMS.txt */

static int isSpace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f'; }

int upd_hash_for(const char* sums, const char* fileName, char out[65])
{
	const char* p = sums;

	if (!sums || !fileName) return 0;
	while (*p)
	{
		const char *a = p, *b, *e = strchr(p, '\n'), *sp, *name;
		size_t nameLen;
		int i;

		if (!e) e = p + strlen(p);
		p = *e ? e + 1 : e;

		/* the line, trimmed */
		b = e;
		while (a < b && isSpace(*a)) a++;
		while (b > a && isSpace(b[-1])) b--;

		/* the first space or tab must be right after the 64 digits of the hash */
		for (sp = a; sp < b && *sp != ' ' && *sp != '\t'; sp++) {}
		if (sp >= b || sp - a != 64) continue;

		name = sp;
		while (name < b && isSpace(*name)) name++;
		while (name < b && *name == '*') name++;
		nameLen = (size_t)(b - name);
		if (nameLen != strlen(fileName) || _strnicmp(name, fileName, nameLen)) continue;

		for (i = 0; i < 64; i++) out[i] = (a[i] >= 'A' && a[i] <= 'Z') ? (char)(a[i] + 32) : a[i];
		out[64] = 0;
		return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------ the release JSON */

/* A JSON string starting at the quote p points at, into out. Returns what follows it, or NULL. */
static const char* jsonString(const char* p, char* out, size_t cap)
{
	size_t n = 0;

	if (*p++ != '"') return NULL;
	while (*p && *p != '"')
	{
		char c = *p++;
		if (c == '\\')
		{
			c = *p++;
			if (!c) return NULL;
			if (c == 'n') c = '\n';
			else if (c == 't') c = '\t';
			else if (c == 'u') { int k; for (k = 0; k < 4 && *p; k++) p++; c = '?'; }
			else if (c != '"' && c != '\\' && c != '/') c = '?';
		}
		if (n + 1 < cap) out[n++] = c;
		else return NULL;               /* longer than anything we expect: refuse it */
	}
	if (*p != '"') return NULL;
	out[n] = 0;
	return p + 1;
}

/* The value of the next "key": "value" from p on, or NULL. */
static const char* jsonValue(const char* p, const char* key, char* out, size_t cap)
{
	char pat[64];
	const char* q;

	_snprintf(pat, sizeof(pat), "\"%s\"", key);
	pat[sizeof(pat) - 1] = 0;
	while ((q = strstr(p, pat)) != NULL)
	{
		p = q + strlen(pat);
		while (isSpace(*p)) p++;
		if (*p != ':') continue;
		p++;
		while (isSpace(*p)) p++;
		if (*p != '"') continue;
		return jsonString(p, out, cap);
	}
	return NULL;
}

/* A file name the setup can be saved under: letters, digits, dot, hyphen, underscore. */
static int safeName(const char* s)
{
	if (!*s || *s == '.') return 0;
	for (; *s; s++)
		if (!((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || (*s >= '0' && *s <= '9') ||
		      *s == '.' || *s == '-' || *s == '_')) return 0;
	return 1;
}

int upd_parse_release(const char* json, UpdRelease* r)
{
	const char* p;
	char url[1024];

	memset(r, 0, sizeof(*r));
	if (!json || !jsonValue(json, "tag_name", r->tag, sizeof(r->tag))) return 0;

	/* every asset: its download address ends with its file name */
	p = json;
	while ((p = jsonValue(p, "browser_download_url", url, sizeof(url))) != NULL)
	{
		const char* name = strrchr(url, '/');
		size_t n;

		if (strncmp(url, "https://", 8) || !name) continue;
		name++;
		n = strlen(name);
		if (!_strnicmp(name, UPD_SETUP_PREFIX, strlen(UPD_SETUP_PREFIX)) && n > 4 &&
		    !_stricmp(name + n - 4, ".exe") && safeName(name) && n < sizeof(r->setupName))
		{
			strcpy(r->setupUrl, url);
			strcpy(r->setupName, name);
		}
		else if (!_stricmp(name, UPD_SUMS_NAME)) strcpy(r->sumsUrl, url);
		else if (!_stricmp(name, UPD_SIG_NAME)) strcpy(r->sigUrl, url);
	}
	return r->tag[0] != 0;
}

/* ------------------------------------------------------------------ HTTPS */

DWORD upd_http_get(const wchar_t* url, const wchar_t* agent, const wchar_t* accept, DWORD timeoutMs,
                   UpdSink sink, void* ctx)
{
	URL_COMPONENTS u;
	wchar_t host[256], path[2048], extra[1024], full[3072], hdr[256];
	HINTERNET s = NULL, c = NULL, rq = NULL;
	DWORD err = 0, status = 0, len, got, clen = 0;
	ULONGLONG done = 0, total = 0;
	static BYTE buf[65536];

	ZeroMemory(&u, sizeof(u));
	u.dwStructSize = sizeof(u);
	u.lpszHostName = host;  u.dwHostNameLength = 256;
	u.lpszUrlPath = path;   u.dwUrlPathLength = 2048;
	u.lpszExtraInfo = extra; u.dwExtraInfoLength = 1024;
	if (!WinHttpCrackUrl(url, 0, 0, &u)) return GetLastError();
	if (u.nScheme != INTERNET_SCHEME_HTTPS) return ERROR_INVALID_PARAMETER;
	_snwprintf(full, 3072, L"%ls%ls", path, extra);
	full[3071] = 0;

	s = WinHttpOpen(agent, WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!s) s = WinHttpOpen(agent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (!s) return GetLastError();
	WinHttpSetTimeouts(s, (int)timeoutMs, (int)timeoutMs, (int)timeoutMs, (int)timeoutMs);

	c = WinHttpConnect(s, host, u.nPort, 0);
	if (c) rq = WinHttpOpenRequest(c, L"GET", full, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (!rq) { err = GetLastError(); goto done; }

	hdr[0] = 0;
	if (accept) { _snwprintf(hdr, 256, L"Accept: %ls\r\n", accept); hdr[255] = 0; }
	if (!WinHttpSendRequest(rq, hdr[0] ? hdr : WINHTTP_NO_ADDITIONAL_HEADERS, hdr[0] ? (DWORD)-1L : 0,
	                        WINHTTP_NO_REQUEST_DATA, 0, 0, 0) ||
	    !WinHttpReceiveResponse(rq, NULL)) { err = GetLastError(); goto done; }

	len = sizeof(status);
	if (!WinHttpQueryHeaders(rq, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
	                         &status, &len, WINHTTP_NO_HEADER_INDEX)) { err = GetLastError(); goto done; }
	if (status != 200) { err = status; goto done; }
	len = sizeof(clen);
	if (WinHttpQueryHeaders(rq, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX,
	                        &clen, &len, WINHTTP_NO_HEADER_INDEX)) total = clen;

	for (;;)
	{
		if (!WinHttpReadData(rq, buf, sizeof(buf), &got)) { err = GetLastError(); break; }
		if (got == 0) break;
		done += got;
		if (!sink(ctx, buf, got, done, total)) { err = ERROR_CANCELLED; break; }
	}

done:
	if (rq) WinHttpCloseHandle(rq);
	if (c) WinHttpCloseHandle(c);
	WinHttpCloseHandle(s);
	return err;
}

/* ------------------------------------------------------------------ SHA-256 */

typedef struct { BCRYPT_ALG_HANDLE alg; BCRYPT_HASH_HANDLE h; } Sha;

static int shaOpen(Sha* s)
{
	s->alg = NULL; s->h = NULL;
	if (BCryptOpenAlgorithmProvider(&s->alg, BCRYPT_SHA256_ALGORITHM, NULL, 0) != 0) return 0;
	if (BCryptCreateHash(s->alg, &s->h, NULL, 0, NULL, 0, 0) != 0) { BCryptCloseAlgorithmProvider(s->alg, 0); return 0; }
	return 1;
}

static int shaClose(Sha* s, BYTE out[32])
{
	int ok = BCryptFinishHash(s->h, out, 32, 0) == 0;
	BCryptDestroyHash(s->h);
	BCryptCloseAlgorithmProvider(s->alg, 0);
	return ok;
}

int upd_sha256(const void* data, size_t n, BYTE out[32])
{
	Sha s;
	int ok;

	if (!shaOpen(&s)) return 0;
	ok = BCryptHashData(s.h, (PUCHAR)data, (ULONG)n, 0) == 0;
	return shaClose(&s, out) && ok;
}

int upd_sha256_handle(HANDLE f, char out[65])
{
	static BYTE buf[65536];
	BYTE h[32];
	DWORD got;
	LARGE_INTEGER zero;
	Sha s;
	int ok = 1, i;

	zero.QuadPart = 0;
	if (!SetFilePointerEx(f, zero, NULL, FILE_BEGIN)) return 0;
	if (!shaOpen(&s)) return 0;
	for (;;)
	{
		if (!ReadFile(f, buf, sizeof(buf), &got, NULL)) { ok = 0; break; }
		if (got == 0) break;
		if (BCryptHashData(s.h, buf, got, 0) != 0) { ok = 0; break; }
	}
	if (!shaClose(&s, h) || !ok) return 0;
	for (i = 0; i < 32; i++) sprintf(out + 2 * i, "%02x", h[i]);
	out[64] = 0;
	return 1;
}

int upd_sha256_file(const wchar_t* path, char out[65])
{
	HANDLE f = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	int ok;

	if (f == INVALID_HANDLE_VALUE) return 0;
	ok = upd_sha256_handle(f, out);
	CloseHandle(f);
	return ok;
}

/* ------------------------------------------------------------------ installed or portable */

int upd_installed_copy(void)
{
	static const HKEY roots[2] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
	wchar_t exe[MAX_PATH], loc[MAX_PATH + 2], *slash;
	DWORD n = GetModuleFileNameW(NULL, exe, MAX_PATH);
	int i;

	if (n == 0 || n >= MAX_PATH) return 0;
	slash = wcsrchr(exe, L'\\');
	if (!slash) return 0;
	slash[1] = 0;                       /* the folder, with its backslash */

	for (i = 0; i < 2; i++)
	{
		DWORD cb = sizeof(wchar_t) * MAX_PATH;
		size_t k;
		if (RegGetValueW(roots[i], UPD_UNINSTALL_KEY, L"InstallLocation", RRF_RT_REG_SZ, NULL, loc, &cb) != ERROR_SUCCESS)
			continue;
		loc[MAX_PATH - 1] = 0;
		k = wcslen(loc);
		if (k && loc[k - 1] != L'\\') { loc[k] = L'\\'; loc[k + 1] = 0; }
		if (!lstrcmpiW(loc, exe)) return 1;
	}
	return 0;
}

/* ------------------------------------------------------------------ the author's signature */

/* X||Y of each public key (P-256), in base64: the author's PC, the author's Mac. */
static const char* const RELEASE_KEYS[] = {
	"A6RpVoY34Wts4tIWkpmbD8BsdZrjb8GXnlS79HO90GfcAPr+dCWy96twq4qxdU8uWPDcJmgyp1UJIc2hLUbEiw==",
	"NE/n9pyImBDUa2NzbD1Yj1H8sVFtEyS4ohI5rWthYucqnl/jJLAin9GpIB6+HC0LCNDGPLx4CF8Crp77+6123g==",
};

/* Base64 into out (at most cap bytes); spaces and line breaks around it are ignored. The number of
 * bytes, or -1 for anything that is not base64 or does not fit. */
static int unbase64(const char* s, BYTE* out, int cap)
{
	int n = 0, bits = 0, pad = 0;
	unsigned acc = 0;

	while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') s++;
	for (; *s; s++)
	{
		int v;
		char ch = *s;
		if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') break;
		if (ch == '=') { pad++; continue; }
		if (pad) return -1;                         /* nothing after the padding */
		if (ch >= 'A' && ch <= 'Z') v = ch - 'A';
		else if (ch >= 'a' && ch <= 'z') v = ch - 'a' + 26;
		else if (ch >= '0' && ch <= '9') v = ch - '0' + 52;
		else if (ch == '+') v = 62;
		else if (ch == '/') v = 63;
		else return -1;
		acc = (acc << 6) | (unsigned)v;
		bits += 6;
		if (bits >= 8)
		{
			bits -= 8;
			if (n >= cap) return -1;
			out[n++] = (BYTE)(acc >> bits);
		}
	}
	for (; *s; s++) if (*s != ' ' && *s != '\t' && *s != '\r' && *s != '\n') return -1;
	if (pad > 2) return -1;
	return n;
}

static int verifyWithKey(const BYTE hash[32], const BYTE sig[64], const char* keyB64)
{
	BCRYPT_ALG_HANDLE alg = NULL;
	BCRYPT_KEY_HANDLE key = NULL;
	BYTE blob[sizeof(BCRYPT_ECCKEY_BLOB) + 64];
	BCRYPT_ECCKEY_BLOB* head = (BCRYPT_ECCKEY_BLOB*)blob;
	int ok = 0;

	if (unbase64(keyB64, blob + sizeof(BCRYPT_ECCKEY_BLOB), 64) != 64) return 0;
	head->dwMagic = BCRYPT_ECDSA_PUBLIC_P256_MAGIC;
	head->cbKey = 32;
	if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_ECDSA_P256_ALGORITHM, NULL, 0) != 0) return 0;
	if (BCryptImportKeyPair(alg, NULL, BCRYPT_ECCPUBLIC_BLOB, &key, blob, sizeof(blob), 0) == 0)
	{
		/* CNG takes an ECDSA signature as r||s, which is what release.sig holds */
		ok = BCryptVerifySignature(key, NULL, (PUCHAR)hash, 32, (PUCHAR)sig, 64, 0) == 0;
		BCryptDestroyKey(key);
	}
	BCryptCloseAlgorithmProvider(alg, 0);
	return ok;
}

int upd_signature_valid(const void* data, size_t n, const char* sig)
{
	BYTE s[64], hash[32];
	size_t k;

	if (!data || !sig || unbase64(sig, s, sizeof(s)) != 64) return 0;
	if (!upd_sha256(data, n, hash)) return 0;
	for (k = 0; k < sizeof(RELEASE_KEYS) / sizeof(RELEASE_KEYS[0]); k++)
		if (verifyWithKey(hash, s, RELEASE_KEYS[k])) return 1;
	return 0;
}

/* ------------------------------------------------------------------ known answers */

static int gFails;

static void expect(FILE* log, int ok, const char* what)
{
	if (!ok) gFails++;
	fprintf(log, "%-4s %s\r\n", ok ? "ok" : "BAD", what);
}

int upd_selftest(FILE* log)
{
	static const char sums[] =
		"0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef  Forja-Setup-9.9.9.exe\r\n"
		"FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210 *forja-portable.zip\n"
		"short  Other.exe\n"
		"  1111111111111111111111111111111111111111111111111111111111111111\tTabbed.exe  \n";
	static const char json[] =
		"{\"url\":\"https://api.github.com/x\",\"html_url\":\"https://github.com/o/r/releases/tag/v9.9.9\","
		"\"tag_name\": \"v9.9.9\",\"assets\":[{\"name\":\"SHA256SUMS.txt\",\"browser_download_url\":"
		"\"https://github.com/o/r/releases/download/v9.9.9/SHA256SUMS.txt\"},{\"name\":\"Forja-Setup-9.9.9.exe\","
		"\"browser_download_url\": \"https://github.com/o/r/releases/download/v9.9.9/Forja-Setup-9.9.9.exe\"},"
		"{\"name\":\"release.sig\",\"browser_download_url\":\"https://github.com/o/r/releases/download/v9.9.9/release.sig\"},"
		"{\"name\":\"bad\",\"browser_download_url\":\"https://github.com/o/r/releases/download/v9.9.9/Forja-Setup-..%5c.exe\"}]}";
	static const BYTE abc[32] = { 0xba,0x78,0x16,0xbf,0x8f,0x01,0xcf,0xea,0x41,0x41,0x40,0xde,0x5d,0xae,0x22,0x23,
	                              0xb0,0x03,0x61,0xa3,0x96,0x17,0x7a,0x9c,0xb4,0x10,0xff,0x61,0xf2,0x00,0x15,0xad };
	char h[65];
	BYTE d[32];
	UpdRelease r;

	gFails = 0;

	expect(log, upd_is_newer(L"v2.1.0", L"2.0.2"), "IsNewer: v2.1.0 is newer than 2.0.2");
	expect(log, upd_is_newer(L"2.0.10", L"2.0.9"), "IsNewer: 2.0.10 is newer than 2.0.9 (numbers, not text)");
	expect(log, upd_is_newer(L"V3.0", L"2.9.9"), "IsNewer: V3.0 is newer than 2.9.9");
	expect(log, upd_is_newer(L"2.1.0", L"2.1"), "IsNewer: 2.1.0 is newer than 2.1 (a missing part is lower)");
	expect(log, !upd_is_newer(L"v2.1.0", L"2.1.0"), "IsNewer: the same version is not newer");
	expect(log, !upd_is_newer(L"v2.0.2", L"2.1.0"), "IsNewer: an older version is not newer");
	expect(log, !upd_is_newer(L"v2.2.0-beta", L"2.1.0"), "IsNewer: a tag with a suffix is ignored");
	expect(log, !upd_is_newer(L"3", L"2.1.0"), "IsNewer: a single number is not a version");
	expect(log, !upd_is_newer(L"", L"2.1.0"), "IsNewer: an empty tag is not newer");
	expect(log, !upd_is_newer(L"1.2.3.4.5", L"1.0"), "IsNewer: five parts are not a version");

	expect(log, upd_hash_for(sums, "Forja-Setup-9.9.9.exe", h) &&
	            !strcmp(h, "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"), "HashFor: \"hash  name\"");
	expect(log, upd_hash_for(sums, "FORJA-PORTABLE.ZIP", h) &&
	            !strcmp(h, "fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210"), "HashFor: \"hash *name\", any case, lowercase out");
	expect(log, upd_hash_for(sums, "Tabbed.exe", h) && h[0] == '1', "HashFor: tab separator and spaces around the line");
	expect(log, !upd_hash_for(sums, "Other.exe", h), "HashFor: a short hash is not a hash");
	expect(log, !upd_hash_for(sums, "Forja-Setup-9.9.exe", h), "HashFor: a missing file has no hash");
	expect(log, !upd_hash_for("", "x", h), "HashFor: empty file");

	expect(log, upd_parse_release(json, &r) && !strcmp(r.tag, "v9.9.9"), "release JSON: tag_name");
	expect(log, !strcmp(r.setupName, "Forja-Setup-9.9.9.exe") &&
	            !strcmp(r.setupUrl, "https://github.com/o/r/releases/download/v9.9.9/Forja-Setup-9.9.9.exe"),
	            "release JSON: the setup asset (an unsafe file name is ignored)");
	expect(log, !strcmp(r.sumsUrl, "https://github.com/o/r/releases/download/v9.9.9/SHA256SUMS.txt"), "release JSON: SHA256SUMS.txt");
	expect(log, !strcmp(r.sigUrl, "https://github.com/o/r/releases/download/v9.9.9/release.sig"), "release JSON: release.sig");
	expect(log, !upd_parse_release("{\"message\":\"Not Found\"}", &r), "release JSON: no tag, no release");

	expect(log, upd_sha256("abc", 3, d) && !memcmp(d, abc, 32), "SHA-256 of \"abc\" (FIPS 180-2 example)");

	/* release.sig: real signatures by the author's two keys (the PC's, the Mac's) of the same bytes, as in
	 * the self-tests of Vigia, Muelle and Haul; any change is refused */
	{
		static const char signedSums[] =
			"0123456789abcdef0123456789abcdef0123456789abcdef0123456789ABCDEF  Vigia-Setup-9.9.9.exe\n"
			"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff *Vigia_Manual_EN.pdf\n";
		static const char pcSig[] = "rEEKYRSXLO7VKbID/El7ZkFBh1uKIPNS38KopNSZ6DEm7knfIUjxch9kmv8xfREc6bLKka+bePFZ1BKzO+15ww==\n";
		static const char macSig[] = "K8+3h6FgPjn2ZyH3HsLWof4obfKfyW00p3YKE9WQFoEF+umfN6/OMxMQNrQbc3XqkKrZSz3QexCBIZEMrdrvKg==";
		char changed[sizeof(signedSums)];
		size_t n = sizeof(signedSums) - 1;

		memcpy(changed, signedSums, sizeof(signedSums));
		changed[0] ^= 1;
		expect(log, upd_signature_valid(signedSums, n, pcSig), "signature: the PC's key");
		expect(log, upd_signature_valid(signedSums, n, macSig), "signature: the Mac's key");
		expect(log, !upd_signature_valid(changed, n, pcSig) && !upd_signature_valid(changed, n, macSig),
		       "signature: one byte changed is refused");
		expect(log, !upd_signature_valid(signedSums, n, "") && !upd_signature_valid(signedSums, n, "not base64!")
		            && !upd_signature_valid(signedSums, n, "QUJD"), "signature: empty, not base64 or too short is refused");
	}

	return gFails;
}
