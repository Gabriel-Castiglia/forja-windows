@echo off
rem  Wyrmhold -- builds build\wyrmhold.exe from src\.
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

echo [2/2] program
gcc -O2 -Wall -Wextra -mwindows -static -DUNICODE -D_UNICODE -Isrc ^
    -o build\wyrmhold.exe src\wyrmhold.c src\pwgen.c build\app.res ^
    -lcomctl32 -lcomdlg32 -lgdi32 -luxtheme -ldwmapi -lbcrypt -lshell32 -lgdiplus -lm
if errorlevel 1 goto :failed

echo.
echo Built: build\wyrmhold.exe
goto :eof

:failed
echo.
echo BUILD FAILED
exit /b 1
