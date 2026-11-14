param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
if(@(Get-NetUDPEndpoint -LocalPort $FiskPort0 -ErrorAction SilentlyContinue).Count){throw "Port $FiskPort0 is occupied"}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
function Wait-Tag($log,$tag,$app,$seconds){
 $until=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $until){
  if($app.HasExited){throw "Process exited before $tag"}
  if($clientApp -and $clientApp.HasExited){throw 'Client exited during full-day test'}
  if($clientLog -and (Test-Path -LiteralPath $clientLog) -and (Select-String -LiteralPath $clientLog -Pattern 'FISKELAGET_CONTENT_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Client runtime failure in $clientLog"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_CONTENT_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $tag -Quiet){return}
  }
  Start-Sleep -Seconds 1
 }
 throw "Timeout $tag in $log"
}
function Start-Case($name,$url,$mode,$settings){
 $capture=Join-Path $run $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $log=Join-Path $run "$name.log"
 $flags=@($url,"-FiskContentSmoke=$mode",'-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,"-port=$FiskPort0",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run $settings)))
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
}
$save=$null;$load=$null;$hostApp=$null;$clientApp=$null
try{
 $save=Start-Case 'preset-save' '/Engine/Maps/Entry?listen' 'preset-save' 'shared-presets'
 Wait-Tag (Join-Path $run 'preset-save.log') 'FISKELAGET_CONTENT_PRESET_SAVED' $save 90
 Start-Sleep -Seconds 2;Stop-Process -InputObject $save;$save.WaitForExit()
 $load=Start-Case 'preset-load' '/Engine/Maps/Entry?listen' 'preset-load' 'shared-presets'
 Wait-Tag (Join-Path $run 'preset-load.log') 'FISKELAGET_CONTENT_PRESET_LOADED' $load 90
 Start-Sleep -Seconds 2;Stop-Process -InputObject $load;$load.WaitForExit()
 $hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log'
 $hostApp=Start-Case 'host' '/Engine/Maps/Entry?listen' 'day' 'host-settings'
 Wait-Tag $hostLog "GameNetDriver.*listening on port $FiskPort0" $hostApp 120
 $clientApp=Start-Case 'client' "127.0.0.1:$FiskPort0" 'day' 'client-settings'
 Wait-Tag $hostLog 'FISKELAGET_CONTENT_DAY_PASSED' $hostApp 1310
 Wait-Tag $clientLog 'FISKELAGET_CONTENT_DAY_PASSED' $clientApp 30
 Start-Sleep -Seconds 2
 $start=Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_DAY_STARTED' | Select-Object -First 1
 function Stamp($line){[datetime]::ParseExact([regex]::Match($line.Line,'\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]').Groups[1].Value,'yyyy.MM.dd-HH.mm.ss:fff',[Globalization.CultureInfo]::InvariantCulture)}
 foreach($log in $hostLog,$clientLog){
  foreach($tag in 'FISKELAGET_SETTLEMENT rows=2','FISKELAGET_CONTENT_REVEAL_PASSED','FISKELAGET_CONTENT_DAY_PASSED','FISKELAGET_CONTENT_RETURNED'){
   if(@(Select-String -LiteralPath $log -Pattern $tag).Count -ne 1){throw "Wrong count $tag in $log"}
  }
  $settlement=Select-String -LiteralPath $log -Pattern 'FISKELAGET_SETTLEMENT rows=2' | Select-Object -First 1
  if((FiskGameSpan (Stamp $start) (Stamp $settlement)) -lt 1020){throw 'Revealed before normal 17:00'}
  $end=Select-String -LiteralPath $log -Pattern FISKELAGET_CONTENT_DAY_PASSED | Select-Object -First 1
  if([Math]::Abs((FiskGameSpan (Stamp $start) (Stamp $end))-1200) -gt (3*$FiskClockScale)){throw 'Incorrect 20-minute wall-clock day'}
  $details=@(Select-String -LiteralPath $log -Pattern 'FISKELAGET_REVEALED_CATCH player=\d+ species=(?:[0-9]|1[0-4]) grams=\d+ mm=\d+ value=\d+ delivered=1')
  if($details.Count -ne 2){throw 'Expected two valid revealed catches'}
 }
 foreach($role in 'host','client'){
  foreach($name in 'help','results-locked','departure-feedback','fish-fight','fish-landed','home','results-before17','results-revealed','day-finished'){
   $shot=Join-Path (Join-Path $run $role) "$name.png"
   if(!(Test-Path -LiteralPath $shot) -or (Get-Item -LiteralPath $shot).Length -lt 1000){throw "Missing screenshot $role/$name"}
  }
 }
 foreach($pair in @(@('preset-save','preset-saved'),@('preset-save','preset-applied'),@('preset-load','preset-reloaded'))){
  if(!(Test-Path -LiteralPath (Join-Path (Join-Path $run $pair[0]) ($pair[1]+'.png')))){throw 'Missing preset screenshot'}
 }
 'PASS: persisted presets across processes, help/feedback, two real catches, no early details, exact 17:00 reveal with species/size/value, one payout, marina return and full normal 20-minute day.'
 "Evidence: $run"
}
finally{foreach($app in $clientApp,$hostApp,$load,$save){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}}
