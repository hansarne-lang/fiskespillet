# Mannskap og overføring av kapteinsrollen

Oppgave #57 bygger på PR #56. K åpner mannskapsoversikten når spilleren er om bord. Piler velger, Enter eller museklikk aktiverer. Gjeldende kaptein velger medspilleren for å gi fra seg rollen. K/Escape/Tilbake lukker. Panelet viser bare serverbekreftet kaptein. Det fortsetter å oppdateres under transport og ved frakobling.

Båteieren er standardkaptein når vedkommende er om bord. Før eieren kommer om bord, velges første ombordværende. En uttrykkelig overført rolle beholdes og tas ikke automatisk tilbake av eieren. Når kapteinen forsvinner, overtar båteieren hvis den er igjen om bord, ellers første gjenværende mannskapsmedlem. En tom båt har ingen kaptein. Dette er sesjonsroller; permanent karriere og eierskap mellom servere endres ikke.

Kaptein-ID, fiskeplass og kommandorevisjon ligger i samme repliserte lesemodell. Overføring, fallback og rutevalg endrer revisjonen. Serveren validerer requesterens faktiske spiller-ID, gjeldende rolle, revisjon og at målet er en annen om bord. Gamle kommandoer avvises også etter at rollen er overført fram og tilbake til samme spiller. Utreise/retur sender nå den observerte revisjonen med forespørselen; rutevalg var allerede revisjonskontrollert. Ingen overføring flytter båten, endrer målet, begynner reisen på nytt eller tilbakestiller klokken. Metning av revisjonstelleren feiler lukket.

Rollebytte er tilgjengelig ved brygga, under utreise, under fiske og under retur. Alle beholder sine egne stenger og fangster. Kapteinsrollen gir bare kontroll over felles rute og tur/retur. Den andre spilleren må ikke lukke en meny eller godkjenne retur. Lokale spillkontroller er sperret mens mannskapsmenyen er åpen, men serverklokken og båten fortsetter. Som andre menyer pauser den ikke en pågående fiskekamp.

## Verifisering

To nye Unreal-tester dekker kapteinsrettigheter, ugyldig mål, gamle revisjoner, overføring tilbake til opprinnelig spiller, standardkaptein, uttrykkelig valg og frakoblingsfallback. De 17 tidligere testene skal fortsatt bestå.

`Tools/Testing/Run-CaptainTransfer.ps1` starter to lokale grafiske Windows-prosesser med isolerte brukerområder og localhost-port 7777. Begge må bruke samme bygg. Den bruker faktisk InputKey-rute for mannskapsmeny, kart, stenger, avreise og retur, og server-RPC-er for avvisningstilfellene. Den nye kapteinen velger rute og starter turen etter at gammel kaptein er avvist. Rollen byttes fram og tilbake under begge reiseetapper, som fortsatt skal ta 12 sekunder. Begge får passive stenger. Ny kaptein starter retur mens den andre holder mannskapsmenyen åpen. Til slutt kobles kapteinen fra, og den gjenværende spilleren skal overta. Sentrale sider tas som bilder.

Prøven forkorter verken forberedelsene eller reisetiden. Napp og nye landinger er ikke målet i denne pakken; stengene trekkes inn ved kapteinens retur. To fysiske PC-er og en ny full 20-minuttersdag inngår ikke. Ingen nye binære ressurser eller runtime-moduler.

Claude/Broen-review venter til lørdag. Endringen holdes på egen gren og utkast-PR; ikke Done før godkjent og integrert.
