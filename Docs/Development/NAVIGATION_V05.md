# v0.5 – Ekkolodd, favoritter, HUD og LAN-tavle

Oppgave [#68](https://github.com/hansarne-lang/fiskespillet/issues/68) bygger på v0.4 / PR #67. Pakken gjennomfører flere visnings- og navigasjonskrav fra godkjent kravspesifikasjon v1.1 kapittel 7. Normal 20-minuttersdag, absolutt returfrist og felles avsløring ved 17:00 beholdes. Dette er en lokal testpakke før review og integrasjon.

## Ekkolodd

F7 åpner en grafisk habitatprofil ved båtens aktuelle fiskeplass. Den viser bunndybde, bunnstruktur, omtrentlige fiskedybder og eierens bekreftede agndybde. Linjen markerer eget agn; symbolene beskriver katalogens omtrentlige habitatbånd. Dette er uttrykkelig en statisk habitatprofil fra det eksisterende sjøkartgrunnlaget, ikke en simulert sanntidsskanning, ekkohistorikk, fisketelling eller garanti om napp. Art, størrelse, faktisk fangst, planlagt napp og skjult utholdenhet brukes ikke i modellen. Ved brygga, under reise/retur og for spillere på land vises ingen målt profil. Dybden endres gjennom vanlig G-oppsett og servervalidering når stanga er ledig.

## Navngitte fiskeplasser

M åpner sjøkartet. Tab veksler mellom kartet og seks lokale favoritter. Piler, 1–6 eller klikk velger favorittplass. F5 lagrer båtens **serverbekreftede valgte rute** i denne plassen. F2 gir favoritten et nytt navn; skriv A–Z/0–9, mellomrom eller bindestrek, fjern med Backspace og bekreft med Enter. Escape avbryter navneredigering. Delete sletter valgt favoritt. Enter bruker favorittens rute gjennom eksisterende kapteinskommando. M eller Escape lukker panelet når navneredigering er avsluttet.

Navnet begrenses til 24 tegn; lagrede norske bokstaver støttes av formatet. Det enkle tastaturfeltet bruker store A–Z og tall og har ikke full Unicode-/IME-redigering. Favorittene er referanser til de tre eksisterende prototypeplassene. Fritt sjøpunkt, nye områder og dorging er ikke implementert. Bare gjeldende kaptein kan velge rute ved brygga. En lokal lagring flytter ikke båten og endrer ingen rettighet eller fiskeparameter.

`FiskelagetPlaces.ini` ligger sammen med journal/oppsett i `Saved/Config/Fiskelaget`, normalt under `Windows/FiskELaget/` i pakket spill. Format 1 validerer hele innholdet før bruk og begrenses til 16 KiB. Ugyldig/fremtidig fil bevares og sperrer overskriving. Lagring leser filen på nytt under en navngitt prosesslås, bevarer andre plasser og bruker midlertidig fil før erstatning. Minneoppsettet endres bare etter vellykket skriving. Samtidig endring av samme favorittplass gir siste fullførte skriving; dette er preferanser, ikke økonomidata. Ingen automatisk synkronisering mellom PC-er/versjonsmapper og ingen backupgjenoppretting for favoritter påstås. Etter kontrollert gjenoppretting av en egen gyldig fil kan ny F5-skriving forsøkes.

## HUD og kamp

HUD-en skalerer samlet med vinduet. Stangkortet viser eget agn/dybde/program og båt/kaptein. Dagsklokke og absolutt 17:00-frist ligger oppe til høyre; konservativ returmargin følger eksisterende ruteberegning. Nederst forklares neste handling fra aktuell fase, ombordstigning, kapteinsrolle, rigg og stangtilstand. Under kamp vises stor linebelastningsmåler, line ute, brems og varsler for utras, slakk, bruddfare og landing. Måleren er ikke en helsebar. Ingen skjult fangsttelling eller fiskestørrelse legges til. Menyer og paneler pauser verken klokken eller kampen; åpning slipper løft/sveiving.

## LAN-tavle

F8 åpner dagens rangering etter salgsinntekt. Like beløp deler plassering (1, 1, 3); spiller-ID gir stabil rekkefølge ved likt resultat. Bare samsvarende dag med bekreftet avsløring og lokal klokke minst 17:00 kan vises. Tab bytter til største landede fisk per spiller i siste 32 serverbekreftede oppgjør, også fisk tapt ved sen retur. Dette er øktens oversikt fra eksisterende historikk, ikke full permanente artsrekorder; F4/F6 er fortsatt personlig lagret journal/artsrekorder. Fremtidig/ennå uavslørt historikk filtreres bort. Ved ny dag er dagsrangeringen låst, mens tidligere landede rekorder er synlige. Tomme visninger er tydelig merket. Piler/PgUp/PgDn blar i seks rader per side. Tavlen utsteder ingen premier eller penger og bruker aldri lokale journalfiler som serverautoritet.

## Verifisering og grenser

Unreal-testene `Fiskelaget.Navigation.*` dekker sonarens steds-/ombordgrense, HUD-faser, dags-ID/17:00, lik rangering, skjult ny historikk og beholdt tapt rekord. Filtestene dekker navn, slot/grunn, separate profiler, prosessfletting av andre plasser, omnavning/sletting, feilskriving og bevaring av korrupt/fremtidig format.

`Tools/Testing/Run-NavigationRelease.ps1` starter isolerte grafiske prosesser for lagring/omnavning, ny prosess som bruker favoritten og en ødelagt favorittfil. Deretter kjører to spillere gjennom normal dag, ekte fiske/oppgjør og ny dag med v0.4s journal-, backup- og historikkregresjon. Nye observasjoner åpner faktiske M/F7/F8-kontroller og tar skjermbilder av habitatprofil, HUD, låst/avslørt tavle og beholdte rekorder på dag 2. To egne prosesser laster igjen journal/favoritt og prøver journalens backupgjenoppretting. Enspiller kjøres separat. Faktiske resultater, kilde-SHA og ikke-kjørte kontroller registreres i issue/PR og pakkens TESTRESULTATER.json; dette dokumentet påstår ikke at en planlagt test allerede er bestått.

Kjent gammel kapteinsbytte-smoke-race er utenfor oppgaven. To prosesser på én PC er ikke bevis for to fysiske PC-er. Grafikken bruker fortsatt prototypens generiske båt/fisk/miljø; ingen nye binære ressurser, betalte tjenester, butikk, disponibel karrieresaldo eller ventende IDÉ-oppgaver inngår.
