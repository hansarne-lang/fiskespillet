# FISKELAGET v0.10 – Havutforskning og sjøpass

Bestilt av prosjekteieren som neste store pakke, [oppgave86](https://github.com/hansarne-lang/fiskespillet/issues/86). Dette dokumentet beskriver funksjoner og grenser. Faktiske bygg- og spillresultater følger Windows-leveransens TESTRESULTATER.json; kildekode alene er ikke et bestått spillbevis.

## Hav-atlas og flere stopp

Åpne M og trykk A for hav-atlaset. Ni navngitte merker viser plass, habitat, bunndybde og omtrentlige hjemreisetider: Korallrevet, Undervannsskrenten, Åpent hav, Lagunen, Pelikanodden, Blårenna, Fregattpasset, Vestbanken og Marlinryggen. 1–9 eller piler markerer; Enter eller klikk velger. A går tilbake til vanlig kart, Tab åpner lagrede favoritter. Vanlige frie sjøpunkter beholdes.

Ved brygga velger kapteinen dagens første plass som før. Fra 02:00 starter R turen. Ved en fiskeplass kan kapteinen velge et nytt sjøpunkt i atlaset eller favorittene. Da starter en fysisk ny etappe, med egen reisetid og øyunngåelse. Alle stenger om bord må være trukket inn eller ferdige; passasjerer, gamle ruterevisjoner, landpunkter og valg etter17:00 avvises. En fangst beholder stedet der den faktisk ble landet. Ekspedisjonsmålet låses fortsatt ved dagens første avreise.

R starter retur også underveis på en utreise eller flytting. Båten vender fra sin aktuelle posisjon; den flyttes ikke til reisens mål først. Returen bruker den beregnede farbare ruten og registrerer når båtens midtpunkt krysser marinasirkelen. Grensen ved17:00 er uendret. Større avstand bruker mer av den samme fisketiden. Reisefart og habitatets eksisterende fangstfordeling beholdes; kartmerkene garanterer ikke bestemte arter.

## Enstemmig reisevisning

V under en etappe markerer ønske om en enkel reisevisning. Alle om bord må ha valgt den før havbildet dekkes av et reisekort med fremdrift. En spiller alene bestemmer selv. V igjen trekker egen stemme og viser sjøen. Ny etappe, nytt mannskap eller nytt døgn nullstiller stemmene. Kart og mannskapspanel kan fortsatt åpnes. R er fortsatt kapteinens kommando.

Dette er et valg av visning. Serverklokken, båtens fysiske bevegelse, ankomsttid og tilgang til fisking endres ikke. Spillere i land og andre observatører ser fortsatt den virkelige båten.

## Personlig sjøpass

L åpner sjøpasset med tolv milepæler: første fangst, tre arter, sju arter, ti solgte fisk,100 kg levert, tre habitater, tre agntyper, fem atlasmerker, fem offshorefangster, fullt oppgradert utstyr, fem fullførte ekspedisjoner og ekspedisjon i tre værtyper.

Fremgang beregnes fra egen allerede bekreftet journal og medaljehistorikk. Ingen ny pengebonus, egen belønningssaldo eller framdriftsfil innføres. Gamle poster uten sjøkoordinater gir ikke oppdiktede atlasbesøk. Tapte fangster øker ikke antall solgt eller levert vekt. Dagens nye framgang skjules til oppgjøret ved17:00. L, F4 eller Escape lukker panelet.

## Eksport og miljø

F10 i journalen eller sjøpasset lager en ny CSV av hele egen bekreftede journal under Saved/Config/Fiskelaget/Exports. CSV-en bruker UTF-8 og semikolon, med egne fangst-/dags-ID-er, dato, art, mål, solgtstatus, salgsverdi, rigg, kamp og eventuelle sjøkoordinater. Den inkluderer tidligere bekreftede poster mens man fisker, og dagens poster først etter oppgjøret. Eksport endrer ingen lagrede data og overskriver ikke gamle eksportfiler. Filen kan åpnes i Excel.

Rutemiljøet får bøyer ved atlasmerkene, et fyr, kjølvann og kosmetiske bakgrunnsbåter. Bakgrunnsbåtene fisker ikke, kolliderer ikke og påvirker ingen sannsynlighet. De bruker dagens felles tid til visningen. Lydkøen har større buffer for å tåle kortvarig last. Eksisterende lydvalg og30/60 FPS virker fortsatt; ingen betalte eller hentede tredjepartsressurser inngår.

## Kompatibilitet og levering

Prosjektversjon0.10.0 og nettverksfamilie10. Alle i samme økt må bruke v0.10. Eldre lagrede journaler, karrierer, medaljer, naust, oppsett og favoritter bevares; lagringsformatene endres ikke av sjøpasset. Ta kopi av Saved/Config/Fiskelaget før oppgradering, inkludert Authority hvis du flytter verten. GitHub synkroniserer kildekode, ikke automatisk lagrede spill. Bruk samme profil på én aktiv spiller om gangen.

Windows ZIP/BAT, denne guiden, testmetadata, kilde-SHA og sjekksummer følger leveransen. Fysisk to-PC-LAN og ytelse på den andre PC-en føres separat når de faktisk er prøvd. Enkle kodebygde modeller, én aktiv båt og fire LAN-spillere beholdes. Veddemål, sleping, flåte, nye priser og tidsbonuser inngår ikke.
