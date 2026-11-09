<#
.SYNOPSIS
 Builds and packages FISKELAGET for Win64 Development and records how long each step takes.

.DESCRIPTION
 Incremental by default: Intermediate, Saved/Cooked and DerivedDataCache are kept between runs,
 so only changed code is recompiled and only changed content is recooked (-iterate).

 -Mode Full  Build Editor (needed for automation tests and cooking), then build/cook/stage/pak/archive.
 -Mode Code  For C++-only changes: build Game, reuse the previous cook (-skipcook) and repackage.
             Falls back to a full cook when no earlier cook exists.
 -SkipEditor Do not build the Editor target (Full mode then cooks with the existing Editor binary).
 -SpilLanExport  After packaging, copy the build to <folder>\v<version> and publish it to the SpilLan
             launcher (Tools/Build/Publish-SpilLan.ps1). Publishing is skipped with a warning when
             SpilLan is not installed.

 build-times.csv in the archive folder records every step, so before/after timings can be compared.

.EXAMPLE
 .\Tools\Build\Build-Fisk.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ArchiveDirectory C:\Fiskespillet\pakke
.EXAMPLE
 .\Tools\Build\Build-Fisk.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ArchiveDirectory C:\Fiskespillet\pakke -Mode Code
.EXAMPLE
 .\Tools\Build\Build-Fisk.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ArchiveDirectory C:\Fiskespillet\pakke -SpilLanExport C:\Fiskespillet\eksport
#>
param(
    [Parameter(Mandatory)][string]$EngineRoot,
    [Parameter(Mandatory)][string]$ArchiveDirectory,
    [ValidateSet('Full', 'Code')][string]$Mode = 'Full',
    [switch]$SkipEditor,
    [string]$SpilLanExport = ''
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$project = Join-Path $root 'FiskELaget.uproject'
$buildBat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
$runUat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
foreach ($file in $project, $buildBat, $runUat) { if (-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Missing: $file" } }
$repoRoot = [IO.Path]::GetFullPath($root).TrimEnd('\') + '\'
if ([IO.Path]::GetFullPath($ArchiveDirectory).StartsWith($repoRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'ArchiveDirectory must be outside the repository (packages are never committed)'
}
New-Item -ItemType Directory -Path $ArchiveDirectory -Force | Out-Null
$times = [System.Collections.Generic.List[object]]::new()
$total = [Diagnostics.Stopwatch]::StartNew()

function Step([string]$name, [scriptblock]$action) {
    $clock = [Diagnostics.Stopwatch]::StartNew()
    "== $name"
    & $action
    $code = $LASTEXITCODE
    $clock.Stop()
    $times.Add([pscustomobject]@{ Step = $name; Seconds = [Math]::Round($clock.Elapsed.TotalSeconds); ExitCode = $code; Mode = $Mode })
    '   {0}: {1}s' -f $name, [Math]::Round($clock.Elapsed.TotalSeconds)
    if ($code -ne 0) { throw "$name failed with exit code $code" }
}

$cooked = Join-Path $root 'Saved\Cooked\Windows'
$skipCook = $Mode -eq 'Code' -and (Test-Path -LiteralPath $cooked)
if ($Mode -eq 'Code' -and -not $skipCook) { 'No earlier cook found; cooking once.' }

try {
    if ($Mode -eq 'Full' -and -not $SkipEditor) {
        Step 'Editor build' { & $buildBat FiskELagetEditor Win64 Development "-Project=$project" -WaitMutex -NoHotReloadFromIDE }
    }
    $uat = @('BuildCookRun', "-project=$project", '-platform=Win64', '-clientconfig=Development', '-build',
        '-stage', '-pak', '-iostore', '-archive', "-archivedirectory=$ArchiveDirectory", '-nocompileeditor',
        '-unattended', '-utf8output', '-nop4')
    if ($skipCook) { $uat += '-skipcook' } else { $uat += @('-cook', '-iterate') }
    Step $(if ($skipCook) { 'Game build + stage (cook reused)' } else { 'Game build + cook + stage' }) { & $runUat @uat }
} finally {
    $total.Stop()
    $times.Add([pscustomobject]@{ Step = 'Total'; Seconds = [Math]::Round($total.Elapsed.TotalSeconds); ExitCode = 0; Mode = $Mode })
    $csv = Join-Path $ArchiveDirectory 'build-times.csv'
    $times | Export-Csv -LiteralPath $csv -NoTypeInformation -Encoding utf8 -Append
    "Timings appended to $csv"
}
$exe = Join-Path $ArchiveDirectory 'Windows\FiskELaget\Binaries\Win64\FiskELaget.exe'
if (Test-Path -LiteralPath $exe) {
    $commit = (& git -C $root rev-parse HEAD).Trim()
    $dirty = [bool](& git -C $root status --porcelain)
    "Package: $exe"
    "Source: $commit$(if ($dirty) { ' (uncommitted changes: not valid as test evidence)' })"
    "SHA256: $((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash)"
}
if ($SpilLanExport) {
    if ($dirty) { throw 'Uncommitted changes: commit before publishing to SpilLan' }
    & (Join-Path $PSScriptRoot 'Publish-SpilLan.ps1') -Package (Join-Path $ArchiveDirectory 'Windows') -ExportDirectory $SpilLanExport
}
