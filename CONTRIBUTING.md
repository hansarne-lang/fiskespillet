# FISKELAGET – Bidra og teste

Status: retningslinjer for det offentlige repositoryet. Nye invitasjoner og rettighetsendringer krever prosjekteieren.

## Utviklere

Prosjekteieren inviterer navngitte venner etter behov og med minste nødvendige rettighet. Repository-/Project-tilgang avklares separat. Hver utvikler bruker egen konto og leser kravspesifikasjon, ARCHITECTURE.md, MODULE_CONTRACTS.md og AGENTS.md.

Finn en GitHub Issue eller opprett et konkret forslag med beskrivelse, prioritet, modul, avhengigheter og akseptansekriterier. Ingen utviklingsoppgave starter uten autoritativ issue og bekreftet tildeling. Bruk egen oppgavegren/worktree. Binære ressurser reserveres før redigering; avtal endring av andres områder først.

PR skal lenke til issue, beskrive konkret før/etter, scope, testresultat og risiko. Oppgi Not Run når test ikke er kjørt. Ikke legg ved lokale nøkler, profiler eller private logger. Review utføres av en annen autorisert person; endringen integreres først etter påkrevde sjekker. Main pushes ikke direkte etter bootstrap.

## Feil og forbedringer

Feilrapport: build-ID/versjon, klient/server-versjon, forventet og faktisk resultat, gjentakelsestrinn, antall LAN-spillere og sanitert logg/skjermbilde. Oppgi hyppighet og om data gikk tapt. Ikke send passord, brukertokens eller personlige maskinlogger ukritisk.

Forbedringsforslag skal vise ønsket effekt og hvilke krav det berører. Spilldesign avgjøres av prosjekteieren. Merking Awaiting Approval betyr at utførelsen venter; den er ikke en tillatelse til å bygge løsningen på forhånd.

## Windows-testversjoner

Testere får ferdigpakket Windows-bygg med for eksempel `FISKELAGET Alpha 0.1.0`, build-ID, commit, protokollversjon, endringsnotat, kjente feil, installasjon/oppstart, LAN-instruksjon og feilrapportlenke. Unreal Editor skal ikke være nødvendig. Ta med nødvendige redistributables i godkjent pakking.

LAN-veiledningen skal angi host/join, nettverksforutsetninger, faktiske porter når de er valgt, brannmuravklaring og versjonskontroll. Ikke gjett porter i en brukerveiledning før de er konfigurert og testet. Tilkobling avviser inkompatibel protokoll med en forståelig melding.

Venner som bare tester skal ikke få kildekodetilgang. Bruk en separat privat kanal for byggeartefakter, valgt og godkjent av prosjekteieren. Release inneholder pakkehash, installasjonsnotat og kompatibilitet. GitHub Release er egnet for bidragsytere med repositorytilgang; det private kildekoderepositoryets release-side er ikke en egen testerrolle uten kildekodetilgang.

## PR-mal som skal etableres

- Issue og TaskId:
- Endring og formål:
- Tillatte filer og eventuelle låser:
- Krav-/kontraktpåvirkning:
- Kjørte tester og bevislenker:
- Tester som ikke er kjørt, med årsak:
- Godkjenning som kreves:
- Uavhengig reviewer:

## Issue-mal som skal etableres

- Problem og avgrensning:
- Ansvarlig og utførende agent:
- Priority og Module i Project:
- Blocked by-relasjoner:
- AllowedFiles:
- Akseptansekriterier og testkrav:
- Eventuell eierbeslutning og Approval:
- Blokkering, neste handling og ansvarlig:

Malene er innholdsforslag; GitHub Issue Forms/PR-mal og automatisk feltvalidering etableres i oppsettsfasen.
