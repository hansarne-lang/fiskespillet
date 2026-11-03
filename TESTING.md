# FISKELAGET – Tester og akseptanse

Status: testplan. Ingen UE-bygg, modell-samarbeidsprøve eller spillregeltest er kjørt i denne leveransen. Dokumentpakken kontrolleres separat for filfullstendighet, kontraktdekning og lokale lenker.

## CI-trinn

| Kontroll | Når | Akseptanse |
|---|---|---|
| repo-contracts | Alle PR-er | Dokumentlenker, kontraktformat, enum-/versjonsregler, fileierskap og ulovlige includes kontrolleres |
| unreal-build | Kode-/Config-/buildendringer | Editor/Win64 bygger med registrert verktøykjede |
| unreal-tests | Relevante endringer | UE Automation gir eksplisitte resultater med commit og motorversjon |
| package-smoke | Integrasjons-/releasekandidat | Windows-build produseres, starter på test-PC og har nødvendig runtime |
| lan-smoke | Nettverk/protokoll/release | To Windows-klienter finner/tilkobler LAN, matcher protokoll og avviser feil versjon |

Definer endringsfiltre og «ikke relevant» eksplisitt. En required kontroll skal avslutte med et tydelig kontrollert utfall, ikke bare mangle. Infrastrukturtester kan innføres nå; tester for fiskereglene aktiveres når funksjonen faktisk implementeres.

## Modulkontrakter

Test nødvendige felter/enheter, ugyldige ID-er, ukjent majorversjon, ugyldig enum, avsender uten autoritet, dobbel levering, hendelser i feil rekkefølge og omstart mellom bokføring og kvittering. Landet, solgt og tapt skal ikke forveksles. Klientprojeksjoner skal ikke lekke skjult art/vekt/utholdenhet.

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
| RULE08 | Ingen dobbeltsalg | Retry, frakobling, restart og dobbelt hendelseslevering gir én utbetaling |
| RULE09 | Karriere mellom LAN-servere | Bekreftet profil, journal, utstyr og rekorder overlever bytte; ugyldig eller eldre profil håndteres kontrollert |

For napp måles også fordelinger i spilltest; en kort tilfeldig prøve kan ikke alene bevise lik nappfrekvens. Ingen funksjonsstubber som alltid returnerer true skal brukes som testbevis.

## Bro og GitHub

Test fire agentidentiteter, ruteisolasjon mellom prosjekter, idempotent send/retry, krasj før/etter ack, køgjenoppretting, filkonflikt på Windows-stier, utløpt lease, bekreftet stopp, grense for tid/forsøk/samtidighet og nektet handling uten godkjenning. API-feil, rate limit og delvis Project-/Issue-skriving skal ikke starte uavklart arbeid.

TODO-akseptansen i TASK_SYSTEM.md skal demonstreres med ekte Codex og Claude Code på hver sin issue. En simulert agenttest er nyttig før dette, men oppfyller ikke brukerens krav om faktisk samarbeid. Begge leser samme GitHub-liste og motpartens status, oppdaterer sin egen issue og leverer separate PR-er. Test at Done avvises før beståtte tester, review og merge.

## Beviskrav

Hvert resultat skal ha issue-ID, agentversjon, worktree, commit-SHA, testkommando/-jobb, start/slutt, utfall og lenke til logg/workflow-run. Skill Passed, Failed, Not Run og Not Applicable. Not Run er aldri Passed. Godkjenning må gjelde den endringen som faktisk er integrert.
