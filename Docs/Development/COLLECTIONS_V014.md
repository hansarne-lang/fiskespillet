# FISKELAGET v0.14 – Fangstjakten

J åpner den nye fangstsentralen både fra hovedmenyen og under vanlig spilling. Lukk andre paneler først. Fangstsentralen bruker bare din egen bekreftede journal; dagens fangster teller først etter oppgjøret 17:00. Spillklokken fortsetter når panelet er åpent.

## Tretti samlinger

- Én samling per art: tre bekreftede landinger av hver av de elleve artene.
- Én per habitat: to landinger fra hvert av de fire habitatene.
- Tre agnsamlinger og tre programsamlinger: tre landinger per agn/program.
- Fem størrelsesmål: minst 3, 10, 25, 50 og 100 kg i én landing.
- Ti landinger, ti solgte fisk, 100 kg levert og alle elleve arter.

Landinger som gikk tapt ved retur teller i art-/habitat-/agn-/størrelsessamlinger, men aldri som solgt fisk eller levert vekt. Samlingene gir ingen penger, utstyrsbonus eller endret fangstsjanse. Fremgangen beregnes fra journalen, slik at omlasting og gjentatt kvittering ikke skaper ekstra fremgang. Fiskeskole og andres fangster teller ikke.

## Dine tre mål

Tab bytter mellom Samlinger, Mine tre mål, Fangstsøk og Oversikt. Piler, PgUp/PgDn, Home og End velger blant alle 30 samlinger. Tast 1, 2 eller 3 fester valget i en av tre plasser og lagrer straks. Et allerede festet mål bytter plass med det andre, så samme mål aldri finnes dobbelt. F5 lagrer på nytt og reparerer en hovedfil som er lest fra backup.

Valgene lagres separat per karriere i Collections-<profil>.ini med backup. Skadde og ukjente fremtidige filer bevares. Tidligere karriere, penger, journal, ruter, stangsett og øvingsresultater endres ikke av fangstsentralen.

## Fangstsøk og oversikt

I søkefanen:

- 1 velger art, 2 habitat, 3 alle/solgt/tapt.
- 4 velger minstevekt: alle, 5, 10, 25 eller 50 kg.
- 5 sorterer etter nyeste, tyngste, fangstverdi eller lengste kamp.
- Home nullstiller filtrene; PgUp/PgDn blar i treffene.

Filtrene kombineres. Oversikt viser landinger, solgt/tapt, landet vekt, største fisk, inntekt fra solgt fangst, arter, habitater, fangstdager og samlet/gjennomsnittlig kamptid. «Fangstverdi» ved sortering er fiskens katalogverdi; tapt fangst gir fortsatt null salgsinntekt.

## Egne rapporter

F10 lager både en HTML-rapport og CSV i Fiskelaget/Exports. Fra søkefanen brukes de gjeldende filtrene; fra de andre fanene eksporteres hele den bekreftede journalen. Rapportene får nye navn hver gang, og gamle rapporter beholdes. HTML viser også de tre festede målene. Ingen nettside, innlogging eller deling er nødvendig.

## Alt fra tidligere pakker

Mangrovebanken og de fire nye artene fra v0.13 følger med. M åpner sjøkart, A i kartet viser 12 atlasmerker, Q viser stangsett og fiskeassistent, O turplan, Y fiskedagbok, L sjøpass og F9/F12 fotomodus/bilde. F6 fra hovedmenyen åpner fiskeskolen med elleve arter. E går om bord; lukk kartet og trykk R ved 02:00 for avreise. Døgnet varer fortsatt 20 minutter.

Begge LAN-PC-er må bruke v0.14.0 (protokoll 14). GitHub synkroniserer kildekode, ikke personlige lagringer. Eldre ZIP-pakker beholdes separat. Fysisk LAN mellom to PC-er må testes på de aktuelle maskinene.
