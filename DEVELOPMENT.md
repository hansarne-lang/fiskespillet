# FISKELAGET – Utviklingsoppsett

## Kloning og forutsetninger

Bruk Git, Git LFS, Python 3.11+ for repositorykontroller, Unreal Engine 5.8.4 og en motorstøttet Windows C++-verktøykjede. Verktøykjede og minimalt UE-prosjekt spores som separate issues. Ingen kompilering eller spillbar prototype er dokumentert av bootstrapen.

Klon `https://github.com/hansarne-lang/fiskespillet.git`. Kjør `git lfs install --local` og `git lfs pull` i klonen. Les krav og issue før en oppgavegren opprettes. Dokumentasjon krever ikke Unreal-installasjon.

## Lokal struktur

Foreslått Windows-struktur er hovedklone i `C:Fiskespilletmain` og worktrees under `C:Fiskespilletworktrees`: fishing, boats-world og economy-career. Logger, koordinatorstate og testbygg skal ligge utenfor repositoryet. Den eksisterende lokale kravoriginalen beholdes.

Hver aktiv gren er `task/<issue-number>-<kort-navn>`. Integrasjonsansvarlig bruker også en oppgavegren. Ingen deler skrivbare Intermediate-, Saved- eller Editor-instansmapper. Start med én tung UE-prosess av gangen og juster etter målt kapasitet.

## Kontroll og integrasjon

Kjør `python Tools/Validation/check_repository.py`. Den kontrollerer dokumentlenker, påkrevde kontrakter/felter, oppgavestatuser, maler og publiseringshygiene. Den kompilerer ikke UE og tester ikke fiskemekanikker.

Push egen gren, åpne PR med issue og testbevis, og sett oppgaven In Review. Main krever kontroll og uavhengig godkjenning når grenreglene er aktivert. GitHub Issues viser om denne etableringen har gjenværende blokkeringer. Ingen automatisk sammenfletting av ukjent kode.

## Bootstrap

Prosjekteieren godkjente arkitektur og GitHub-oppsett 3. november 2026 og valgte offentlig repository. Den første rot-commiten etablerer det tomme repositoryets nødvendige dokumenter, maler og kontrolljobb. Dette er det dokumenterte bootstrapunntaket. Alle etterfølgende endringer går gjennom PR.
