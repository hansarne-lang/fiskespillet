# Samarbeidsprøve for felles GitHub-oppgaver

Dato: 3. november 2026. Codex arbeider på [issue 6](https://github.com/hansarne-lang/fiskespillet/issues/6), Claude Code på [issue 7](https://github.com/hansarne-lang/fiskespillet/issues/7).

## Formål og avgrensning

Prøven undersøker om begge agentene kan lese samme GitHub-liste, oppdatere hver sin oppgave og observere motpartens status uten manuell kopiering av meldinger. GitHub Issues eier oppgavene; [det private Project](https://github.com/users/hansarne-lang/projects/1) eier felles status og agentfelter. Lokal output er testbevis, ikke en alternativ TODO-liste.

Arbeidet er eksplisitt koordinert: Codex skriver bare denne filen, Claude bare `Docs/Development/CLAUDE_ONBOARDING.md`. De bruker separate grener og worktrees. Ingen spillmekanikk, Unreal Editor eller motorbygg inngår.

## Observerte kontrollpunkter

Codex leste GitHub direkte med `gh project item-list 1 --owner hansarne-lang --format json --limit 100`. Før Claude startet arbeidet, viste listen issue 6 som In Progress / Codex-Fishing og issue 7 som Ready / Claude-BoatsWorld. Agentenes senere oppdateringer og konkrete PR-/testlenker føres i de to GitHub-issuene.

Ved neste direkte lesing observerte Codex at Claude hadde oppdatert issue 7 til In Progress mens issue 6 fortsatt var In Progress. Claudes første rapport bekreftet at den selv hadde lest den samme listen og sett Codex-statusen før og etter egen oppdatering. Første testforsøk ble avvist av kommandotillatelsene og ble korrekt rapportert Not Run. Oppfølgingen med enkeltkommandoer ble deretter fullført: repositorykontrollen og alle åtte testene besto, og Claude opprettet [PR 11](https://github.com/hansarne-lang/fiskespillet/pull/11). [Claudes egen resultatkommentar](https://github.com/hansarne-lang/fiskespillet/issues/7#issuecomment-6599683033) knytter dette til commit `de4c10b47b36cb6552a0a73d408880a2733483f0`.

Ved avslutningen leste begge agentene felleslisten igjen og observerte at både issue 6 og issue 7 var In Review. De to åpne PR-ene er fortsatt ikke integrert, og oppgavene skal derfor ikke stå som Done.

Begge agentene skal lese data gjennom den godkjente GitHub-innloggingen. En lokal kopi av et gammelt skjermbilde eller en prompt med forhåndsfylte statuser er ikke bevis på gjensidig lesing.

## Testgrunnlag

Bootstrap-commit: `153606fd628e8c749b580b0b658255746f331ab1`. Repositorykontrollen og åtte regresjonstester besto lokalt og i [GitHub Actions](https://github.com/hansarne-lang/fiskespillet/actions/runs/41607246812). Disse testene omfatter blant annet manglende kontrakt, feil status, brutt dokumentlenke, manglende LFS-låseregel og utrygg automatisk Done-overgang.

Den enkelte PR må ha egne faktiske testresultater for sin commit. Bootstrapresultatet er ikke automatisk et testresultat for senere endringer. Ingen Unreal-byggtest eller spillregeltest er kjørt av denne prøven.

## Gjennomgang og integrasjon

Etter gyldige tester og opprettet PR settes oppgaven In Review. Done krever uavhengig godkjenning og integrasjon. En agent skal ikke lukke oppgaven bare fordi dokumentet er skrevet. Testet commit, workflow-run og PR skal lenkes i oppgaven.

Produksjonskoordinatorens atomiske claims, filleases, krasjgjenoppretting og tids-/kostnadsgrenser er separate leveranser i [issue 4](https://github.com/hansarne-lang/fiskespillet/issues/4). Denne manuelle, kontrollerte tildelingen erstatter ikke disse mekanismene.

Avvist dobbelt-claim, automatisk Awaiting Approval-blokkering, uløst avhengighet, filkonflikt og gjenoppretting etter GitHub-brudd er ikke demonstrert av denne dokumentprøven. Disse tilfellene må testes når koordinatoren implementeres.
