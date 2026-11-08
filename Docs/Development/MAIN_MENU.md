# Hovedmeny og LAN-oppstart

Oppgave #30 bygger videre på PR #51. Vanlig oppstart viser Spill, Bli med venner, Innstillinger og Avslutt over den eksisterende 3D-marinaen. Spill åpner en LAN-vert. Bli med venner bruker en IPv4-adresse med valgfri port; standardporten er 7777. Den lokale prøven bruker 127.0.0.1. Dette er en enkel prototypemeny, laget fra kode uten nye binære ressurser.

Mus, piltaster eller Tab velger, Enter aktiverer og Escape går tilbake. Adressefeltet støtter tall, punktum, kolon, Backspace og Ctrl+A for å erstatte teksten. Adressen valideres før nettverkskall og kan ikke inneholde reisevalg eller kartbaner. Mislykket tilkobling og Avbryt gir en brukbar meny igjen. Et tilkoblingsforsøk avbrytes etter 15 sekunder uten forbindelse.

Innstillinger tilbyr hovedlyd På/Av og bildefrekvens 30/60. Valgene brukes gjennom motorens lyd- og bildeinnstillinger og lagres i lokal brukerinnstilling. Escape i en økt åpner Fortsett, Innstillinger, Til hovedmeny og Avslutt. Menyen pauser ikke LAN-klokken. Å forlate verten bryter også klientenes forbindelse; klientene får en melding og kan prøve igjen fra hovedmenyen.

Menyinput håndteres før spillbindingene. Kontroller som E, R og F betjener derfor ikke båten mens menyen er åpen. Eksisterende fiskeregler, fangstidentiteter og oppgjørsbarrieren ved 17:00 er uendret. Ingen ny modul eller tredjepartstjeneste er lagt til. `UFiskGameInstance` beholder innstillinger og tilkoblingsmeldinger gjennom reise mellom menyer og økter.

## Verifisering

Tre Unreal-menymodelltester dekker IPv4-/portgrenser, avviste reisevalg, navigasjon, innstillinger, avbrytelse og tekstredigering. De eksisterende ti spilltestene skal også bestå.

`Tools/Testing/Run-MainMenu.ps1` starter Windows-pakken med isolert brukerområde, vanlig grafikk og lokale tilkoblinger. Den bruker kontrollerens InputKey-rute for tastatur og den samme knappehandleren som museinput. Vert og klient starter via menyen, går om bord, reiser ved 02:00 og setter ut hver sin stang. Prøven sjekker at menyen ikke pauser dagsklokken, at innstillinger leses av en ny prosess, og at klienten kommer tilbake til menyen når verten forlater økten. En separat prosess tester Avbryt, mislykket forbindelse og Avslutt. Bilder tas av alle sentrale sider.

Prøven binder bare localhost, bruker standardport 7777 og en ubrukt feiltestport 17784, og stanser hvis portene er opptatt. Vert, klient og feilprøve har hver sin `-UserDir` under loggmappen, så de får ulike karrierer. Når verten har lagret innstillingene, kopieres bare `FiskelagetSettings.ini` og `GameUserSettings.ini` til klientens profil. Det viser at innstillingene overlever til en ny prosess (#126). To fysiske PC-er er fortsatt utsatt. Claude/Broen-review venter til lørdag; PR-en beholdes som utkast.
