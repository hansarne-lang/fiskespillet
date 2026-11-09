<#
.SYNOPSIS
 Runs several Tools/Testing/Run-*.ps1 scripts in parallel and records how long each one takes.

.DESCRIPTION
 Each parallel slot gets its own UDP port range (FISK_TEST_PORT_BASE = 7777 + 100 * slot) and
 its own log folder. Scripts whose ports are fixed inside the game (main menu, solo menu) run
 alone afterwards on the normal ports. summary.csv in the log folder lists result and duration.

 -ClockScale sets the test-only day clock acceleration (FISK_CLOCK_SCALE). -Release forces
 the normal 20-minute day (scale 1) and every script, which is what release evidence needs.

 Scripts that already passed with the same game binary, script, test harness and clock scale are
 not run again (passed-cache.jsonl in the log folder, see run_cache.py). They show as Cached in
 summary.csv. -Force runs them anyway. -Release never skips anything.

 -Fast is for PR runs: when select_tests.py answers ALL, the slow real-time scripts are skipped
 and left to the nightly Release run. Scripts it names directly always run.

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
    [switch]$Release,
    [switch]$Force,
    [switch]$Fast
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'FiskRunAllLib.ps1')
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$exclusive = @('MainMenu', 'SoloPlay')   # use 7777/17784 from game code; cannot be moved yet
# Scenarios with game-time windows too short for an accelerated clock always run at scale 1.
# SeaChart: FiskGroundSmoke.cpp captures chart-underway only between 5 and 6 game seconds.
# The others failed at scale 4 on 9 November 2026 (panels, shared plan, day 2, screenshots; #136)
# but passed at scale 1 in the release run on ab10973.
$realtimeOnly = @('SeaChart', 'AnglerRelease', 'ExpeditionRelease', 'ExplorationRelease', 'FishingRelease',
    'MangroveRelease', 'NavigationRelease', 'SkipperRelease')
function Get-Python { foreach ($name in 'python', 'py') { if (Get-Command $name -ErrorAction SilentlyContinue) { return $name } }; return $null }
$all = @(Get-ChildItem -LiteralPath $PSScriptRoot -Filter 'Run-*.ps1' | Where-Object { $_.BaseName -ne 'Run-All' } |
    ForEach-Object { $_.BaseName.Substring(4) } | Sort-Object)

if ($Release) {
    if ($Scripts.Count -or $ChangedSince) { throw '-Release always runs every script; do not combine it with -Scripts or -ChangedSince' }
    $ClockScale = 1
    $selected = $all
} elseif ($Scripts.Count) {
    $selected = $Scripts
} elseif ($ChangedSince) {
    $python = Get-Python
    if (-not $python) { throw 'Python is needed for -ChangedSince (select_tests.py)' }
    $selectArgs = @((Join-Path $PSScriptRoot 'select_tests.py'), '--base', $ChangedSince)
    if ($env:GITHUB_ACTIONS -eq 'true') { $selectArgs += '--committed-only' }   # the build rewrites some tracked files
    $out = @(& $python @selectArgs)
    if ($LASTEXITCODE -ne 0) { throw 'select_tests.py failed' }
    $selected = if ($out -contains 'ALL') { $all } else { $out }
    # -Fast (PR runs): when a change needs every script, leave the slow real-time scripts (20-30
    # minutes each) to the nightly Release run. A script select_tests.py picks by name still runs.
    if ($Fast -and $out -contains 'ALL') {
        $slow = @($realtimeOnly | Where-Object { $_ -ne 'SeaChart' })
        "Fast: left to the nightly Release run: $($slow -join ', ')"
        $selected = @($selected | Where-Object { $slow -notcontains $_ })
    }
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
function Get-Scale([string]$name) { if ($realtimeOnly -contains $name) { 1 } else { $ClockScale } }
$cacheFile = Join-Path $LogDirectory 'passed-cache.jsonl'
$cacheTool = Join-Path $PSScriptRoot 'run_cache.py'
$python = Get-Python
$cached = @()
if (-not $Release -and -not $Force) {
    if ($python) {
        $cached = @(& $python $cacheTool skip --cache $cacheFile --exe $exeHash ($selected | ForEach-Object { "$($_):$(Get-Scale $_)" }))
        if ($LASTEXITCODE -ne 0) { Write-Warning 'run_cache.py failed; every selected script runs'; $cached = @() }
    } else { Write-Warning 'Python not found; scripts that already passed run again' }
}
foreach ($name in $cached) {
    $results.Add([pscustomobject]@{ Script = $name; Result = 'Cached'; ExitCode = 0; Seconds = 0; Slot = 0; ClockScale = (Get-Scale $name) })
    '{0,-20} {1,-6} (passed before with this build)' -f $name, 'Cached'
}
$selected = @($selected | Where-Object { $cached -notcontains $_ })
"Scripts: $($selected -join ', ')"
"Clock scale: $ClockScale  Parallel: $Parallel  Source: $sourceCommit"

function Quote([string]$text) { "'" + $text.Replace("'", "''") + "'" }

function Start-Script([string]$name, [int]$slot) {
    $path = Join-Path $PSScriptRoot "Run-$name.ps1"
    $logs = Join-Path $run $name   # Run-HarbourGuide refuses an existing folder, so create only the parent
    $codeFile = Join-Path $run "$name.exitcode.txt"
    $params = [ordered]@{ Package = $Package; LogDirectory = $logs }
    $parameters = (Get-Command $path).Parameters.Keys
    if ($parameters -contains 'SourceCommit') { $params.SourceCommit = $sourceCommit }
    if ($parameters -contains 'ExpectedExecutableSha256') { $params.ExpectedExecutableSha256 = $exeHash }
    # The child writes its own result to $codeFile. Process.ExitCode is not reliable for a
    # Start-Process -PassThru child with redirected output in Windows PowerShell 5.1.
    $splat = ($params.GetEnumerator() | ForEach-Object { "$($_.Key) = $(Quote $_.Value)" }) -join '; '
    $wrapper = @"
`$ProgressPreference = 'SilentlyContinue'   # progress records would land in err.txt as CLIXML
`$ErrorActionPreference = 'Stop'
`$code = 1
try {
    `$params = @{ $splat }
    `$global:LASTEXITCODE = 0
    & $(Quote $path) @params
    `$code = if (`$LASTEXITCODE) { [int]`$LASTEXITCODE } else { 0 }
} catch {
    [Console]::Error.WriteLine((`$_ | Out-String))
    `$code = 1
} finally {
    [IO.File]::WriteAllText($(Quote $codeFile), [string]`$code)
}
exit `$code
"@
    $encoded = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($wrapper))
    $arguments = @('-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass', '-OutputFormat', 'Text', '-EncodedCommand', $encoded)
    $scale = Get-Scale $name
    $env:FISK_TEST_PORT_BASE = [string](7777 + 100 * $slot)
    $env:FISK_CLOCK_SCALE = [string]$scale
    try {
        $process = Start-Process -FilePath $shell -ArgumentList $arguments -PassThru -WindowStyle Hidden `
            -RedirectStandardOutput (Join-Path $run "$name.out.txt") -RedirectStandardError (Join-Path $run "$name.err.txt")
        $null = $process.Handle   # also helps Process.ExitCode, which is only a fallback
    } finally {
        Remove-Item Env:FISK_TEST_PORT_BASE, Env:FISK_CLOCK_SCALE -ErrorAction SilentlyContinue
    }
    [pscustomobject]@{ Name = $name; Slot = $slot; Scale = $scale; Process = $process; CodeFile = $codeFile; Clock = [Diagnostics.Stopwatch]::StartNew() }
}

function Complete-Script($job) {
    $job.Process.WaitForExit()
    $job.Clock.Stop()
    # Known result: the code the child wrote itself. Otherwise Process.ExitCode, and if that is
    # also missing the result is unknown and counts as a failure (never as exit 0).
    $code = $null
    if (Test-Path -LiteralPath $job.CodeFile) {
        $text = (Get-Content -LiteralPath $job.CodeFile -Raw).Trim()
        $parsed = 0
        if ([int]::TryParse($text, [ref]$parsed)) { $code = $parsed }
    }
    if ($null -eq $code) { $code = $job.Process.ExitCode }
    $err = Join-Path $run "$($job.Name).err.txt"
    $failed = ($null -eq $code) -or $code -ne 0 -or (Test-ErrorOutput $err)
    $row = [pscustomobject]@{
        Script = $job.Name; Result = $(if ($failed) { 'Failed' } else { 'Passed' }); ExitCode = $(if ($null -eq $code) { 'unknown' } else { $code })
        Seconds = [Math]::Round($job.Clock.Elapsed.TotalSeconds); Slot = $job.Slot; ClockScale = $job.Scale
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
$passedNow = @($results | Where-Object Result -eq 'Passed')
if ($python -and $passedNow.Count) {
    & $python $cacheTool record --cache $cacheFile --exe $exeHash --evidence $run ($passedNow | ForEach-Object { "$($_.Script):$($_.ClockScale)" })
    if ($LASTEXITCODE -ne 0) { Write-Warning 'run_cache.py could not record the passed scripts' }
}
$failedCount = @($results | Where-Object Result -eq 'Failed').Count
$serial = ($results | Measure-Object -Property Seconds -Sum).Sum
"Total wall time: $([Math]::Round($wall.Elapsed.TotalSeconds))s (sum of scripts: ${serial}s). Evidence: $run"
if ($ClockScale -ne 1) { "NOTE: clock scale $ClockScale is a quick check. Release evidence requires -Release (normal 20-minute days)." }
if ($failedCount) { throw "$failedCount test script(s) failed. See $run" }
