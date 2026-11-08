param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss);New-Item -ItemType Directory -Path $run -Force | Out-Null
$hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log'
function Wait-Event($log,$pattern,$app,$seconds){
 $deadline=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $deadline){
  if($app.HasExited){throw "Process exited before $pattern. $log"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_CAPTAIN_TEST_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $pattern -Quiet){return}
  };Start-Sleep -Seconds 1
 };throw "Timeout $pattern. $log"
}
function Exactly($log,$pattern,$count){if(@(Select-String -LiteralPath $log -Pattern $pattern).Count -ne $count){throw "Wrong count $pattern. $log"}}
function Start-Player($role,$url,$log){
 $capture=Join-Path $run $role;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $flags=@($url,'-FiskCaptainSmoke','-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,"-port=$FiskPort0",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run "$role-settings")))
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
}
$hostApp=$null;$clientApp=$null
try{
 if(@(Get-NetUDPEndpoint -LocalPort $FiskPort0 -ErrorAction SilentlyContinue).Count){throw "Port $FiskPort0 is occupied"}
 $hostApp=Start-Player 'host' '/Engine/Maps/Entry?listen' $hostLog
 Wait-Event $hostLog "GameNetDriver.*listening on port $FiskPort0" $hostApp 120
 $clientApp=Start-Player 'client' "127.0.0.1:$FiskPort0" $clientLog
 Wait-Event $hostLog 'FISKELAGET_CAPTAIN_TEST_PASSED' $hostApp 300
 Wait-Event $clientLog 'FISKELAGET_CAPTAIN_TEST_PASSED' $clientApp 30
 Start-Sleep -Seconds 2
 Exactly $hostLog 'FISKELAGET_CAPTAIN_TRANSFER_ACCEPTED' 5
 Exactly $hostLog 'FISKELAGET_CAPTAIN_TRANSFER_REJECTED' 7
 # Since v0.21 R at sea goes to the return button: a passenger is refused on the client (no RPC),
 # and the captain's return is logged as RETURN_BUTTON instead of CAPTAIN_TRIP_AUTHORIZED (#123).
 Exactly $hostLog 'FISKELAGET_CAPTAIN_TRIP_REJECTED' 2
 Exactly $hostLog 'FISKELAGET_CAPTAIN_TRIP_AUTHORIZED' 1
 Exactly $hostLog 'FISKELAGET_RETURN_BUTTON accepted=1' 1
 Exactly $hostLog 'FISKELAGET_RETURN_BUTTON accepted=0' 0
 Exactly $hostLog 'FISKELAGET_CHART_ACCEPTED' 1
 Exactly $hostLog 'FISKELAGET_CHART_REJECTED' 1
 Exactly $hostLog 'FISKELAGET_CAPTAIN_FORMER_DENIED' 1
 Exactly $hostLog 'FISKELAGET_CAPTAIN_OTHER_MENU_DID_NOT_BLOCK' 1
 Exactly $hostLog 'FISKELAGET_ROD_FISHING player=' 2
 foreach($role in 'host','client'){
  foreach($name in 'crew-initial','crew-guest-captain','captain-route','crew-moving','crew-fishing','crew-return-guest','crew-home'){
   $path=Join-Path (Join-Path $run $role) "$name.png";if(!(Test-Path -LiteralPath $path) -or (Get-Item -LiteralPath $path).Length -lt 1000){throw "Missing screenshot $path"}
  }
 }
 Stop-Process -InputObject $clientApp;$clientApp.WaitForExit()
 Wait-Event $hostLog 'FISKELAGET_CAPTAIN_FALLBACK_PASSED' $hostApp 30
 Start-Sleep -Seconds 2
 if(!(Test-Path -LiteralPath (Join-Path $run 'host\crew-fallback.png'))){throw 'Missing fallback screenshot'}
 'PASS: five captain handoffs at dock and during both travel legs, shared role/route, stale and unauthorized commands rejected, two passive rods, unpaused crew menu, old captain cannot block return, and disconnect fallback.'
 "Evidence: $run"
}
finally{foreach($app in $clientApp,$hostApp){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}}
