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

## 2026-11-08 — Karriereprofiler stoles på mellom LAN-verter

Prosjekteieren valgte 8. november 2026 alternativ 1 i [#112](https://github.com/hansarne-lang/fiskespillet/issues/112), bekreftet fra prosjekteierens konto i [PR #120](https://github.com/hansarne-lang/fiskespillet/pull/120#issuecomment-6719892215) for revisjon ebe6910: FISKELAGET er et LAN-spill mellom venner, og verten stoler på karrieren klienten sender inn, så lenge kvitteringskjeden er gyldig og ikke er i konflikt med vertens egen kopi.

Bakgrunn: Karrieren skal følge spilleren mellom LAN-verter (IDÉ-004, RULE09), og v0.7-godkjenningen slår fast at kopiering av profiler utenfor spillet ikke kan hindres mellom uavhengige verter. En ny vert kan ikke skille ekte utbetalinger fra andre verter fra oppdiktede, og siden enhver spiller kan være vert, gir heller ikke signering et reelt vern. En endret klient kan derfor ta med seg oppdiktet saldo. Dette godtas.

Mens spilleren er tilkoblet, er serverautoriteten uendret: bare vertens oppgjør lager nye utbetalinger, og kjøp kontrolleres mot godtatt saldo. Tilliten gjelder ved hver ny tilkobling, også til samme vert. Konkurranser eller rangeringer der saldo fra andre verter avgjør utfallet, krever en ny beslutning. Se ARCHITECTURE.md, «Tillit til karriereprofiler på LAN».

## 2026-11-08 — Testklokke og raskere testkjøring

Prosjekteieren ba i Claude Code-sesjonen 8. november 2026 om å gjennomføre forslag 1–6 for kortere tid fra bygg til ferdig test. Det inkluderer en testklokke som går raskere. Godkjenningen er bekreftet fra prosjekteierens konto i [PR #117](https://github.com/hansarne-lang/fiskespillet/pull/117#issuecomment-6719632634) for revisjon 360f9ed.

Regelen «ingen klokkeakselerasjon i tester» erstattes for raske kontroller. `-FiskClockScale=N` (1–20) skalerer serverens dagsklokke og dermed alt som bruker den. Normalverdien er 1. Shipping-bygg ignorerer flagget. Spillregler, tider og priser er uendret. Utgivelsesbevis for dagslengde, 17:00-frist, reisetid og napp krever fortsatt normal klokke (`Run-All.ps1 -Release`).

## 2026-11-08 — Lisens: alle rettigheter forbeholdt

Prosjekteieren valgte alternativ A i [#10](https://github.com/hansarne-lang/fiskespillet/issues/10#issuecomment-6726548569) (godkjent [her](https://github.com/hansarne-lang/fiskespillet/issues/10#issuecomment-6726563802)): Repositoryet forblir offentlig for innsyn, men alle rettigheter er forbeholdt. `LICENSE` gir ingen lisens til bruk, endring eller distribusjon utover GitHubs vilkår. Unreal Engine er underlagt Epics egne vilkår. Valget kan senere åpnes til en ikke-kommersiell eller åpen lisens, mens en gitt lisens ikke kan trekkes tilbake. GPL og andre sterke copyleft-lisenser er frarådet sammen med Unreal Engine.

## 2026-11-08 — Særregler for strandbarens veddemål

Prosjekteieren godkjente forslaget i [#15](https://github.com/hansarne-lang/fiskespillet/issues/15#issuecomment-6726551924) i sin helhet ([godkjenning](https://github.com/hansarne-lang/fiskespillet/issues/15#issuecomment-6726564377)). Hovedregelen fra IDÉ-005 ligger fast. «Fiskeren» er stangeieren, også på delt båt og ved hjelp til landing.

1. **Innsats:** fast valg mellom $50, $100 og $250, trukket én gang ved registrering.
2. **Antall:** ett veddemål per spiller per dag.
3. **Ulike innsatser:** Vinnerne deler hele potten i forhold til innsatsen.
4. **Lik størstevekt:** Alle som har satset på en av fiskerne med lik største vekt (samme gram), er vinnere og deler etter punkt 3.
5. **Ingen riktige tips:** Alle får hele innsatsen tilbake.
6. **Ingen tellende fangst** (ingen av fiskerne det er satset på, leverte innen fristen): Alle får innsatsen tilbake.
7. **Avrunding:** Beløp regnes i øre. Hver andel rundes ned, og restøren går til vinneren med størst innsats, ved likhet til den som registrerte først. Summen av utbetalingene er alltid lik potten.
8. **Frakobling:** Veddemålet står. Utbetalingen er en egen oppgjørskvittering med unik ID per dag og veddemål, og godskrives ved neste tilkobling. Samme kvittering gir aldri ny utbetaling.

Innsatsgrensen begrenser også virkningen av saldo fra andre verter, som ikke kan kontrolleres ([#112](https://github.com/hansarne-lang/fiskespillet/issues/112)). Regneeksemplene står i kravspesifikasjonen. Gjennomføringen gjøres i [#22](https://github.com/hansarne-lang/fiskespillet/issues/22).

## 2026-11-09 — Codex tas ut av utviklingen, og testing får budsjett

- Prosjekteieren har bestemt at Codex ikke brukes i utviklingen fremover. Alle agentroller i [AGENTS.md](AGENTS.md) er Claude Code-sesjoner.
- Windows-bygg og -tester kjøres av Claude Code i en økt på prosjekteierens PC (lokal terminal eller Remote Control), eller av prosjekteieren med `Tools/Testing/Test-PR.ps1`.
- Broen-reviewen med Codex som koordinator ([BROEN_REVIEW.md](Docs/Development/BROEN_REVIEW.md)) er satt på pause. Statusen `claude-review` kreves ikke. Kode og tester beholdes som historikk.
- Testing følger [testbudsjettet](TESTING.md#testbudsjett): faste nivåer, én kort rapport, kjente feil merket med issue og ingen ny kjøring av samme bygg. Issue #137.

## 2026-11-09 — Windows-bygg på prosjekteierens PC

- Prosjekteieren valgte å la sin egen PC være byggemaskin for GitHub Actions (self-hosted runner) i stedet for at bygg og tester startes for hånd. Issue #5.
- Fordi repoet er offentlig, kjører jobben aldri kode fra forks: bare PR-er fra grener i repoet, push og manuell kjøring. Eksterne bidragsytere må godkjennes i GitHub. Jobben har bare lesetilgang.
- Runneren kjører i prosjekteierens innloggede skrivebordsøkt (ikke som tjeneste), fordi røyktestene trenger skjerm og SpilLan-publiseringen må kjøre som brukeren.
- `windows-build` er ikke påkrevd for fletting, så main stoppes ikke når PC-en er av. Oppsett: [WINDOWS_CI.md](Docs/Development/WINDOWS_CI.md).

## 2026-11-10 — Veddemål: hvilke fiskere som sammenlignes

Prosjekteieren godkjente forslaget fra #143: Ved kåring av største fisk sammenlignes bare tellende fangster (levert innen fristen, fisken tilhører stangeieren) fra fiskere som var ute på sjøen da veddemålene kunne registreres. Spillere som allerede var i land, er ikke med i sammenligningen, selv om de har levert fisk. Gjelder #22 steg 2.

## 2026-11-11 — Strandbaren: strandpromenade og drinker

Prosjekteieren sa ja til begge punktene i B i [forslaget i #18](https://github.com/hansarne-lang/fiskespillet/issues/18#issuecomment-6803816061):

- **Gange utenfor marinaen:** en strandpromenade fra baren vestover og en platting ut over sanden. Gangplanet holdes i bryggehøyde, så promenaden er en brygge over stranda; solsenger står på sanden ved siden av. Ingen gange på selve sanden eller utenfor marinaområdet ellers.
- **Drinker i baren:** gratis og kun til pynt. Spilleren bestiller med 8, 9 eller 0, og bestillingen vises som en melding i baren. Ingen pris, saldo, effekt eller lagring.

## 2026-11-11 — Flytte båten til nye fiskesteder underveis

Prosjekteieren valgte begge endringene:

1. **Nytt mål på vei ut:** kapteinen kan velge et nytt punkt på sjøkartet (eller neste stopp i turplanen) mens båten er på vei ut, ikke bare når den ligger på fiskeplassen. Båten svinger fra der den er. På vei hjem og ved brygga gjelder de gamle reglene.
2. **Ledige stenger trekkes inn:** stenger som bare står ute og venter på napp, trekkes inn for alle om bord når kapteinen flytter. En stang med fisk på (napp, kamp eller klar til landing) stopper fortsatt flyttingen, og det gjør også en tvungen hjemtur fra lasten.

Tidsrommet 02:00–17:00 og kravet om oppdatert båtstatus er uendret.

## 2026-11-14 — Felles veiing når alle er i land

Bestilt av prosjekteieren 14. november 2026, [oppgave 183](https://github.com/hansarne-lang/fiskespillet/issues/183): når alle er i land, veies all fisk, minste først og største til slutt. Da får man også vite prisen. Alle deltakere ser hvem som har fisket hvor mange, og hvor mange som er veid.

- **Når:** veiingen starter så snart alle båter som dro ut er tilbake i marinaen, også før 17:00. Er noen fortsatt ute kl. 17:00, starter den da, og fangsten deres går tapt som før.
- **Tempo:** av seg selv. Tre sekunder per fisk, fem sekunder for nummer to til fire fra toppen og åtte sekunder for dagens største. En lang dag veies raskere, slik at veiingen tar høyst omtrent to minutter.
- **Per fisk:** alle ser fisker, art, vekt og pris. En tavle viser antall fisket og antall veid per deltaker.

Dette erstatter regelen om at fangst og penger holdes skjult til felles avsløring ved minutt 17. Fangsten er fortsatt skjult til den er på vekta, og oppgjør, utbetaling, journal og veddemål kommer når siste fisk er veid. Båter som ikke har dratt ut, teller ikke som ute. Starter veiingen før 17:00, kan ingen båt dra ut igjen den dagen.
