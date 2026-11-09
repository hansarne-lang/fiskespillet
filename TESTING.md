# FISKELAGET – Tester og akseptanse

Status: testplan. Ingen UE-bygg, modell-samarbeidsprøve eller spillregeltest er kjørt i denne leveransen. Dokumentpakken kontrolleres separat for filfullstendighet, kontraktdekning og lokale lenker.

## CI-trinn

| Kontroll | Når | Akseptanse |
|---|---|---|
| repo-contracts | Alle PR-er | Dokumentlenker, kontraktformat, enum-/versjonsregler, fileierskap og ulovlige includes kontrolleres |
| unreal-build | Kode-/Config-/buildendringer | Editor/Win64 bygger med registrert verktøykjede. Kjøres som `windows-build` på prosjekteierens PC (self-hosted runner), se [WINDOWS_CI.md](Docs/Development/WINDOWS_CI.md) |
| unreal-tests | Relevante endringer | UE Automation gir eksplisitte resultater med commit og motorversjon |
| package-smoke | Integrasjons-/releasekandidat | Windows-build produseres, starter på test-PC og har nødvendig runtime |
| lan-smoke | Nettverk/protokoll/release | To Windows-klienter finner/tilkobler LAN, matcher protokoll og avviser feil versjon |

Definer endringsfiltre og «ikke relevant» eksplisitt. En required kontroll skal avslutte med et tydelig kontrollert utfall, ikke bare mangle. Infrastrukturtester kan innføres nå; tester for fiskereglene aktiveres når funksjonen faktisk implementeres.

## Raskere bygg og testkjøring

Verktøyene er lagt til 8. november 2026. Ingen av dem er kjørt på Windows ennå. Første kjøring skal registrere faktiske tider før og etter.

| Verktøy | Formål |
|---|---|
| `Tools/Build/Build-Fisk.ps1` | Inkrementelt bygg og pakking. `-Mode Code` gjenbruker forrige cook ved rene C++-endringer. Tiden for hvert steg legges i `build-times.csv` i arkivmappen. `-SpilLanExport` publiserer til SpilLan til slutt. |
| `Tools/Build/Publish-SpilLan.ps1` | Kopierer pakken til `v<versjon>` med kortbilde og kjører `SpilLan.exe publiser`. Se [SPILLAN.md](Docs/Development/SPILLAN.md). |
| `Tools/Build/Test-Automation.ps1` | Kjører `Fiskelaget.*`-automasjonstestene uten grafikk (`-nullrhi`). Krever bygget Editor. |
| `Tools/Testing/Run-All.ps1` | Kjører grafiske `Run-*.ps1` i parallell med egne porter (`7777 + 100 × plass`), og lager `summary.csv` med resultat og varighet. |
| `Tools/Testing/Test-RunAllLib.ps1` | Regresjonsprøver for hvordan `Run-All.ps1` klassifiserer `err.txt` (bare PowerShell-fremdrift ignoreres). |
| `Tools/Testing/select_tests.py` | Velger skript ut fra endrede filer. Ukjente stier gir alle skript. Dokumentasjon gir ingen. |
| `Tools/Testing/Test-PR.ps1` | Ett fast løp per nivå (`-Level Build`, `PR` eller `Release`) som ender i én kort `report-<tid>.md`. Exit 0 betyr ingen nye feil. |
| `Tools/Testing/report.py` | Lager rapporten fra `summary.csv`, `index.json` og spillbinærens SHA256. Viser bare første `FISKELAGET_*_FAILED`-linjer (høyst tre per skript) med fil og linje, og skjuler personlige stier. |
| `Tools/Testing/known_failures.json` | Kjente feil med åpen issue. Rapporten merker dem «kjent (#nr)» og resten «NY». Fjern linjen når issuen er løst. |
| `Tools/Testing/run_cache.py` | Husker skript som har bestått med samme spillbinær, testskript, testoppsett og klokkefaktor. `Run-All.ps1` hopper over dem (vises som `Cached`). `-Force` kjører alt, og `-Release` hopper aldri over noe. |
| `-FiskClockScale=N` / `FISK_CLOCK_SCALE` | Testklokke 1–20 ganger raskere. Dag, avreise, reisetid, napp og kamp følger den samme klokken. Kompileres bort i Shipping. |

Lagdelt kjøring:

1. **Hver endring:** `check_repository.py`, `check_bootstrap.py`, Python-testene og `Test-Automation.ps1`.
2. **PR med kode:** `Run-All.ps1 -ChangedSince origin/main` (klokkefaktor 4 som standard).
3. **Utgivelse:** `Run-All.ps1 -Release`. Normal 20-minuttersdag (faktor 1) og alle skript. Bare dette gjelder som utgivelsesbevis for dagslengde, 17:00-frist og reisetid.

## Testbudsjett

Testing skal gi svar med færrest mulig kjøringer og minst mulig tekst. Det gjelder både mennesker og AI-agenter:

1. **Velg nivå etter oppgaven, og kjør bare det:**
   - `Build`: spillbar versjon eller publisering. Ingen tester.
   - `PR`: vanlig kodeendring.
   - `Release`: før en versjon slippes, eller når prosjekteieren ber om det.
2. **Én kommando:** `Tools/Testing/Test-PR.ps1 -Level <nivå> ...`. Ikke sett sammen egne løp av enkeltskript uten grunn.
3. **Post rapporten, ikke logger.** `report-<tid>.md` er hele PR-kommentaren. Logger åpnes bare for feil merket NY, og da bare filen og linjen rapporten viser.
4. **Kjente feil undersøkes ikke på nytt.** Feil merket «kjent (#nr)» hører til den issuen. En ny kjent feil legges i `known_failures.json` sammen med issuen sin.
5. **Kjør et skript på nytt høyst én gang** for å skille en ustabil test fra en ekte feil. Samme bygg testes ikke to ganger. Cachen hopper over det som allerede har bestått.
6. **Manuelle kontroller forsøkes ikke av AI.** Ekte mus, to fysiske PC-er og lytting føres i PR-en som «Venter på menneske».
7. **Rapporten skal være kort:** commit, SHA256, tabellen fra `report.py`, kjente avvik og hva som venter på menneske. Ingen avskrift av logger.

Skript som kjøres direkte, uten miljøvariabler, oppfører seg som før: port 7777 og faktor 1. `Run-MainMenu` og `Run-SoloPlay` kjøres alene etter de andre, fordi spillkoden selv bruker port 7777 og 17784. `Run-SeaChart` kjøres alltid med faktor 1, fordi røyktesten tar et skjermbilde i et vindu på ett spillsekund. Det samme gjelder `AnglerRelease`, `ExpeditionRelease`, `ExplorationRelease`, `FishingRelease`, `MangroveRelease`, `NavigationRelease` og `SkipperRelease`. De feilet med faktor 4 (panel, delt plan, dag 2 og skjermbilder), men besto med faktor 1 (#136).

Kjente begrensninger: klokkefaktoren endrer også kampens tempo, så automatisk kamphjelp og pilot må vise at de består ved faktoren som brukes. Røykprøvene for Cargo, Collection, Fleet, Harbour, Help og Home har ingen `Run-*.ps1` i repoet og dekkes ikke av `Run-All.ps1`.

## Modulkontrakter

Test nødvendige felter/enheter, ugyldige ID-er, ukjent majorversjon, ugyldig enum, avsender uten autoritet, dobbel levering, hendelser i feil rekkefølge og omstart mellom bokføring og kvittering. Landet, solgt og tapt skal ikke forveksles. Klientprojeksjoner skal ikke lekke dagens skjulte fangstresultater før minutt 17; dette gjelder også journal, statistikk, rekordvarsler, trofévegg, saldo og underliggende klientpayload. Utholdenhet vises aldri som helsebar.

## Planlagte spillregeltester

| ID | Regel | Viktigste tilfeller |
|---|---|---|
| RULE01 | Maks tre aktive stenger per spiller | 0–3 godtas; fjerde avvises; båtens felles kapasitet kan være strengere |
| RULE02 | Ingen nappreduksjon fra andre båter | Samme seed/oppsett under identiske forhold har samme beregnede nappgrunnlag; andre båter endrer ikke parametrene |
| RULE03 | Fullt lager gir tvungen retur | Rett under, lik og over kapasitet; kamper avsluttes og stenger trekkes inn |
| RULE04 | Tillatt sleping gir redusert fart | Under, lik og over slepegrensen; retur starter umiddelbart; ingen videre fisking |
| RULE05 | Absolutt minutt 17 | Før, ved og etter 1 020 000 ms med eiergodkjent definisjon av ankomst; ingen unntak |
| RULE06 | For sent gir tap av hele dagens fangst | Alle dagens usolgte poster tapes; tidligere penger/utstyr og gyldige rekorder beholdes |
| RULE07 | Bare kapteinen kan starte frivillig retur | Passasjer avvises; gyldig kaptein godtas; overført kapteinsrolle kontrolleres ved kommando |
| RULE08 | Ingen tidlig utbetaling eller dobbeltsalg | Ingen fangstsalg, fangstpremie eller veddemålsgevinst før minutt 17; retry, frakobling, restart og dobbelt hendelseslevering gir én utbetaling |
| RULE09 | Karriere mellom LAN-servere | Bekreftet profil, journal, utstyr og rekorder overlever bytte; ugyldig eller eldre profil håndteres kontrollert |

## Nye planlagte akseptanseprøver fra krav v1.1

Disse prøvene er ikke kjørt. Prototypeomfanget avgjøres i #14; særregler for veddemål i #15 og ankomstregistrering i #16.

| ID | Regel | Viktigste tilfeller |
|---|---|---|
| RULE10 | Felles avsløring ved minutt 17 | Fisker, mannskap og andre spillere; tidlig landing/levering; HUD, journal, statistikk, trofévegg, varsel og nettverkspayload |
| RULE11 | Reisehopp gir ingen tidsfordel | Full enighet og én uenig; identisk serverstyrt ankomst; observatør ser fysisk reise; kaptein kan starte retur |
| RULE12 | Sosial strandbar etter tidlig retur | Én spiller i land og én fortsatt ute; resultattavle skjult til minutt 17 |
| RULE13 | Større offshore-muligheter med større risiko | Startbåt og bedre båt, valgt avstand/habitat, kamp, sleping, returtid og dokumenterte fordelinger |
| RULE14 | Personlige naust følger karrieren | Navn, besøk, kosmetiske kjøp, omstart, annen LAN-server og utsatte rekordoppdateringer |
| RULE15 | Serverstyrte veddemål og bevart pott | Bare gyldig deltaker/mål, saldo/innsats én gang, kun fangst levert i tide, godkjente særtilfeller og én utbetaling ved minutt 17 |

For napp måles også fordelinger i spilltest; en kort tilfeldig prøve kan ikke alene bevise lik nappfrekvens. Ingen funksjonsstubber som alltid returnerer true skal brukes som testbevis.

## Bro og GitHub

Test fire agentidentiteter, ruteisolasjon mellom prosjekter, idempotent send/retry, krasj før/etter ack, køgjenoppretting, filkonflikt på Windows-stier, utløpt lease, bekreftet stopp, grense for tid/forsøk/samtidighet og nektet handling uten godkjenning. API-feil, rate limit og delvis Project-/Issue-skriving skal ikke starte uavklart arbeid.

TODO-akseptansen i TASK_SYSTEM.md skal demonstreres med to ekte Claude Code-sesjoner på hver sin issue (Codex er tatt ut 9. november 2026). En simulert agenttest er nyttig før dette, men oppfyller ikke brukerens krav om faktisk samarbeid. Begge leser samme GitHub-liste og motpartens status, oppdaterer sin egen issue og leverer separate PR-er. Test at Done avvises før beståtte tester, review og merge.

## Beviskrav

Hvert resultat skal ha issue-ID, agentversjon, worktree, commit-SHA, testkommando/-jobb, start/slutt, utfall og lenke til logg/workflow-run. Skill Passed, Failed, Not Run og Not Applicable. Not Run er aldri Passed. Godkjenning må gjelde den endringen som faktisk er integrert.
