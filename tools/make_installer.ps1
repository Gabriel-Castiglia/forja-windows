# Wyrmhold -- builds the setup (out\Wyrmhold-Setup-<version>.exe) with Inno Setup 6.
# Author: Gabriel Castiglia - 2026-11-10
#
# Run after build.bat and tools\make_pdf.ps1: the setup carries build\wyrmhold.exe and the
# PDF guides. The version is read from src\wyrmhold.c, so the setup cannot name another one.

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$src = Get-Content -Raw -Encoding UTF8 (Join-Path $root 'src\wyrmhold.c')
if ($src -notmatch '#define APP_VERSION\s+L"([\d.]+)"') { throw 'APP_VERSION not found' }
$version = $Matches[1]

foreach ($f in @('build\wyrmhold.exe', 'docs\wyrmhold_guide_en.pdf', 'docs\wyrmhold_guide_jp.pdf', 'docs\wyrmhold_guide_es.pdf',
                 'installer\wizard.bmp', 'installer\wizard-small.bmp')) {
    if (-not (Test-Path (Join-Path $root $f))) { throw "missing $f" }
}

$iscc = @("$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe", "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) { throw 'Inno Setup 6 not found (winget install JRSoftware.InnoSetup)' }

& $iscc /Q "/DAppVersion=$version" (Join-Path $root 'installer\Wyrmhold.iss')
if ($LASTEXITCODE) { throw "ISCC failed ($LASTEXITCODE)" }
"written: " + (Join-Path $root "out\Wyrmhold-Setup-$version.exe")
