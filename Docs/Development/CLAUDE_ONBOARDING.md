# FISKELAGET – Oppstart for ny utvikler

Status: kort oppstartsveiledning skrevet i samarbeidsprøven [issue #7](https://github.com/hansarne-lang/fiskespillet/issues/7). Den erstatter ikke [AGENTS.md](../../AGENTS.md), [TASK_SYSTEM.md](../../TASK_SYSTEM.md), [CONTRIBUTING.md](../../CONTRIBUTING.md) eller [DEVELOPMENT.md](../../DEVELOPMENT.md); ved avvik gjelder de.

## 1. Les grunnlaget

Les i denne rekkefølgen før du skriver noe:

1. [Kravspesifikasjonen](../Requirements/FISKELAGET_Kravspesifikasjon_v1.0.md) – godkjente spillregler.
2. [ARCHITECTURE.md](../../ARCHITECTURE.md) og [MODULE_CONTRACTS.md](../../MODULE_CONTRACTS.md) – moduler, eierskap og hendelseskontrakter.
3. [AGENTS.md](../../AGENTS.md) og [TASK_SYSTEM.md](../../TASK_SYSTEM.md) – arbeidsregler og statusmodell.
4. [TESTING.md](../../TESTING.md) – hvilke kontroller som gjelder for hvilken endring.

## 2. Finn og claim en koordinert oppgave

GitHub Issues og det felles Project er eneste sannhetskilde. Ikke hold en egen TODO-liste.

```sh
gh project item-list 1 --owner hansarne-lang --format json --limit 100 \
  --jq '.items[] | {n: .content.number, status: .status, agent: .agent, title: .title}'
gh issue view <nr> --repo hansarne-lang/fiskespillet
```

- Velg bare en issue med Status `Ready`, riktig `Agent`/ansvarlig og ferdige avhengigheter («Blocked by»). Aldri start `Blocked`, `Awaiting Approval` eller en issue som allerede er `In Progress`.
- Kontroller seksjonene `AllowedFiles`, `Akseptansekriterier` og `Testkrav` i issuen. Mangler noe, spør i issuen i stedet for å gjette.
- Claim gjennom koordinatoren. Når tildelingen er avtalt, sett status og les den tilbake:

```sh
gh project item-edit 1 --owner hansarne-lang \
  --url https://github.com/hansarne-lang/fiskespillet/issues/<nr> \
  --field Status --value "In Progress"
gh issue comment <nr> --repo hansarne-lang/fiskespillet \
  --body "Claim: <agent/person>, gren task/<nr>-<kort-navn>, AllowedFiles: <stier>"
```

Start ikke arbeidet før tilbakelesingen viser `In Progress`.

## 3. Egen gren og worktree

Én gren og én worktree per issue, navngitt `task/<nr>-<kort-navn>`:

```sh
git fetch origin
git worktree add ../<kort-navn> -b task/<nr>-<kort-navn> origin/main
```

Push aldri til `main`. Del ikke `Intermediate/`, `Saved/` eller en kjørende Unreal Editor-instans med andre worktrees.

## 4. Unngå filkonflikter

- Endre kun filene i issuens `AllowedFiles`. Alt annet koordineres i issuen først, også «små reparasjoner».
- `Tools/**`, `Config/**`, `.github/**`, `*.Build.cs`, `*.uproject` og kontrakter eies av Integration med mindre issuen sier noe annet.
- Binære Unreal-ressurser (`*.uasset`, `*.umap`) låses før redigering: `git lfs lock <sti>`, og `git lfs unlock <sti>` etterpå.
- Kontroller før commit at bare tillatte filer er endret:

```sh
git status
git diff --stat origin/main
```

## 5. Kjør kontrollene

Det finnes to helt forskjellige typer kontroll. Hold dem adskilt i rapporter.

**Repositorykontroll** (krever bare Python 3.11+, ingen Unreal):

```sh
python Tools/Validation/check_repository.py
python -m unittest discover -s Tools/Validation -p "test_*.py" -v
```

Den første gir JSON med `"status": "passed"` eller `"failed"` og sjekker påkrevde filer, kontrakter, statusmodell, maler, lokale lenker og publiseringshygiene (for eksempel tokens og private stier). Den andre kjører enhetstestene for selve kontrollen. Ingen av dem kompilerer Unreal eller tester spillmekanikk.

**UE-bygg og Unreal-tester** (`unreal-build`, `unreal-tests`, `package-smoke` i [TESTING.md](../../TESTING.md)) krever Unreal Engine 5.8.4 og en godkjent Windows C++-verktøykjede. Disse er ennå ikke etablert i repositoryet (se issue #3 og #5). Rapporter dem som `Not Run` til de faktisk finnes og er kjørt.

## 6. Commit, PR og rapport

```sh
git add <tillatte filer>
git commit -m "<type>: <kort beskrivelse> (#<nr>)"
git push origin HEAD:task/<nr>-<kort-navn>
gh pr create --repo hansarne-lang/fiskespillet --base main \
  --head task/<nr>-<kort-navn> --title "<tittel>" --body "Refs #<nr>. ..."
```

- PR-teksten lenker issuen, beskriver endringen og oppgir faktisk testresultat: `Passed`, `Failed`, `Not Run` eller `Not Applicable`. `Not Run` er aldri `Passed`.
- Ikke merge eller godkjenn din egen PR. Review gjøres av en annen autorisert person.
- Når PR finnes og kontrollene har bestått: sett issuen til `In Review` og kommenter PR-lenke, commit-SHA og testutfall i issuen. Sett aldri `Done` selv; det gjør integrasjonen etter review, beståtte sjekker og merge.
- Blir du blokkert, sett `Blocked` og skriv årsak, neste handling og ansvarlig i issuen.

## Verifiseringsnote

Under arbeidet med #7 (3. november 2026) leste Claude Code den samme felles GitHub-listen (Project 1 hos hansarne-lang) med kommandoen i avsnitt 2 og observerte at #6 «Samarbeidsprøve – Codex dokumenterer GitHub-testbevis» sto som `Codex-Fishing` / `In Progress`, mens #7 sto som `Claude-BoatsWorld` / `Ready` før claim. Bare repositorykontrollene i avsnitt 5 er kjørt for denne endringen. Fiskespillet, Unreal-bygg og spilltester er ikke kjørt eller testet.
