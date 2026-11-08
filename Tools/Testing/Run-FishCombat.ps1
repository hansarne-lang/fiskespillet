param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$LogDirectory,
    [ValidateSet('land','break','slack')][string]$Case = 'land',
    [switch]$Graphics,
    [switch]$Audio,
    [string]$CaptureDirectory = ''
)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference = 'Stop'
$exe = Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if (!(Test-Path -LiteralPath $exe -PathType Leaf)) { throw 'Packaged executable missing' }
New-Item -ItemType Directory -Path $LogDirectory -Force | Out-Null
$stamp = Get-Date -Format yyyyMMdd-HHmmss
$hostLog = Join-Path $LogDirectory "combat-$Case-host-$stamp.log"
$clientLog = Join-Path $LogDirectory "combat-$Case-client-$stamp.log"
$flags = @('-FiskLanSmoke',"-FiskFightCase=$Case",'-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag)
if (!$Audio) { $flags += '-nosound' }
if ($Graphics) { $flags += @('-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-ExecCmds="t.MaxFPS 30"') }
else { $flags += '-nullrhi' }
$hostCapture = @(); $clientCapture = @()
if ($CaptureDirectory) {
    foreach ($role in 'host','client') { New-Item -ItemType Directory -Path (Join-Path $CaptureDirectory $role) -Force | Out-Null }
    $hostCapture = @(('-FiskScreenshotDir="{0}"' -f (Join-Path $CaptureDirectory 'host')))
    $clientCapture = @(('-FiskScreenshotDir="{0}"' -f (Join-Path $CaptureDirectory 'client')))
}
function Events($log, $pattern) { @(Select-String -LiteralPath $log -Pattern $pattern) }
function Wait-Event($log, $pattern, $process, $seconds) {
    $deadline = (Get-Date).AddSeconds((FiskTimeout $seconds))
    while ((Get-Date) -lt $deadline) {
        if ($process.HasExited) { throw "Process exited before $pattern. Log: $log" }
        if ((Test-Path -LiteralPath $log) -and (Select-String -LiteralPath $log -Pattern $pattern -Quiet)) { return }
        Start-Sleep -Seconds 1
    }
    throw "Timeout: $pattern. Log: $log"
}
function Exactly($log, $pattern, $count) {
    $found = @(Events $log $pattern)
    if ($found.Count -ne $count) { throw "Expected $count events matching $pattern; found $($found.Count). Log: $log" }
}
$clientApp = $null
$hostApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@('/Engine/Maps/Entry?listen',"-port=$(FiskFixedPort 17781)",('-abslog="{0}"' -f $hostLog),('-UserDir="{0}"' -f ($hostLog -replace '\.log$','-profile'))) + $flags + $hostCapture) -PassThru -WindowStyle Hidden
try {
    Wait-Event $hostLog 'FISKELAGET_BOOTSTRAP_READY' $hostApp 90
    $clientApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@("127.0.0.1:$(FiskFixedPort 17781)",('-abslog="{0}"' -f $clientLog),('-UserDir="{0}"' -f ($clientLog -replace '\.log$','-profile'))) + $flags + $clientCapture) -PassThru -WindowStyle Hidden
    Wait-Event $hostLog 'FISKELAGET_MARINA_REACHED crew=2' $hostApp 700
    Wait-Event $clientLog 'FISKELAGET_MARINA_REACHED crew=2' $clientApp 20
    Wait-Event $hostLog 'FISKELAGET_TEST_PRIVATE revealed=0 rows=0' $hostApp 5
    Wait-Event $clientLog 'FISKELAGET_TEST_PRIVATE revealed=0 rows=0' $clientApp 5
    foreach ($log in $hostLog,$clientLog) {
        Exactly $log 'FISKELAGET_TEST_PARALLEL_FIGHTS player=' 1
        Exactly $log 'FISKELAGET_TEST_EARLY_LANDING player=' 1
        Exactly $log 'FISKELAGET_TEST_PRIVATE revealed=0 rows=0' 1
        if (@(Events $log 'Fatal error|Assertion failed|Unhandled Exception').Count) { throw "Runtime failure in $log" }
    }
    $landCount = if ($Case -eq 'land') { 2 } else { 1 }
    Exactly $hostLog 'FISKELAGET_FISH_LANDED player=' $landCount
    $landings = @(Events $hostLog 'FISKELAGET_FISH_LANDED player=(\d+)')
    foreach ($landing in $landings) {
        $id = $landing.Matches[0].Groups[1].Value
        Exactly $hostLog "FISKELAGET_FIGHT_STARTED player=$id$" 1
        Exactly $hostLog "FISKELAGET_FISH_READY player=$id$" 1
        Exactly $hostLog "FISKELAGET_FISH_LANDED player=$id$" 1
        $fight = @(Events $hostLog "FISKELAGET_FIGHT_STARTED player=$id$")[0]
        $ready = @(Events $hostLog "FISKELAGET_FISH_READY player=$id$")[0]
        if ($fight.LineNumber -ge $ready.LineNumber -or $ready.LineNumber -ge $landing.LineNumber) { throw 'Invalid fight transition order' }
        if (@(Events $hostLog "FISKELAGET_FISH_RUN player=$id$").Count -lt 1) { throw 'No new fish run' }
        Exactly $hostLog "FISKELAGET_DRAG player=$id percent=30$" 1
        Exactly $hostLog "FISKELAGET_DRAG player=$id percent=40$" 1
    }
    Exactly $hostLog 'FISKELAGET_TEST_DUPLICATE_LANDING player=' 1
    if ($Case -eq 'land') {
        Exactly $hostLog 'FISKELAGET_FISH_LOST player=' 0
        Exactly $clientLog 'FISKELAGET_TEST_DUPLICATE_LANDING player=' 1
        $ids = @($landings | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
        if ($ids.Count -ne 2) { throw 'Both landings must belong to distinct players' }
    }
    else {
        $reason = if ($Case -eq 'break') { 'LineBreak' } else { 'Slack' }
        Exactly $hostLog "FISKELAGET_FISH_LOST player=\d+ reason=$reason$" 1
        Exactly $clientLog "FISKELAGET_FISH_LOST player=\d+ reason=$reason$" 1
        Exactly $clientLog 'FISKELAGET_TEST_LOSS_LANDING player=' 1
        Exactly $clientLog 'FISKELAGET_FISH_LANDED player=' 0
        $loss = @(Events $hostLog "FISKELAGET_FISH_LOST player=\d+ reason=$reason$")[0]
        if ($loss.LineNumber -ge $landings[0].LineNumber) { throw 'Surviving fight must continue after the other player loses' }
    }
    Exactly $clientLog 'FISKELAGET_TEST_GUEST_RETURN player=' 1
    $captain = @(Events $hostLog 'FISKELAGET_TEST_CAPTAIN_RETURN player=')
    $return = @(Events $hostLog 'FISKELAGET_RETURN_UNDERWAY crew=2')
    if ($captain.Count -ne 1 -or $return.Count -ne 1 -or $captain[0].LineNumber -ge $return[0].LineNumber) { throw 'Only captain may return' }
    if ($Graphics -and $Case -eq 'land') {
        foreach ($log in $hostLog,$clientLog) {
            foreach ($seat in 0,1) {
                foreach ($event in 'FISH_VISUAL_NEAR','FISH_VISUAL_LANDING','FISH_VISUAL_FINISHED','FISH_SPLASH') {
                    Exactly $log "FISKELAGET_$event seat=$seat$" 1
                }
            }
        }
        if ($CaptureDirectory) {
            foreach ($role in 'host','client') {
                foreach ($file in 'fish-near-boat.png','fish-landing.png') {
                    $shot = Join-Path (Join-Path $CaptureDirectory $role) $file
                    if (!(Test-Path -LiteralPath $shot) -or (Get-Item -LiteralPath $shot).Length -lt 1000) { throw "Missing screenshot: $shot" }
                }
            }
        }
    }
    if ($Audio) { foreach ($log in $hostLog,$clientLog) { Exactly $log 'FISKELAGET_REEL_AUDIO_CONSUMED' 1 } }
    "PASS: case=$Case; simultaneous fights, expected independent outcomes, no duplicate landing, captain return, private results."
    "Host log: $hostLog"
    "Client log: $clientLog"
}
finally {
    if ($clientApp -and !$clientApp.HasExited) { Stop-Process -InputObject $clientApp }
    if (!$hostApp.HasExited) { Stop-Process -InputObject $hostApp }
}
