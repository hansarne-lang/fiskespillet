<#
.SYNOPSIS
 Copies a packaged FISKELAGET build to its own version folder and publishes it to the SpilLan launcher.

.DESCRIPTION
 1. Reads the version from Config/DefaultGame.ini (ProjectVersion) and checks that the game code
    reports the same release.
 2. Copies the package (the Windows folder with FiskELaget.exe) to <ExportDirectory>\v<version>.
    Saved folders from earlier test runs are left out. An existing version folder is never
    overwritten: every new build needs a higher version, because SpilLan treats two different
    builds with the same number as a conflict.
 3. Copies Grafikk/spillan.png to the root of the version folder (the card image in SpilLan).
 4. Runs SpilLan.exe publiser. If SpilLan is not found, publishing is skipped with a warning and
    the script still succeeds.

 The game is started with -SaveToUserDir, so career, journal, settings and logs are kept in
 %LOCALAPPDATA%\FiskELaget\Saved and survive SpilLan replacing the version folder.

.EXAMPLE
 .\Tools\Build\Publish-SpilLan.ps1 -Package C:\Fiskespillet\pakke\Windows -ExportDirectory C:\Fiskespillet\eksport
.EXAMPLE
 # Publish an existing version folder again, for example after SpilLan was missing the first time
 .\Tools\Build\Publish-SpilLan.ps1 -Package C:\Fiskespillet\pakke\Windows -ExportDirectory C:\Fiskespillet\eksport -Republish
#>
param(
    [Parameter(Mandatory)][string]$Package,
    [Parameter(Mandatory)][string]$ExportDirectory,
    [string]$SpilLan = $(if ($env:SPILLAN) { $env:SPILLAN } else { 'C:\SpilLan\SpilLan.exe' }),
    [switch]$Republish
)
$ErrorActionPreference = 'Stop'
$GameId = 'fiskespillet'          # Fixed for good once published. Never 'spillan'.
$DisplayName = 'FISKELAGET'
$StartFile = 'FiskELaget.exe'      # Relative to the version folder
$GameArguments = @('-SaveToUserDir')
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

$ini = Get-Content -LiteralPath (Join-Path $root 'Config\DefaultGame.ini') -Raw
if ($ini -notmatch '(?m)^ProjectVersion=(\d+\.\d+\.\d+)\s*$') { throw 'ProjectVersion x.y.z missing in Config/DefaultGame.ini' }
$version = $Matches[1]
$gameInstance = Get-Content -LiteralPath (Join-Path $root 'Source\FiskGame\Private\UI\FiskGameInstance.cpp') -Raw
if ($gameInstance -notmatch [regex]::Escape("SetProjectVersion(TEXT(`"$version`"))")) {
    throw "FiskGameInstance.cpp does not report release $version. Update both versions together."
}

$repoRoot = [IO.Path]::GetFullPath($root).TrimEnd('\') + '\'
if ([IO.Path]::GetFullPath($ExportDirectory).StartsWith($repoRoot, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'ExportDirectory must be outside the repository (builds are never committed)'
}
$target = Join-Path $ExportDirectory "v$version"
$image = Join-Path $root 'Grafikk\spillan.png'
if ((Get-Item -LiteralPath $image).Length -gt 4MB) { throw 'Grafikk/spillan.png is larger than 4 MB' }

if (Test-Path -LiteralPath $target) {
    if (-not $Republish) {
        throw "$target already exists. Raise ProjectVersion (and the release in FiskGameInstance.cpp) for a new build, or use -Republish to publish this folder again unchanged."
    }
    "Using existing $target unchanged"
} else {
    foreach ($exe in (Join-Path $Package $StartFile), (Join-Path $Package 'FiskELaget\Binaries\Win64\FiskELaget.exe')) {
        if (-not (Test-Path -LiteralPath $exe -PathType Leaf)) { throw "Packaged executable missing: $exe" }
    }
    New-Item -ItemType Directory -Path $target -Force | Out-Null
    & robocopy $Package $target /E /XD Saved /NFL /NDL /NJH /NJS /NP | Out-Null
    if ($LASTEXITCODE -ge 8) { throw "robocopy failed with exit code $LASTEXITCODE" }
    Copy-Item -LiteralPath $image -Destination (Join-Path $target 'spillan.png')
    "Exported v$version to $target"
    "SHA256: $((Get-FileHash -LiteralPath (Join-Path $target 'FiskELaget\Binaries\Win64\FiskELaget.exe') -Algorithm SHA256).Hash)"
}
$global:LASTEXITCODE = 0

if (-not (Test-Path -LiteralPath $SpilLan -PathType Leaf)) {
    Write-Warning "SpilLan was not found at $SpilLan. Publishing skipped. Set SPILLAN or -SpilLan and run again with -Republish."
    return
}
$arguments = @('publiser', $target, '--spill', $GameId, '--navn', $DisplayName, '--versjon', $version,
    '--start', $StartFile, '--motor', 'unreal')
foreach ($value in $GameArguments) { $arguments += "--arg=$value" }
"== SpilLan publiser $GameId $version"
& $SpilLan @arguments
if ($LASTEXITCODE -ne 0) { throw "SpilLan publiser failed with exit code $LASTEXITCODE" }
"Published $DisplayName $version to SpilLan"
