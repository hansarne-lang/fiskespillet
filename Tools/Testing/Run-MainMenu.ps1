param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$stamp=Get-Date -Format yyyyMMdd-HHmmss
$run=Join-Path $LogDirectory $stamp
New-Item -ItemType Directory -Path $run -Force | Out-Null
$userDir=Join-Path $run 'settings'
$flags=@('-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',('-UserDir="{0}"' -f $userDir))
$hostLog=Join-Path $run 'host.log';$clientLog=Join-Path $run 'client.log';$failureLog=Join-Path $run 'failure.log'
function Events($log,$pattern){@(Select-String -LiteralPath $log -Pattern $pattern)}
function Exactly($log,$pattern,$count){$found=@(Events $log $pattern);if($found.Count -ne $count){throw "Expected $count events for $pattern; got $($found.Count). $log"}}
function Wait-Event($log,$pattern,$process,$seconds){
 $deadline=(Get-Date).AddSeconds($seconds)
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
function Start-Case($role,$log){
 $capture=Join-Path $run $role
 New-Item -ItemType Directory -Path $capture -Force | Out-Null
 Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList (@('/Engine/Maps/Entry',"-FiskMenuSmoke=$role",('-abslog="{0}"' -f $log),('-FiskScreenshotDir="{0}"' -f $capture))+$flags) -PassThru -WindowStyle Hidden
}
$clientApp=$null;$failureApp=$null;$hostApp=$null
try{
 $busy=@(Get-NetUDPEndpoint -LocalPort 7777,17784 -ErrorAction SilentlyContinue)
 if($busy.Count){throw 'Menu test ports are already in use'}
 $hostApp=Start-Case 'host' $hostLog
 Wait-Event $hostLog 'FISKELAGET_MENU_HOST_READY' $hostApp 120
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
 if(@(Get-ChildItem -LiteralPath $userDir -Filter GameUserSettings.ini -Recurse).Count -lt 1){throw 'Saved settings file missing'}
 Stop-Process -InputObject $clientApp; $clientApp.WaitForExit(); Stop-Process -InputObject $hostApp; $hostApp.WaitForExit()
 $failureApp=Start-Case 'failure' $failureLog
 Wait-Event $failureLog 'FISKELAGET_MENU_FAILURE_PASSED' $failureApp 65
 if(!$failureApp.WaitForExit(15000)){throw 'Quit button did not close process'}
 if($failureApp.ExitCode -ne 0){throw "Nonzero quit exit $($failureApp.ExitCode)"}
 Exactly $failureLog 'FISKELAGET_MENU_CANCELLED' 1
 Exactly $failureLog 'FISKELAGET_MENU_CONNECTION_FAILED' 1
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
