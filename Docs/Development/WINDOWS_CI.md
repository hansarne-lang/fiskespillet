# Windows-bygg i GitHub Actions

Issue [#5](https://github.com/hansarne-lang/fiskespillet/issues/5). Prosjekteierens PC er byggemaskin («self-hosted runner») for arbeidsflyten [`.github/workflows/windows.yml`](../../.github/workflows/windows.yml). Den bygger, tester og eventuelt publiserer automatisk med [`Tools/Testing/Test-PR.ps1`](../../Tools/Testing/Test-PR.ps1).

## Når den kjører

| Hendelse | Nivå | SpilLan |
|---|---|---|
| PR fra en gren i dette repoet (ikke utkast) | `PR`: bygg, automasjonstester og røyktestene endringen trenger. Krever endringen alle, hoppes de sju trege sanntidsskriptene over (de kjøres om natta) | Nei |
| Push til `main` | `Quick`: bygg og automasjonstester (noen minutter) | Nei |
| Hver natt kl. 01:17 | `Release` på `main`: alle røyktester med normal klokke (3–4 timer) | Nei |
| Push til `release/**` | `Build` | **Ja**, publiseres |
| Manuell kjøring (Actions → Windows build and test → Run workflow) | Valgfritt: Build, PR eller Release | Valgfritt |

Den korte testrapporten vises i jobbens sammendrag og lagres som artefakten `testrapport`. Jobben er grønn når det ikke finnes nye feil (se [testbudsjettet](../../TESTING.md#testbudsjett)). Står PC-en av eller runneren er stoppet, venter jobben i kø, men GitHub avbryter den etter et døgn. Da må den startes på nytt. PR-kjøringene bruker klokkefaktor 4, hopper over ventetiden fra alle båter er hjemme til 17:00 og kjører tre skript om gangen. Nattkjøringen bruker normal klokke uten hopp. `windows-build` er ikke en påkrevd sjekk for fletting, så main stoppes ikke når PC-en er av.

## Sikkerhet

Repoet er offentlig. En self-hosted runner kjører koden den får, med tilgangene til brukeren den kjører som. Derfor:

1. **Ingen kode fra forks.** Jobben har en betingelse som bare tillater PR-er fra grener i dette repoet, push og manuell kjøring.
2. **Godkjenning av eksterne bidragsytere** (gjøres én gang i GitHub): *Settings → Actions → General → Fork pull request workflows from outside collaborators →* **Require approval for all outside collaborators**.
3. Jobben har bare lesetilgang til repoet (`permissions: contents: read`), og den lagrer ikke GitHub-legitimasjon i mappa (`persist-credentials: false`).
4. Bare de som kan pushe til repoet, det vil si prosjekteieren og agentene hans, kan starte jobber på PC-en.

## Oppsett (én gang, ca. 10 minutter)

1. **Registrer PC-en:** På GitHub: repoet → *Settings → Actions → Runners → New self-hosted runner →* **Windows**, x64. Følg kommandoene GitHub viser i PowerShell:
   - Last ned og pakk ut i `C:\actions-runner`.
   - Kjør `config.cmd` med URL og token fra siden. Svar slik på spørsmålene:
     - *runner group:* Enter (standard)
     - *name:* for eksempel `fiske-pc`
     - *additional labels:* **`fiskelaget`**
     - *work folder:* Enter (standard)
     - *run as service:* **N**. Røyktestene åpner spillvinduer og trenger en innlogget skrivebordsøkt. En Windows-tjeneste har ingen skjerm, og SpilLan-publiseringen må kjøre som din bruker.
2. **Start den:** Dobbeltklikk `C:\actions-runner\run.cmd`. Den skal vise «Listening for Jobs». For å starte automatisk ved innlogging legger du en snarvei til `run.cmd` i oppstartsmappa (`Win+R` → `shell:startup`).
3. **Slå på godkjenning for forks** (punkt 2 under Sikkerhet).
4. **Valgfritt:** Ligger Unreal et annet sted enn `C:\Program Files\Epic Games\UE_5.8`, legg til variabelen `UE_ROOT` under *Settings → Secrets and variables → Actions → Variables*.

Byggemaskinen trenger det samme som ved manuell bygging: Unreal Engine 5.8, Visual Studio 2022 med C++, Git og Git LFS, Python 3.11 eller nyere, og SpilLan 0.2.8 eller nyere i `C:\SpilLan` for publisering. Bygg og logger ligger i `C:\Fiskespillet\ci`, og eksporten til SpilLan i `C:\Fiskespillet\eksport`. Testcachen i `C:\Fiskespillet\ci\logs` gjør at samme bygg ikke testes to ganger.

## Når noe går galt

- **Jobben står i kø:** PC-en er av, eller `run.cmd` kjører ikke.
- **Bygget feiler:** se loggen for steget «Build and test» i jobben. Rapporten under viser feil merket NY.
- **Publiseringen feiler med «already exists»:** versjonsnummeret må økes for hvert bygg i `release/**`.
