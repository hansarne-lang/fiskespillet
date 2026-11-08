param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log'
function Wait-Event($log,$pattern,$app,$seconds){
 $deadline=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $deadline){
  if($app.HasExited){throw "Process exited before $pattern. $log"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_RIG_TEST_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $pattern -Quiet){return}
  }
  Start-Sleep -Seconds 1
 }
 throw "Timeout $pattern. $log"
}
function Start-Player($role,$url,$log){
 $capture=Join-Path $run $role
 New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $flags=@($url,'-FiskRigSmoke','-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,"-port=$FiskPort0",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run "$role-settings")))
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
}
$hostApp=$null;$clientApp=$null
try{
 if(@(Get-NetUDPEndpoint -LocalPort $FiskPort0 -ErrorAction SilentlyContinue).Count){throw "Port $FiskPort0 is occupied"}
 $hostApp=Start-Player 'host' '/Engine/Maps/Entry?listen' $hostLog
 Wait-Event $hostLog "GameNetDriver.*listening on port $FiskPort0" $hostApp 120
 $clientApp=Start-Player 'client' "127.0.0.1:$FiskPort0" $clientLog
 Wait-Event $hostLog 'FISKELAGET_RIG_TEST_PASSED' $hostApp 700
 Wait-Event $clientLog 'FISKELAGET_RIG_TEST_PASSED' $clientApp 120
 foreach($log in $hostLog,$clientLog){
  foreach($tag in 'RIG_MENU_PASSED','RIG_RECAST_PASSED','RIG_TEST_PASSED'){
   if(@(Select-String -LiteralPath $log -Pattern "FISKELAGET_$tag").Count -ne 1){throw "Wrong count $tag in $log"}
  }
 }
 foreach($id in 256,257){
  if(@(Select-String -LiteralPath $hostLog -Pattern "FISKELAGET_RIG_REJECTED player=$id ").Count -ne 5){throw "Expected five rejected edits for player $id"}
  if(@(Select-String -LiteralPath $hostLog -Pattern "FISKELAGET_CATCH_RECORDED player=$id ").Count -ne 1){throw "Expected one landing for player $id"}
 }
 foreach($role in 'host','client'){
  foreach($name in 'rig-ready','rig-after-withdraw','rig-fight','rig-landed','rig-after-landing'){
   $path=Join-Path (Join-Path $run $role) "$name.png"
   if(!(Test-Path -LiteralPath $path) -or (Get-Item -LiteralPath $path).Length -lt 1000){throw "Missing screenshot $path"}
  }
 }
 'PASS: two independent server-approved rigs, native panel navigation/input isolation, clock continuity, invalid/stale/active edit rejection, withdrawal/recast, real random bites, combat and landing with settings retained.'
 "Evidence: $run"
}
finally{foreach($app in $clientApp,$hostApp){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}}
