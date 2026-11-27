# Forja -- the engine checks, run through the command line (/out), and the self-test.
# Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
# Date: November 9, 2026
# SPDX-License-Identifier: GPL-2.0-only
#
# First calibrates check_passwords.py: a file of correct passwords made here by the
# same rules must pass, and five copies of it, each spoiled in one way, must fail. Only then
# are the program's outputs checked. Output files go to a temporary folder.

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$exe  = Join-Path $root 'build\forja.exe'
$chk  = Join-Path $PSScriptRoot 'check_passwords.py'
$tmp  = Join-Path ([IO.Path]::GetTempPath()) ('forja_checks_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $tmp | Out-Null
$fail = 0

function Check($file, $len, $avoid, $count, [bool]$mustPass, $label) {
    $out = & python -I $chk $file $len $avoid $count 2>&1
    $passed = ($LASTEXITCODE -eq 0)
    $good = ($passed -eq $mustPass)
    if (-not $good) { $script:fail++ }
    '{0,-4} {1,-58} expected {2,-4} got {3}' -f ($(if ($good) {'ok'} else {'BAD'})), $label,
        ($(if ($mustPass) {'pass'} else {'fail'})), ($(if ($passed) {'pass'} else {'fail'}))
    if (-not $good) { $out | ForEach-Object { "      $_" } }
}

# --- calibration: known answers ---------------------------------------------------------
$ref = Join-Path $tmp 'reference.txt'
$py = @"
import secrets, string, sys
sym='!@#`$%&*+-=?'; amb='0O1lI|``\'"'
lo=''.join(c for c in string.ascii_lowercase if c not in amb)
up=''.join(c for c in string.ascii_uppercase if c not in amb)
dg=''.join(c for c in string.digits if c not in amb)
al=lo+up+dg+sym
with open(sys.argv[1],'wb') as f:
    for _ in range(20000):
        p=[secrets.choice(lo),secrets.choice(up),secrets.choice(dg),secrets.choice(sym)]+[secrets.choice(al) for _ in range(16)]
        secrets.SystemRandom().shuffle(p); f.write((''.join(p)+'\r\n').encode())
"@
$pyFile = Join-Path $tmp 'ref.py'
Set-Content -LiteralPath $pyFile -Value $py -Encoding utf8
& python -I $pyFile $ref
Check $ref 20 1 20000 $true 'calibration: reference passwords'

$lines = [IO.File]::ReadAllLines($ref)
function Spoil($name, [scriptblock]$edit) {
    $copy = [string[]]$lines.Clone()
    & $edit $copy
    $f = Join-Path $tmp "$name.txt"
    [IO.File]::WriteAllText($f, (($copy -join "`r`n") + "`r`n"))
    return $f
}
Check (Spoil 'short'     { param($c) $c[7]  = $c[7].Substring(0,19) }) 20 1 20000 $false 'calibration: one password one char short'
Check (Spoil 'ambiguous' { param($c) $c[9]  = 'O' + $c[9].Substring(1) }) 20 1 20000 $false 'calibration: an ambiguous O'
Check (Spoil 'nosymbol'  { param($c) $c[11] = 'aB3defghijkmnopqrstu' }) 20 1 20000 $false 'calibration: a password with no symbol'
Check (Spoil 'foreign'   { param($c) $c[13] = $c[13].Substring(0,19) + '^' }) 20 1 20000 $false 'calibration: a character outside the alphabet'
Check (Spoil 'biased'    { param($c) for ($i=0; $i -lt $c.Length; $i++) { $c[$i] = $c[$i] -replace '[b-h]','a' } }) 20 1 20000 $false 'calibration: lowercase skewed towards a'

# --- the program -------------------------------------------------------------------------
foreach ($case in @(
    @{len=20;  avoid=1; count=100000; args=@('/len:20','/count:100000')},
    @{len=20;  avoid=0; count=100000; args=@('/len:20','/count:100000','/ambiguous')},
    @{len=4;   avoid=1; count=50000;  args=@('/len:4','/count:50000')},
    @{len=128; avoid=1; count=3000;   args=@('/len:128','/count:3000')},
    @{len=4;   avoid=1; count=10;     args=@('/len:1','/count:10')},
    @{len=128; avoid=1; count=10;     args=@('/len:999','/count:10')},
    @{len=20;  avoid=1; count=5;      args=@()}
)) {
    $f = Join-Path $tmp ('out_{0}_{1}_{2}.txt' -f $case.len, $case.avoid, $case.count)
    $p = Start-Process -FilePath $exe -ArgumentList ($case.args + "/out:$f") -Wait -PassThru -NoNewWindow
    if ($p.ExitCode -ne 0) { "BAD  exit code $($p.ExitCode) for $($case.args -join ' ')"; $fail++; continue }
    Check $f $case.len $case.avoid $case.count $true ("program: " + ($(if ($case.args) {$case.args -join ' '} else {'(defaults)'})))
}

# the program's own self-test: clipboard wiping (on the real clipboard, saved and put back),
# its settings in a throw-away .ini, and the update's pure functions (IsNewer, HashFor...)
''
$log = Join-Path $tmp 'selftest.log'
$p = Start-Process -FilePath $exe -ArgumentList "/selftest:$log" -Wait -PassThru -NoNewWindow
if (Test-Path $log) { Get-Content -LiteralPath $log | Select-Object -Skip 1 }
if ($p.ExitCode -ne 0) { "BAD  self-test exit code $($p.ExitCode)"; $fail++ }

Remove-Item -Recurse -Force $tmp

# every other option (character sets, exclusions, PINs, passphrases) and the exact entropy
''
& python -I (Join-Path $PSScriptRoot 'check_options.py') $exe
if ($LASTEXITCODE) { $fail++ }
''
if ($fail) { "FAILED: $fail check(s)"; exit 1 } else { 'ALL CHECKS PASSED'; exit 0 }
