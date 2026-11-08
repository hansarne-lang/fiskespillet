param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$stamp=Get-Date -Format yyyyMMdd-HHmmss
$run=Join-Path $LogDirectory $stamp
New-Item -ItemType Directory -Path $run -Force | Out-Null
# Each process has its own profile so host and client get different careers (#126). The client
# still proves settings persistence: only the settings files the host saved are copied to it.
$profiles=@{host=(Join-Path $run 'host-profile');join=(Join-Path $run 'join-profile');failure=(Join-Path $run 'failure-profile')}
$flags=@('-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag)
$hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log';$failureLog=Join-Path $run 'failure.log'
function Events($log,$pattern){@(Select-String -LiteralPath $log -Pattern $pattern)}
function Exactly($log,$pattern,$count){$found=@(Events $log $pattern);if($found.Count -ne $count){throw "Expected $count events for $pattern; got $($found.Count). $log"}}
function Wait-Event($log,$pattern,$process,$seconds){
 $deadline=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $deadline){
  if($process.HasExited){throw "Process exited before $pattern. $log"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_MENU_TEST_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $pattern -Quiet){return}
  }
  Start-Sleep -Seconds 1
 }
 throw "Timeout $pattern. $log"
}
function Copy-SettingsOnly($from,$to){
 $settings=Join-Path $from 'Saved\Config\Fiskelaget\FiskelagetSettings.ini'
 if(!(Test-Path -LiteralPath $settings -PathType Leaf)){throw "Host settings file missing: $settings"}
 $files=@(Get-Item -LiteralPath $settings)+@(Get-ChildItem -LiteralPath $from -Filter GameUserSettings.ini -Recurse -File)
 foreach($file in $files){
  $relative=$file.FullName.Substring($from.TrimEnd('\').Length).TrimStart('\')
  $target=Join-Path $to $relative
  New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force | Out-Null
  Copy-Item -LiteralPath $file.FullName -Destination $target
 }
 $copied=@(Get-ChildItem -LiteralPath $to -File -Recurse | Where-Object { $_.Name -notin 'FiskelagetSettings.ini','GameUserSettings.ini' })
 if($copied.Count){throw "Only settings may be shared with the client; found $($copied.Name -join ', ')"}
}
function Start-Case($role,$log){
 $capture=Join-Path $run $role
 New-Item -ItemType Directory -Path $capture -Force | Out-Null
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@('/Engine/Maps/Entry',"-FiskMenuSmoke=$role",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f $profiles[$role]))+$flags) -PassThru -WindowStyle Hidden
}
$clientApp=$null;$failureApp=$null;$hostApp=$null
try{
 $busy=@(Get-NetUDPEndpoint -LocalPort 7777,17784 -ErrorAction SilentlyContinue)
 if($busy.Count){throw 'Menu test ports are already in use'}
 $hostApp=Start-Case 'host' $hostLog
 Wait-Event $hostLog 'FISKELAGET_MENU_HOST_READY' $hostApp 120
 Wait-Event $hostLog 'FISKELAGET_SETTINGS_SAVED sound=0 fps=30' $hostApp 30
 Copy-SettingsOnly $profiles.host $profiles.join
 $clientApp=Start-Case 'join' $clientLog
 Wait-Event $hostLog 'FISKELAGET_MENU_GAMEPLAY_PASSED crew=2 rods=2' $hostApp 230
 Wait-Event $clientLog 'FISKELAGET_MENU_CLIENT_GAMEPLAY_PASSED' $clientApp 20
 Wait-Event $hostLog 'FISKELAGET_MENU_RETURNED_HOME' $hostApp 30
 Wait-Event $clientLog 'FISKELAGET_MENU_DISCONNECT_RECOVERED' $clientApp 40
 Exactly $hostLog 'FISKELAGET_MENU_INPUT_ISOLATED' 1
 Exactly $hostLog 'FISKELAGET_MENU_VALIDATION_PASSED' 1
 Exactly $hostLog 'FISKELAGET_SETTINGS_SAVED sound=0 fps=60' 1
 Exactly $hostLog 'FISKELAGET_SETTINGS_SAVED sound=0 fps=30' 1
 Exactly $clientLog 'FISKELAGET_SETTINGS_LOADED sound=0 fps=30' 1
 Exactly $clientLog 'FISKELAGET_MENU_SETTINGS_PERSISTED' 1
 Exactly $clientLog 'FISKELAGET_MENU_POINTER row=1' 1
 Exactly $hostLog 'FISKELAGET_MENU_CLOCK_CONTINUED' 1
 Exactly $hostLog 'FISKELAGET_MENU_HOST$' 1
 Exactly $hostLog 'FISKELAGET_MENU_LEAVE$' 1
 Exactly $hostLog 'FISKELAGET_ROD_FISHING player=' 2
 Exactly $hostLog 'FISKELAGET_TRIP_ARRIVED crew=2' 1
 if(@(Events $hostLog 'FISKELAGET_CAREER_READY player=').Count -lt 2){throw 'Host did not accept two separate careers'}
 if(@(Get-ChildItem -LiteralPath $profiles.host -Filter GameUserSettings.ini -Recurse).Count -lt 1){throw 'Saved settings file missing'}
 Stop-Process -InputObject $clientApp; $clientApp.WaitForExit(); Stop-Process -InputObject $hostApp; $hostApp.WaitForExit()
 $failureApp=Start-Case 'failure' $failureLog
 Wait-Event $failureLog 'FISKELAGET_MENU_FAILURE_PASSED' $failureApp 65
 if(!$failureApp.WaitForExit(15000)){throw 'Quit button did not close process'}
 if($failureApp.ExitCode -ne 0){throw "Nonzero quit exit $($failureApp.ExitCode)"}
 Exactly $failureLog 'FISKELAGET_MENU_CANCELLED' 1
 if(@(Events $failureLog 'FISKELAGET_MENU_CONNECTION_FAILED').Count -lt 1){throw 'No connection failure reported'}
 Exactly $failureLog 'FISKELAGET_MENU_QUIT' 1
 foreach($role in 'host','join','failure'){
  $shots=switch($role){'host'{@('home','invalid-address','settings-before','settings-after','session','returned-home')};'join'{@('home','join','disconnect')};'failure'{@('connecting','cancelled','connection-failed')}}
  foreach($shot in $shots){$path=Join-Path (Join-Path $run $role) "$shot.png";if(!(Test-Path -LiteralPath $path) -or (Get-Item -LiteralPath $path).Length -lt 1000){throw "Missing screenshot $path"}}
 }
 'PASS: native menu navigation, settings persistence, IPv4 join, cancellation/failure, host/client gameplay, continuing clock, disconnect recovery and quit.'
 "Evidence: $run"
}
finally{
 foreach($app in $failureApp,$clientApp,$hostApp){if($app -and !$app.HasExited){Stop-Process -InputObject $app}}
}
