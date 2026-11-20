# Forja -- builds the setup (out\Forja-Setup-<version>.exe) with Inno Setup 6.
# Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
# Date: November 10, 2026
# SPDX-License-Identifier: GPL-2.0-only
#
# Run after build.bat and tools\make_pdf.ps1: the setup carries build\forja.exe and the
# PDF guides. The version is read from src\version.h, so the setup cannot name another one.

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$src = Get-Content -Raw -Encoding UTF8 (Join-Path $root 'src\version.h')
if ($src -notmatch '#define WH_VERSION_STR\s+"([\d.]+)"') { throw 'WH_VERSION_STR not found' }
$version = $Matches[1]

foreach ($f in @('build\forja.exe', 'docs\forja_guide_en.pdf', 'docs\forja_guide_jp.pdf', 'docs\forja_guide_es.pdf',
                 'installer\wizard.bmp', 'installer\wizard-small.bmp')) {
    if (-not (Test-Path (Join-Path $root $f))) { throw "missing $f" }
}

$iscc = @("$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe", "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $iscc) { throw 'Inno Setup 6 not found (winget install JRSoftware.InnoSetup)' }

& $iscc /Q "/DAppVersion=$version" (Join-Path $root 'installer\Forja.iss')
if ($LASTEXITCODE) { throw "ISCC failed ($LASTEXITCODE)" }
"written: " + (Join-Path $root "out\Forja-Setup-$version.exe")
