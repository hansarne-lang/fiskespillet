param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
if(@(Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue).Count){throw 'Port 7777 is occupied'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$log=Join-Path $run 'solo.log'
$flags=@('-FiskMenuSmoke=solo','-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $run),('-UserDir="{0}"' -f (Join-Path $run 'settings')))
$app=$null
try {
 $app=Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
 $deadline=(Get-Date).AddSeconds(300)
 $passed=$false
 while((Get-Date) -lt $deadline){
  if($app.HasExited){throw 'Solo process exited before completion'}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_MENU_TEST_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Solo runtime failure: $log"}
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_SOLO_TEST_PASSED players=1 crew=1' -Quiet){$passed=$true;break}
  }
  Start-Sleep -Seconds 1
 }
 if(!$passed){throw "Solo test timed out: $log"}
 Start-Sleep -Seconds 2
 foreach($tag in 'FISKELAGET_DAY_STARTED','FISKELAGET_SOLO_STARTED','FISKELAGET_TRIP_UNDERWAY','FISKELAGET_TRIP_ARRIVED','FISKELAGET_ROD_FISHING','FISKELAGET_RETURN_UNDERWAY','FISKELAGET_MARINA_REACHED','FISKELAGET_SOLO_TEST_PASSED'){
  if(@(Select-String -LiteralPath $log -Pattern $tag).Count -ne 1){throw "Wrong event count: $tag"}
 }
 foreach($name in 'solo-menu','solo-ready','solo-fishing','solo-home'){
  $shot=Join-Path $run "$name.png"
  if(!(Test-Path -LiteralPath $shot) -or (Get-Item -LiteralPath $shot).Length -lt 1000){throw "Missing screenshot: $name"}
 }
 'PASS: one real player starts via menu, becomes captain, cannot leave early, clock continues with menu open, R departs at 02:00, F fishes, R returns.'
 "Evidence: $run"
}
finally {if($app -and !$app.HasExited){Stop-Process -InputObject $app}}
