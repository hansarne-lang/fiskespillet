# Stangoppsett ved den første fiskeplassen

Oppgave #53 bygger på PR #52. G åpner oppsettet for spillerens ene stang når spilleren er om bord og stanga er ledig. Piler velger rad og endrer valg; Enter eller et museklikk går til neste verdi. G, Escape eller Ferdig lukker. Spillkontroller er sperret mens panelet er åpent, men serverklokken fortsetter. Escape lukker først oppsettet; neste Escape åpner øktmenyen.

Agn: naturlig agnfisk, blekksprut eller metalljigg. Fiskedybde: 10–60 meter i trinn på 10. Programmer: rolig, pulserende og aktivt. Valgene vises først når verten har godkjent dem. De beholdes gjennom innhenting, landing, tap og ny utsetting i økten. Denne pakken lagrer ikke permanent karriere og innfører ikke kjøp eller flere stenger.

Den eksisterende faste plassen bruker foreløpige testverdier: 60 meter bunndybde og fisketegn ved 20–40 meter. Dybde innenfor fisketegnene og et passende agnprogram gir god tilpasning. Testparene er agnfisk/rolig, blekksprut/pulserende og metalljigg/aktiv. Én av de to betingelsene gir middels tilpasning; ingen gir dårlig. Dette er balansering ved én prototypeplass, ikke artsfasit for hele spillet. Senere habitatoppslag kan erstatte disse parametrene.

Serveren trekker tilfeldig ventetid per ny utsetting: godt oppsett 120–180 sekunder, middels 180–300 og dårlig 300–480. Hver utsetting bruker en egen servergenerert identitet som frø for sin egen tilfeldigsekvens. Andre spillere eller stenger deler ikke og forbruker ikke denne sekvensen. UI viser ingen nedtelling til napp. Fiskedybden gjelder agnet ved posisjonshold; strøm og dorging simuleres ikke i denne pakken.

Bare eierens kontroller kan be om endring. Serveren kontrollerer ombordstigning, båtfase, dagsfase, ledig stang, utsettingsidentitet, oppsettsrevisjon og alle verdier. Under fiske, utras, kamp, landingens tre sekunder og hjemreise avvises endring. Gamle, doble eller ugyldige forespørsler endrer ikke oppsettet. Oppsettet repliseres kun til stangeieren. Nappets planlagte tidspunkt og fangstresultater inngår ikke i oppsettets nettverkspakke. Oppgjøret ved 17:00 er uendret.

## Verifisering

To Unreal-modelltester dekker gyldige valg, ugyldige verdier, gamle revisjoner, kvalitetsnivåer, tilfeldig variasjon, gjennomsnitt og uavhengige tilfeldigsekvenser. De 13 eksisterende Unreal-testene skal fortsatt bestå.

`Tools/Testing/Run-RigSetup.ps1` kjører to lokale grafiske Windows-prosesser med isolerte brukerinnstillinger og localhost-port 7777. Begge bruker kontrollerens faktiske InputKey-rute for panelet og spillhandlingene. Verten velger blekksprut/40 m/pulserende, klienten metalljigg/20 m/aktiv. Prøven dekker ugyldige og gamle forespørsler, aktive endringer, inputisolasjon, uavbrutt klokke, hovedmenykonflikt, innhenting, ny utsetting, faktiske tilfeldige napp, kamp og landing. Sentrale skjermbilder lagres. Ingen test endrer produksjonens klokke eller ventetid. To fysiske PC-er og full 20-minutters dag er egne prøver; denne pakken krever ikke en ny full dagsprøve.

Claude/Broen-review venter til lørdag. Egen gren og utkast-PR; ingen integrasjon før godkjenning og påkrevde kontroller.
