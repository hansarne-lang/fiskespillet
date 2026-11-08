# FISKELAGET

FISKELAGET er et planlagt realistisk, sosialt fiskespill for Windows PC. Inntil 12 spillere skal kunne spille sammen på LAN i en fiktiv karibisk øygruppe, med egne eller delte båter og permanent karriere.

## Prosjektstatus

Spillbar UE5.8-utviklingsversjon med én spiller eller inntil tolv over LAN, egne eller delte båter,15 arter, fem habitater,20-minuttersdager og lagret karriere. Fysisk LAN og ytelse på flere PC-er må fortsatt prøves.

v0.22 legger til F10-havneguide til båt, bar og tolv naust, møbel- og kantklaring på bryggene, tre gåkameraer, hilsener mens du går og rikere terreng på de tre øyene. Se [spillguiden](Docs/Development/ISLAND_HARBOUR_V022.md). v0.21 gir et gåbart bryggenett med serverstyrt gange/løping, minikart, nærhetsvalg, gåanimasjoner og levende klesfarger. T åpner bryggeliv i land; E bruker naust, bar eller båt i nærheten. Se [spillguiden](Docs/Development/PIER_LIFE_V021.md). v0.20 gir en levende havn med synlige fiskere, vinkebevegelser, original havneatmosfære og bar-/havne-/følg-båt-kamera. Artsboken får et 3D-galleri med15egne artsmodeller. B/V viser havnen; I/V ved marinaen viser galleriet. Se [spillguiden](Docs/Development/LIVING_HARBOUR_V020.md). Flåten med12båter/spillere, naustbutikk, fiskeinnhold, turplan/foto og lagringer beholdes. Modeller og figurer er fortsatt stiliserte; ferdig realistisk grafikk påstås ikke.

Windows-pakken startes fra BAT uten Unreal Editor. Velg Spill, E for ombordstigning, R fra02:00 og F ved fiskeplassen. En tydelig nappelyd varsler egne stenger. Å etter napp eller AI-knappen lar testhjelpen forsøke én vanlig kamp og landing; Å/manuelle kontroller tar over igjen. Kapteinen bruker R eller knappen «Tilbake til marina» for normal hjemreise. Alle LAN-deltakere må bruke samme versjon. Kildeutvikling bruker Unreal5.8.4 og verktøyene i DEVELOPMENT.md. Pakken inneholder faktisk test-SHA, kontrollsummer, resultater og kjente begrensninger. Lagringer er lokale; GitHub synkroniserer ikke spillprofiler mellom PC-er.

GitHub Issues er oppgavelisten. Historiske åpne oppgaver avstemmes mot de integrerte leveransene i #100; nye restoppgaver skal beskrive det som faktisk gjenstår. Fysisk LAN mellom to PC-er må dokumenteres ved faktisk kjøring.

Repositoryet er offentlig etter prosjekteierens beslutning. Offentlig tilgjengelighet gir ikke automatisk en åpen kildekodelisens; lisens og tredjepartsrettigheter må avklares før gjenbruk eller videre distribusjon.

## Dokumentasjon

- [Gjeldende kravspesifikasjon v1.1](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md) og [Word-utgave](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.1.docx)
- [Samlet idébank](Docs/Requirements/FISKELAGET_Idebank_2026-11-03.md) og [tidligere krav v1.0](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.0.md)
- [Arkitektur](ARCHITECTURE.md) og [modulkontrakter](MODULE_CONTRACTS.md)
- [Utviklingsoppsett](DEVELOPMENT.md), [tester](TESTING.md) og [bidrag](CONTRIBUTING.md)
- [Felles oppgavesystem](TASK_SYSTEM.md) og [AI-samarbeid](AI_BRIDGE.md)
- [Agentregler](AGENTS.md), [tilgang](SECURITY.md), [beslutninger](DECISIONS.md) og [endringer](CHANGELOG.md)

## Oppgaver og kvalitet

[GitHub Issues](https://github.com/hansarne-lang/fiskespillet/issues) er den autoritative oppgavelisten. [Det private GitHub Project](https://github.com/users/hansarne-lang/projects/1) gir felles oversikt over de samme oppgavene. Statusene er Backlog, Ready, In Progress, Blocked, In Review og Done; Awaiting Approval er en separat godkjenningsmarkering.

Endringer går gjennom pull request og relevante kontroller. En oppgave er ikke Done før tester, godkjenning og integrasjon er bekreftet. CI i bootstrapfasen kontrollerer repositoryet; dette er ikke en Unreal-byggtest.

Kjør den lokale repositorykontrollen med `python Tools/Validation/check_repository.py` fra repositoryroten. Kontrollen krever bare Python 3.11 eller nyere og standardbiblioteket.


v0.18 Naustliv: H, deretter N, aapner naustbutikken med ni permanente lys-, brygge- og trofepakker. Gratis grunnvalg beholdes. Se [spillguiden](Docs/Development/HARBOUR_LIFE_V018.md).
