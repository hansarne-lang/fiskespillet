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
