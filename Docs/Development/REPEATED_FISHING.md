# Flere fangster på samme tur

Oppgave #50 bygger videre på PR #49. F kan sette ut på nytt etter landing, avbrudd eller tap. X trekker inn egen ventende stang eller avbryter egen kamp. En landet fisk beholdes; X gir ingen gjenutsetting av en allerede landet fisk.

Etter landing fullføres den eksisterende tre sekunders animasjonen før en ny utsetting tillates. HUD viser når stanga er klar. Hver utsetting får et nytt, tilfeldig identitetsmerke. Alle kontrollmeldinger og utsettingsforsøk bærer merket de ble sendt med. Serveren avviser gamle merker og gjentatte forsøk. En slik avvisning endrer ingen ny kamp. Serveren håndhever fortsatt eier, fiskeplass og dagsfase.

Livsløpsporten `FFiskRodCycle` holder styr på aktiv utsetting og tidspunktet en ny kan begynne. Den samme porten brukes av spillkoden og testes uten en spillverden. Hver gyldig landing får en egen fangst-ID. Avbrudd registrerer ingen fangst. Nye identitetsmerker tilbakestiller også fiskens presentasjon, slik at senere fisker kan bli synlige og få sine egne landingsanimasjoner. Offentlig visning inneholder et tilfeldig animasjonsmerke, ingen fangstteller, art, vekt eller salgsverdi.

Dagens detaljerte fangstresultater og saldo forblir skjult til 17:00. Testfiskens eksisterende pris på 10 fiskepenger og vekt på 1 kg er uendret. Ingen permanent karriere, lager-/slepebalanse eller automatisk ny fiskedag innføres her.

## Verifisering

Unreal-testene dekker livsløp, dupliserte og gamle kommandoer, eierisolasjon og fire unike fangster fordelt på to spillere med ett oppgjør. De tidligere kamp-, klokke- og fristtestene skal også bestå.

`Tools/Testing/Run-RepeatedFishing.ps1` bruker to lokale nettverksprosesser med vanlig dagsklokke og vanlig nappintervall. Begge trekker først inn en ventende stang. Når begge har fisk på, avbryter gjesten egen kamp mens verten fortsetter. Begge lander deretter to fisk hver og returnerer. Testen prøver også gjentatt F, for tidlig ny utsetting, gjentatt landing og gamle F-/X-/sveive-/landingskommandoer. Et gammelt landingsforsøk sendes også når neste fisk faktisk er klar til landing, og den nye kampen skal fortsatt vente på gyldig handling.

Prøven fortsetter til dagsslutt ved 20:00. Den krever to leverte testfisk og 20 fiskepenger per spiller ved 17:00, ingen tidlige resultatdata, ingen ekstra oppgjør og at server- og klientklokke følger virkelig tid. Med `-Graphics -Audio -CaptureDirectory <mappe>` tas egne bilder for hver landingsanimasjon og lokal lydstrøm kontrolleres. To fysiske PC-er er fortsatt utsatt. Claude-review via Broen venter til lørdag, og endringen legges som PR-utkast.
