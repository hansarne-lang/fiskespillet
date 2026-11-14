param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$LogDirectory,
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
$hostLog = Join-Path $LogDirectory "repeat-host-$stamp.log"
$clientLog = Join-Path $LogDirectory "repeat-client-$stamp.log"
$flags = @('-FiskLanSmoke','-FiskRepeatSmoke','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,'-ExecCmds="t.MaxFPS 30"')
if (!$Audio) { $flags += '-nosound' }
if ($Graphics) { $flags += @('-windowed','-ResX=1280','-ResY=720','-RenderOffscreen') }
else { $flags += '-nullrhi' }
$hostCapture = @(); $clientCapture = @()
if ($CaptureDirectory) {
    foreach ($role in 'host','client') { New-Item -ItemType Directory -Path (Join-Path $CaptureDirectory $role) -Force | Out-Null }
    $hostCapture = @(('-FiskScreenshotDir="{0}"' -f (Join-Path $CaptureDirectory 'host')))
    $clientCapture = @(('-FiskScreenshotDir="{0}"' -f (Join-Path $CaptureDirectory 'client')))
}
function Events($log,$pattern) { @(Select-String -LiteralPath $log -Pattern $pattern) }
function Exactly($log,$pattern,$count) {
    $found = @(Events $log $pattern)
    if ($found.Count -ne $count) { throw "Expected $count events for $pattern; got $($found.Count). Log: $log" }
}
function Wait-Event($log,$pattern,$process,$seconds) {
    $deadline = (Get-Date).AddSeconds((FiskTimeout $seconds))
    while ((Get-Date) -lt $deadline) {
        if ($process.HasExited) { throw "Process exited before $pattern. Log: $log" }
        if (Test-Path -LiteralPath $log) {
            if (Select-String -LiteralPath $log -Pattern 'FISKELAGET_REPEAT_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet) { throw "Runtime failure in $log" }
            if (Select-String -LiteralPath $log -Pattern $pattern -Quiet) { return }
        }
        Start-Sleep -Seconds 1
    }
    throw "Timeout: $pattern. Log: $log"
}
function Time-Of($event) {
    if ($event.Line -notmatch '^\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]') { throw 'Missing timestamp' }
    [DateTime]::ParseExact($Matches[1],'yyyy.MM.dd-HH.mm.ss:fff',[Globalization.CultureInfo]::InvariantCulture)
}
$clientApp = $null
$hostApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@('/Engine/Maps/Entry?listen',"-port=$(FiskFixedPort 17782)",('-abslog="{0}"' -f $hostLog),('-UserDir="{0}"' -f ($hostLog -replace '\.log$','-profile'))) + $flags + $hostCapture) -PassThru -WindowStyle Hidden
try {
    Wait-Event $hostLog 'FISKELAGET_BOOTSTRAP_READY' $hostApp 90
    $clientApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@("127.0.0.1:$(FiskFixedPort 17782)",('-abslog="{0}"' -f $clientLog),('-UserDir="{0}"' -f ($clientLog -replace '\.log$','-profile'))) + $flags + $clientCapture) -PassThru -WindowStyle Hidden
    Wait-Event $hostLog 'FISKELAGET_DAY_STARTED' $hostApp 90
    Wait-Event $hostLog 'FISKELAGET_MARINA_REACHED crew=2' $hostApp 1000
    Wait-Event $clientLog 'FISKELAGET_MARINA_REACHED crew=2' $clientApp 20
    Wait-Event $hostLog 'FISKELAGET_TEST_DAY_END ms=1200000' $hostApp 1250
    Wait-Event $clientLog 'FISKELAGET_TEST_DAY_END ms=1200000' $clientApp 20
    Exactly $hostLog 'FISKELAGET_CATCH_RECORDED player=' 4
    Exactly $hostLog 'FISKELAGET_FISH_LANDED player=' 4
    $records = @(Events $hostLog 'FISKELAGET_CATCH_RECORDED player=(\d+) catch=([0-9A-Fa-f]{32}) cast=([0-9A-Fa-f]{32})')
    $catchIds = @($records | ForEach-Object { $_.Matches[0].Groups[2].Value } | Sort-Object -Unique)
    $castIds = @($records | ForEach-Object { $_.Matches[0].Groups[3].Value } | Sort-Object -Unique)
    $players = @($records | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
    if ($catchIds.Count -ne 4 -or $castIds.Count -ne 4 -or $players.Count -ne 2) { throw 'Expected four distinct catches/casts belonging to two players' }
    foreach ($id in $players) {
        Exactly $hostLog "FISKELAGET_CATCH_RECORDED player=$id " 2
        Exactly $hostLog "FISKELAGET_ROD_CYCLE player=$id cast=[0-9A-Fa-f]{32} state=1$" $(if($id -eq $players[0]){3}else{4})
        $first = @($records | Where-Object { $_.Matches[0].Groups[1].Value -eq $id })[0]
        $old = $first.Matches[0].Groups[3].Value
        foreach ($action in 0,2,6,7) { Exactly $hostLog "FISKELAGET_ROD_COMMAND_REJECTED player=$id action=$action expected=$old current=(?!$old)[0-9A-Fa-f]{32}$" $(if($action -eq 7){2}else{1}) }
        if (@(Events $hostLog "FISKELAGET_CAST_REJECTED player=$id expected=$old current=(?!$old)[0-9A-Fa-f]{32}$").Count -lt 1) { throw 'Stale cast not rejected' }
        $ready = @(Events $hostLog "FISKELAGET_ROD_CYCLE player=$id cast=[0-9A-Fa-f]{32} state=1$")[-1]
        if ((FiskGameSpan (Time-Of $first) (Time-Of $ready)) -lt 2.9) { throw 'New cast started before landing animation ended' }
    }
    Exactly $hostLog 'FISKELAGET_ROD_CANCELLED player=' 3
    Exactly $hostLog 'FISKELAGET_REPEAT_CANCEL_FIGHT player=' 0
    Exactly $clientLog 'FISKELAGET_REPEAT_CANCEL_FIGHT player=' 1
    $cancelled = @(Events $hostLog 'FISKELAGET_ROD_CYCLE player=\d+ cast=([0-9A-Fa-f]{32}) state=7$')
    foreach ($event in $cancelled) { if ($castIds -contains $event.Matches[0].Groups[1].Value) { throw 'Cancelled cast credited as catch' } }
    foreach ($log in $hostLog,$clientLog) {
        Exactly $log 'FISKELAGET_REPEAT_CANCEL_WAITING player=' 1
        Exactly $log 'FISKELAGET_REPEAT_PARALLEL player=' 1
        Exactly $log 'FISKELAGET_REPEAT_LANDED player=\d+ ordinal=1 ' 1
        Exactly $log 'FISKELAGET_REPEAT_LANDED player=\d+ ordinal=2 ' 1
        Exactly $log 'FISKELAGET_REPEAT_EARLY_CAST player=' 1
        Exactly $log 'FISKELAGET_REPEAT_STALE_ACTIONS player=' 1
        Exactly $log 'FISKELAGET_REPEAT_STALE_READY player=' 1
        Exactly $log 'FISKELAGET_REPEAT_STALE_READY_SURVIVED player=' 1
        Exactly $log 'FISKELAGET_TEST_PRIVATE revealed=0 rows=0' 1
        Exactly $log 'FISKELAGET_SETTLEMENT rows=2' 1
        Exactly $log 'FISKELAGET_RESULT player=\d+ delivered=2 lost=0 balance=\d+$' 2
        foreach($result in @(Events $log 'FISKELAGET_RESULT player=(\d+) delivered=2 lost=0 balance=(\d+)$')){
            $playerId=$result.Matches[0].Groups[1].Value
            $details=@(Events $log "FISKELAGET_REVEALED_CATCH player=$playerId species=\d+ grams=\d+ mm=\d+ value=(\d+) delivered=1")
            $sum=0;foreach($detail in $details){$sum += [int]$detail.Matches[0].Groups[1].Value}
            if($details.Count -ne 2 -or $sum -ne [int]$result.Matches[0].Groups[2].Value){throw 'Actual catch values do not match payment'}
        }
        $start = @(Events $hostLog 'FISKELAGET_DAY_STARTED')[0]
        $settle = @(Events $log 'FISKELAGET_SETTLEMENT rows=2')[0]
        $end = @(Events $log 'FISKELAGET_TEST_DAY_END ms=1200000')[0]
        if ([Math]::Abs((FiskGameSpan (Time-Of $start) (Time-Of $settle)) - 1020) -gt (2.0*$FiskClockScale)) { throw 'Settlement time drift' }
        if ([Math]::Abs((FiskGameSpan (Time-Of $start) (Time-Of $end)) - 1200) -gt (2.0*$FiskClockScale)) { throw 'Day length drift' }
        foreach ($sample in @(Events $log 'FISKELAGET_TEST_CLOCK ms=(\d+)')) {
            $game = [int]$sample.Matches[0].Groups[1].Value / 1000.0
            if ([Math]::Abs((FiskGameSpan (Time-Of $start) (Time-Of $sample)) - $game) -gt (2.0*$FiskClockScale)) { throw 'Clock sample drift' }
        }
    }
    $cap = @(Events $hostLog 'FISKELAGET_REPEAT_CAPTAIN_RETURN player=')
    $ret = @(Events $hostLog 'FISKELAGET_RETURN_UNDERWAY crew=2')
    if ($cap.Count -ne 1 -or $ret.Count -ne 1 -or $cap[0].LineNumber -ge $ret[0].LineNumber) { throw 'Captain return missing' }
    if ($Graphics -and $CaptureDirectory) {
        foreach ($role in 'host','client') {
            $shots = @(Get-ChildItem -LiteralPath (Join-Path $CaptureDirectory $role) -Filter 'fish-landing-*.png')
            if ($shots.Count -ne 2 -or @($shots | Where-Object Length -lt 1000).Count) { throw 'Expected two landing images per player' }
        }
    }
    if ($Audio) { foreach ($log in $hostLog,$clientLog) { Exactly $log 'FISKELAGET_REEL_AUDIO_CONSUMED' 1 } }
    'PASS: four unique catches; repeated casting, waiting/fight cancellation, stale-command rejection, 17:00 settlement and 20-minute day.'
    "Host log: $hostLog"
    "Client log: $clientLog"
}
finally {
    if ($clientApp -and !$clientApp.HasExited) { Stop-Process -InputObject $clientApp }
    if (!$hostApp.HasExited) { Stop-Process -InputObject $hostApp }
}
