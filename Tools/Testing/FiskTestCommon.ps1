# Shared settings for Tools/Testing/Run-*.ps1. Dot-source it right after param().
#
# FISK_TEST_PORT_BASE  First UDP port for this run (default 7777). Run-All.ps1 gives each
#                      parallel slot its own base so several scripts can run at once.
# FISK_CLOCK_SCALE     Test-only day clock acceleration, 1-20 (default 1 = normal 20-minute day).
#                      Passed to the game as -FiskClockScale=N. Release evidence uses 1.
# FISK_SKIP_IDLE       1 = the game's host jumps from the moment every boat is home to 17:00 (PR runs).
#                      Read by the game itself from the environment. Release evidence never sets it.
#
# Default values keep every script byte-for-byte equivalent to the previous behaviour.

$FiskPortBase = 7777
if ($env:FISK_TEST_PORT_BASE) {
    $parsedPort = 0
    if (-not [int]::TryParse($env:FISK_TEST_PORT_BASE, [ref]$parsedPort) -or $parsedPort -lt 1024 -or $parsedPort -gt 65000) {
        throw "FISK_TEST_PORT_BASE must be an integer 1024-65000, got '$($env:FISK_TEST_PORT_BASE)'"
    }
    $FiskPortBase = $parsedPort
}
$FiskPort0 = $FiskPortBase
$FiskPort1 = $FiskPortBase + 1
$FiskPort2 = $FiskPortBase + 2

$FiskClockScale = 1
if ($env:FISK_CLOCK_SCALE) {
    $parsedScale = 0
    if (-not [int]::TryParse($env:FISK_CLOCK_SCALE, [ref]$parsedScale) -or $parsedScale -lt 1 -or $parsedScale -gt 20) {
        throw "FISK_CLOCK_SCALE must be an integer 1-20, got '$($env:FISK_CLOCK_SCALE)'"
    }
    $FiskClockScale = $parsedScale
}
$FiskClockFlag = "-FiskClockScale=$FiskClockScale"

# Fixed-port scripts (17780+) get their offset from the same base so parallel slots never collide.
function FiskFixedPort([int]$port) { $port + ($FiskPortBase - 7777) }

# Timeouts that cover game-clock waits shrink with the clock scale. Short waits (start-up,
# connection, single actions) are wall-clock bound and are left unchanged.
function FiskTimeout([double]$seconds) {
    if ($FiskClockScale -le 1 -or $seconds -lt 300) { return $seconds }
    [Math]::Max(300, [Math]::Ceiling($seconds / $FiskClockScale) + 120)
}

# Wall-clock seconds measured from log timestamps converted to game seconds.
function FiskGameSeconds([double]$wallSeconds) { $wallSeconds * $FiskClockScale }

$FiskSkipIdle = $env:FISK_SKIP_IDLE -eq '1'
# Game seconds between two log timestamps. With FISK_SKIP_IDLE the host jumps over waiting time and logs
# FISKELAGET_CLOCK_SKIPPED ... skipped_ms=N. Jumps logged between the two times are added back, so the
# checks that a day was not shortened still hold. Reads every *.log under the script's $LogDirectory.
function FiskGameSpan([datetime]$from, [datetime]$to) {
    $skipped = 0.0
    if ($FiskSkipIdle -and $LogDirectory -and (Test-Path -LiteralPath $LogDirectory)) {
        $pattern = '\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\].*FISKELAGET_CLOCK_SKIPPED .*skipped_ms=(\d+)'
        foreach ($hit in @(Get-ChildItem -LiteralPath $LogDirectory -Recurse -File -Filter '*.log' | Select-String -Pattern $pattern)) {
            $at = [datetime]::ParseExact($hit.Matches[0].Groups[1].Value, 'yyyy.MM.dd-HH.mm.ss:fff', [Globalization.CultureInfo]::InvariantCulture)
            if ($at -gt $from -and $at -le $to) { $skipped += [double]$hit.Matches[0].Groups[2].Value / 1000.0 }
        }
    }
    (FiskGameSeconds ($to - $from).TotalSeconds) + $skipped
}
