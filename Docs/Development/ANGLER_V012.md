# FISKELAGET v0.12 – Fiskerpakken

Bestilt som neste store pakke i [oppgave 90](https://github.com/hansarne-lang/fiskespillet/issues/90). Faktiske tester, kildeversjon og begrensninger følger Windows-pakkens TESTRESULTATER.json.

## Fiskeskole fra hovedmenyen

Trykk F6 eller klikk kortet «Fiskeskolen» på hovedmenyen. Skolen er lokal og kan bare åpnes før en fiskedag er startet. Den gir ingen penger, fangster, medaljer, utstyr eller karriererekorder.

Kontrolløvelsen lar deg utføre løft, senking, sveiving, stopp, brems og landing i rekkefølge. Tab bytter til treningskamp. Piler velger en av de sju eksisterende artene, G veksler mellom standard og virtuelt oppgradert øvingsutstyr, og Enter starter eller starter på nytt. Begge utstyrsvariantene er gratis øving; de endrer ikke eid utstyr.

Treningskampen bruker den samme FFiskFightModel som vanlig fiske, med vanlig tidssteg, artsprofiler, utras, pumpekreditt, brems, slakk og linebrudd. Venstre mus løfter, slipp senker; høyre mus sveiver, musehjul justerer brems. Slipp sveiva og senk under utras. Mellomrom lander når modellen sier klar. Skolen viser veiledning og registrert tid, ugunstig belastning og sveiving under utras. Den lover ikke at en bestemt teknikk alltid vil lykkes.

Fullførte kamper lagres som egne øvingsresultater for art og virtuell utstyrsvariant. Forsøk og beste fullførte tid holdes adskilt fra fangstjournalen. En fullføring kan ikke telles to ganger eller gjenbrukes som et nytt forsøk. F6 eller Escape avslutter; en pågående øving avbrytes. Ingen fiskedag eller fiskedagsklokke startes av skolen.

## Fiskeassistent og arbeidsbenk

Q åpner arbeidsbenken i en vanlig spilløkt. Piler velger målart og viser offentlig kataloginformasjon om habitat og fiskekamp. Målarten leser ingen fisk på kroken og påvirker ikke fangsttrekningen.

G velger agn i rådet. Dybdeforslaget ligger i fiskesjiktet for båtens nåværende habitat, og agnprogrammet passer valgt agn etter den eksisterende kvalitetsregelen. Det gir «god tilpasning» i riggmodellen, ingen garanti for napp eller en bestemt art. Agnvalget endrer ikke artsfordelingen i denne prototypen. Skal du til et annet habitat, velger kapteinen fortsatt plass i M/O som før.

1/2/3 velger en egen stang. Enter sender rådet til vanlig serverkontroll for valgt stang. F4 sender rådet som et samlet sett til alle dine egne stenger. Det overtar ikke kapteinens rolle og setter ikke ut stenger automatisk. Q/Escape lukker.

## Tre komplette stangsett

I Q-panelet veksler Tab mellom tre egne stangsett. F5 lagrer de nåværende riggoppsettene til alle egne stenger, opptil tre. F2 redigerer navn; Enter lagrer og Escape avbryter. F8 sender hele valgt sett til serveren.

Et sett må ha samme antall oppsett som dine nåværende egne holdere. Et sett laget for én stang anvendes ikke delvis på en båt med tre egne holdere. Serveren kontrollerer eier, dag, hvert kast, hver riggrevisjon, habitatdybde, tilgjengelighet og alle berørte stengers status før noen rigg endres. Et avvist sett endrer ingen stenger. Anvendelse krever brygge eller fiskeplass, før17:00, og alle egne stenger trygge. En enkelt stang kan fortsatt konfigureres med det vanlige G-panelet.

## Egne nappvarsler

Eierens HUD får korte meldinger når en egen stang får napp/første utras, blir klar til landing eller mister fisken. Meldingen viser stangnummer og valgknapp, ikke skjult art, vekt, verdi eller andres fangst. Varslene følger eksisterende eierreplikerte stangtilstander og gir ingen ny informasjon om dagens oppgjør.

## Lagring og kompatibilitet

Stangsett og øvingsresultater lagres profilbundet i Saved/Config/Fiskelaget/Angler-<profil-ID>.ini, med egen .bak og filgrense32 KiB. Tre stangsett og fjorten øvingsrader støttes. Backup kan brukes når hovedfilen er skadet; en uttrykkelig lagring reparerer. Fremmed profil, to skadede filer, for stor fil eller nyere format bevares og blokkerer lagring. Trening kan fortsatt brukes uten lagringsbekreftelse. Ingen gamle journal-, karriere-, naust-, rute- eller riggfiler endres av det nye lagringsformatet.

Spillversjon0.12.0 og nettverksfamilie12. Alle i samme økt må bruke v0.12. Ta kopi av hele Saved/Config/Fiskelaget før flytting/oppgradering, inkludert Authority når verten flyttes. GitHub flytter ikke personlige lagringer. Bruk samme profil på én aktiv spiller om gangen.

Pakken beholder sju arter, eksisterende priser, én aktiv båt, fire LAN-spillere, vanlig20-minuttersdag og returfrist17:00. Ingen ny pengebonus, tidsbonus, innkjøpte ressurser, flåte eller veddemål. Fysisk to-PC-LAN og ytelse på den andre maskinen registreres bare ved faktisk prøving.
