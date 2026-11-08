<#
.SYNOPSIS
 Runs several Tools/Testing/Run-*.ps1 scripts in parallel and records how long each one takes.

.DESCRIPTION
 Each parallel slot gets its own UDP port range (FISK_TEST_PORT_BASE = 7777 + 100 * slot) and
 its own log folder. Scripts whose ports are fixed inside the game (main menu, solo menu) run
 alone afterwards on the normal ports. summary.csv in the log folder lists result and duration.

 -ClockScale sets the test-only day clock acceleration (FISK_CLOCK_SCALE). -Release forces
 the normal 20-minute day (scale 1) and every script, which is what release evidence needs.

.EXAMPLE
 # Quick check of what a branch changed, four times faster clock, two scripts at a time
 .\Tools\Testing\Run-All.ps1 -Package C:\Fiskespillet\pakke -LogDirectory C:\Fiskespillet\logs -ChangedSince origin/main

.EXAMPLE
 # Full release run with real 20-minute days
 .\Tools\Testing\Run-All.ps1 -Package C:\Fiskespillet\pakke -LogDirectory C:\Fiskespillet\logs -Release -Parallel 3
#>
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$LogDirectory,
    [string[]]$Scripts = @(),
    [string]$ChangedSince = '',
    [ValidateRange(1,8)][int]$Parallel = 2,
    [ValidateRange(1,20)][int]$ClockScale = 4,
    [switch]$Release
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$exclusive = @('MainMenu', 'SoloPlay')   # use 7777/17784 from game code; cannot be moved yet
$all = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter 'Run-*.ps1' | Where-Object { $_.BaseName -ne 'Run-All' } |
    ForEach-Object { $_.BaseName.Substring(4) } | Sort-Object)

if ($Release) {
    if ($Scripts.Count -or $ChangedSince) { throw '-Release always runs every script; do not combine it with -Scripts or -ChangedSince' }
    $ClockScale = 1
    $selected = $all
} elseif ($Scripts.Count) {
    $selected = $Scripts
} elseif ($ChangedSince) {
    $python = if (Get-Command python -ErrorAction SilentlyContinue) { 'python' } else { 'py' }
    $out = @(& $python (Join-Path $PSScriptRoot 'select_tests.py') --base $ChangedSince)
    if ($LASTEXITCODE -ne 0) { throw 'select_tests.py failed' }
    $selected = if ($out -contains 'ALL') { $all } else { $out }
} else {
    $selected = $all
}
foreach ($name in $selected) { if ($all -notcontains $name) { throw "Unknown test script: $name" } }
if (-not $selected.Count) { 'No graphical tests are needed for this change.'; return }

$exe = Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Packaged executable missing: $exe" }
$run = Join-Path $LogDirectory ('all-' + (Get-Date -Format yyyyMMdd-HHmmss))
New-Item -ItemType Directory -Path $run -Force | Out-Null
$shell = (Get-Process -Id $PID).Path
$sourceCommit = (& git -C $root rev-parse HEAD).Trim()
$exeHash = (Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash
$results = [System.Collections.Generic.List[object]]::new()
$wall = [Diagnostics.Stopwatch]::StartNew()
"Scripts: $($selected -join ', ')"
"Clock scale: $ClockScale  Parallel: $Parallel  Source: $sourceCommit"

function Start-Script([string]$name, [int]$slot) {
    $path = Join-Path $PSScriptRoot "Run-$name.ps1"
    $logs = Join-Path $run $name   # Run-HarbourGuide refuses an existing folder, so create only the parent
    $arguments = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $path, '-Package', $Package, '-LogDirectory', $logs)
    $parameters = (Get-Command $path).Parameters.Keys
    if ($parameters -contains 'SourceCommit') { $arguments += @('-SourceCommit', $sourceCommit) }
    if ($parameters -contains 'ExpectedExecutableSha256') { $arguments += @('-ExpectedExecutableSha256', $exeHash) }
    $env:FISK_TEST_PORT_BASE = [string](7777 + 100 * $slot)
    $env:FISK_CLOCK_SCALE = [string]$ClockScale
    try {
        $process = Start-Process -FilePath $shell -ArgumentList $arguments -PassThru -WindowStyle Hidden `
            -RedirectStandardOutput (Join-Path $run "$name.out.txt") -RedirectStandardError (Join-Path $run "$name.err.txt")
    } finally {
        Remove-Item Env:FISK_TEST_PORT_BASE, Env:FISK_CLOCK_SCALE -ErrorAction SilentlyContinue
    }
    [pscustomobject]@{ Name = $name; Slot = $slot; Process = $process; Clock = [Diagnostics.Stopwatch]::StartNew() }
}

function Complete-Script($job) {
    $job.Process.WaitForExit()
    $job.Clock.Stop()
    $code = $job.Process.ExitCode
    $err = Join-Path $run "$($job.Name).err.txt"
    $failed = $code -ne 0 -or ((Test-Path -LiteralPath $err) -and (Get-Item -LiteralPath $err).Length -gt 0)
    $row = [pscustomobject]@{
        Script = $job.Name; Result = $(if ($failed) { 'Failed' } else { 'Passed' }); ExitCode = $code
        Seconds = [Math]::Round($job.Clock.Elapsed.TotalSeconds); Slot = $job.Slot; ClockScale = $ClockScale
    }
    $results.Add($row)
    '{0,-20} {1,-6} {2,6}s' -f $row.Script, $row.Result, $row.Seconds
}

$queue = [System.Collections.Generic.Queue[string]]::new()
$selected | Where-Object { $exclusive -notcontains $_ } | ForEach-Object { $queue.Enqueue($_) }
$running = @()
$freeSlots = [System.Collections.Generic.Queue[int]]::new()
1..$Parallel | ForEach-Object { $freeSlots.Enqueue($_) }
try {
    while ($queue.Count -or $running.Count) {
        while ($queue.Count -and $freeSlots.Count) { $running += Start-Script $queue.Dequeue() $freeSlots.Dequeue() }
        Start-Sleep -Milliseconds 500
        $done = @($running | Where-Object { $_.Process.HasExited })
        foreach ($job in $done) { Complete-Script $job; $freeSlots.Enqueue($job.Slot) }
        $running = @($running | Where-Object { -not $_.Process.HasExited })
    }
    # Fixed-port scripts run one at a time on the normal ports (slot 0 = 7777).
    foreach ($name in ($selected | Where-Object { $exclusive -contains $_ })) {
        Complete-Script (Start-Script $name 0)
    }
} finally {
    foreach ($job in $running) { if (-not $job.Process.HasExited) { Stop-Process -Id $job.Process.Id -Force -ErrorAction SilentlyContinue } }
    $wall.Stop()
    $results | Export-Csv -LiteralPath (Join-Path $run 'summary.csv') -NoTypeInformation -Encoding utf8
}
$failedCount = @($results | Where-Object Result -eq 'Failed').Count
$serial = ($results | Measure-Object -Property Seconds -Sum).Sum
"Total wall time: $([Math]::Round($wall.Elapsed.TotalSeconds))s (sum of scripts: ${serial}s). Evidence: $run"
if ($ClockScale -ne 1) { "NOTE: clock scale $ClockScale is a quick check. Release evidence requires -Release (normal 20-minute days)." }
if ($failedCount) { throw "$failedCount test script(s) failed. See $run" }
