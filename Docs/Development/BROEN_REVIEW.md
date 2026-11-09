# FISKELAGET review via Broen

> **Satt på pause 9. november 2026.** Flyten bruker Codex som koordinator, og Codex er tatt ut av utviklingen (se [DECISIONS.md](../../DECISIONS.md)). Beskrivelsen, `Tools/Coordination/review_handshake.py` og testene beholdes som historikk. Statusen `claude-review` kreves ikke.

Prosjekteieren har valgt Claude Code som reviewer og et sporbart handshake på Broen som review-port. GitHub skal kreve både `repo-contracts` og `claude-review` på gjeldende commit. Dette erstatter behovet for en separat GitHub-identitet til formell Approve. PR og testkrav består, og ingen automatisk merge inngår.

## Avgrensning og tillit

Dette er en avgrenset, manuelt koordinert reviewflyt i [oppgave #24](https://github.com/hansarne-lang/fiskespillet/issues/24). Generelle produksjonsclaims, filreservasjoner og automatisk oppgavestart i #4 er fortsatt egne leveranser. GitHub Issues og det eksisterende private Project er oppgavestatusens sannhetskilde.

En egen Claude Code-prosess gjør reviewet med lesende verktøy og eksisterende abonnementsinnlogging. Codex er koordinator og filskriver. Claude får ikke GitHub-write, Edit eller Write i reviewprosessen. Det brukes ingen nye kontoer, nøkler eller betalt API-tjeneste.

GitHub-statusen publiseres av den betrodde lokale koordinatoren gjennom prosjekteierens eksisterende GitHub-tilgang. Den er ikke en uavhengig signatur fra en annen GitHub-konto. Broen sin lokale avsenderregistrering og koordinatorens prosesslogg utgjør tillitsgrensen; JSON-felter alene beviser ikke hvilken modell som har arbeidet. En person eller prosess med samme lokale/GitHub-skrivetilgang kan forfalske status. Denne modellen erstatter ikke en separat GitHub App hvis det senere kreves sterkere isolasjon.

## Isolert FISKELAGET-rom

Bruk en egen Broen-instans på loopback med separat kildemappe, chat/postkasse, datamappe og midlertidig token. Bare `BROEN_DATA` er utilstrekkelig: eksisterende Broen skriver også chatfiler relativt til sin egen kildemappe. Kopier transportkoden, ikke et annet prosjekts state, konfigurasjon, regler eller samtaler.

Vekker og automatisk aktivering skal være slått av i denne instansen. Claude startes én gang av koordinatoren med streng MCP-konfigurasjon som bare peker på FISKELAGET-rommet. Ingen globale MCP-/kontoinnstillinger eller andre aktive Broen-instanser endres. Instansen stoppes etter reviewet; rå prosesslogger og tokenfiler forblir private og utenfor Git.

## Handshake på en bestemt commit

1. Les repository, PR, head-SHA, base-SHA og faktisk CI fra GitHub. Krev åpen PR, fulle SHA-er og bestått `repo-contracts`.
2. Koordinatoren sender `REVIEW_REQUEST` som Codex til Claude. Den inneholder unik forespørsels-ID, 256-bit tilfeldig nonce, utløp innen 15 minutter, prosjekt, repository, PR, head og base.
3. Claude leser faktisk PR-diff og kontrollresultater, kvitterer mottak og sender `REVIEW_RESULT` som Claude til Codex gjennom samme Broen. Resultatet gjentar korrelasjonen og inneholder verdict, oppsummering og funn. P0–P2 og alle funn merket blocking hindrer godkjenning. P3 kan være valgfrie forslag.
4. Koordinatoren sender `REVIEW_ACK` til Claude med Broen-resultatets meldings-ID og SHA-256 av resultatpayloaden. ACK betyr at dette konkrete resultatet er mottatt; den gir ikke i seg selv godkjenning.
5. Koordinatoren leser head, base og CI på nytt og kjører verifieren mot de faktiske transportpostene og prosessbeviset. Bare APPROVED uten blokkerende funn, vellykket Claude-prosess og grønn CI gir en verifisert kvittering.
6. Én koordinator må serialisere verifisering, journalføring og publisering. Forespørsels-ID-en konsumeres varig i en privat leveringslogg før grønn status publiseres. En tidligere konsumert forespørsel gir ikke ny godkjenning. Loggen er ikke en oppgaveliste.
7. Publiser Claudes faktiske resultat som tydelig attribuert GitHub COMMENT-review på riktig SHA. Publiser deretter statuskonteksten `claude-review` på samme head-SHA, med lenke til reviewet og kvitteringens hash/ID-er. Ikke utsted forfatterens egen formelle Approve.

Konvolutten bruker AI_BRIDGE.md sine felter. `Payload` har `repository`, `pr_number`, `head_sha`, `base_sha`, `nonce`, `request_id` og `expires_at` i alle tre meldinger. Resultatet har dessuten `verdict`, `summary` og `findings`; ACK har `result_message_id` og `result_digest`. Meldings-ID og avsender i Broen sin transportlogg kontrolleres i tillegg til konvolutten.

Utløpet gjelder selve reviewrunden. En kvittering som ble verifisert og konsumert innen tidsgrensen, kan bevares som historisk bevis for den commiten. Den kan aldri flyttes til en ny head eller base. Ved ny push mangler den nye commiten `claude-review`, slik at den påkrevde kontrollen blokkerer integrasjon til et nytt handshake er gjennomført. Streng CI kreves fortsatt mot oppdatert base.

## Verifier og tester

Verifieren publiserer ingen GitHub-status selv. Koordinatoren skal hente og kontrollere GitHub-fakta; et selvskrevet eller PR-forfatterlevert audit-packet er ikke reviewbevis. Manglende/mislykket verifisering må aldri omgås ved å poste grønn status manuelt.

```text
python Tools/Coordination/review_handshake.py <privat-audit-packet.json> --consumed <privat-leveringslogg.json>
python -m unittest discover -s Tools/Coordination -p 'test_*.py' -v
```

CLI-en leser en eventuell konsumlogg; den oppdaterer den ikke. Varig konsum og API-publisering er koordinatorens ansvar. Ved usikkert API-utfall leses GitHub tilbake før retry. Ny agentstart eller ny godkjenning skal ikke utledes av et tidsavbrudd.

Negative tester dekker feil PR/prosjekt/repository/SHA/nonce/avsender, manglende svar/ACK, feil rekkefølge, duplikater, utløp, replay, blokkerende funn, feil resultatdigest, mislykket reviewer og manglende/feil/gammel CI. `repo-contracts` kjører disse testene i tillegg til repositorykontrollene. Ingen test påstår utført spill- eller UE-verifisering.

## Aktivering av GitHub-porten

Før endring av main-regelen skal en ekte Broen-rundtur og protokolltestene være bestått, og review-portens endring skal være gjennomgått av Claude. Bevar påkrevd PR, diskusjonsavklaring, streng `repo-contracts`, forbud mot force-push/sletting og eksisterende øvrige vern. Legg til `claude-review` som påkrevd status og fjern det gamle kravet om en formell godkjenning fra en annen GitHub-identitet. Kravet om godkjenning av siste push må da også erstattes av SHA-bundet handshake.

Les reglene tilbake og kontroller at PR-er uten `claude-review` blokkeres. Automatisk Done ved issue-lukking skal fortsatt være deaktivert. En oppgave er først Done etter beståtte tester, godkjent handshake på gjeldende SHA og faktisk integrasjon.
