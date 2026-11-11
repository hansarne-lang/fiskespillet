# FISKELAGET – Arkitektur

Status: arkitektur godkjent av prosjekteieren 3. november 2026. [Kravspesifikasjon v1.1](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md) er styrende; UE5 er valgt i masterprompten.

## Modulær monolitt

Ett UE-prosjekt `FiskELaget.uproject`, med to innledende runtime-moduler: `FiskCore` og `FiskGame`. Ikke én plugin per funksjon. En egen editor-modul opprettes bare ved faktisk behov for editorverktøy. Modulnavnet `FiskGame` er en fysisk pakking av de logiske funksjonene, ikke en tiende spillmodul.

| Logisk modul | Eier data og regler | Foreslått plassering |
|---|---|---|
| FiskCore | Sterke ID-er, delte verdiobjekter, kontrakter, tjenestegrensesnitt, versjoner | `Source/FiskCore/` |
| Fishing | Stangtilstand, napp, kroking, kamp og landing | `Source/FiskGame/Private/Fishing/` |
| Boats | Båt, kaptein, lager, transport, sleping og retur | `Source/FiskGame/Private/Boats/` |
| World | Habitat, dybde, kart, sjøpunkter og ekkolodd | `Source/FiskGame/Private/World/` |
| DayCycle | Felles klokke, faser, frist og dags-ID | `Source/FiskGame/Private/DayCycle/` |
| Economy | Priser, kjøp, salg, premier og hovedbok | `Source/FiskGame/Private/Economy/` |
| Career | Karriere, utstyr, journal, rekorder og varig profil | `Source/FiskGame/Private/Career/` |
| Multiplayer | LAN-økter, identitet, replisering, meldingsvalidering | `Source/FiskGame/Private/Multiplayer/` |
| UI | Menyer og lesemodeller for HUD, kart og marina | `Source/FiskGame/Private/UI/` |

FiskCore skal ikke kjenne konkrete funksjonsimplementasjoner eller UI-ressurser. Funksjonsområdene skal ikke inkludere hverandres private headere. Kun grensesnitt og verdiobjekter som faktisk deles plasseres i FiskCore. FiskGame sitt integrasjonslag setter sammen implementasjonene og ruter hendelser/kommandoer.

```mermaid
flowchart TD
  Game[FiskGame integrasjonslag] --> Core[FiskCore kontrakter og porter]
  Features[Fishing Boats World DayCycle Economy Career Multiplayer UI] --> Core
  Core --> UE[Unreal basismoduler]
```

Pilene viser kompileringsavhengigheter. Funksjonsområdene har ingen direkte kompileringsavhengigheter til hverandre. UE-avhengigheter holdes minimale og listes eksplisitt i Build.cs ved opprettelse. En enkel grensekontroll i CI skal finne ulovlige includes; dette er en praktisk avgrensning, ikke et fullstendig språkbasert vern når områdene ligger i samme fysiske modul.

### Include-grenser i FiskGame

`Tools/Validation/check_includes.py` kjøres i CI. Hver mappe under `Source/FiskGame/Private/` er et område: Angler, Boats, Bootstrap, Career, Collections, DayCycle, Economy, Expedition, Fishing, UI, Voyage og World.

| Fil i | Kan inkludere |
|---|---|
| Et funksjonsområde | Eget område, FiskCore, `Source/FiskGame/Public` og Unreal Engine |
| `Bootstrap/` og `FiskGameModule.cpp` (integrasjonslaget) | Alle områder |
| `Source/FiskGame/Public` | FiskCore og Unreal Engine |

Et funksjonsområde inkluderer ikke andre områder eller integrasjonslaget. Avhengigheter mellom områder går gjennom grensesnitt og verdiobjekter i FiskCore. En ny tillatt avhengighet krever arkitekturgodkjenning og en oppføring i `ALLOWED` i kontrollen.

Brudd som fantes da kontrollen ble innført (420 includes i 114 filer, november 2026), står i `Tools/Validation/include_exceptions.json`. Listen kan bare krympe. Nye brudd feiler CI, og det gjør også unntak som ikke lenger brukes. Fjern dem med `python Tools/Validation/check_includes.py --prune`. `--summary` viser bruddene per områdepar.

v0.17 bruker motorens innebygde `ProceduralMeshComponent` i FiskGame for formet skrog, havflate og slepefisk. Pluginen aktiveres eksplisitt i prosjektfilen og er en privat renderingsavhengighet i FiskGame. FiskCore beholder bare Core/CoreUObject; ingen ny spillmodul, kjøpt ressurs eller tjeneste er innført. Bootstrapkontrollen håndhever den utvidede, eksplisitte avhengighetslisten.

### Spillerkontrolleren og komponentene

`AFiskBoardingController` (`Boats/`) er spillerens `APlayerController`. Den leser input, ruter den til riktig del, eier panelene, kameraet og den korte meldingslinjen, og holder spillerens stenger og valgt stang. Nettverkskallene for hvert område ligger i egne `UActorComponent`-er som er replikert og eid av kontrolleren, så `Server`- og `Client`-kallene går over spillerens egen tilkobling (#113, [plan](Docs/Development/CONTROLLER_SPLIT_PLAN.md)):

| Komponent | Ansvar |
|---|---|
| `UFiskPierWalk` (`World/`) | Gange på brygga |
| `UFiskMarinaSync` | Naust, strandbar og havn |
| `UFiskRecoverySync` | Gjenhenting av journal og ekspedisjonsposter etter tilkobling |
| `UFiskCareerSync` | Karriere, butikk, båtvalg, 17:00-oppgjøret og strandbarens veddemål |
| `UFiskFleetSync` | Valg, henting og forlating av båt i flåten |
| `UFiskExpeditionCommands` | Dagens ekspedisjonsmål, klarmarkering og meldinger |
| `UFiskBoatCommands` | Ombordstigning, avreise, retur, rute, reisevisning og kaptein |
| `UFiskRodCommands` | Stangvalg, rigg, kast, kamphandlinger og stangsett |

Komponentene uten egen mappe ligger i `Boats/Controller/`. De leser og endrer kontrollerens felles tilstand bare gjennom dens offentlige funksjoner. Røyktestenes tilstand ligger i `FFiskBoardingSmokeState`, ikke i kontrollerens header. Når avhengighetene går gjennom FiskCore, kan komponentene flyttes til sitt logiske område.

## Kommunikasjon og serverautoritet

Multiplayer oversetter en autentisert spillers forespørsel til en validert kommando hos eiermodulen. Eiermodulen godkjenner selv rettigheter og tilstand. Serveren eier spillutfallet; klienten viser repliserte lesemodeller. UI skriver aldri direkte i økonomi, båt eller profil.

World leverer habitat- og dybdeoppslag gjennom porter. Fishing publiserer landing; Boats avgjør lagring/transport; DayCycle avgjør tidsfrist; Economy godkjenner salg; Career materialiserer bekreftede endringer i varig profil. Ingen mottaker skal kunne endre avsenderens interne objekter.

I første prototype kjøres autoritative spilloverganger sekvensielt i serverens simulasjonskontekst. Tunge IO-jobber skal ikke endre spilltilstand fra egne tråder. Dokumentert rekkefølge og oppgjørsbarriere sikrer at salg ikke blir avhengig av tilfeldig leveringsrekkefølge mellom `BoatArrived` og `DayDeadlineReached`.

## Oppgjør og eierskap

Boats eier lagerposter. Fishing eier kampframdrift, men ingen salgsrett. Economy eier transaksjoner; Career eier den lagrede projeksjonen, ikke en ekstra uavhengig pengesaldo som kan godkjenne kjøp. Salg bruker serverbekreftet fangst-ID, båtankomst, dags-ID og oppgjørsstatus. Unik transaksjonsnøkkel hindrer dobbel utbetaling; transaksjon og utgående bekreftelse må kunne gjenopptas etter krasj.

Fristen er absolutt ved minutt 17. Eksakt registrering av grenseankomst og marinasone er en åpen designavklaring fra kravspesifikasjonen og skal godkjennes før implementasjon. Ingen tidsbuffer eller «nesten framme»-unntak innføres.

Dagens bekreftede fangstresultater skal holdes skjult for alle, også fisker og mannskap, til felles avsløring ved minutt 17. Dette gjelder art, eksakt vekt/lengde, antall, verdi, journal, statistikk, plasseringer og nye rekorder. Serveren registrerer landing og levering fortløpende, men salg, fangstpremier og veddemålsgevinster krediteres først ved oppgjøret. Tidligere disponible penger kan brukes før dette.

Først kontrolleres levering og frist mot det autoritative grunnlaget; deretter gjennomføres felles avsløring og økonomisk oppgjør. Tidlig ankomst gir ingen salgsrett før minutt 17. Retry og gjenoppkobling skal ikke utløse ekstra utbetaling. Den samme avsløringsregelen gjelder HUD, journal, rekordvarsler, trofévegg og alle klientlesemodeller. Skjulte data skal ikke sendes i underliggende klientpayload før riktig tidspunkt. Fiskens skjulte utholdenhet skal fortsatt ikke vises som helsebar. En full serverhendelse er derfor ikke en offentlig nettverkspakke.

Reisehopp er et valg av presentasjon etter enighet fra alle om bord. Serverklokke, båtens fysiske reise og ankomsttid fortsetter uendret. Det endrer ikke kapteinens myndighet til å starte retur. Strandbar, naust, offshorebalanse og veddemål bygges innenfor de eksisterende logiske modulene; ingen ny fysisk runtime-modul opprettes av disse designvalgene.

### Tillit til karriereprofiler på LAN

Serverautoriteten gjelder spillet i den pågående økten: dagsklokke, fangst, levering, oppgjør, kjøp og nye kvitteringer. Den gjelder ikke karrierehistorikk som spilleren tar med fra andre verter.

Når en spiller kobler til, sender klienten sin lagrede karriere. Verten kontrollerer at kvitteringskjeden henger sammen (`FFiskCareerProfile::IsValid`), at profilen ikke er i konflikt med vertens egen kopi (`Extends`), og at utbetalinger fra dager i vertens egen journal stemmer (`FiskJournalAuthority::RecoverIncome`). Utbetalinger fra andre verter kan ikke kontrolleres og godtas slik de er. En endret klient kan dermed bringe med seg oppdiktet saldo. Dette er en godkjent begrensning (DECISIONS.md, 8. november 2026), ikke en feil som skal rettes i prototypen.

Mens spilleren er tilkoblet, kan klienten ikke kreditere seg selv: nye utbetalinger lages bare av vertens oppgjør, og kjøp kontrolleres mot saldoen verten har godtatt. Ved hver ny tilkobling, også til samme vert, gjelder tilliten over. Ekstra utbetalinger som legges etter vertens kopi, godtas da.

## Innhold og mulig videre oppdeling

Bruk `Content/FiskELaget/<Modul>/` for ressurseierskap. Felles kart og binære ressurser redigeres av én agent om gangen. Splitt logiske områder til flere fysiske UE-moduler først når avhengigheter, byggetid eller teamarbeid viser et konkret behov, og registrer endringen i DECISIONS.md.
