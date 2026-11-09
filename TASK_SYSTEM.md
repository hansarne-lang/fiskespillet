# FISKELAGET – Felles oppgavesystem

Status: vedtatt oppgavemodell; se GitHub Issues for faktisk fremdrift og eventuell Projects-blokkering. Dette dokumentet beskriver modellen; det er ikke en lokal TODO-liste med konkurrerende oppgavestatus.

## Én sannhetskilde

Hver utviklingsoppgave er én GitHub Issue. GitHub Projects viser de samme issue-postene og deres metadata. Ikke bruk lokale markdownavkryssinger, brodatabasen eller agentplaner som autoritativ oppgaveliste. En lokal arbeidsplan kan beskrive fremgangsmåten for den tilordnede issuen, men må lenke dit og ikke overstyre statusen.

TaskId er issue-URL-en; repository-ID, issue-nummer og GitHub node-ID lagres som tekniske referanser. Lokale midlertidige utkast får ikke TaskId og kan ikke startes før GitHub har bekreftet opprettelsen. Ikke opprett lokale oppgavenummer som senere konkurrerer med GitHub-numrene.

## Felter og eierskap

| Felt | Autoritativ plassering | Regel |
|---|---|---|
| ID og beskrivelse | Issue-URL og issue-body | Formål, avgrensning og akseptansekriterier er obligatoriske |
| Ansvarlig | GitHub assignee | Ansvarlig menneske eller godkjent botkonto; ingen oppdiktede agentkontoer |
| Utførende agent | Project-felt `Agent` | Claude-Integration, Claude-Fishing, Claude-BoatsWorld, Claude-EconomyCareer, Human eller Unassigned. Codex-verdiene er historiske (Codex er tatt ut 9. november 2026) |
| Prioritet | Project single-select `Priority` | P0 Kritisk, P1 Høy, P2 Normal, P3 Lav |
| Modul | Project single-select `Module` | Én av ni moduler, Platform eller CrossModule |
| Status | Project `Status` | De seks tilstandene nedenfor; issues er ellers open/closed |
| Avhengigheter | GitHub issue dependencies | «Blocked by»/«Blocking» peker til konkrete issues |
| Fileierskap | Issue-body, seksjon `AllowedFiles` | Repository-relative tillatte stier/glober; sammenholdes med AGENTS.md |
| Godkjenning | Project `Approval` og issue-label | Not Required, Awaiting Approval, Approved; label `approval:awaiting` når det ventes |
| Blokkering | Issue-body og Project `Next Action Owner` | Årsak, neste konkrete handling og ansvarlig |
| Endring og bevis | Issue/PR-lenker og sjekkresultater | Gren, commit-SHA, PR, workflow-run, testresultat og integrert commit |

Prioritet/modul har én redigerbar kilde i Project, ikke parallelle autoritative labels. Agentfeltet skiller AI-rollen fra GitHub-kontoen som utfører API-kallet. Godkjenning krever sporbar bekreftelse fra prosjekteierens konto; en agent kan ikke selv fjerne venteflagget som bevis på godkjenning.

GitHub støtter single-select-felter og issue-avhengigheter. Faktiske felt- og option-ID-er leses fra det opprettede Project og lagres i lokal integrasjonskonfigurasjon; de skal ikke hardkodes fra skjermnavn. [Project-felter](https://docs.github.com/en/issues/planning-and-tracking-with-projects/understanding-fields/about-single-select-fields), [issue-avhengigheter](https://docs.github.com/en/issues/tracking-your-work-with-issues/using-issues/creating-issue-dependencies)

## Statusoverganger

| Status | Vilkår for å være her | Videre overgang |
|---|---|---|
| Backlog | Beskrevet behov, ennå ikke startklar | Ready etter avklaring av felter, omfang og avhengigheter |
| Ready | Testkrav og filområde klare, avhengigheter Done, nødvendig godkjenning gitt | In Progress først etter eksklusiv tildeling og bekreftet GitHub-oppdatering |
| In Progress | Én utførende agent, gyldig reservasjon, gren og worktree registrert | In Review med PR/testbevis, eller Blocked |
| Blocked | Årsak og ansvarlig for neste handling oppgitt | Ready når hindringen er løst; krever ny reservasjon |
| In Review | PR klar; arbeid og testresultater tilgjengelige | Done etter godkjent og integrert endring med beståtte tester; ellers In Progress/Blocked |
| Done | Akseptansekriterier, relevante tester, godkjenning og integrasjon bekreftet | Reopen ved feil med begrunnelse |

`Awaiting Approval` er en separat godkjenningsmarkering, ikke en syvende arbeidsstatus. En slik oppgave får normalt Status=Blocked, Approval=Awaiting Approval, årsak og prosjekteieren som neste beslutningstaker. Den må aldri plukkes fra Ready før godkjenningen kan verifiseres.

Automatisk «issue closed → Done» slås ikke på ukritisk. PR-merge kan lukke en issue før alle bevis er kontrollert; integrasjonen må verifisere SHA, tester, review og merge først. Feilaktig lukking flagges og oppgaven gjenåpnes/returneres til korrekt status med forklaring. Hver utviklingsoppgave skal ha en integrert endring, også når endringen bare er dokumentasjon.

## Hente neste oppgave uten dobbeltstart

1. Koordinatoren leser oppdatert GitHub-liste med paginering. Kandidater må være Ready, ikke ventende på godkjenning, og ha alle avhengigheter ferdige. Manglende eller ulesbar informasjon betyr ikke startklar.
2. Sorter etter Priority, deretter tidspunkt for Ready og issue-nummer. Avgrens til agentens modul- og filrettigheter og ingen overlappende aktive filreservasjoner.
3. En enkelt koordinator serialiserer claim-operasjoner i lokal transaksjon. Den reserverer kandidatens issue-ID og normaliserte filområder med kort levetid. Reservasjonen er en kjørelease, ikke en alternativ oppgavestatus.
4. Les issue og Project på nytt. Ved endret assignee/status/godkjenning avbrytes claim. Oppdater autoritative GitHub-felter, registrer agent, worktree, gren og lease-ID i issue-kommentar, og les tilbake resultatet.
5. Først etter bekreftet GitHub-status In Progress får adapteren klarsignal. API-feil eller usikkert utfall gir ingen agentstart; finn igjen idempotensnøkkelen før en ny skriving.
6. Heartbeat foreslås hvert 30. sekund, lease 120 sekunder. Mistet lease eller GitHub-forbindelse stopper nye mutasjoner. Ny agent får ikke overta før gammel prosess er bekreftet stoppet og GitHub er avstemt. Tidsutløp alene beviser ikke at en agent har stanset.

GitHub-oppdateringer på tvers av Issue og Projects er ikke én atomisk transaksjon. Lokal serialisering og tilbakekontroll reduserer risikoen, men kan ikke låse en menneskelig GitHub-redigering. Alle AI-starter og menneskers oppgavestart skal derfor bruke samme claim-prosess. En oppdaget ekstern konflikt stopper arbeidet og varsles. Flere uavhengige koordinatorer skal ikke kjøre samtidig; senere fler-PC-støtte krever én delt claim-tjeneste.

Samarbeid på samme issue krever eksplisitt registrert koordinering med primæreier, deloppgaver og adskilte filer. Standard er én issue per agent.

## Dashboard for prosjekteieren

Ett Project med disse lagrede visningene: oversikt som board etter Status; «Claude» filtrert på agentfeltet; «Blokkert» med årsak og Next Action Owner; «Venter på godkjenning»; «Til gjennomgang» med PR/testlenker; og «Ferdig» med integrert commit. Alle bygger på samme GitHub-data. Ingen egen lokal statusfarge skal overstyre Project; en lokal visning viser sist synkronisert tidspunkt og tydelig frakobling.

## GitHub-integrasjon

Prosjekteieren har godkjent Claude Code som reviewer med et [SHA-bundet Broen-handshake](Docs/Development/BROEN_REVIEW.md). Når den testede porten er aktivert, er påkrevd `claude-review` sammen med `repo-contracts` godkjenningsgrunnlaget, uten behov for en annen GitHub-konto. Claude gjennomgår faktisk endring i en separat lesende prosess; koordinatoren kontrollerer avsender, nonce, head/base-SHA, resultat, ACK og testbevis før status publiseres. Dette er ingen egen formell GitHub-Approve fra PR-forfatteren. Ny push krever ny kontroll. Done krever fortsatt faktisk integrasjon.

Issues/PR-er administreres gjennom den godkjente GitHub-koblingen. Projects krever en autentisert API-kobling med Projects-rettighet. Aktive Projects-ID-er og felter leses fra GitHub, ikke fra en separat lokal TODO-liste.

I den første bootstrapfasen, før Project er tilgjengelig, registreres Status, Priority, Module og Agent i den tilhørende GitHub-issuen. Når Project opprettes, flyttes den redigerbare kilden til Project-feltene og gamle bootstrapfelt merkes som historikk. Det skal aldri finnes to aktive statuskilder. Oppgavestart er manuell og koordinert til claim-tjenesten er implementert; automatisk henting forblir deaktivert.

Lokal outbox kan kun inneholde leveringsstatus og idempotente forespørsler. GitHub må bekrefte oppretting/tildeling før en agent starter. Ved nettverksbrudd starter ingen nye oppgaver.

## Akseptanseprøve

Opprett etter godkjenning to separate GitHub-issues for dokumentendringer: én Claude Code-sesjon eier en testprosedyre, en annen eier en onboardingtekst. Begge skal lese de samme to issue-ID-ene via godkjent integrasjon, claim hver sin og se motpartens In Progress-status etter GitHub-synkronisering uten manuell meldingskopiering.

Hver leverer egen gren, worktree, commit og PR. Dokumentkontroller skal kjøres og review gis av annen autorisert person før integrasjon. Done skal først registreres etter beståtte sjekker og merge. Demonstrer også avvist dobbelt-claim, Awaiting Approval-blokkering, uløst avhengighet, filkonflikt og gjenoppretting etter GitHub-brudd. Bevisloggen skal inneholde issue-/Project-/PR-lenker, commit-SHA-er og faktisk teststatus. Denne prøven er planlagt og ikke gjennomført.
