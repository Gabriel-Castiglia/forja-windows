/**
  update.h
  Forja: strong passwords, passphrases and PINs made on your computer.

  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
  Date: November 20, 2026
  SPDX-License-Identifier: GPL-2.0-only

  Updates that install themselves: the parts that do not touch the window. The flow (one
  question, download, checksum, silent setup, reopen) lives in forja.c; this file has the
  pure functions it relies on, the HTTPS client (WinHTTP), SHA-256 (BCrypt) and the test of
  whether this copy is the installed one or a portable one.
**/

#ifndef FORJA_UPDATE_H
#define FORJA_UPDATE_H

#include <windows.h>
#include <stdio.h>

#define UPD_API_URL   L"https://api.github.com/repos/Gabriel-Castiglia/forja-windows/releases/latest"
#define UPD_SETUP_PREFIX "Forja-Setup-"
#define UPD_SUMS_NAME    "SHA256SUMS.txt"

/* What the latest release offers. Empty strings for what it lacks. */
typedef struct {
	char tag[64];                       /* as published, e.g. "v2.1.0" */
	char setupUrl[1024], setupName[128];
	char sumsUrl[1024];
} UpdRelease;

/* Receives the body of a download, piece by piece; returns 0 to stop. */
typedef int (*UpdSink)(void* ctx, const BYTE* data, DWORD n, ULONGLONG done, ULONGLONG total);

/* 1 when the tag (a leading v or V is ignored) is a newer version than mine. Both must be
 * 2 to 4 numbers separated by dots; a missing part counts as lower than 0, so 2.1 < 2.1.0. */
int upd_is_newer(const wchar_t* tag, const wchar_t* mine);

/* The hash of one file in a SHA256SUMS.txt ("hash  name" or "hash *name" per line, name
 * compared without regard to case) into out, in lowercase; 1 if found, 0 if not. */
int upd_hash_for(const char* sums, const char* fileName, char out[65]);

/* The latest release out of the GitHub API's JSON. 1 if a tag was found. */
int upd_parse_release(const char* json, UpdRelease* r);

/* An HTTPS GET; redirections are followed. 0 when the whole body was delivered, otherwise
 * a Win32 / WinHTTP error code, or the HTTP status when it is not 200. */
DWORD upd_http_get(const wchar_t* url, const wchar_t* agent, const wchar_t* accept, DWORD timeoutMs,
                   UpdSink sink, void* ctx);

/* SHA-256 of a buffer (32 bytes out) and of a file (64 lowercase hex digits out). 1 = done. */
int upd_sha256(const void* data, size_t n, BYTE out[32]);
int upd_sha256_file(const wchar_t* path, char out[65]);

/* 1 when this executable is the one the setup installed (its folder is the InstallLocation
 * the setup recorded), 0 for a portable copy, which cannot update itself. */
int upd_installed_copy(void);

/* Known answers for the pure functions above, written to log. Returns the failures. */
int upd_selftest(FILE* log);

#endif
