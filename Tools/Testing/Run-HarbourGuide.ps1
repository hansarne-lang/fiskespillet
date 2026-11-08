param(
 [Parameter(Mandatory=$true)][string]$Package,
 [Parameter(Mandatory=$true)][string]$LogDirectory,
 [Parameter(Mandatory=$true)][ValidatePattern('^[a-f0-9]{40}$')][string]$SourceCommit,
 [Parameter(Mandatory=$true)][ValidatePattern('^[a-fA-F0-9]{64}$')][string]$ExpectedExecutableSha256
)
$ErrorActionPreference='Stop'
$exe=Join-Path $Package 'FiskELaget/Binaries/Win64/FiskELaget.exe'
if((Get-FileHash -LiteralPath $exe -Algorithm SHA256).Hash -ne $ExpectedExecutableSha256){throw 'Packaged executable does not match tested source receipt'}
if(Test-Path -LiteralPath $LogDirectory){throw 'Refusing to reuse test evidence'}
New-Item -ItemType Directory -Path $LogDirectory | Out-Null
$apps=@{};$logs=@()
function Launch($name,$url,$port,$role=-1,$reload=$false,$profile=''){
 $capture=Join-Path $LogDirectory $name;New-Item -ItemType Directory -Path $capture -Force | Out-Null
 if(!$profile){$profile=Join-Path $LogDirectory ($name+'-profile')}
 $data=Join-Path $profile 'Saved/Config/Fiskelaget';New-Item -ItemType Directory -Path $data -Force | Out-Null
 [IO.File]::WriteAllText((Join-Path $data 'FiskelagetSettings.ini'),"[Fiskelaget.Settings]`r`nVersion=1`r`nSound=1`r`nFrameRate=30`r`n",[Text.UTF8Encoding]::new($false))
 $log=Join-Path $LogDirectory ($name+'.log');$script:logs+=$log
 $mode=if($reload){'reload'}else{'guide'}
 $flags=@($url,"-port=$port",'-unattended','-nosplash','-multihome=127.0.0.1',"-FiskPierSmoke=$mode",('-abslog="{0}"' -f $log),('-UserDir="{0}"' -f $profile),('-FiskScreenshotDir="{0}"' -f $capture))
 if($role -ge 0){$flags+=@("-FiskPierPeer=$role",'-FiskGuideLan')}
 if($reload){$flags+=@('-nullrhi','-nosound','-ExecCmds="t.MaxFPS 10"')}else{$flags+=@('-windowed','-ResX=1280','-ResY=720','-RenderOffscreen')}
 $script:apps[$name]=Start-Process -FilePath $exe -WorkingDirectory $capture -ArgumentList $flags -PassThru -WindowStyle Hidden
}
function Await($name,$tag,$seconds){
 $until=(Get-Date).AddSeconds($seconds);$log=Join-Path $LogDirectory ($name+'.log')
 while((Get-Date) -lt $until){
  foreach($p in $logs){if((Test-Path $p) -and (Select-String -LiteralPath $p -Pattern 'FISKELAGET_GUIDE_FAILED|FISKELAGET_PIER_FAILED|Fatal error|Assertion failed|Unhandled Exception' -Quiet)){throw "Guide test failed: $p"}}
  if((Test-Path $log) -and (Select-String -LiteralPath $log -Pattern $tag -Quiet)){return}
  if($apps[$name].HasExited){throw "Test exited: $name"};Start-Sleep -Seconds 1
 };throw "Timeout: $name $tag"
}
function Stop-All {foreach($p in $apps.Values){if($p -and !$p.HasExited){Stop-Process -InputObject $p;$p.WaitForExit()}}}
try{
 foreach($port in 17940..17944){if(@(Get-NetUDPEndpoint -LocalPort $port -ErrorAction SilentlyContinue).Count){throw "Test port occupied: $port"}}
 Launch 'solo' '/Engine/Maps/Entry?listen' 17940
 Launch 'host' '/Engine/Maps/Entry?listen' 17941 0
 Await 'host' 'GameNetDriver.*listening on port 17941' 90
 Launch 'guest' '127.0.0.1:17941' 17941 1
 foreach($name in 'solo','host','guest'){Await $name 'FISKELAGET_GUIDE_PASSED' 650}
 Start-Sleep -Seconds 2;Stop-All
 $index=0;foreach($name in 'solo','host','guest'){$index++;Launch ('reload-'+$name) '/Engine/Maps/Entry?listen' (17941+$index) -1 $true (Join-Path $LogDirectory ($name+'-profile'));Await ('reload-'+$name) 'FISKELAGET_PIER_PASSED mode=reload' 90;Stop-All}
 @{result='Passed';sourceCommit=$SourceCommit;sha256=$ExpectedExecutableSha256;graphicalProcesses=3;physicalMachines=1;destinations=14;networkPeers=2;freshReloads=3} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $LogDirectory 'runner-result.json') -Encoding utf8
 'PASS: 14 destinations, actual key/menu takeover, three cameras, ordinary boarding, two guided LAN walkers and three fresh reloads'
}finally{Stop-All}
