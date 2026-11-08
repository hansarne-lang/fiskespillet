<#
.SYNOPSIS
 Runs the Unreal automation tests (Fiskelaget.*) headless, without graphics or a packaged build.

.DESCRIPTION
 Uses UnrealEditor-Cmd with -nullrhi. The pure rule tests (fight model, career receipts, day clock,
 settlement, journal, routes) take seconds here instead of needing a graphical LAN run.
 Requires the Editor target to be built (Tools/Build/Build-Fisk.ps1 -Mode Full).

.EXAMPLE
 .\Tools\Build\Test-Automation.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ReportDirectory C:\Fiskespillet\logs\automation
.EXAMPLE
 .\Tools\Build\Test-Automation.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ReportDirectory C:\Fiskespillet\logs\automation -Filter Fiskelaget.DayCycle
#>
param(
    [Parameter(Mandatory)][string]$EngineRoot,
    [Parameter(Mandatory)][string]$ReportDirectory,
    [ValidatePattern('^Fiskelaget(\.[A-Za-z0-9]+)*$')][string]$Filter = 'Fiskelaget'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$project = Join-Path $root 'FiskELaget.uproject'
$editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $editor -PathType Leaf)) { throw "Missing: $editor" }
$run = Join-Path $ReportDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$log = Join-Path $run 'automation.log'
$clock = [Diagnostics.Stopwatch]::StartNew()
$arguments = @($project, "-ExecCmds=`"Automation RunTests $Filter;Quit`"", '-TestExit="Automation Test Queue Empty"',
    "-ReportExportPath=`"$run`"", "-abslog=`"$log`"", '-unattended', '-nopause', '-nullrhi', '-nosound', '-nosplash', '-stdout')
$process = Start-Process -FilePath $editor -ArgumentList $arguments -PassThru -NoNewWindow -Wait
$clock.Stop()

$index = Join-Path $run 'index.json'
if (-not (Test-Path -LiteralPath $index)) { throw "No automation report was written (exit code $($process.ExitCode)). Log: $log" }
$report = Get-Content -LiteralPath $index -Raw -Encoding utf8 | ConvertFrom-Json
$passed = [int]$report.succeeded + [int]$report.succeededWithWarnings
$failed = [int]$report.failed
$notRun = [int]$report.notRun
"Automation $Filter : passed=$passed failed=$failed notRun=$notRun in $([Math]::Round($clock.Elapsed.TotalSeconds))s"
"Report: $run"
foreach ($test in @($report.tests | Where-Object { $_.state -eq 'Fail' })) { "FAILED: $($test.fullTestPath)" }
if ($passed -eq 0) { throw 'No automation tests ran: not a pass' }
if ($failed -gt 0 -or $process.ExitCode -ne 0) { throw "Automation tests failed. Log: $log" }
