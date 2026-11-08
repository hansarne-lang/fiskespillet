# FISKELAGET – Beslutninger

## Godkjent 3. november 2026

- Behold eksisterende `hansarne-lang/fiskespillet` offentlig. Ikke opprett et nytt repository eller endre til privat.
- Bruk den godkjente kravspesifikasjonen gjennom v4.1, Unreal Engine 5 og modulær monolitt.
- Start med FiskCore/FiskGame som fysisk pakking av ni logiske moduler.
- Etabler GitHub-oppsett og felles oppgavesystem. GitHub Issues er sannhetskilden, og Projects gir felles oversikt.
- Bruk separate oppgavegrener/worktrees, tester og review. Den første rot-commiten er bootstrapunntaket.
- Ingen implementasjon av fiskespillets mekanikker inngår i etableringen.

## Godkjent spilldesign i kravspesifikasjon v1.1

- IDÉ-001: tropisk strandbar som sosialt samlingspunkt ved tidlig retur. Sanntidsvisning av innkommende båter/slepefisk og resultattavle er tilleggsidéer innenfor senere valgt omfang.
- IDÉ-002: sammenhengende levende 3D-båtreise, faste kameravinkler og ingen fri bevegelse om bord. Enstemmig reisehopp gir ingen ekstra fisketid og endrer ikke serverklokke, båtferd eller ankomst. Kapteinen kan fortsatt starte retur.
- IDÉ-003: større mulighet for enorme og verdifulle fisker langt offshore, med mer reise-, kamp- og sleperisiko. Små båter kan fortsatt prøve seg; konkrete sannsynligheter/avstander balanseres.
- IDÉ-004: eget enkelt, navngitt naust med utstyr, trofévegg, veranda, besøk og kosmetikk. Progresjonen følger karrieren mellom LAN-servere.
- IDÉ-005: veddemål i strandbaren etter egen ankomst på fiskere som fortsatt er ute. Bare enkeltfisk levert innen fristen teller; vinnerne deler hele potten.
- Dagens bekreftede fangstresultater skjules for alle til felles avsløring ved minutt 17, også for fiskeren og mannskapet. Journal, statistikk, trofévegg og rekordvarsler følger samme regel. Synlige båter, fisker og sleping kan fortsatt observeres.
- Levering registreres ved ankomst, men salg, fangstpremier og veddemålsgevinster venter til minutt 17, slik at saldo og kjøpekraft ikke røper fangsten. Tidligere disponible penger kan brukes før oppgjøret; kjøp og innsatser belastes når de gjennomføres.

Disse valgene er uttrykkelig gitt av prosjekteieren 3. november 2026. Detaljene står i [kravspesifikasjon v1.1](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md). [Prototypeomfang (#14)](https://github.com/hansarne-lang/fiskespillet/issues/14), [veddemålsregler og fangsteierskap (#15)](https://github.com/hansarne-lang/fiskespillet/issues/15) og [ankomstdefinisjon (#16)](https://github.com/hansarne-lang/fiskespillet/issues/16) er egne eierbeslutninger. Oppgavestatus følger GitHub, ikke dette dokumentet.

## Beslutningsflyt

Spilldesign, større arkitektur, nye tilganger, betaling, destruktive handlinger og endringer i publiseringsomfang går til prosjekteieren. Awaiting Approval i GitHub blokkerer utførelsen. Godkjenning skal gjelde konkret issue, scope og revisjon.

Uavklarte forhold og gjennomføringsstatus føres i GitHub Issues, ikke i en lokal konkurrerende oppgaveliste. Åpne spilldesignavklaringer står i kravspesifikasjonen.


## 2026-11-04 — Marinasone og returfrist

Prosjekteieren godkjente at serveren registrerer ankomst når båtens midtpunkt krysser en synlig marinasone. Ankomst senest ved 1 020 000 ms teller som i tide. Serveren behandler autoritativ kryssing før oppgjøret, også når begge skjer i samme ramme. En forsinket klientmelding flytter ingen ankomsttid. Testprototypen bruker en sirkel med radius fem meter rundt bryggepunktet. Størrelsen er en foreløpig testverdi.

## 2026-11-05 — Eiergodkjent v0.7

Prosjekteieren svarte «Ja – bygg hele v0.7-pakken» på det konkrete forslaget i [#72](https://github.com/hansarne-lang/fiskespillet/issues/72). Godkjenningen omfatter varig karriere med $30 000 engangs teststartsaldo, egne fangstinntekter først ved17:00, butikkprisene $250/$1 000/$1 500/$25 000 og foreløpige utstyrsbonuser15% bruddgrense/15% innsveiving/10% pumping.

Én aktiv felles båt velges av verten/eieren ved brygga. Offshorebåten har4plasser,6holdere og25% høyere fart. Fordelingen for1/2/3/4om bord er3 /3+3 /2+2+2 /2+2+1+1, med maksimalt3stenger per spiller. Startbåtens godkjente to-alene/én-hver-fordeling beholdes. Fire LAN-spillere, fritt valg av gyldige sjøpunkter, migrerte koordinatfavoritter og rikere kildebygd miljø/lyd inngår.

Den begrensede økonomi-/profilarkitekturen i v0.7-kontrakten er godkjent innen de eksisterende to UE-modulene. Historisk journal etterbetales ikke. Offline-profilkopiering kan ikke forhindres mellom uavhengige servere. Flere samtidige båter, lager/overlast/sleping, offshore-sannsynlighet, reisehopp, strandbar, naust og veddemål inngår ikke; deres øvrige åpne eierbeslutninger beholdes. Tallene er testbalanse, ikke endelig økonomi.

## 2026-11-08 — Testklokke og raskere testkjøring

Prosjekteieren ba i Claude Code-sesjonen 8. november 2026 om å gjennomføre forslag 1–6 for kortere tid fra bygg til ferdig test. Det inkluderer en testklokke som går raskere. Godkjenningen er bekreftet fra prosjekteierens konto i [PR #117](https://github.com/hansarne-lang/fiskespillet/pull/117#issuecomment-6719632634) for revisjon 360f9ed.

Regelen «ingen klokkeakselerasjon i tester» erstattes for raske kontroller. `-FiskClockScale=N` (1–20) skalerer serverens dagsklokke og dermed alt som bruker den. Normalverdien er 1. Shipping-bygg ignorerer flagget. Spillregler, tider og priser er uendret. Utgivelsesbevis for dagslengde, 17:00-frist, reisetid og napp krever fortsatt normal klokke (`Run-All.ps1 -Release`).
