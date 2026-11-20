@echo off
rem  Forja -- builds build\forja.exe from src\.
rem  Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
rem  Date: November 9, 2026
rem  SPDX-License-Identifier: GPL-2.0-only
rem
rem  Needs mingw-w64 GCC on the PATH (MSYS2 mingw64) and nothing else. The executable is
rem  static: it runs on a machine with no runtime installed, from any folder (portable).

setlocal
if "%MINGW%"=="" set MINGW=C:\msys64\mingw64\bin
set PATH=%MINGW%;%PATH%
cd /d "%~dp0"

if not exist build mkdir build

echo [1/2] resources
windres -I src src\app.rc -O coff -o build\app.res
if errorlevel 1 goto :failed

rem  Some mingw-w64 builds (WinLibs) link a default-manifest.o of their own next to ours, and the
rem  program ends up with two manifests. An empty default-manifest.o found first (-B) keeps only ours.
if not exist build\nodefman mkdir build\nodefman
echo.> build\nodefman\empty.c
gcc -c build\nodefman\empty.c -o build\nodefman\default-manifest.o
if errorlevel 1 goto :failed

echo [2/2] program
gcc -B build\nodefman\ -O2 -Wall -Wextra -mwindows -static -DUNICODE -D_UNICODE -Isrc ^
    -o build\forja.exe src\forja.c src\pwgen.c src\update.c build\app.res ^
    -lcomctl32 -lcomdlg32 -lgdi32 -luxtheme -ldwmapi -lbcrypt -lshell32 -lgdiplus -lwinhttp -ladvapi32 -lm
if errorlevel 1 goto :failed

echo.
echo Built: build\forja.exe
goto :eof

:failed
echo.
echo BUILD FAILED
exit /b 1
