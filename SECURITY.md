# FISKELAGET – Tilgang og sikkerhet

Repositoryet er offentlig. Commit aldri hemmeligheter, lokale profiler, personlige logger, samtaleeksporter, byggmaskinlegitimasjon eller koordinatorstate. Offentlige dokumenter beskriver produkt og arbeidsflyt; lokal miljøkartlegging lagres utenfor Git.

Prosjekteieren administrerer tilgang og designbeslutninger. Utviklere bruker individuelle kontoer, egne grener og tildelte filområder. AI-rollen registreres separat fra GitHub-kontoen. CODEOWNERS er en review-regel, ikke filreservasjon eller identitetskontroll for agentprosesser.

Bruk repositoryets minimale GITHUB_TOKEN til lesende kontroller. Projects-skriving krever godkjent Projects-tilgang; ikke legg en personlig token i et offentlig workflow. App-/OAuth-tilgang oppbevares i egnet beskyttet nøkkellager eller som begrenset GitHub-secret etter eksplisitt oppsett.

Kjør ukjente PR-er på isolerte GitHub-hosted runners uten hemmeligheter. Ikke bruk en personlig utviklermaskin som generell selvhostet PR-runner. Ikke kjør PR-kode med privilegert pull_request_target. Senere UE-bygg krever et egnet isolert Windows-byggmiljø; oppsettet spores som egen issue.

Nye invitasjoner, betalte tjenester, større arkitekturendringer, designendringer og destruktive handlinger krever prosjekteieren. Testere skal kunne få ferdigpakket Windows-bygg uten kildekodetilgang som forutsetning, selv om denne kildekoden er offentlig.

Rapporter ikke en privat nøkkel eller detaljert utnyttbar sikkerhetsfeil i en offentlig issue. Bruk GitHubs private sårbarhetsrapportering hvis den er aktivert, eller kontakt prosjekteieren gjennom en avtalt privat kanal.
