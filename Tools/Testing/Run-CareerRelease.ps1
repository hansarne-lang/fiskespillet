param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory,[ValidateSet('All','Solo','Lan')][string]$Phase='All',[string]$LegacyProfile='')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget/Binaries/Win64/FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
if(@(Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue).Count){throw 'Port 7777 is occupied'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$active=@();$logs=@()
function Start-Case($name,$mode,$url,$profile,$cash=''){
 $capture=Join-Path $run $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $log=Join-Path $run "$name.log"
 $flags=@($url,'-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',"-FiskCareerReleaseSmoke=$mode",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run $profile)))
 if($cash -ne ''){$flags += "-FiskCareerExpectedCash=$cash"}
 Start-Process -FilePath $exe -WorkingDirectory $capture -ArgumentList $flags -PassThru -WindowStyle Hidden
}
function Wait-Tag($log,$tag,$app,$seconds){
 $until=(Get-Date).AddSeconds($seconds)
 while((Get-Date) -lt $until){
  if($app.HasExited){throw "Process exited before $tag in $log"}
  foreach($file in $logs){if((Test-Path -LiteralPath $file) -and (Select-String -LiteralPath $file -Pattern 'FISKELAGET_CAREER_RELEASE_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Runtime failure: $file"}}
  if((Test-Path -LiteralPath $log) -and (Select-String -LiteralPath $log -Pattern $tag -Quiet)){return}
  Start-Sleep -Seconds 1
 }
 throw "Timeout $tag in $log"
}
function Stop-Case($app){if($app -and !$app.HasExited){Stop-Process -InputObject $app;$app.WaitForExit()}}
function Check-Shots($name,$shots){foreach($shot in $shots){$file=Join-Path (Join-Path $run $name) "$shot.png";if(!(Test-Path -LiteralPath $file) -or (Get-Item -LiteralPath $file).Length -lt 1000){throw "Missing $name/$shot"}}}
function Career-File($profile){$files=@(Get-ChildItem -LiteralPath (Join-Path $run $profile) -File -Recurse -Filter FiskelagetCareer.ini);if($files.Count -ne 1){throw "Wrong canonical career count: $profile"};$files[0].FullName}
function Career-Cash($profile){$text=[IO.File]::ReadAllText((Career-File $profile));$match=[regex]::Match($text,'(?m)^Cash=(\d+)\r?$');if(!$match.Success){throw 'Missing cash'};[int64]$match.Groups[1].Value}
function Reload-Profile($role,$profile,$expected){
 $name="reload-$role";$app=Start-Case $name 'verify' '/Engine/Maps/Entry?listen' $profile $expected;$script:active+=$app;$log=Join-Path $run "$name.log";$script:logs+=$log
 Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=verify' $app 90
 Start-Sleep -Seconds 2;Stop-Case $app;Check-Shots $name @('career-reloaded')
 $file=Career-File $profile
 if(!(Test-Path -LiteralPath ($file+'.bak'))){throw 'Missing career backup'}
 [IO.File]::WriteAllText($file,'deliberately-damaged-test-primary')
 $name="recover-$role";$app=Start-Case $name 'verify' '/Engine/Maps/Entry?listen' $profile $expected;$script:active+=$app;$log=Join-Path $run "$name.log";$script:logs+=$log
 Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=verify' $app 90
 if(!(Select-String -LiteralPath $log -Pattern 'FISKELAGET_CAREER_LOADED.*recovery=1' -Quiet)){throw 'Backup was not used'}
 Start-Sleep -Seconds 2;Stop-Case $app
 if((Career-Cash $profile) -ne $expected){throw 'Recovery lost cash'}
 Check-Shots $name @('career-reloaded')
}
try{
 if($Phase -ne 'Lan'){
  $app=Start-Case 'solo' 'solo' '/Engine/Maps/Entry?listen' 'solo-profile';$active+=$app;$log=Join-Path $run 'solo.log';$logs+=$log
  Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=solo' $app 900
  Start-Sleep -Seconds 2;Stop-Case $app
  Check-Shots 'solo' @('career-new','career-owned','free-sea-route','offshore-ready','passive-rods','fight-switch','offshore-landed','offshore-home')
  if(!(Select-String -LiteralPath $log -Pattern 'FISKELAGET_OFFSHORE_ALLOCATION.*rods=3' -Quiet)){throw 'Three rods not allocated'}
  if(!(Select-String -LiteralPath $log -Pattern 'FISKELAGET_JOURNEY_AUDIO_CONSUMED' -Quiet)){throw 'Journey audio was not consumed'}
  if(@(Select-String -LiteralPath $log -Pattern 'FISKELAGET_ROD_FISHING player=').Count -ne 3){throw 'Wrong number of solo casts'}
  if((Career-Cash 'solo-profile') -ne 225000){throw 'Wrong solo wallet'}
  Reload-Profile 'solo' 'solo-profile' 225000
  $badRoot=Join-Path $run 'blocked-profile/Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $badRoot -Force | Out-Null
  $bad=Join-Path $badRoot 'FiskelagetCareer.ini';[IO.File]::WriteAllText($bad,'bad-primary-preserve');[IO.File]::WriteAllText($bad+'.bak','bad-backup-preserve')
  $app=Start-Case 'blocked' 'blocked' '/Engine/Maps/Entry?listen' 'blocked-profile';$active+=$app;$log=Join-Path $run 'blocked.log';$logs+=$log
  Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=blocked' $app 90
  Start-Sleep -Seconds 2;Stop-Case $app
  if([IO.File]::ReadAllText($bad) -ne 'bad-primary-preserve' -or [IO.File]::ReadAllText($bad+'.bak') -ne 'bad-backup-preserve'){throw 'Invalid profiles overwritten'}
  if($LegacyProfile){
   $dest=Join-Path $run 'legacy-profile/Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $dest -Force | Out-Null
   foreach($name in 'FiskelagetJournal.ini','FiskelagetJournal.ini.bak','FiskelagetPlaces.ini','FiskelagetRigPresets.ini'){
    $source=Join-Path $LegacyProfile $name;if(Test-Path -LiteralPath $source){Copy-Item -LiteralPath $source -Destination (Join-Path $dest $name)}
   }
   $app=Start-Case 'migration' 'migration' '/Engine/Maps/Entry?listen' 'legacy-profile';$active+=$app;$log=Join-Path $run 'migration.log';$logs+=$log
   Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=migration' $app 90
   Start-Sleep -Seconds 2;Stop-Case $app
   if(!(Select-String -LiteralPath (Join-Path $dest 'FiskelagetPlaces.ini') -Pattern '^Version=2$' -Quiet)){throw 'Places did not migrate to coordinates'}
  }
  'PASS: real shop purchases, receipt replay/stale rejection, offshore selection, three independent rods, free chart click, fight input release, audio, landing/return, fresh-process career reload, backup repair and two-invalid-file preservation.'
 }
 if($Phase -eq 'Solo'){"Evidence: $run";return}
 $roles=@('host','guest1','guest2','guest3');$apps=@{}
 $apps.host=Start-Case 'host' 'host' '/Engine/Maps/Entry?listen' 'host-profile';$active+=$apps.host;$hostLog=Join-Path $run 'host.log';$logs+=$hostLog
 Wait-Tag $hostLog 'GameNetDriver.*listening on port 7777' $apps.host 120
 foreach($role in $roles[1..3]){$apps[$role]=Start-Case $role $role '127.0.0.1:7777' "$role-profile";$active+=$apps[$role];$logs+=Join-Path $run "$role.log"}
 Wait-Tag $hostLog 'FISKELAGET_CAREER_RELEASE_PASSED mode=host' $apps.host 1500
 foreach($role in $roles[1..3]){Wait-Tag (Join-Path $run "$role.log") "FISKELAGET_CAREER_RELEASE_PASSED mode=$role" $apps[$role] 30}
 Start-Sleep -Seconds 2
 function Stamp($line){[datetime]::ParseExact([regex]::Match($line.Line,'\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}:\d{3})\]').Groups[1].Value,'yyyy.MM.dd-HH.mm.ss:fff',[Globalization.CultureInfo]::InvariantCulture)}
 $start=Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_SESSION_DAY number=1' | Select-Object -First 1
 $allocations=@()
 foreach($role in $roles){
  $log=Join-Path $run "$role.log"
  $reveal=Select-String -LiteralPath $log -Pattern 'FISKELAGET_SETTLEMENT rows=4' | Select-Object -First 1
  $day2=Select-String -LiteralPath $log -Pattern 'FISKELAGET_SESSION_DAY number=2' | Select-Object -First 1
  if(!$reveal -or ((Stamp $reveal)-(Stamp $start)).TotalSeconds -lt 1020){throw "Early or missing reveal: $role"}
  if(!$day2 -or ((Stamp $day2)-(Stamp $start)).TotalSeconds -lt 1200){throw "Day shortened: $role"}
  if(@(Select-String -LiteralPath $log -Pattern 'FISKELAGET_CAREER_REVEAL_PASSED').Count -ne 1){throw 'Wrong reveal count'}
  $allocation=Select-String -LiteralPath $log -Pattern 'FISKELAGET_OFFSHORE_ALLOCATION' | Select-Object -First 1
  $allocations += [int][regex]::Match($allocation.Line,'rods=(\d+)').Groups[1].Value
  Check-Shots $role @('career-new','career-owned','offshore-ready','passive-rods','offshore-landed','offshore-home','career-before17','career-after17','career-day2')
 }
 if((($allocations | Sort-Object) -join ',') -ne '1,1,2,2'){throw 'Wrong six-holder distribution'}
 $cash=Career-Cash 'guest3-profile';$prior=@(Get-Content -LiteralPath $hostLog).Count
 Stop-Case $apps.guest3
 $until=(Get-Date).AddSeconds(90);$left=$false
 while((Get-Date) -lt $until){
  $tail=Get-Content -LiteralPath $hostLog | Select-Object -Skip $prior
  if($tail | Select-String -Pattern 'FISKELAGET_STARTER_BOAT_CREW 3' -Quiet){$left=$true;break}
  Start-Sleep -Seconds 1
 }
 if(!$left){throw 'Guest disconnect did not release crew slot'}
 $rejoin=Start-Case 'rejoin-guest3' 'rejoin' '127.0.0.1:7777' 'guest3-profile' $cash;$active+=$rejoin;$log=Join-Path $run 'rejoin-guest3.log';$logs+=$log
 Wait-Tag $log 'FISKELAGET_CAREER_RELEASE_PASSED mode=rejoin' $rejoin 90
 Start-Sleep -Seconds 2;Check-Shots 'rejoin-guest3' @('rejoined-day2');Stop-Case $rejoin
 foreach($role in $roles){Stop-Case $apps[$role]}
 foreach($role in $roles){$cash=Career-Cash "$role-profile";Reload-Profile $role "$role-profile" $cash}
 'PASS: four separate careers, six holders allocated 2+2+1+1, four real landings, full normal day, shared 17:00 reveal, own income exactly once, day 2 without duplicate credit, fresh-process reload and backup recovery for all four profiles.'
 "Evidence: $run"
}
finally{foreach($app in $active){Stop-Case $app}}
