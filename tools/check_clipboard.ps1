# Forja -- clipboard behaviour, checked from outside the program.
# Copyright (C) 2026 Gabriel Castiglia. Licensed under the GNU GPL, version 2 only.
# Date: November 9, 2026 (2.1.0: November 20, 2026)
# SPDX-License-Identifier: GPL-2.0-only
#
# Run it in an interactive desktop session with Windows PowerShell 5.1 (STA, needed to read
# the clipboard). It copies the program into a temporary folder with an empty forja.ini
# beside it (portable use, so the real settings are never read or written), starts it with
# /clearafter:5, sends it menu commands with PostMessage (same integrity level, so UIPI does
# not filter them) and reads the clipboard back:
#   1. Copy selected  -> one password of 20 characters, the three "keep out of history"
#      formats present, and the clipboard empty again after 5 s;
#   2. Copy all, then something else is copied -> after 5 s that something else is intact;
#   3. Copy selected, then the window is closed -> the clipboard is empty at once;
#   4. the empty forja.ini did not get the /clearafter value (it is for the session only).
# The text on the clipboard before the run is put back at the end. Writes its report to the
# file given as the first argument. src/forja.c /selftest checks the same in-process.

param([Parameter(Mandatory)] [string]$Report)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System; using System.Text; using System.Runtime.InteropServices;
public static class W {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern bool OpenClipboard(IntPtr h);
  [DllImport("user32.dll")] public static extern bool CloseClipboard();
  [DllImport("user32.dll")] public static extern uint EnumClipboardFormats(uint f);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetClipboardFormatName(uint f, StringBuilder s, int n);
  public static string Formats() {
    var sb = new StringBuilder(); if (!OpenClipboard(IntPtr.Zero)) return "<cannot open>";
    uint f = 0; while ((f = EnumClipboardFormats(f)) != 0) { var n = new StringBuilder(128);
      GetClipboardFormatName(f, n, 128); sb.Append(n.Length > 0 ? n.ToString() : f.ToString()).Append(';'); }
    CloseClipboard(); return sb.ToString(); }
}
"@

$src = Join-Path (Split-Path -Parent $PSScriptRoot) 'build\forja.exe'
$tmp = Join-Path ([IO.Path]::GetTempPath()) ('forja_clip_' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory $tmp | Out-Null
$exe = Join-Path $tmp 'forja.exe'
$ini = Join-Path $tmp 'forja.ini'
Copy-Item -LiteralPath $src -Destination $exe
Set-Content -LiteralPath $ini -Value '' -Encoding ascii

$out = New-Object System.Collections.Generic.List[string]
$fail = 0
function Say($ok, $what) { if (-not $ok) { $script:fail++ }; $out.Add(('{0,-4} {1}' -f ($(if ($ok) {'ok'} else {'BAD'})), $what)) }
function Text { try { [Windows.Forms.Clipboard]::GetText() } catch { '' } }
function Start-App {
  $p = Start-Process -FilePath $exe -ArgumentList '/lang:en', '/clearafter:5' -PassThru
  for ($i = 0; $i -lt 50 -and $p.MainWindowHandle -eq 0; $i++) { Start-Sleep -Milliseconds 100; $p.Refresh() }
  Start-Sleep -Milliseconds 500
  return $p
}
$WM_COMMAND = 0x111; $WM_CLOSE = 0x10; $IDM_COPY = 302; $IDM_COPYALL = 303
$before = Text

try {
  # 1
  $p = Start-App
  [W]::PostMessage($p.MainWindowHandle, $WM_COMMAND, [IntPtr]$IDM_COPY, [IntPtr]::Zero) | Out-Null
  Start-Sleep -Milliseconds 600
  $t = Text; $f = [W]::Formats()
  Say ($t.Length -eq 20) "1. copy selected: clipboard holds $($t.Length) characters (expected 20)"
  foreach ($n in 'ExcludeClipboardContentFromMonitorProcessing','CanIncludeInClipboardHistory','CanUploadToCloudClipboard') {
    Say ($f -like "*$n*") "1. format present: $n"
  }
  Start-Sleep -Seconds 6
  Say ((Text) -eq '') "1. after 6 s (/clearafter:5) the clipboard is empty"

  # 2
  [W]::PostMessage($p.MainWindowHandle, $WM_COMMAND, [IntPtr]$IDM_COPYALL, [IntPtr]::Zero) | Out-Null
  Start-Sleep -Milliseconds 600
  $lines = (Text) -split "`r`n" | Where-Object { $_ }
  Say ($lines.Count -eq 5) "2. copy all: $($lines.Count) lines (expected 5)"
  [Windows.Forms.Clipboard]::SetText('something else')
  Start-Sleep -Seconds 6
  Say ((Text) -eq 'something else') "2. something copied later is left alone after 6 s"

  # 3
  [W]::PostMessage($p.MainWindowHandle, $WM_COMMAND, [IntPtr]$IDM_COPY, [IntPtr]::Zero) | Out-Null
  Start-Sleep -Milliseconds 600
  Say ((Text).Length -eq 20) "3. copy selected again: 20 characters"
  [W]::PostMessage($p.MainWindowHandle, $WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
  $p.WaitForExit(5000) | Out-Null
  Say ($p.HasExited) "3. the window closed"
  Say ((Text) -eq '') "3. the clipboard is empty right after closing"

  # 4
  $saved = Get-Content -LiteralPath $ini -Raw
  Say ($saved -notmatch 'ClearSeconds' -and $saved -notmatch 'ClearClipboard') "4. /clearafter was not written to the .ini"
} finally {
  if ($before) { [Windows.Forms.Clipboard]::SetText($before) } else { [Windows.Forms.Clipboard]::Clear() }
  Remove-Item -Recurse -Force $tmp -ErrorAction SilentlyContinue
}

$out.Add($(if ($fail) { "FAILED: $fail" } else { 'ALL CLIPBOARD CHECKS PASSED' }))
$out | Set-Content -LiteralPath $Report -Encoding utf8
