# FISKELAGET

FISKELAGET er et planlagt realistisk, sosialt fiskespill for Windows PC. Inntil 12 spillere skal kunne spille sammen på LAN i en fiktiv karibisk øygruppe, med egne eller delte båter og permanent karriere.

## Prosjektstatus

Spillbar UE5.8-prototype med solo og fire LAN-spillere, fiskekamp,20minuttersdager, lagret karriere/butikk, to båttyper og frie sjøpunkter. v0.8-grenen legger til sju arter, artsbok, personlige naust, besøk, strandbar og dagslys. Arkitekturen bruker fortsatt FiskCore/FiskGame. Se [v0.8-kontrakten](Docs/Development/MARINA_DISCOVERY_V08.md) og [oppgave74](https://github.com/hansarne-lang/fiskespillet/issues/74) for faktisk utviklings-/teststatus; uintegrerte grener er ikke en godkjent hovedrelease.

Windows-testpakker startes via medfølgende BAT uten Unreal Editor. Velg Spill, E for ombordstigning, R fra spillklokken02:00 og F ved fiskeplassen. P: butikk, M: kart, H: naust, B: strandbar, I: artsbok. Kildeutvikling bruker lokal Unreal5.8.4 og C++-verktøyene i DEVELOPMENT.md. Gamle profiler kopieres med backup til den nye pakkens Saved/Config/Fiskelaget.

v0.8.1 retter gjenoppretting av skadede naustrevisjoner, visning av serverbekreftede husvalg mens lokal lagring venter, og plasseringen av hjelpetekst. Se [kodegjennomgangen](Docs/Development/REVIEW_V08_STABILITY_V081.md). [Forslaget til v0.9](Docs/Development/EXPEDITIONS_V09_PLAN.md) om ekspedisjoner, værpresentasjon og journalgjenhenting er planlagt innhold, ikke implementert funksjonalitet.

Repositoryet er offentlig etter prosjekteierens beslutning. Offentlig tilgjengelighet gir ikke automatisk en åpen kildekodelisens; lisens og tredjepartsrettigheter må avklares før gjenbruk eller videre distribusjon.

## Dokumentasjon

- [Assetliste og fremdrift](Docs/Assets/FISKELAGET_Assetliste.md), [assetstandard](Docs/Assets/FISKELAGET_Assetstandard.md) og [assetdokumentasjon med forhåndsbilder](Docs/Assets/README.md)

- [Assetliste og fremdrift](Docs/Assets/FISKELAGET_Assetliste.md), [assetstandard](Docs/Assets/FISKELAGET_Assetstandard.md) og [assetdokumentasjon med forhåndsbilder](Docs/Assets/README.md)

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

