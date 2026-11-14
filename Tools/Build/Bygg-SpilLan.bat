@echo off
setlocal EnableExtensions
title FISKELAGET - bygg og publiser til SpilLan
rem Dobbeltklikk for aa bygge FISKELAGET og publisere det til SpilLan.
rem Valgfritt: gren som argument, f.eks.  Bygg-SpilLan.bat release/0.24.0-spillan
rem Stier kan overstyres med miljoevariablene FISK_REPO, UE_ROOT og SPILLAN.

set "BRANCH=release/0.24.0-spillan"
if not "%~1"=="" set "BRANCH=%~1"
if not defined UE_ROOT set "UE_ROOT=C:\Program Files\Epic Games\UE_5.8"
if not defined SPILLAN set "SPILLAN=C:\SpilLan\SpilLan.exe"
set "PAKKE=C:\Fiskespillet\pakke-spillan"
set "LOGG=C:\Fiskespillet\logs"
set "EKSPORT=C:\Fiskespillet\eksport"

echo.
echo === FISKELAGET: bygg og publiser til SpilLan ===
echo Gren: %BRANCH%
echo.

rem --- Finn fiskespillet-mappa ---
set "REPO="
if exist "%~dp0FiskELaget.uproject" set "REPO=%~dp0."
if not defined REPO if exist "%~dp0..\..\FiskELaget.uproject" set "REPO=%~dp0..\.."
if not defined REPO if defined FISK_REPO set "REPO=%FISK_REPO%"
if not defined REPO if exist "C:\Fiskespillet\main\FiskELaget.uproject" set "REPO=C:\Fiskespillet\main"
if not defined REPO set /p "REPO=Fant ikke fiskespillet-mappa. Lim inn stien til den: "
if not exist "%REPO%\FiskELaget.uproject" (
    echo FEIL: %REPO% er ikke fiskespillet-mappa ^(FiskELaget.uproject mangler^).
    goto :feil
)

rem --- Sjekk verktoeyene ---
where git >nul 2>nul || (echo FEIL: Git er ikke installert. & goto :feil)
where python >nul 2>nul || where py >nul 2>nul || (echo FEIL: Python er ikke installert. & goto :feil)
if not exist "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" (
    echo FEIL: Fant ikke Unreal Engine i "%UE_ROOT%". Sett UE_ROOT til riktig mappe.
    goto :feil
)
if not exist "%SPILLAN%" (
    echo FEIL: Fant ikke SpilLan i "%SPILLAN%". Sett SPILLAN til riktig sti.
    goto :feil
)
set "SPV="
for /f "usebackq delims=" %%v in (`"%SPILLAN%" versjon 2^>nul`) do set "SPV=%%v"
powershell -NoProfile -Command "try { if ([version]('%SPV%' -replace '-.*$','') -lt [version]'0.2.8') { exit 1 } } catch { exit 1 }"
if errorlevel 1 (
    echo FEIL: SpilLan er versjon "%SPV%". Trenger 0.2.8 eller nyere.
    echo Last ned SpilLan-windows fra https://github.com/hansarne-lang/spillan/actions og erstatt "%SPILLAN%".
    goto :feil
)
echo SpilLan %SPV% funnet.

rem --- Hent grenen ---
pushd "%REPO%" || goto :feil
set "START="
for /f "delims=" %%b in ('git rev-parse --abbrev-ref HEAD') do set "START=%%b"
git status --porcelain | findstr . >nul && (
    echo FEIL: fiskespillet-mappa har endringer som ikke er committet. Rydd eller commit dem foerst.
    goto :feil_pop
)
echo Henter %BRANCH% ...
git fetch origin || goto :feil_pop
git checkout -q -B "%BRANCH%" "origin/%BRANCH%" || goto :feil_pop

rem --- Bygg, pakk og publiser (10-15 minutter, foerste gang lenger) ---
powershell -NoProfile -ExecutionPolicy Bypass -File "Tools\Testing\Test-PR.ps1" -Level Build -EngineRoot "%UE_ROOT%" -ArchiveDirectory "%PAKKE%" -LogDirectory "%LOGG%" -SpilLanExport "%EKSPORT%"
set "RESULT=%ERRORLEVEL%"
if defined START git checkout -q "%START%"
popd
if not "%RESULT%"=="0" goto :feil

echo.
echo === FERDIG ===
echo Aapne SpilLan og trykk Installer og saa Start paa FISKELAGET-kortet.
echo.
pause
exit /b 0

:feil_pop
if defined START git checkout -q "%START%" 2>nul
popd
:feil
echo.
echo Noe gikk galt. Kopier teksten over og lim den inn hos Claude.
echo.
pause
exit /b 1
