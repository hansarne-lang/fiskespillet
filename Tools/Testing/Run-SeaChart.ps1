param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log'
function Wait-Event($log,$pattern,$app,$seconds){
 $deadline=(Get-Date).AddSeconds($seconds)
 while((Get-Date) -lt $deadline){
  if($app.HasExited){throw "Process exited before $pattern. $log"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_CHART_TEST_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $pattern -Quiet){return}
  }
  Start-Sleep -Seconds 1
 }
 throw "Timeout $pattern. $log"
}
function Start-Player($role,$url,$log){
 $capture=Join-Path $run $role;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $flags=@($url,'-FiskChartSmoke','-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run "$role-settings")))
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
}
$hostApp=$null;$clientApp=$null
try{
 if(@(Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue).Count){throw 'Port 7777 is occupied'}
 $hostApp=Start-Player 'host' '/Engine/Maps/Entry?listen' $hostLog
 Wait-Event $hostLog 'GameNetDriver.*listening on port 7777' $hostApp 120
 $clientApp=Start-Player 'client' '127.0.0.1:7777' $clientLog
 Wait-Event $hostLog 'FISKELAGET_CHART_TEST_PASSED' $hostApp 700
 Wait-Event $clientLog 'FISKELAGET_CHART_TEST_PASSED' $clientApp 90
 foreach($log in $hostLog,$clientLog){
  foreach($tag in 'CHART_SELECTION_PASSED','CHART_TRAVEL_PASSED','CHART_TEST_PASSED'){
   if(@(Select-String -LiteralPath $log -Pattern "FISKELAGET_$tag").Count -ne 1){throw "Wrong count $tag in $log"}
  }
 }
 foreach($id in 256,257){
  $expected=if($id -eq 256){5}else{4}
  if(@(Select-String -LiteralPath $hostLog -Pattern "FISKELAGET_CHART_REJECTED player=$id ").Count -ne $expected){throw "Wrong route rejection count for $id"}
  if(@(Select-String -LiteralPath $hostLog -Pattern "FISKELAGET_RIG_REJECTED player=$id ").Count -ne 1){throw "Expected invalid reef depth rejection for $id"}
  if(@(Select-String -LiteralPath $hostLog -Pattern "FISKELAGET_CATCH_RECORDED player=$id ").Count -ne 1){throw "Expected one landing for $id"}
 }
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_CHART_ACCEPTED').Count -ne 1){throw 'Expected one captain selection'}
 foreach($role in 'host','client'){
  foreach($name in 'chart-initial','chart-selected','rig-dropoff','chart-underway','chart-fight','chart-landed','chart-return','chart-home'){
   $path=Join-Path (Join-Path $run $role) "$name.png"
   if(!(Test-Path -LiteralPath $path) -or (Get-Item -LiteralPath $path).Length -lt 1000){throw "Missing screenshot $path"}
  }
 }
 'PASS: captain-only replicated destination, stale/invalid/active route rejection, chart input isolation, ground-specific rigs, measured 40-second shared travel, real bites, two landings and measured shared return with marina crossing.'
 "Evidence: $run"
}
finally{foreach($app in $clientApp,$hostApp){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}}
