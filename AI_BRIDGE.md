# FISKELAGET – AI-samarbeid

## Vedtatt retning

Fra 9. november 2026 brukes bare Claude Code. Codex er tatt ut av utviklingen (se [DECISIONS.md](DECISIONS.md)). Flere Claude Code-sesjoner skal bruke egne worktrees og GitHub Issues som autoritativ oppgaveliste. GitHub Projects viser de samme oppgavene. En lokal, prosjektisolert koordinator skal håndtere meldinger, filreservasjoner, begrensede kjøringer og GitHub-synkronisering. En lokal database skal ikke eie en konkurrerende TODO-liste.

Eksisterende Broen er identifisert som mulig grunnlag. FISKELAGET skal få eget prosjektrom og egne agentidentiteter uten å endre andre prosjekters aktive instanser. Implementasjon og faktisk samarbeidsdemonstrasjon spores som separate GitHub-issues. Ingen automatisert agentkjøring er aktivert av denne dokumentasjonen.

Reviewflyten under er **satt på pause** fordi den forutsetter Codex som koordinator. Koden og testene beholdes som historikk til en ny koordinator eventuelt avtales i #4. Prosjekteieren hadde særskilt valgt lesende Claude-review med SHA-bundet handshake i #24. Denne avgrensede, manuelt startede reviewflyten bruker et isolert FISKELAGET-rom og erstatter ikke produksjonskoordinatoren i #4. [Reviewprotokollen](Docs/Development/BROEN_REVIEW.md) beskriver forespørsel, resultat, ACK, tester og den påkrevde GitHub-statusen `claude-review`.

## Meldingskontrakt

Alle arbeidsmeldinger skal ha MessageId (UUID), TaskId (GitHub issue-URL), Sender, Recipient, MessageType, Timestamp (RFC3339 UTC), Payload, RelatedFiles, RelatedCommit og Status. Status gjelder leveringen, ikke oppgaven. Tilleggsfelter er SchemaVersion, ProjectId, CorrelationId, RunId, Branch, WorktreeId og PullRequestUrl.

Meldingstypene er TASK_ASSIGNED, STATUS_UPDATE, QUESTION, ANSWER, CONTRACT_CHANGE_REQUEST, BLOCKED, REVIEW_REQUEST, REVIEW_RESULT, TASK_COMPLETED og ERROR. Senderidentitet bindes av adapteren. TaskCompleted er en anmodning om sluttkontroll og kan ikke selv sette Done.

## Tilstand og samtidighet

Én lokal koordinator serialiserer claims og filleases. Den leser autoritativ GitHub-status, prioritet, godkjenning og avhengigheter før tildeling, skriver tilbake til GitHub og kontrollerer resultatet før start. Eksterne endringer eller delvis feil fører til stopp og avstemming. Utløpt lease er ikke alene bevis på at gammel agent har stoppet.

Meldinger leveres minst én gang med eksplisitt kvittering og idempotent behandling. Varig inbox/outbox og auditlogg lagres transaksjonelt, med testet backup/restore. Logger skal ikke inneholde nøkler. Nettverksbrudd gir ingen nye oppgavestarter.

## Godkjenning og grenser

Prosjekteieren godkjenner design, større arkitektur, tilgangsendringer, betaling, offentlig publisering utover autorisert scope og destruktive operasjoner. En agent kan ikke godkjenne seg selv ved å sette et felt eller sende en melding.

Første samarbeidsprøve begrenses til to dokumentoppgaver, 15 minutter per oppgave, 30 minutter totalt, ett automatisk retry og 100 meldinger. Betalt API-bruk er deaktivert. Fire agentroller er et senere kapasitetsmål; én UE-bygg-/Editor-jobb om gangen er utgangspunktet. Ubegrenset eller uovervåket kjøring er ikke tillatt.

## Klientgrensesnitt

Claude Code-sesjonene kan kobles til prosjektets MCP-verktøy. En koordinatoradapter kan bruke Claude Code print/strukturert output når installerte versjoner er kvalifisert. MCP er verktøytransport, ikke en komplett oppgavekø. Versjoner, tillatelser, stopp og ressursgrenser skal testes før aktivering.

Se [TASK_SYSTEM.md](TASK_SYSTEM.md), [AGENTS.md](AGENTS.md) og [TESTING.md](TESTING.md) for portene som må håndheves. Ingen eksisterende globale konto-/MCP-innstillinger skal endres uten avklaring.
