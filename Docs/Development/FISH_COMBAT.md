# Fiskekamp med brems og belastning

Oppgave #48, bygget videre fra PR #47. Serveren eier hver spillers uavhengige kampmodell. Dette er en generell prototypefisk; artsatferd, utstyrsbalanse og varig karriere kommer senere.

## Spilling

Sett ut med F. Napp kommer etter et tilfeldig intervall på 2–3 minutter. Første utras varer tre sekunder og krever ingen handling. Hold venstre museknapp for å løfte stanga, slipp for å senke den. Hold høyre for å sveive. Pump ved å løfte uten sveiving og deretter senke og sveive inn. Musehjul opp strammer bremsen; ned løsner den. Mellomrom lander når fisken er nær og under kontroll. F setter ut på nytt etter tap.

Bremsen starter på 40 % og justeres i trinn på 10 prosentpoeng mellom 10 og 100 %. HUD viser brems, line ute, linebelastning, utras og returfrist. Belastning over 100 % i ett sekund gir linebrudd. Belastning under 20 % i tre sekunder kan løsne kroken. Å la stanga stå nede uten sveiving senker belastningen gradvis. Reager på advarslene ved å avlaste eller ta opp slakk.

Fisken gjør nye tilfeldige utras tidlig i kampen. Senk stanga og slipp sveiva under utras. En skjult utholdenhetsmodell avgjør når fisken kan landes; den vises ikke som helsebar. Alle tall er foreløpige testverdier.

## Nettverk og presentasjon

Kampen går i faste steg på 50 ms, drevet av serverens monotone dagsklokke. Rendering og antall mottatte knappekommandoer gir ingen ekstra fremdrift. Klienten sender bare egne kontrollhandlinger. Eksakt belastning og brems replikeres samlet bare til stangeieren. Medspillere får fysiske visningsdata for line, løftet stang og utras. Fangst-ID og registrering av landing beholder duplikatvernet. Et tap registrerer ingen fangst.

Stangbøy, line og fisk følger kampens visningsdata. Enkle dråper vises nær fisken og ved begynnelsen av landing. Lokal, syntetisk snellelyd skiller mellom sveiving og utras, og er stille ellers. Ressursene lages fra kode og motorens eksisterende grunnformer. Ingen nye binære ressurser eller eksterne lydopptak inngår.

Dagens fangstresultater og økonomiske oppgjør følger fortsatt avsløringen ved 17:00. Båtreise, frist og oppgjør er uendret i denne oppgaven.

## Verifikasjon

Unreal-testene `Fiskelaget.Fight.*` dekker pumping og nye utras over 100 tilfeldige frø, brudd/slakk med en uavhengig kamp, avlastning før brudd, gyldig brems, tidssteg og lydsignal. De eksisterende dag- og oppgjørstestene skal også bestå.

`Tools/Testing/Run-FishCombat.ps1` starter to lokale nettverksprosesser. `-Case land` krever to samtidige kamper med hver sin landing; `-Case break` og `-Case slack` krever at gjesten taper mens verten fortsetter og lander. Tidlig, gjentatt og etter-tap-landing forsøkes og avvises. Bare kapteinen får starte hjemturen. Før oppgjøret kreves en skjult resultatvisning uten rader. `-Graphics -Audio -CaptureDirectory <mappe>` kontrollerer også visningshendelser, skjermbilder og at lydsystemet konsumerer den lokale lydstrømmen. Dette siste beviser avspilling i motoren; menneskelig vurdering av lydkvalitet inngår ikke i automatikken.

Testene endrer ingen spilltider. Forberedelser, reise og tilfeldig napp kjøres med vanlig serverklokke. To prosesser på samme PC verifiserer lokal nettverkssynkronisering; to fysiske PC-er er fortsatt utsatt av prosjekteieren. Hele den 20 minutter lange dagen ble testet i den tidligere dagsoppgjørsoppgaven; kampoppgaven gjentar grense- og oppgjørstestene uten å påstå en ny full dagstest.

Claude-review via Broen venter til lørdag. Ingen merge eller Done-status før den avtalte gjennomgangen og integrasjonen.
