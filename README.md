# FISKELAGET

FISKELAGET er et planlagt realistisk, sosialt fiskespill for Windows PC. Inntil 12 spillere skal kunne spille sammen på LAN i en fiktiv karibisk øygruppe, med egne eller delte båter og permanent karriere.

## Prosjektstatus

Dette repositoryet etablerer utviklingsgrunnlaget. Spillmotoren er Unreal Engine 5, og arkitekturen bruker FiskCore og FiskGame med ni logiske funksjonsområder. Selve spillmekanikkene er ikke implementert.

Repositoryet er offentlig etter prosjekteierens beslutning. Offentlig tilgjengelighet gir ikke automatisk en åpen kildekodelisens; lisens og tredjepartsrettigheter må avklares før gjenbruk eller videre distribusjon.

## Dokumentasjon

- [Kravspesifikasjon](Docs/Requirements/FISKELAGET_Kravspesifikasjon_v1.0.md)
- [Arkitektur](ARCHITECTURE.md) og [modulkontrakter](MODULE_CONTRACTS.md)
- [Utviklingsoppsett](DEVELOPMENT.md), [tester](TESTING.md) og [bidrag](CONTRIBUTING.md)
- [Felles oppgavesystem](TASK_SYSTEM.md) og [AI-samarbeid](AI_BRIDGE.md)
- [Agentregler](AGENTS.md), [tilgang](SECURITY.md), [beslutninger](DECISIONS.md) og [endringer](CHANGELOG.md)

## Oppgaver og kvalitet

[GitHub Issues](https://github.com/hansarne-lang/fiskespillet/issues) er den autoritative oppgavelisten. GitHub Projects gir felles oversikt når integrasjonen er aktiv. Statusene er Backlog, Ready, In Progress, Blocked, In Review og Done; Awaiting Approval er en separat godkjenningsmarkering.

Endringer går gjennom pull request og relevante kontroller. En oppgave er ikke Done før tester, godkjenning og integrasjon er bekreftet. CI i bootstrapfasen kontrollerer repositoryet; dette er ikke en Unreal-byggtest.

Kjør den lokale repositorykontrollen med `python Tools/Validation/check_repository.py` fra repositoryroten. Kontrollen krever bare Python 3.11 eller nyere og standardbiblioteket.
