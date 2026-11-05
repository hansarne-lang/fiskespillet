# v0.6 – Flere stenger, artsprofiler og kameraer

[Oppgave #70](https://github.com/hansarne-lang/fiskespillet/issues/70) bygger på v0.5 / PR #69. Prosjekteieren godkjente 5. november 2026 det konkrete forslaget med svaret «Ja – bygg denne pakken». Stangplassfordelingen og de foreløpige artsverdiene nedenfor er dermed godkjent for denne testpakken. Eksisterende 20-minuttersdag, returfrist, artspriser og hemmelighold beholdes.

## To stenger på startbåten

Startbåten har fortsatt to samlede stangplasser. Én spiller om bord kan bruke begge; med to spillere reserveres én plass til hver. Denne båtens kapasitet begrenser prototypen til to stenger alene, selv om fullspillets personlige grense er tre. Ingen kan overta andres stang. Frakobling frigjør plass; ny ombordstigning er som før bare mulig ved brygga. Hvis en ekstra plass blir utilgjengelig, avbrytes eventuell ekstra stang og valget går tilbake til stang 1.

Uten åpne menyer velger 1/2 egen stang. F setter ut, G åpner oppsettet, X trekker inn/avbryter, og mus/brems/mellomrom styrer valgt stang. Begge kan fiske passivt, men spilleren sender kampinput til bare én om gangen. Bytte slipper løft og sveiving på begge; en ubetjent kamp fortsetter og kan tapes ved slakk. Første utras på den andre stanga er også hørbart. Stangkort viser begge statusene og markerer valget, uten dagens art, eksakte mål eller fangsttall.

Hver stang har egen rigg, utsettings-ID, tilfeldig nappsekvens, private fangstdata og kampmodell. De tre lagrede oppsettene er en felles oppsettsbank som kan brukes på valgt stang; de er ikke tre ekstra stenger. G-panelets 1/2/3 velger fortsatt lagringsplass. Valg, innhenting, avbrudd og ny utsetting på én stang endrer ikke den andre. Serveren kontrollerer ombordstigning, båtkapasitet, stangvalg/revisjon/dag, utsettings-ID, oppsettsrevisjon og fase. Forsinkede kommandoer fra tidligere valg/dag avvises. Ny dag nullstiller begge fiskeforløp, beholder begge rigger og velger stang 1. Retur stopper begge stengers aktive fiske.

Nettverket sender eierens valg gjennom ordnede, pålitelige svar og eierbegrensede stangvisninger. Kamp- og riggforespørsler inneholder valg/revisjon/dag i tillegg til tidligere utsettings-/riggrevisjon. Kompatibilitetshjelpere brukes av eksisterende interne tester og sender fortsatt den fullstendige validerte kommandoen. Det er ingen støtte for blandede spillversjoner på samme LAN.

## Artsbestemt kamp

| Art | Utrasfart | Utrasvarighet | Skjult utholdenhetstapping | Pumping |
|---|---|---|---|---|
| Snapper | 0.80 | 0.85 | 1.25 | 1.20 |
| Amberjack | 1.00 | 1.00 | 1.00 | 1.00 |
| Gulfinnet tunfisk | 1.30 | 1.35 | 0.75 | 0.85 |

Verdiene skalerer den eksisterende generiske modellen og er foreløpig spillbalanse, ikke biologiske data. Snapper er kortere/enklere; amberjack beholder grunnprofilen; tunfisk får lengre utras og tregere innsveiving. Bremsens trinn og slakk-/bruddgrenser beholdes. Første utras bruker samme artsprofil. Modellen bruker fortsatt faste steg på 50 ms. Profilen kommer bare fra serverens private art; ingen art, eksakt vekt, profilparametre eller utholdenhet legges til stangas nettverksvisning. Fysisk atferd kan antyde fisk slik kravene tillater, men bekreftelsen kommer samlet ved 17:00. Det generiske fiskeutseendet beholdes.

## Kamera og øvrige menyer

C veksler lokalt mellom babord, styrbord og oversikt. Dette er faste vinkler som følger båten. Ingen fri bevegelse, stemmegivning, reisehopp, endret klokke eller påvirkning av en annen spillers kamera inngår. Valg av stang og kamera er uavhengige. F7 viser agndybden til valgt stang. Menyer pauser verken tid eller kamp og bruker fortsatt egne taster; lukk dem før stang-/kameravalg.

## Kapteinstestens synkronisering

Den gamle prøven kunne overføre kapteinsrollen før gjestens kontroll av passasjermenyen var ferdig. Prøven venter nå på en eksplisitt serverobservert avvisning fra gjestens kontroll før verten overfører rollen. Den ekstra negative forespørselen telles i skriptets forventninger. Dette endrer testens koordinering, ikke kapteinsmyndighet eller reisetid. Faktisk resultat registreres i issue/PR; en rettet test er ikke automatisk en bestått test.

## Verifisering og leveranse

`Fiskelaget.MultiRod.*` kontrollerer kapasitet/valg, gamle revisjoner/dager, separate utsettings-ID-er og oppsett. `Fiskelaget.Fight.SpeciesProfilesAndLanding` prøver 100 frø per art og faste tidssteg, forventet rekkefølge i gjennomsnittlig kampvarighet og avlastet input. Eksisterende modelltester skal fortsatt bestå.

`Tools/Testing/Run-FishingRelease.ps1 -Phase Solo` kjører to passive solostenger med ulike oppsett, innhenting/ny utsetting av stang 2, gamle valgkommandoer, bytte under faktisk kamp, sluppet input, kameraer, landing og retur. `-Phase Lan` kjører v0.5s fulle normale dag-/journal-/favoritt-/backup-/dag-2-prøve med ekstra kontroll av delt stangkapasitet, avviste kommandoer, lokale kameraer og nullstilling av ekstra stang. Standard All kjører begge sekvensielt. Kapteinsbytte og vanlig enspillermeny prøves med egne eksisterende skript.

Pakken skal inneholde Windows-spill, norsk guide, BAT, kilde-SHA, faktiske testresultater og sjekksummer uten testprofiler. Ingen tester påstås bestått før faktisk kjøring på oppgitt kilde. To fysiske PC-er oppgis ikke testet hvis bare localhost brukes. Gjennomgang og godkjenning før integrasjon beholdes; ingen merge/Done eller offentlig binærrelease fra denne oppgaven.

Butikk, disponibel karrieresaldo, nye båter, fangstlager/overlast/sleping, nye arter eller områder og ventende IDÉ-oppgaver inngår ikke. Journalens og favorittfilenes eksisterende lagringsgrenser, frakoblingsbegrensning og manuelle overføring mellom PC-er beholdes.
