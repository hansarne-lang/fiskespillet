param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
if(@(Get-NetUDPEndpoint -LocalPort $FiskPort0 -ErrorAction SilentlyContinue).Count){throw "Port $FiskPort0 is occupied"}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$active=@();$clientApp=$null;$clientLog=$null
function Wait-Tag($log,$tag,$app,$seconds){
 $until=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $until){
  if($app.HasExited){throw "Process exited before $tag"}
  foreach($file in @($log,$clientLog)){
   if($file -and (Test-Path -LiteralPath $file) -and (Select-String -LiteralPath $file -Pattern 'FISKELAGET_JOURNAL_FAILED|FISKELAGET_SESSION_FAILED|FISKELAGET_CONTENT_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Runtime failure in $file"}
  }
  if($clientApp -and $clientApp.HasExited){throw 'Client exited during session test'}
  if((Test-Path -LiteralPath $log) -and (Select-String -LiteralPath $log -Pattern $tag -Quiet)){return}
  Start-Sleep -Seconds 1
 }
 throw "Timeout $tag in $log"
}
function Start-Case($name,$url,$mode,$content,$profile,$journal=''){
 $capture=Join-Path $run $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $working=Join-Path $capture 'working';New-Item -ItemType Directory -Path $working -Force | Out-Null
 $log=Join-Path $run "$name.log"
 $flags=@($url,'-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,"-port=$FiskPort0",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run $profile)))
 if($mode){$flags += "-FiskSessionSmoke=$mode"}
 if($content){$flags += "-FiskContentSmoke=$content"}
 if($journal){$flags += "-FiskJournalSmoke=$journal"; $depth=if($name.Contains("client")){90}else{80}; $flags += "-FiskJournalExpectedDepth=$depth"}
 Start-Process -FilePath $exe -WorkingDirectory $working -ArgumentList $flags -PassThru -WindowStyle Hidden
}
try{
 $save=Start-Case 'save' '/Engine/Maps/Entry?listen' 'settings-save' 'preset-save' 'shared-profile';$active+=$save
 Wait-Tag (Join-Path $run 'save.log') 'FISKELAGET_SESSION_SETTINGS_PASSED' $save 90
 Wait-Tag (Join-Path $run 'save.log') 'FISKELAGET_CONTENT_PRESET_SAVED' $save 90
 Start-Sleep -Seconds 2;Stop-Process -InputObject $save;$save.WaitForExit()
 $load=Start-Case 'load' '/Engine/Maps/Entry?listen' 'settings-load' 'preset-load' 'shared-profile';$active+=$load
 Wait-Tag (Join-Path $run 'load.log') 'FISKELAGET_SESSION_SETTINGS_PASSED' $load 90
 Wait-Tag (Join-Path $run 'load.log') 'FISKELAGET_CONTENT_PRESET_LOADED' $load 90
 Start-Sleep -Seconds 2;Stop-Process -InputObject $load;$load.WaitForExit()
 $fresh=Start-Case 'fresh' '/Engine/Maps/Entry?listen' 'settings-default' '' 'fresh-profile' 'empty';$active+=$fresh
 Wait-Tag (Join-Path $run 'fresh.log') 'FISKELAGET_SESSION_SETTINGS_PASSED' $fresh 90
 Wait-Tag (Join-Path $run 'fresh.log') 'FISKELAGET_JOURNAL_STATE_PASSED' $fresh 90
 Stop-Process -InputObject $fresh;$fresh.WaitForExit()
 foreach($name in 'FiskelagetRigPresets.ini','FiskelagetSettings.ini'){
  if(@(Get-ChildItem -LiteralPath (Join-Path $run 'shared-profile') -Filter $name -File -Recurse).Count -ne 1){throw "Expected one canonical $name"}
 }
 foreach($name in 'save','load'){
  if(@(Get-ChildItem -LiteralPath (Join-Path $run "$name\working") -Filter '*.ini').Count){throw 'Settings leaked into working directory'}
 }
 'PASS: settings/presets persist across processes and working directories, with isolated fresh defaults.'
 $hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log'
 $hostApp=Start-Case 'host' '/Engine/Maps/Entry?listen' 'day' 'day' 'host-profile' 'day';$active+=$hostApp
 Wait-Tag $hostLog "GameNetDriver.*listening on port $FiskPort0" $hostApp 120
 $clientApp=Start-Case 'client' "127.0.0.1:$FiskPort0" 'day' 'day' 'client-profile' 'day';$active+=$clientApp
 Wait-Tag $hostLog 'FISKELAGET_SESSION_PASSED' $hostApp 1530
 Wait-Tag $clientLog 'FISKELAGET_SESSION_PASSED' $clientApp 30
 Start-Sleep -Seconds 2
 function Stamp($line){[datetime]::ParseExact([regex]::Match($line.Line,'\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]').Groups[1].Value,'yyyy.MM.dd-HH.mm.ss:fff',[Globalization.CultureInfo]::InvariantCulture)}
 $start=Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_SESSION_DAY number=1' | Select-Object -First 1
 foreach($log in $hostLog,$clientLog){
  foreach($tag in 'FISKELAGET_CONTENT_DAY_PASSED','FISKELAGET_CONTENT_REVEAL_PASSED','FISKELAGET_SESSION_PASSED'){
   if(@(Select-String -LiteralPath $log -Pattern $tag).Count -ne 1){throw "Wrong event count $tag"}
  }
  $second=Select-String -LiteralPath $log -Pattern 'FISKELAGET_SESSION_DAY number=2' | Select-Object -First 1
  if(!$second -or (FiskGameSpan (Stamp $start) (Stamp $second)) -lt 1200){throw 'Day 2 started before full day 1'}
  $reveal=Select-String -LiteralPath $log -Pattern 'FISKELAGET_SETTLEMENT rows=2' | Select-Object -First 1
  if(!$reveal -or (FiskGameSpan (Stamp $start) (Stamp $reveal)) -lt 1020){throw 'Early day 1 reveal'}
  if(@(Select-String -LiteralPath $log -Pattern 'FISKELAGET_SETTLEMENT rows=').Count -ne 1){throw 'Unexpected second settlement'}
  if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_MENU_NETWORK_FAILURE|FISKELAGET_MENU_TRAVEL_FAILURE' -Quiet){throw 'Connection broke across day transition'}
 }
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_NEXT_DAY_ACCEPTED').Count -ne 1){throw 'Day advanced other than once'}
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_NEXT_DAY_REJECTED').Count -lt 3){throw 'Missing early/guest/stale day rejections'}
 foreach($role in 'host','client'){
  foreach($name in 'help','results-before17','results-revealed','day-finished','history-day1','day2-start','day2-results-locked','day2-old-history','day2-fishing','day2-home'){
   $file=Join-Path (Join-Path $run $role) "$name.png"
   if(!(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).Length -lt 1000){throw "Missing screenshot $role/$name"}
  }
 }
 foreach($log in $hostLog,$clientLog){
  if(@(Select-String -LiteralPath $log -Pattern 'FISKELAGET_JOURNAL_DAY_PASSED').Count -ne 1){throw 'Journal did not pass before day-2 return'}
 }
 Stop-Process -InputObject $clientApp;$clientApp.WaitForExit();Stop-Process -InputObject $hostApp;$hostApp.WaitForExit()
 $clientApp=$null;$clientLog=$null
 foreach($role in 'host','client'){
  $profile="$role-profile"
  $journalFiles=@(Get-ChildItem -LiteralPath (Join-Path $run $profile) -Filter FiskelagetJournal.ini -File -Recurse)
  if($journalFiles.Count -ne 1){throw "Missing canonical journal for $role"}
  $file=$journalFiles[0].FullName
  if(!(Test-Path -LiteralPath ($file+'.bak'))){throw 'Missing journal backup'}
  $name="reload-$role";$app=Start-Case $name '/Engine/Maps/Entry' '' '' $profile 'reload';$active+=$app
  Wait-Tag (Join-Path $run "$name.log") 'FISKELAGET_JOURNAL_RELOAD_PASSED mode=reload' $app 90
  Start-Sleep -Seconds 2;Stop-Process -InputObject $app;$app.WaitForExit()
  Copy-Item -LiteralPath $file -Destination (Join-Path $run "$role-before-recovery.ini")
  [IO.File]::WriteAllText($file,'deliberately-truncated-test-primary')
  $name="recover-$role";$app=Start-Case $name '/Engine/Maps/Entry' '' '' $profile 'recover';$active+=$app
  Wait-Tag (Join-Path $run "$name.log") 'FISKELAGET_JOURNAL_RELOAD_PASSED mode=recover' $app 90
  Start-Sleep -Seconds 2;Stop-Process -InputObject $app;$app.WaitForExit()
  if(!(Select-String -LiteralPath $file -Pattern 'Version=1' -Quiet)){throw 'Recovered primary not repaired by F5'}
  foreach($shotRole in "reload-$role","recover-$role"){
   foreach($shot in 'journal-reloaded','records-reloaded'){
    if(!(Test-Path -LiteralPath (Join-Path (Join-Path $run $shotRole) "$shot.png"))){throw "Missing $shotRole/$shot"}
   }
  }
  foreach($shot in 'journal-day1','journal-filter-empty','records-day1'){
   if(!(Test-Path -LiteralPath (Join-Path (Join-Path $run $role) "$shot.png"))){throw "Missing $role/$shot"}
  }
 }
 $badRoot=Join-Path $run 'blocked-profile/Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $badRoot -Force | Out-Null
 $badFile=Join-Path $badRoot 'FiskelagetJournal.ini'
 [IO.File]::WriteAllText($badFile,'bad-primary-preserve');[IO.File]::WriteAllText($badFile+'.bak','bad-backup-preserve')
 $blocked=Start-Case 'blocked' '/Engine/Maps/Entry' '' '' 'blocked-profile' 'blocked';$active+=$blocked
 Wait-Tag (Join-Path $run 'blocked.log') 'FISKELAGET_JOURNAL_STATE_PASSED mode=blocked' $blocked 90
 Start-Sleep -Seconds 2;Stop-Process -InputObject $blocked;$blocked.WaitForExit()
 if([IO.File]::ReadAllText($badFile) -ne 'bad-primary-preserve' -or [IO.File]::ReadAllText($badFile+'.bak') -ne 'bad-backup-preserve'){throw 'Corrupt files overwritten'}
 'PASS: only own confirmed 17:00 catch entered journal, filters/records worked, no duplicate on day 2; both profiles reloaded in fresh processes and recovered/repaired backups; two corrupt files remained untouched.'
 'PASS: full normal day 1 with two real catches, 17:00 reveal, 20:00 finish, host-only single next day without reconnect, retained history/crew/rigs, rejected stale commands and fresh fishing/return in day 2.'
 "Evidence: $run"
}
finally {foreach($app in $active){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}}
