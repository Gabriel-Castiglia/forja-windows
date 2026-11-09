@echo off
rem  Random Password Generator -- builds build\randompassgen.exe from src\.
rem  Author: Gabriel Castiglia - 2026-11-09
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
    -o build\randompassgen.exe src\randompassgen.c src\pwgen.c build\app.res ^
    -lcomctl32 -lcomdlg32 -lgdi32 -luxtheme -ldwmapi -lbcrypt -lshell32
if errorlevel 1 goto :failed

echo.
echo Built: build\randompassgen.exe
goto :eof

:failed
echo.
echo BUILD FAILED
exit /b 1
