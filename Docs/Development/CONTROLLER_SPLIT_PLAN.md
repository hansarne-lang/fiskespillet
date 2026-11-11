# Plan: oppdeling av AFiskBoardingController (#113)

Status: godkjent av prosjekteieren 8. november 2026 i [PR #122](https://github.com/hansarne-lang/fiskespillet/pull/122#issuecomment-6719895702) for revisjon ec611e0, med forslagene slik de står: komponenter i `Boats/Controller/` først, og hvert steg integreres for seg. Steg 1 er levert i samme PR. Hvert av stegene 2–9 er en egen PR som bygges og LAN-testes på Windows før neste steg. Steg 2 starter når steg 1 er bygget og testet.

## Utgangspunkt

`AFiskBoardingController` (`Source/FiskGame/Private/Boats/`) er spillerens `APlayerController` og har vokst til å eie nesten all spillerlogikk:

- 43 nettverkskall (`UFUNCTION(Server|Client)`) fra ni ulike områder.
- Omtrent 1 070 linjer i ni `.cpp`-filer og en header på 324 linjer.
- 71 filer inkluderer headeren, så hver endring der kompilerer store deler av FiskGame på nytt.
- Over halvparten av bruddene på include-grensene (#114) går gjennom denne klassen.

## Mål

Kontrolleren skal bare gjøre det en `APlayerController` må: lese input, rute den til riktig del, holde panelene og kameraet og vise kortvarige meldinger. Hvert område eier sine egne nettverkskall og sin egen tilstand i en `UActorComponent` som henger på kontrolleren.

Ingen spillregler, protokollregler eller lagringsformater endres. Navnene på nettverkskallene endres ikke i stegene. De flyttes bare til en komponent.

## Teknisk mønster

- Hver komponent er en `UActorComponent` med `SetIsReplicatedByDefault(true)`, opprettet i kontrollerens konstruktør med `CreateDefaultSubobject`, slik `UFiskPierWalk` og `UFiskFishingHelp` allerede gjør.
- Komponentens eier er spillerens `APlayerController`, så `Server`- og `Client`-kall fungerer over samme tilkobling som i dag.
- Kontrolleren får en tynn `Get…()`-tilgang til hver komponent. Kode som i dag kaller `PC->ServerX(...)`, kaller `PC->GetY()->ServerX(...)`.
- Komponentene leser kontrollerens felles tilstand (valgt båt, `PlayerState`, `ClientActionFeedback`) gjennom kontrollerens offentlige funksjoner, ikke gjennom `friend`.
- Hvor komponentene skal ligge, er et valg som må godkjennes. Forslaget er å legge dem i `Boats/Controller/` til alt er flyttet, og deretter flytte dem til sitt logiske område når avhengighetene går gjennom FiskCore.

## Steg

| Steg | Innhold | Nettverkskall som flyttes | Risiko |
|---|---|---|---|
| 1 ✅ | Røyktesttilstand ut av headeren til `FFiskBoardingSmokeState` (`Boats/FiskBoardingSmokeState.h`) | Ingen | Lav: bare testkode, ingen nettverksendring |
| 2 ✅ | Brygge: flytt kallene inn i den eksisterende `UFiskPierWalk` | `ServerPierMode`, `ServerPierInput`, `ClientPierMode` | Lav |
| 3 ✅ | `UFiskMarinaSync` (`Boats/Controller/FiskMarinaSync.*`): naust, bar og havn. `.Marina.cpp` er flyttet dit, med sine include-unntak (antallet er uendret) | `ServerMarinaIdentity`, `ClientMarinaIdentity`, `ServerMarinaRecords`, `ServerMarinaSay`, `ServerHarbourPlace` | Lav |
| 4 ✅ | `UFiskRecoverySync` (`Boats/Controller/FiskRecoverySync.*`): gjenhenting av journal og ekspedisjonsposter. `.Recovery.cpp` er flyttet dit; ekspedisjonskommandoene (steg 7) ligger til da i `.Fleet.cpp`. Unntakene fulgte med, og ett ble overflødig (418) | `ClientJournalBatch`, `ServerJournalAck`, `ClientExpeditionBatch`, `ServerExpeditionAck` | Middels: må tåle frakobling og ny tilkobling |
| 5 ✅ | `UFiskCareerSync` (`Boats/Controller/FiskCareerSync.*`): karriere, butikk, båtvalg, 17:00-oppgjøret og strandbarens veddemål (`ServerPlaceBet`, `ServerBetBoard`, `ClientBetBoard` fra #22). Karrierepanelet (P) ble i kontrolleren; `ServerSelectSeaPoint` og `ServerTravelVote` ligger til steg 8 i `.Fleet.cpp`. `.Career.cpp` er borte, og 6 include-unntak ble overflødige (412) | `ServerCareerHello`, `ClientCareerSnapshot`, `ServerCareerAck`, `ServerBuyCareer`, `ServerChooseBoat` | Middels: penger og lagring |
| 6 ✅ | `UFiskFleetSync` (`Boats/Controller/FiskFleetSync.*`): valg, henting og forlating av båt. Valgt båt (`SelectedFleetBoat`) er fortsatt replikert fra kontrolleren; flåtepanelet (Tab) ble i kontrolleren. Antall include-unntak er uendret (412): to nye i komponenten, to borte i `.Fleet.cpp` | `ServerFleetSelect`, `ServerFleetLeave` | Middels |
| 7 ✅ | `UFiskExpeditionCommands` (`Boats/Controller/FiskExpeditionCommands.*`): dagens ekspedisjon, med meldingssperren på 2 s. Sjekken av spillmodus bruker `AGameModeBase`, så komponenten ikke trenger Bootstrap-headeren. Antall include-unntak er uendret (412) | `ServerExpeditionChoose`, `ServerExpeditionReady`, `ServerExpeditionSay` | Lav |
| 8 | `UFiskBoatCommands`: reise, rute og kaptein (`.Voyage.cpp`, `.Return.cpp`) | `ServerStartBoatTrip`, `ServerReturnToMarina`, `ServerTravelVote`, `ServerSelectSeaPoint`, `ServerSelectGround`, `ClientGroundReply`, `ServerVoyageReplace`, `ServerVoyageNext`, `ServerTransferCaptain`, `ClientCaptainReply`, `ServerBoardStarterBoat` | Høy: kjernen i spillflyten |
| 9 | `UFiskRodCommands`: stenger, rigg og kamp (`.Angler.cpp` og stangdelen av `.cpp`) | `ServerSelectRod`, `ClientRodSelection`, `ServerCastSelectedRod`, `ServerActOnSelectedRod`, `ServerConfigureSelectedRod`, `ClientRigReply`, `ServerApplyRigBundle`, `ClientRigBundleReply` | Høy: fiskekamp og nappetider |

`ServerNextDay` og `ClientActionFeedback` blir værende i kontrolleren, fordi de hører til økten og den felles meldingslinjen.

Rekkefølgen går fra lav til høy risiko, så mønsteret er prøvd før kampen og reisen flyttes.

## Krav til hvert steg

1. Egen gren og egen PR som refererer #113.
2. Windows-bygg av Editor og Game.
3. `Tools/Build/Test-Automation.ps1` (fra #117) eller tilsvarende `Fiskelaget.*`-kjøring.
4. De berørte `Tools/Testing/Run-*.ps1` (velg dem med `select_tests.py` fra #117), og minst én LAN-økt med to klienter.
5. Når include-kontrollen fra #114 er flettet: kjør `check_includes.py --prune`, slik at unntakslisten krymper.
6. Ingen endring i spillregler eller lagringsformat. Protokollversjonen økes bare hvis et kall faktisk endrer signatur.

## Beslutninger som trengs

- Godkjenning av mønsteret og rekkefølgen over.
- Hvor komponentene skal ligge (forslag: `Boats/Controller/` først).
- Om stegene skal integreres hver for seg (anbefalt) eller samles i én utgivelse.
