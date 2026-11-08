# Første sjøkart og tre prototypeplasser

Oppgave #55 bygger på PR #54. M åpner kartet når spilleren er om bord. Piler velger rad, Enter eller museklikk velger plass, og M/Escape/Tilbake lukker. G og fiskekontroller når ikke spillet mens kartet er åpent. Klokken fortsetter. Kartet kan leses under fiske og retur; lukk det før R starter tur eller retur. Escape lukker først kartet og kan deretter åpne øktmenyen.

Bare kapteinen kan velge en av de tre plassene ved brygga før avreise. Passasjeren kan lese kartet, men ikke bestemme ruten. Serveren kontrollerer kaptein, båtfase, dagsfase, ruterevisjon og plass-ID. Ruten repliseres i én samlet lesemodell. Den er låst under hele turen, også ved fiske og hjemreise. Dette er første trinn mot kravet om valg av ethvert gyldig sjøpunkt; frie sjøpunkter og ruteplanlegging rundt flere øyer er fortsatt senere arbeid.

| Plass | Reise én vei | Bunndybde | Omtrentlige fisketegn |
|---|---:|---:|---:|
| Korallrevet | 12 s | 60 m | 20–40 m |
| Undervannsskrenten | 40 s | 120 m | 60–90 m |
| Åpent hav | 75 s | 250 m | 20–60 m |

Alle tall er foreløpige prototypeverdier. Den eksisterende korallrevruten beholder 12 sekunder for regresjonstestene. Det styrende dokumentets foreslåtte lengre reisetid for revet kan balanseres senere. Fisketegn avslører ingen bekreftet art, antall, størrelse eller verdi. Nye større fangstpremier og offshore-sannsynligheter er ikke innført.

World eier katalogen, og den lille offentlige verdiavtalen `FiskFishingGround.h` deles av integrasjonslaget. Ingen ny runtime-modul eller binær ressurs. Båten går lineært i serverens monotone virkelige tid, og begge klienter ser samme mål, progresjon og posisjon. Returen bruker samme avstand og reisetid. Ankomst registreres når båtens midtpunkt krysser den godkjente sirkelen på 500 cm; eksakt 17:00 teller. Beregningen bruker valgt rute og avrunder opp til første millisekund innenfor sirkelen, uavhengig av frame-rekkefølge.

Kartet viser faktisk valgt rute, båtposisjon og en konservativ margin beregnet til bryggas midtpunkt. Ved brygga regnes ventetid fram til 02:00, utreise og retur med. Under utreise regnes resterende utreise pluss retur. Ved fiske brukes hele hjemreisen, og under retur bare resterende hjemreise. Gul tekst betyr høyst ett minutt margin, rød betyr null eller negativ margin. Dette er et avrundet estimat og utvider aldri fristen. Allerede hjemme vises ankomststatus.

Stangoppsettet viser valgt steds bunndybde og fisketegn. Dybden velges i 10-meters trinn, til maksimalt bunndybden eller 120 meter i denne pakken. Serveren avviser oppsett under bunnen. Dersom kapteinen bytter fra en dyp plass til revet med en dyp rigg bevart, avvises utsetting til fiskeren retter oppsettet med G; andre spilleres oppsett endres ikke automatisk. Nappets kvalitetsnivå bruker stedets omtrentlige fiskedybde, sammen med de eksisterende agn-/programparene. Første utras starter med valgt dybde og slipper ut inntil 30 meter ekstra line; aktiv kamp overtar denne linelengden. En dyp rigg begynner dermed ikke kampen med en kortere line enn agndybden. Fangstresultatene og oppgjørsbarrieren ved 17:00 er uendret.

## Verifisering

To nye Unreal-tester dekker katalog, gyldige dybder og nappvurdering, konservativ margin og eksakt ankomst ved fristen for alle tre rutene. De 15 eksisterende testene skal fortsatt bestå.

`Tools/Testing/Run-SeaChart.ps1` kjører to lokale grafiske Windows-prosesser med localhost-port 7777 og isolerte brukerinnstillinger. Den bruker kontrollerens InputKey-rute for kart, rigg, avreise, kamp og retur. Server-RPC-er prøves også direkte med ugyldig ID, gammel revisjon, passasjer og forespørsler under reise, fiske og retur. Begge bruker skrenten, velger 80/90 meter, reiser i målt virkelig tid, får egne tilfeldige napp og lander hver sin fisk før kapteinen starter hjemturen. Prøven sjekker posisjon, reisetid begge veier, marinasonekryssing og uendrede oppsett. Målingen av klientens retur begynner ved første observerte returtilstand, uavhengig av når dens landingsanimasjon avsluttes. Sentrale sider tas som skjermbilder. Ingen produksjonsklokke eller nappetid forkortes. To fysiske PC-er og en ny full 20-minuttersdag inngår ikke i denne prøven.

Claude/Broen-review venter til lørdag. Egen gren, utkast-PR, ingen integrasjon før godkjent review og påkrevde kontroller.
