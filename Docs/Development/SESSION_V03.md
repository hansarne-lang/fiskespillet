# v0.3 – flere fiskedager i samme økt

Oppgave #64 bygger på PR #63 / v0.2. N starter neste dag etter full normal 20:00 og ferdig oppgjør. Bare den lokale listen-verten har denne retten. Kapteinen kan være en annen spiller; det gir ikke rett til å starte dagen på nytt. Serveren kontrollerer forventet dagnummer. Tidlige, gjentatte og foreldede forespørsler avvises.

Alle blir i samme verden og nettverksøkt. Båten settes ved brygga, stenger/kamper/fangstledger nullstilles, mens mannskap, kaptein, fiskeplass og godkjente riggvalg beholdes. Ny dag gir ny stangidentitet, økt rigg- og ruterevisjon og en ny monoton starttid. Dagnummer og klokke replikeres som ett snapshot. En gammel oppgjørsvisning vises aldri som den nye dagens resultater mens separate aktorer replikeres. Automatisk resultatpanel armeres på nytt. Maksimalt dagnummer er 9999 for å avvise overflow.

F3 viser sammendrag fra de siste 32 serveroppgjørene i denne økten, nyeste først: levert/tapt, levert vekt, dagsinntekt og største landet fisk. Bare en oppgjort ledger ved eller etter 17:00 kan legges til. Samme/eldre dag kan ikke legges til på nytt, heller ikke etter at den har falt ut av listen. Tomme dager bevares som oppgjort uten fangst. Sen retur gir null inntekt og tapte fisker, men største landet fisk beholdes. Dagens nye resultater er skjult før 17:00. Historikken lagres ikke på disk, gir ingen varig karriere eller disponibel kontosaldo og nullstilles når økten avsluttes. Dagens F2 beholder detaljert fangstliste. Piler/PgUp/PgDn blar.

Returmarginen er en prognose basert på offentlig klokke og eksisterende reisetid til bryggas sentrum (et konservativt anslag; faktisk marinasone krysses litt tidligere), uten fangstvekt eller annet privat innhold. Ved ett minutts margin blir teksten gul og det gis én beskjed; negativ margin gir rød tekst og én ny beskjed. Nøyaktig null margin regnes fortsatt som mulig ankomst i tide. Verken prognose eller varsel endrer serverens absolutte frist, rute, fart eller kapteinens beslutning.

## Lokal lagring

Oppsett og lyd/FPS bruker en stabil sti fra `FPaths::ProjectSavedDir()/Config/Fiskelaget`. Egen `-UserDir` isolerer spillprosesser i test. `FiskelagetRigPresets.ini` inneholder tre validerte oppsett; `FiskelagetSettings.ini` har skjema 1 og validert lyd/30–60 FPS. Filer skrives via midlertidig fil og erstatning. Lagre-feil gir beskjed, ikke en falsk lagret-bekreftelse. Hvis ny presetfil mangler, forsøkes et eldre relativt v0.2-oppsett lest; første eksplisitte lagring går til ny sti. Den gamle filen slettes ikke. En ugyldig eksisterende ny fil gir tomme plasser og henter ikke foreldede eldre data.

Ingen binære ressurser eller ny spillbalanse inngår. Generisk fiskevisning og kampmodell beholdes. Kjent eldre kapteinsbytte-smoke-race og manglende prøve på to fysiske PC-er oppgis fortsatt. Testresultater og kilde-SHA registreres i issue/PR og lokal pakke; planlagte tester er ikke bestått før faktisk kjøring.
