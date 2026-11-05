# v0.4 – lokal fangstjournal og artsrekorder

Oppgave #66 bygger på PR #65 / e0fd5682af87f57ffd1880d04605c5ea144643a4. F4 åpner egen journal fra hovedmeny eller spill; F6 veksler journal/artsrekorder. 1/2/3 sorterer dato/art/vekt, 4 filtrerer alle/solgt/tapt, 5 filtrerer art. Piler/PgUp/PgDn blar. F5 forsøker lagring på nytt. Menyer pauser ikke klokke eller kamp og åpning slipper løfte-/sveiveinput.

## Bekreftelse og hemmelighold

Serveren registrerer agn, dybde, program, fiskeplass, UTC-tid, dagtid og kampvarighet ved autoritativ landing. Kampvarighet måles fra starten av første automatiske utras til landing. Disse metadataene lagres i den private ledgeren sammen med de uendrede fangstdataene. Det replikerte oppgjøret får en egen tilfeldig dag-GUID, bevart gjennom dagen og rotert ved N. Ingen catch-entropi utledes av denne GUID-en.

Journalen tar bare imot eget spillerresultat fra den avslørte, gjeldende dagen ved eller etter 1 020 000 ms. Art, eksakte mål, antall, verdi, metadata og nye rekorder kommer ikke tidligere. Oppgjørsraden valideres samlet, inkludert summer og metadata. Ingen delvis tilføying ved ugyldig fangst, konflikt eller full journal. Samme fangst-ID og identiske data er idempotente; samme ID med andre data avvises. Tidligere bekreftede poster kan vises under en ny fiskedag.

Hovedtall er landet antall, landet vekt, faktisk salgsinntekt og største landet fisk. Sen retur gir tap uten salgsinntekt; den landede fisken teller fortsatt i statistikk og artsrekord. Artsrekorden viser vekt og lengde for den største landede fisken per art. Utstyr og båt er fortsatt prototypens faste utstyr/startbåt; ingen utstyrs- eller båtprogresjon oppfinnes i journalen.

## Lokal fil og sikkerhetskopi

`FiskelagetJournal.ini` ligger ved stangoppsett og innstillinger, normalt `Windows/FiskELaget/Saved/Config/Fiskelaget`. `-UserDir` isolerer testprofiler. Journalformatet er versjon 1 med dag-/fangst-GUID, dagnummer, validert fangst, salgsstatus og fangstmetadata. Tidspunkt lagres og vises eksplisitt i UTC. Grensene er 4096 poster og 4 MiB; overskridelse gir beskjed uten å slette gamle poster. Heltall parses med streng format-/områdekontroll.

En navngitt Windows-mutex med prosjekts- og filavgrenset navn samordner lesing/skriving mellom prosesser. Innen låsen leses den nyeste gyldige generasjonen fra hovedfil eller `.bak`, og validerte poster flettes med minnet uten duplikater. Motstridende ID-er avvises. Nye data skrives til `.tmp`; en validert forrige generasjon kopieres via `.bak.tmp` til `.bak`, og så erstattes hovedfilen. Første lagring lager også backup. En forlatt prosesslås gir ikke tillit til diskdata: begge filer valideres alltid.

En ugyldig hovedfil og gyldig backup gir synlig gjenopprettingsmelding. Den siste hovedfilens ekstra poster kan mangle hvis bare en eldre backup er gyldig. Neste vellykkede F5 lager ny hovedfil og bevarer en gyldig backup. To ugyldige/ukjente filer overskrives ikke automatisk. Midlertidig lås-/skrivefeil gir ingen falsk lagrebekreftelse; nye poster kan vises i minnet med tydelig varsel og F5-retry. Ugyldige eller for mange oppgjørsposter gir særskilt varsel som beholdes også når en allerede gyldig journalfil kan lagres.

## Omfang

Dette er lokal journal/statistikk, ikke en komplett karriere, butikk, disponibel saldo, profilautorisasjon eller skyfunksjon. Journaldata gir ingen rettigheter på serveren. Oppgjøret må mottas mens egen spiller er tilkoblet. Innhenting av savnede oppgjør etter frakobling er ikke implementert. Filene følger ikke automatisk mellom PC-er eller forskjellige utpakkede versjonsmapper; kopier brukerdata bevisst etter avsluttet spill. En lokal kopi er ikke et uavhengig bevis mot bevisst redigering av egne filer.

Alle tidligere spillregler, N-dagbytte, F2/F3 og enspiller/LAN beholdes. Ingen nye ressurser, arter, priser eller kampmodeller inngår. Testresultater registreres i issue/PR på eksakt kilde; planlagte prøver er ikke bestått før faktisk kjøring. Den lange prøven bruker normal 20-minuttersdag, fortsetter til dag 2 og kontrollerer bare egen journal før/etter 17:00. Nye prosesser kontrollerer lasting fra hovedmenyen og backupgjenoppretting. To ødelagte filer prøves i en separat testprofil. Ingen personlige testprofiler eller logger skal i ZIP-en.
