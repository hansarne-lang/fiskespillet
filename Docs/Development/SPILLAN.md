# FISKELAGET i SpilLan

FISKELAGET kan installeres, oppdateres og startes fra launcheren SpilLan (`hansarne-lang/spillan`, 0.2.8 eller nyere). Oppgaven er issue #130.

## Faste verdier

| Hva | Verdi |
|---|---|
| Spill-id | `fiskespillet` (kan ikke endres etter første publisering) |
| Visningsnavn | `FISKELAGET` |
| Versjon | `ProjectVersion` i `Config/DefaultGame.ini`. Den må stemme med `SetProjectVersion` i `FiskGameInstance.cpp` |
| Startfil | `FiskELaget.exe` i rota av versjonsmappa |
| Faste argumenter | `-SaveToUserDir` |
| Motor | `unreal` |
| Bilde | [`Grafikk/spillan.png`](../../Grafikk/spillan.png), 960 × 540, kopieres til rota av hver eksport |
| Porter | 7777 og 17784. SpilLan bruker 47800 og 47801, og dem bruker ikke spillet |

## Bygge og publisere

```powershell
.\Tools\Build\Build-Fisk.ps1 -EngineRoot 'C:\Program Files\Epic Games\UE_5.8' -ArchiveDirectory C:\Fiskespillet\pakke -SpilLanExport C:\Fiskespillet\eksport
```

Til slutt kjører byggeskriptet `Tools/Build/Publish-SpilLan.ps1`. Det gjør dette:

1. Kopierer pakken (`pakke\Windows`) til `eksport\v<versjon>`. `Saved`-mapper fra tidligere testkjøringer blir ikke med.
2. Legger `spillan.png` i rota av versjonsmappa.
3. Kjører `SpilLan.exe publiser`. Stien er `SPILLAN`, ellers `C:\SpilLan\SpilLan.exe`. Finnes ikke SpilLan, hoppes publiseringen over med en advarsel. Kjør da skriptet igjen med `-Republish` når SpilLan er på plass.

Skriptet nekter å skrive over en versjonsmappe som finnes fra før. Hvert nytt bygg må ha et høyere versjonsnummer, fordi to ulike bygg med samme nummer gir konflikt i SpilLan. Øk både `ProjectVersion` og `SetProjectVersion` i samme endring. Byggeskriptet nekter også å publisere med endringer som ikke er committet.

## Lagring

SpilLan bytter ut hele versjonsmappa ved oppdatering og sletter gamle versjoner. En Development-pakke lagrer ellers i `FiskELaget\Saved` inne i pakken. SpilLan starter derfor spillet med `-SaveToUserDir`, og da ligger karriere, journal, naust, innstillinger og logger i `%LOCALAPPDATA%\FiskELaget\Saved`.

Har du spilt fra en vanlig pakke før, ligger lagringene dine i `<pakke>\Windows\FiskELaget\Saved\Config\Fiskelaget`. Kopier den mappa til `%LOCALAPPDATA%\FiskELaget\Saved\Config\Fiskelaget` før første start fra SpilLan for å beholde karrieren.

## Spillernavn

SpilLan starter spillet med `--spillan --navn <spillernavn>`. Navnet blir standard visningsnavn for en ny profil, men bare hvis du ikke har lagret et eget navn i naustet (H). Tegn som ikke er bokstaver, tall, mellomrom, `-` eller `_`, fjernes, og navnet kortes til 24 tegn. Uten argumentene starter spillet som før.

Loggen viser `FISKELAGET_LAUNCHER spillan=<0|1> name=<0|1> userdir=<0|1>` ved oppstart.

## Sjekkliste før første publisering

- [ ] Spill-id `fiskespillet` og visningsnavn `FISKELAGET` er godkjent av prosjekteieren
- [x] `spillan.png` (960 × 540, under 4 MB) legges i rota av eksportmappa
- [x] Versjonsnummeret må øke for hvert bygg (skriptet nekter å skrive over)
- [ ] Versjonsmappa starter på en ren maskin uten Unreal (Visual C++-runtime kan trengs)
- [x] Lagring og innstillinger ligger i brukermappa (`-SaveToUserDir`)
- [x] Spillet tåler `--spillan --navn X`
- [x] Byggeskriptet kjører `SpilLan.exe publiser …` til slutt
- [ ] Bygget, startet og publisert på Windows (se PR-en for testresultater)
