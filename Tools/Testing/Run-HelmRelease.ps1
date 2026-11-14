param([Parameter(Mandatory)][string]$Package,[Parameter(Mandatory)][string]$LogDirectory)
# Hand steering (#176 step 3): one captain leaves the dock under the helm (W), turns (D), stops to make a
# fishing spot (S), moves on again and lets R's autopilot take the boat home. Uses the packaged game.
. (Join-Path $PSScriptRoot 'FiskTestCommon.ps1')
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe'
if(!(Test-Path -LiteralPath $exe -PathType Leaf)){throw 'Packaged executable missing'}
$run=Join-Path $LogDirectory (Get-Date -Format yyyyMMdd-HHmmss)
New-Item -ItemType Directory -Path $run -Force | Out-Null
$hostLog=Join-Path $run 'host.log'
function Wait-Event($log,$pattern,$app,$seconds){
 $deadline=(Get-Date).AddSeconds((FiskTimeout $seconds))
 while((Get-Date) -lt $deadline){
  if($app.HasExited){throw "Process exited before $pattern. $log"}
  if(Test-Path -LiteralPath $log){
   if(Select-String -LiteralPath $log -Pattern 'FISKELAGET_HELM_SMOKE_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet){throw "Runtime failure in $log"}
   if(Select-String -LiteralPath $log -Pattern $pattern -Quiet){return}
  }
  Start-Sleep -Seconds 1
 }
 throw "Timeout $pattern. $log"
}
$hostApp=$null
try{
 if(@(Get-NetUDPEndpoint -LocalPort $FiskPort0 -ErrorAction SilentlyContinue).Count){throw "Port $FiskPort0 is occupied"}
 $capture=Join-Path $run 'host';New-Item -ItemType Directory -Path $capture -Force | Out-Null
 $flags=@('/Engine/Maps/Entry?listen','-FiskHelmSmoke','-windowed','-ResX=1280','-ResY=720','-RenderOffscreen','-unattended','-nosplash','-multihome=127.0.0.1',$FiskClockFlag,"-port=$FiskPort0",('-abslog="{0}"' -f $hostLog),('-FiskScreenshotDir="{0}"' -f $capture),('-UserDir="{0}"' -f (Join-Path $run 'host-settings')))
 $hostApp=Start-Process -FilePath $exe -WorkingDirectory (Split-Path -Parent $exe) -ArgumentList $flags -PassThru -WindowStyle Hidden
 Wait-Event $hostLog 'FISKELAGET_HELM_SMOKE_PASSED' $hostApp 600
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_HELM_SMOKE_PASSED').Count -ne 1){throw 'Expected one pass'}
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_HELM_TAKEN').Count -lt 2){throw 'Expected the helm taken at the dock and again at the fishing spot'}
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_HELM_STOPPED').Count -ne 1){throw 'Expected one stop that made a fishing spot'}
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_VOYAGE_LEG serial=\d+ returning=1').Count -ne 1){throw 'Expected R to start one autopilot leg home'}
 if(@(Select-String -LiteralPath $hostLog -Pattern 'FISKELAGET_MARINA_REACHED').Count -lt 1){throw 'Boat never reached the marina'}
 Start-Sleep -Seconds 2
 foreach($name in 'helm-underway','helm-stopped'){
  $path=Join-Path $capture "$name.png"
  if(!(Test-Path -LiteralPath $path) -or (Get-Item -LiteralPath $path).Length -lt 1000){throw "Missing screenshot $path"}
 }
 'PASS: departure under the helm, rudder turn, stop with the throttle released makes a fishing spot, moving on again and R autopilot home with marina arrival.'
 "Evidence: $run"
}
finally{if($hostApp -and !$hostApp.HasExited){Stop-Process -InputObject $hostApp}}
