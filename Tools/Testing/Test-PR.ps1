<#
.SYNOPSIS
 One fixed test run per level, ending in one short report (report.md). Post the report, not logs.

.DESCRIPTION
 -Level Build    Build and package only. For a playable version; no tests.
 -Level PR       Build, automation tests, the graphical scripts select_tests.py picks for the change
                 (compared with -Base), then the report. Scripts that already passed with the same
                 build are skipped (see Run-All.ps1 -Force).
 -Level Release  Build, automation tests and every graphical script with the normal 20-minute day.

 Failures listed in known_failures.json are marked with their issue; the rest are NY. The exit code
 is 0 when there are no new failures and 1 when there are. Manual checks (a real mouse, two PCs)
 are never attempted here; list them in the PR as "Venter på menneske".

.EXAMPLE
 .\Tools\Testing\Test-PR.ps1 -Level PR -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ArchiveDirectory C:\Fiskespillet\pakke -LogDirectory C:\Fiskespillet\logs
#>
param(
    [ValidateSet('Build', 'PR', 'Release')][string]$Level = 'PR',
    [Parameter(Mandatory)][string]$EngineRoot,
    [Parameter(Mandatory)][string]$ArchiveDirectory,
    [Parameter(Mandatory)][string]$LogDirectory,
    [string]$Base = 'origin/main',
    [ValidateRange(1,8)][int]$Parallel = 2,
    [switch]$Force
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$python = @('python', 'py' | Where-Object { Get-Command $_ -ErrorAction SilentlyContinue })[0]
if (-not $python) { throw 'Python is needed for the report (Tools/Testing/report.py)' }
New-Item -ItemType Directory -Path $LogDirectory -Force | Out-Null
$problems = [System.Collections.Generic.List[string]]::new()
$missingEvidence = $false   # A step that failed without leaving a report never counts as a pass

& (Join-Path $root 'Tools\Build\Build-Fisk.ps1') -EngineRoot $EngineRoot -ArchiveDirectory $ArchiveDirectory
$package = Join-Path $ArchiveDirectory 'Windows'
$exe = Join-Path $package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
$arguments = @((Join-Path $PSScriptRoot 'report.py'), '--level', $Level, '--exe', $exe)

if ($Level -ne 'Build') {
    $automationDir = Join-Path $LogDirectory 'automation'
    $automationStarted = Get-Date
    try { & (Join-Path $root 'Tools\Build\Test-Automation.ps1') -EngineRoot $EngineRoot -ReportDirectory $automationDir }
    catch { $problems.Add("Automation: $($_.Exception.Message)") }
    $index = Get-ChildItem -LiteralPath $automationDir -Filter index.json -Recurse -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -ge $automationStarted } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($index) { $arguments += @('--automation', $index.FullName) } else { $problems.Add('Automation wrote no index.json'); $missingEvidence = $true }

    $started = Get-Date
    $runAll = @{ Package = $package; LogDirectory = $LogDirectory; Parallel = $Parallel }
    if ($Level -eq 'Release') { $runAll.Release = $true } else { $runAll.ChangedSince = $Base; if ($Force) { $runAll.Force = $true } }
    $runAllFailed = $false
    try { & (Join-Path $PSScriptRoot 'Run-All.ps1') @runAll }
    catch { $problems.Add("Run-All: $($_.Exception.Message)"); $runAllFailed = $true }
    $run = Get-ChildItem -LiteralPath $LogDirectory -Directory -Filter 'all-*' -ErrorAction SilentlyContinue |
        Where-Object { $_.CreationTime -ge $started } | Sort-Object CreationTime -Descending | Select-Object -First 1
    if ($run) { $arguments += @('--run', $run.FullName) }
    elseif ($runAllFailed) { $missingEvidence = $true }
    else { 'No graphical scripts were needed for this change.' }
}

$out = Join-Path $LogDirectory ('report-' + (Get-Date -Format yyyyMMdd-HHmmss) + '.md')
$arguments += @('--out', $out)
& $python @arguments
$code = $LASTEXITCODE
foreach ($problem in $problems) { "NOTE: $problem" }
if ($missingEvidence) { 'FAILED: a test step stopped before it wrote results; the report is incomplete.'; $code = 1 }
"Report: $out"
exit $code
