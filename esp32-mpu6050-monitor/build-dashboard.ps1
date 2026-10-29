$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
$html = [IO.File]::ReadAllText((Join-Path $root 'dashboard.html'))
if ($html.Contains(')DASH"')) { throw 'HTML delimiter collision' }
$header = '#pragma once' + "`n" + 'const char DASHBOARD[] PROGMEM=R"DASH(' + $html + ')DASH";' + "`n"
[IO.File]::WriteAllText((Join-Path $root 'gateway\dashboard.h'), $header, [Text.UTF8Encoding]::new($false))
$js = [regex]::Match($html,'(?s)<script>(.*?)</script>').Groups[1].Value
[IO.File]::WriteAllText((Join-Path $root 'dashboard-check.js'), $js)
& node --check (Join-Path $root 'dashboard-check.js')
if ($LASTEXITCODE -ne 0) { throw 'Dashboard JavaScript failed syntax check' }
Write-Output 'Dashboard header generated; JavaScript syntax OK.'
