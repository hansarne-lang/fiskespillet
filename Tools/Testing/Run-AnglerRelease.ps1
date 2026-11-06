param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget/Binaries/Win64/FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
foreach($port in 7777,7778,7779){if(@(Get-NetUDPEndpoint -LocalPort $port -ErrorAction SilentlyContinue).Count){throw "Port $port is occupied"}}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$apps=@{};$logs=@()
function Start-Case($name,$career,$mode,$url,$profile,$weather,$port,$drop=$false){
 $capture=Join-Path $run $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $data=Join-Path $run "$profile/Saved/Config/Fiskelaget";New-Item -ItemType Directory -Path $data -Force | Out-Null
 $settings=Join-Path $data 'FiskelagetSettings.ini'
 if(!(Test-Path -LiteralPath $settings)){[IO.File]::WriteAllText($settings,"[Fiskelaget.Settings]`r`nVersion=1`r`nSound=1`r`nFrameRate=30`r`n")}
 $log=Join-Path $run "$name.log";$script:logs+=$log
 $flags=@($url,"-port=$port","-FiskWeather=$weather","-FiskExpectedWeather=$weather",'-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run $profile)))
 if($mode -in @("recovery","restart","reload")){$flags+="-FiskExpeditionSmoke=$mode";$flags+='-FiskSkipperSmoke=reload';$flags+='-FiskAnglerSmoke=reload'}else{$flags+="-FiskExplorationSmoke=$mode";$flags+='-FiskSkipperSmoke=full';$flags+='-FiskAnglerSmoke=full'}
 if($career){$flags+="-FiskCareerReleaseSmoke=$career"}
 if($drop){$flags+='-FiskDropJournalAck'}
 $app=Start-Process -FilePath $exe -WorkingDirectory $capture -ArgumentList $flags -PassThru -WindowStyle Hidden
 $script:apps[$name]=$app
}
function Check-Failures{
 foreach($file in $logs){if((Test-Path -LiteralPath $file) -and (Select-String -LiteralPath $file -Pattern 'FISKELAGET_(CAREER_RELEASE|EXPEDITION|EXPLORATION|SKIPPER|ANGLER)_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Runtime failure: $file"}}
}
function Wait-Tag($name,$tag,$seconds){
 $log=Join-Path $run "$name.log";$until=(Get-Date).AddSeconds($seconds)
 while((Get-Date) -lt $until){Check-Failures;if($apps[$name].HasExited){throw "Process $name exited before $tag"}
  if((Test-Path -LiteralPath $log) -and (Select-String -LiteralPath $log -Pattern $tag -Quiet)){return};Start-Sleep -Seconds 1}
 throw "Timeout $name : $tag"
}
function Stop-Case($name){$app=$apps[$name];if($app -and !$app.HasExited){Stop-Process -InputObject $app;$app.WaitForExit()}}
try{
 Start-Case 'host' 'host' 'full' '/Engine/Maps/Entry?listen' 'host-profile' 0 7777
 Wait-Tag 'host' 'GameNetDriver.*listening on port 7777' 120
 foreach($role in 'guest1','guest2','guest3'){Start-Case $role $role $(if($role -eq 'guest3'){'offline'}else{'full'}) '127.0.0.1:7777' "$role-profile" 0 7777 ($role -eq 'guest1')}
 Start-Case 'solo' 'solo' 'full' '/Engine/Maps/Entry?listen' 'solo-profile' 1 7778
 Wait-Tag 'guest3' 'FISKELAGET_EXPLORATION_DISCONNECT_READY' 1250
 Stop-Case 'guest3'
 Wait-Tag 'host' 'FISKELAGET_SESSION_DAY number=2' 400
 Start-Case 'recovery' '' 'recovery' '127.0.0.1:7777' 'guest3-profile' 0 7777 $true
 Wait-Tag 'recovery' 'FISKELAGET_EXPEDITION_PASSED mode=recovery' 90
 Wait-Tag 'recovery' 'FISKELAGET_SKIPPER_PASSED mode=reload' 30
 Wait-Tag 'recovery' 'FISKELAGET_ANGLER_PASSED mode=reload' 30
 foreach($role in 'host','guest1','guest2','solo'){Wait-Tag $role 'FISKELAGET_EXPLORATION_PASSED mode=full' 150}
 foreach($role in 'host','guest1','guest2','solo'){Wait-Tag $role 'FISKELAGET_SKIPPER_PASSED mode=full' 60}
 foreach($role in 'host','guest1','guest2','solo'){Wait-Tag $role 'FISKELAGET_ANGLER_PASSED mode=full' 60}
 foreach($role in 'host','guest1','guest2'){Wait-Tag $role "FISKELAGET_CAREER_RELEASE_PASSED mode=$role" 30}
 foreach($role in 'host','guest1','guest2','solo','recovery'){Stop-Case $role}
 # New server process reuses its confirmed authority archive. No client journal is imported.
 Start-Case 'restart-host' '' 'reload' '/Engine/Maps/Entry?listen' 'host-profile' 0 7777
 Wait-Tag 'restart-host' 'GameNetDriver.*listening on port 7777' 120
 Start-Case 'restart-guest' '' 'restart' '127.0.0.1:7777' 'guest3-profile' 0 7777 $true
 Wait-Tag 'restart-guest' 'FISKELAGET_EXPEDITION_PASSED mode=restart' 90
 Wait-Tag 'restart-host' 'FISKELAGET_EXPEDITION_PASSED mode=reload' 90
 Wait-Tag 'restart-host' 'FISKELAGET_SKIPPER_PASSED mode=reload' 30
 Wait-Tag 'restart-host' 'FISKELAGET_ANGLER_PASSED mode=reload' 30
 Wait-Tag 'restart-guest' 'FISKELAGET_SKIPPER_PASSED mode=reload' 30
 Wait-Tag 'restart-guest' 'FISKELAGET_ANGLER_PASSED mode=reload' 30
 Start-Sleep -Milliseconds 1000
 Stop-Case 'restart-host';Stop-Case 'restart-guest'
 foreach($role in 'guest1','guest2','solo'){
  Start-Case "reload-$role" '' 'reload' '/Engine/Maps/Entry?listen' "$role-profile" 0 7777
  Wait-Tag "reload-$role" 'FISKELAGET_EXPEDITION_PASSED mode=reload' 90
  Wait-Tag "reload-$role" 'FISKELAGET_SKIPPER_PASSED mode=reload' 30
  Wait-Tag "reload-$role" 'FISKELAGET_ANGLER_PASSED mode=reload' 30
  Start-Sleep -Milliseconds 1000
  Stop-Case "reload-$role"
 }
 Check-Failures
 'PASS: two normal exploration days, four-player multi-stop navigation, unanimous travel view, offline settlement/recovery, lost acknowledgement, own income, named rig reloads and authority restart.'
 "Evidence: $run"
}finally{foreach($app in $apps.Values){if($app -and !$app.HasExited){Stop-Process -InputObject $app;$app.WaitForExit()}}}
