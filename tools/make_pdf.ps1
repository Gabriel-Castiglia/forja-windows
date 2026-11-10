# Wyrmhold -- converts docs\wyrmhold_guide_*.docx to PDF with LibreOffice (headless), or with
# Microsoft Word when LibreOffice is not installed.
# Author: Gabriel Castiglia - 2026-11-09 (2.0.0: 2026-11-10)

$ErrorActionPreference = 'Stop'
$docs = Join-Path (Split-Path -Parent $PSScriptRoot) 'docs'
$files = Get-ChildItem -LiteralPath $docs -Filter 'wyrmhold_guide_*.docx'
$soffice = @("$env:ProgramFiles\LibreOffice\program\soffice.exe", "${env:ProgramFiles(x86)}\LibreOffice\program\soffice.exe") |
    Where-Object { Test-Path $_ } | Select-Object -First 1

if ($soffice) {
    & $soffice --headless --convert-to pdf --outdir $docs ($files | ForEach-Object FullName) | Out-Null
    foreach ($f in $files) {
        $pdf = [IO.Path]::ChangeExtension($f.FullName, '.pdf')
        if (-not (Test-Path $pdf) -or (Get-Item $pdf).LastWriteTime -lt $f.LastWriteTime) { throw "not converted: $($f.Name)" }
        "written: $pdf"
    }
    return
}

$word = New-Object -ComObject Word.Application
$word.Visible = $false
$word.DisplayAlerts = 0
try {
    foreach ($f in $files) {
        $pdf = [IO.Path]::ChangeExtension($f.FullName, '.pdf')
        $d = $word.Documents.Open($f.FullName, $false, $true)
        $d.ExportAsFixedFormat($pdf, 17)        # 17 = wdExportFormatPDF
        $d.Close($false)
        "written: $pdf"
    }
} finally {
    $word.Quit()
    [Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null
}
