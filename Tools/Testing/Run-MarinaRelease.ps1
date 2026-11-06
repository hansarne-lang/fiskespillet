param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory,[ValidateSet('All','Solo','Lan')][string]$Phase='All',[string]$LegacyProfile='')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget/Binaries/Win64/FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
if(@(Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue).Count){throw 'Port7777 occupied'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss);New-Item -ItemType Directory -Path $run -Force | Out-Null
$active=@();$logs=@()
function Start-Case($name,$mode,$url,$data,$cash=-1){
 $capture=Join-Path $run $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $log=Join-Path $run "$name.log"
 $flags=@($url,'-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',"-FiskMarinaSmoke=$mode",('-FiskScreenshotDir="{0}"' -f $capture),('-abslog="{0}"' -f $log),('-UserDir="{0}"' -f (Join-Path $run $data)))
 if($cash -ge 0){$flags+="-FiskMarinaExpectedCash=$cash"}
 Start-Process -FilePath $exe -WorkingDirectory $capture -ArgumentList $flags -PassThru -WindowStyle Hidden
}
function Stop-Case($app){if($app -and !$app.HasExited){Stop-Process -InputObject $app;$app.WaitForExit()}}
function Wait-Tag($file,$pattern,$app,$seconds=180){
 $end=(Get-Date).AddSeconds($seconds)
 while((Get-Date) -lt $end){
  if($app.HasExited){throw "Process exited: $file"}
  foreach($other in $logs){if((Test-Path -LiteralPath $other) -and (Select-String -LiteralPath $other -Pattern 'FISKELAGET_MARINA_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Runtime failure: $other"}}
  if((Test-Path -LiteralPath $file) -and (Select-String -LiteralPath $file -Pattern $pattern -Quiet)){return}
  Start-Sleep -Seconds 1
 };throw "Timeout $pattern in $file"
}
function Shots($name,$wanted){foreach($shot in $wanted){$file=Join-Path $run "$name/$shot.png";if(!(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).Length -lt 1000){throw "Missing screenshot $file"}}}
function Cash($data){$file=Join-Path $run "$data/Saved/Config/Fiskelaget/FiskelagetCareer.ini";$m=Select-String -LiteralPath $file -Pattern '^Cash=(\d+)$';if(!$m){throw 'Career cash missing'};[long]$m.Matches[0].Groups[1].Value}
function Marina-File($data){$dir=Join-Path $run "$data/Saved/Config/Fiskelaget";$files=@(Get-ChildItem -LiteralPath $dir -File -Filter 'FiskelagetMarina-*.ini');if($files.Count -ne 1){throw 'Expected one bound marina file'};$files[0].FullName}
try{
 if($Phase -ne 'Lan'){
  $app=Start-Case 'solo' 'solo' '/Engine/Maps/Entry?listen' 'solo-profile';$active+=$app;$logs+=Join-Path $run 'solo.log'
  Wait-Tag $logs[-1] 'FISKELAGET_MARINA_PASSED mode=solo' $app
  Start-Sleep -Seconds 2;Stop-Case $app;Shots 'solo' @('naust-eget','strandbar','artsbok-marlin');if((Cash 'solo-profile') -ne 3000000){throw 'Cosmetics changed money'}
  $app=Start-Case 'reload' 'reload' '/Engine/Maps/Entry?listen' 'solo-profile' 3000000;$active+=$app;$logs+=Join-Path $run 'reload.log'
  Wait-Tag $logs[-1] 'FISKELAGET_MARINA_PASSED mode=reload' $app;Start-Sleep -Seconds 2;Stop-Case $app;Shots 'reload' @('naust-reloaded')
  $file=Marina-File 'solo-profile';[IO.File]::WriteAllText($file,'damaged-primary')
  $app=Start-Case 'recover' 'recover' '/Engine/Maps/Entry?listen' 'solo-profile' 3000000;$active+=$app;$logs+=Join-Path $run 'recover.log'
  Wait-Tag $logs[-1] 'FISKELAGET_MARINA_PASSED mode=recover' $app;Start-Sleep -Seconds 2;Stop-Case $app;Shots 'recover' @('naust-recovered')
  if(!(Select-String -LiteralPath $logs[-1] -Pattern 'FISKELAGET_MARINA_LOADED blocked=0 recovery=1' -Quiet)){throw 'Backup was not actually used'}
  $bad=Join-Path $run 'blocked-profile/Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $bad -Force | Out-Null
  foreach($name in 'FiskelagetCareer.ini','FiskelagetCareer.ini.bak'){Copy-Item -LiteralPath (Join-Path $run "solo-profile/Saved/Config/Fiskelaget/$name") -Destination (Join-Path $bad $name)}
  $badFile=Join-Path $bad (Split-Path -Leaf $file);[IO.File]::WriteAllText($badFile,'bad-primary-preserve');[IO.File]::WriteAllText($badFile+'.bak','bad-backup-preserve')
  $app=Start-Case 'blocked' 'blocked' '/Engine/Maps/Entry?listen' 'blocked-profile';$active+=$app;$logs+=Join-Path $run 'blocked.log'
  Wait-Tag $logs[-1] 'FISKELAGET_MARINA_PASSED mode=blocked' $app;Start-Sleep -Seconds 2;Stop-Case $app
  if([IO.File]::ReadAllText($badFile) -ne 'bad-primary-preserve' -or [IO.File]::ReadAllText($badFile+'.bak') -ne 'bad-backup-preserve'){throw 'Invalid cosmetics overwritten'}
  if($LegacyProfile){
   $dest=Join-Path $run 'legacy-profile/Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $dest -Force | Out-Null
   foreach($name in 'FiskelagetCareer.ini','FiskelagetCareer.ini.bak','FiskelagetJournal.ini','FiskelagetJournal.ini.bak','FiskelagetPlaces.ini','FiskelagetRigPresets.ini','FiskelagetSettings.ini'){if(Test-Path -LiteralPath (Join-Path $LegacyProfile $name)){Copy-Item -LiteralPath (Join-Path $LegacyProfile $name) -Destination (Join-Path $dest $name)}}
   $expected=Cash 'legacy-profile';$app=Start-Case 'migration' 'migrate' '/Engine/Maps/Entry?listen' 'legacy-profile' $expected;$active+=$app;$logs+=Join-Path $run 'migration.log'
   Wait-Tag $logs[-1] 'FISKELAGET_MARINA_PASSED mode=migrate' $app;Start-Sleep -Seconds 2;Stop-Case $app;Shots 'migration' @('v07-artsbok');if((Cash 'legacy-profile') -ne $expected){throw 'Migration changed wallet'}
  }
  'PASS: real H/B/I input including shortcut letters and digits, cosmetic choices, book, messages/rate limit, rejected foreign edit and early trophy, fresh-process persistence, real previous-generation backup recovery, invalid-file preservation and optional v0.7 migration.'
 }
 if($Phase -eq 'Solo'){"Evidence: $run";return}
 $hostApp=Start-Case 'host' 'host' '/Engine/Maps/Entry?listen' 'host-profile';$active+=$hostApp;$logs+=Join-Path $run 'host.log'
 Wait-Tag $logs[-1] 'GameNetDriver.*listening on port 7777' $hostApp
 $guest=Start-Case 'guest' 'guest' '127.0.0.1:7777' 'guest-profile';$active+=$guest;$logs+=Join-Path $run 'guest.log'
 Wait-Tag (Join-Path $run 'host.log') 'FISKELAGET_MARINA_PASSED mode=host' $hostApp
 Wait-Tag (Join-Path $run 'guest.log') 'FISKELAGET_MARINA_PASSED mode=guest' $guest
 Start-Sleep -Seconds 2;Stop-Case $guest;Stop-Case $hostApp
 foreach($role in 'host','guest'){Shots $role @('naust-eget','strandbar','artsbok-marlin','naust-besoek');if((Cash "$role-profile") -ne 3000000){throw 'LAN cosmetics changed money'}}
 if(@(Select-String -LiteralPath (Join-Path $run 'host.log') -Pattern 'FISKELAGET_MARINA_IDENTITY accepted=0').Count -lt 2){throw 'Foreign edits were not rejected'}
 if(@(Select-String -LiteralPath (Join-Path $run 'host.log') -Pattern 'FISKELAGET_MARINA_RECORDS_REJECTED').Count -lt 2){throw 'Early trophy requests not rejected'}
 'PASS: two graphical LAN careers, separate named/styled houses, actual peer visits, messages, owner validation, no early trophies and unchanged independent wallets.'
 "Evidence: $run"
}finally{foreach($app in $active){Stop-Case $app}}
