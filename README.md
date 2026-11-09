# FISKELAGET

FISKELAGET er et planlagt realistisk, sosialt fiskespill for Windows PC. Inntil 12 spillere skal kunne spille sammen på LAN i en fiktiv karibisk øygruppe, med egne eller delte båter og permanent karriere.

## Prosjektstatus

Spillbar Unreal Engine 5.8-utviklingsversjon, **v0.22**. Spill alene eller inntil tolv sammen over LAN, med egne eller delte båter, 15 arter i fem habitater, 20-minutters fiskedager og lagret karriere. Modeller og figurer er stiliserte; ferdig realistisk grafikk påstås ikke. Fysisk LAN og ytelse på flere PC-er må fortsatt prøves.

- **Spille:** installer og start spillet fra launcheren SpilLan ([oppskrift](Docs/Development/SPILLAN.md)), eller start Windows-pakken med `Start-FISKELAGET.bat`. Unreal Editor trengs ikke. Alle på LAN må bruke samme versjon. Taster og spillflyt står i [spillerguiden](Docs/PLAYER_GUIDE.md).
- **Nytt i hver versjon:** [endringsloggen](CHANGELOG.md).
- **Utvikle:** Unreal 5.8.4 og verktøyene i [DEVELOPMENT.md](DEVELOPMENT.md). Hver Windows-pakke inneholder faktisk test-SHA, kontrollsummer, resultater og kjente begrensninger.

Lagringer er lokale; GitHub synkroniserer ikke spillprofiler mellom PC-er.

Repositoryet er offentlig etter prosjekteierens beslutning, men **alle rettigheter er forbeholdt**. Se [LICENSE](LICENSE). Offentlig innsyn gir ingen rett til gjenbruk eller videre distribusjon. Unreal Engine er underlagt Epics egne vilkår.

## Dokumentasjon

- [Spillerguide](Docs/PLAYER_GUIDE.md) og [endringslogg](CHANGELOG.md)
- [Assetliste og fremdrift](Docs/Assets/FISKELAGET_Assetliste.md), [assetstandard](Docs/Assets/FISKELAGET_Assetstandard.md) og [assetdokumentasjon med forhåndsbilder](Docs/Assets/README.md) (status 5. november 2026)
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

Historiske åpne oppgaver avstemmes mot de integrerte leveransene i #100; nye restoppgaver skal beskrive det som faktisk gjenstår. Fysisk LAN mellom to PC-er må dokumenteres ved faktisk kjøring.

