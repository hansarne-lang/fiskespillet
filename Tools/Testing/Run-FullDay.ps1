param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$LogDirectory,
    [switch]$Graphics,
    [ValidateRange(0,120)][int]$ClockOnlySeconds = 0
)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference = 'Stop'
$exe = Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Missing packaged executable: $exe" }
New-Item -ItemType Directory -Path $LogDirectory -Force | Out-Null
$stamp = Get-Date -Format yyyyMMdd-HHmmss
$hostLog = Join-Path $LogDirectory "full-day-host-$stamp.log"
$clientLog = Join-Path $LogDirectory "full-day-client-$stamp.log"
$flags = @('-FiskLanSmoke','-unattended','-nosplash','-nosound','-multihome=127.0.0.1',$FiskClockFlag)
if ($Graphics) { $flags += @('-windowed','-ResX=800','-ResY=600','-RenderOffscreen') }
else { $flags += '-nullrhi' }

function Wait-Event($log, $pattern, $process, $seconds) {
    $deadline = (Get-Date).AddSeconds((FiskTimeout $seconds))
    while ((Get-Date) -lt $deadline) {
        if ($process.HasExited) { throw "Exited before $pattern. Log: $log" }
        if ((Test-Path -LiteralPath $log) -and (Select-String -LiteralPath $log -Pattern $pattern -Quiet)) { return }
        Start-Sleep -Seconds 1
    }
    throw "Timed out waiting for $pattern. Log: $log"
}

function Events($log, $pattern) { @(Select-String -LiteralPath $log -Pattern $pattern) }
function Event-Time($event) {
    if ($event.Line -notmatch '^\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]') { throw 'Missing log timestamp' }
    [DateTime]::ParseExact($Matches[1], 'yyyy.MM.dd-HH.mm.ss:fff', [Globalization.CultureInfo]::InvariantCulture)
}

$hostApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@('/Engine/Maps/Entry?listen',"-port=$(FiskFixedPort 17780)",('-abslog="{0}"' -f $hostLog)) + $flags) -PassThru -WindowStyle Hidden
try {
    Wait-Event $hostLog 'FISKELAGET_BOOTSTRAP_READY' $hostApp 90
    $clientApp = Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@("127.0.0.1:$(FiskFixedPort 17780)",('-abslog="{0}"' -f $clientLog)) + $flags) -PassThru -WindowStyle Hidden
    Wait-Event $hostLog 'FISKELAGET_DAY_STARTED' $hostApp 90
    if ($ClockOnlySeconds -gt 0) {
        $stopwatch = [Diagnostics.Stopwatch]::StartNew()
        while ($stopwatch.Elapsed.TotalSeconds -lt $ClockOnlySeconds) {
            if ($hostApp.HasExited -or $clientApp.HasExited) { throw 'Process exited during clock sample' }
            Start-Sleep -Seconds 1
        }
    }
    else {
        Wait-Event $hostLog 'FISKELAGET_TEST_DAY_END ms=1200000' $hostApp 1250
        Wait-Event $clientLog 'FISKELAGET_TEST_DAY_END ms=1200000' $clientApp 20
    }

    $start = @(Events $hostLog 'FISKELAGET_DAY_STARTED')[0]
    $startTime = Event-Time $start
    $samples = @(Events $hostLog 'FISKELAGET_TEST_CLOCK ms=(\d+)')
    if ($samples.Count -lt 2) { throw 'Insufficient clock samples' }
    foreach ($sample in $samples) {
        $seconds = [int]$sample.Matches[0].Groups[1].Value / 1000.0
        $actual = (FiskGameSeconds ((Event-Time $sample) - $startTime).TotalSeconds)
        if ([Math]::Abs($actual - $seconds) -gt (1.0*$FiskClockScale)) { throw "Clock drift: game=$seconds wall=$actual" }
    }
    $lastHost = $samples[-1]
    $clientSamples = @(Events $clientLog 'FISKELAGET_TEST_CLOCK ms=(\d+)')
    if ($clientSamples.Count -lt 2) { throw 'Insufficient client clock samples' }
    foreach ($sample in $clientSamples) {
        $seconds = [int]$sample.Matches[0].Groups[1].Value / 1000.0
        $actual = (FiskGameSeconds ((Event-Time $sample) - $startTime).TotalSeconds)
        if ([Math]::Abs($actual - $seconds) -gt (1.0*$FiskClockScale)) { throw "Client clock drift: game=$seconds wall=$actual" }
    }
    if ($ClockOnlySeconds -gt 0) {
        'PASS: server and client clock samples track elapsed wall time.'
        return
    }

    $landed = @(Events $hostLog 'FISKELAGET_FISH_LANDED player=(\d+)')
    $ids = @($landed | ForEach-Object { $_.Matches[0].Groups[1].Value } | Sort-Object -Unique)
    if ($landed.Count -ne 2 -or $ids.Count -ne 2) { throw 'Expected exactly one server landing per player' }
    foreach ($id in $ids) {
        $fight = @(Events $hostLog "FISKELAGET_FIGHT_STARTED player=$id$")
        $ready = @(Events $hostLog "FISKELAGET_FISH_READY player=$id$")
        $caught = @(Events $hostLog "FISKELAGET_FISH_LANDED player=$id$")
        if ($fight.Count -ne 1 -or $ready.Count -ne 1 -or $fight[0].LineNumber -ge $ready[0].LineNumber -or $ready[0].LineNumber -ge $caught[0].LineNumber) { throw "Bad landing sequence for $id" }
    }
    foreach ($log in $hostLog,$clientLog) {
        foreach ($event in 'EARLY_LANDING','DUPLICATE_LANDING') {
            if (@(Events $log "FISKELAGET_TEST_${event} player=").Count -ne 1) { throw "Missing $event in $log" }
        }
        if (@(Events $log 'FISKELAGET_TEST_PRIVATE revealed=0 rows=0').Count -ne 1) { throw 'Early result exposure' }
        if (@(Events $log 'FISKELAGET_SETTLEMENT rows=2').Count -ne 1) { throw 'Missing or repeated settlement' }
        $results = @(Events $log 'FISKELAGET_RESULT player=(\d+) delivered=1 lost=0 balance=(\d+)$')
        if ($results.Count -ne 2) { throw 'Incorrect settlement results' }
        foreach($result in $results){
            $playerId=$result.Matches[0].Groups[1].Value
            $details=@(Events $log "FISKELAGET_REVEALED_CATCH player=$playerId species=\d+ grams=\d+ mm=\d+ value=(\d+) delivered=1")
            if($details.Count -ne 1 -or [int]$details[0].Matches[0].Groups[1].Value -ne [int]$result.Matches[0].Groups[2].Value){throw 'Catch value does not match payment'}
        }
        $revealed = @(Events $log 'FISKELAGET_SETTLEMENT rows=2')[0]
        if ((FiskGameSeconds ((Event-Time $revealed) - $startTime).TotalSeconds) -lt 1020.0) { throw 'Result revealed before 17:00' }
        if (@(Events $log 'FISKELAGET_MARINA_REACHED crew=2').Count -ne 1) { throw 'Both players did not return' }
        $end = @(Events $log 'FISKELAGET_TEST_DAY_END ms=1200000')[0]
        $duration = (FiskGameSeconds ((Event-Time $end) - $startTime).TotalSeconds)
        if ([Math]::Abs($duration - 1200.0) -gt (2.0*$FiskClockScale)) { throw "Incorrect day length: $duration" }
    }
    if (@(Events $clientLog 'FISKELAGET_TEST_GUEST_RETURN player=').Count -ne 1) { throw 'Guest did not try return' }
    $captain = @(Events $hostLog 'FISKELAGET_TEST_CAPTAIN_RETURN player=')
    $return = @(Events $hostLog 'FISKELAGET_RETURN_UNDERWAY crew=2')
    if ($captain.Count -ne 1 -or $return.Count -ne 1 -or $captain[0].LineNumber -ge $return[0].LineNumber) { throw 'Unauthorized return' }
    'PASS: two landings, private results until 17:00, one settlement, captain return and 20-minute wall-clock day.'
    "Logs: $hostLog ; $clientLog"
}
finally {
    if ($clientApp -and -not $clientApp.HasExited) { Stop-Process -InputObject $clientApp }
    if (-not $hostApp.HasExited) { Stop-Process -InputObject $hostApp }
}
