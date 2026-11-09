# Random Password Generator -- converts docs\*.docx to PDF with Microsoft Word.
# Author: Gabriel Castiglia - 2026-11-09

$ErrorActionPreference = 'Stop'
$docs = Join-Path (Split-Path -Parent $PSScriptRoot) 'docs'
$word = New-Object -ComObject Word.Application
$word.Visible = $false
$word.DisplayAlerts = 0
try {
    foreach ($f in Get-ChildItem -LiteralPath $docs -Filter 'randompassgen_guide_*.docx') {
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
