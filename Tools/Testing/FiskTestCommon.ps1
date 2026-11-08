# Shared settings for Tools/Testing/Run-*.ps1. Dot-source it right after param().
#
# FISK_TEST_PORT_BASE  First UDP port for this run (default 7777). Run-All.ps1 gives each
#                      parallel slot its own base so several scripts can run at once.
# FISK_CLOCK_SCALE     Test-only day clock acceleration, 1-20 (default 1 = normal 20-minute day).
#                      Passed to the game as -FiskClockScale=N. Release evidence uses 1.
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
