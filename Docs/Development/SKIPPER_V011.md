# FISKELAGET v0.11 – Skipperpakken

Bestilt som neste store pakke i [oppgave 88](https://github.com/hansarne-lang/fiskespillet/issues/88). Faktiske tester og kildeversjon følger Windows-pakkens TESTRESULTATER.json.

## Felles turplan

O åpner skipperens turplan. Alle om bord ser samme plan med opptil fem sjøpunkter. Piler velger et av de ni atlasmerkene, A legger til, Backspace fjerner siste stopp og Delete tømmer. Kartet viser farbare etapper rundt øyene og samlet avstand inkludert hjemreise. Kapteinen redigerer ved brygga. Duplikater, landpunkter, gamle dagskommandoer og gamle revisjoner avvises.

Enter velger neste stopp. Ved brygga velges første destinasjon; lukk O og trykk R fra 02:00 for avreise. Ved en fiskeplass starter Enter neste etappe bare når alle stenger om bord er trukket inn eller ferdige. X trekker inn valgt stang. Ankomst markerer et stopp som besøkt. Båten går aldri automatisk videre. En avbrutt etappe markeres ikke som besøkt. R vender hjem fra faktisk posisjon, slik som i v0.10. Klokken, reisefart og 17:00-frist beholdes. Planens stopp beholdes ved neste døgn, mens fremdriften nullstilles.

Det vanlige sjøkartet M og atlaset M + A kan fortsatt brukes. Velger du en annen plass manuelt, beholder turplanen neste ubesøkte stopp. Kapteinsbytte overfører også retten til å bruke fellesplanen. Passasjerer kan se planen og lagre egen kopi, men ikke endre eller starte den.

## Tre egne ruteoppsett

I O-panelet velger 1/2/3 ditt oppsett. F5 lagrer en kopi av den felles planen. F2 åpner navn; bokstaver, tall, mellomrom og Backspace redigerer, Enter lagrer, Escape avbryter. F8 sender valgt eget oppsett til kapteinens serverkontroll. Det blir en ny felles plan bare hvis serveren godtar den ved brygga.

Egne ruter lagres profilbundet i Saved/Config/Fiskelaget/Routes-<profil-ID>.ini. Siste gyldige generasjon beholdes som .bak. En skadet hovedfil kan leses fra backup; F5 er et uttrykkelig nytt lagringsforsøk. Hvis begge filer er skadet, profilen er feil eller formatet er nyere enn støttet, bevares filene og lagring blokkeres. Gamle karrierer uten rutefil får tre tomme oppsett. Ingen gamle brukerfiler endres av å åpne panelet.

## Fotomodus

Gå om bord og trykk F9 med andre paneler lukket. Piler justerer vinkel, PgUp/PgDn avstand og musehjul bildeutsnitt. Home nullstiller. H skjuler eller viser fototeksten. F12 ber om et nytt PNG-bilde under Saved/Config/Fiskelaget/Photos; skjul teksten med H først hvis du ønsker et rent bilde. F9 eller Escape går tilbake til den vanlige kameravinkelen.

Kameraet tilhører bare din lokale spiller. Det flytter ikke båten eller andre kameraer. **Klokken og fiskingen fortsetter.** Lukk fotomodus for å håndtere stang, kart eller retur. Modusen avslører ingen skjult fangststatistikk og åpner ingen andre spilleres journal.

## Fiskedagbok og turrapport

Y åpner din varige fiskedagbok. Turfanen grupperer bekreftede fangster etter unik dags-ID, også når to ulike økter begge har en «dag 1». Piler velger tur, PgUp/PgDn blar i fangster. Kartet viser bare registrerte fangststeder. Gamle poster uten koordinater får ingen oppdiktede kartpunkter. Dager uten journalførte fangster vises ikke som fangstturer.

Tab bytter mellom turer, arter, habitater og agn. Oppsummeringen viser landet antall/vekt, solgt og tapt ved retur, inntekt, største fisk og registrert kamptid. Landet vekt inkluderer fisk som gikk tapt ved sen retur; inntekt gjelder bare faktisk solgt fisk. Dagens nye poster vises først etter det vanlige oppgjøret 17:00.

F10 eksporterer den valgte turen til en ny HTML-rapport i Saved/Config/Fiskelaget/Exports. Rapporten kan åpnes i nettleser eller skrives ut. Den inneholder bare din valgte bekreftede fangsttur og bruker ingen eksterne skript eller nettressurser. Eksport overskriver ikke tidligere rapporter. Den eksisterende CSV-eksporten i F4/L beholdes.

## Kompatibilitet

Spillversjon 0.11.0 og nettverksfamilie 11. Alle spillere i samme økt må bruke v0.11. Eldre klientprogrammer avvises; eldre journaler, karrierer, naust, medaljer, favoritter og stangoppsett beholdes. Ta kopi av hele Saved/Config/Fiskelaget før flytting eller oppgradering, inkludert Authority når verten flyttes. GitHub synkroniserer kildekode, ikke personlige lagringer. Bruk samme profil på én aktiv spiller om gangen.

Pakken beholder sju arter, eksisterende priser, fire LAN-spillere og én aktiv båt. Ingen pengebonus, tidsbonus, kjøpte ressurser, flåte eller veddemål legges til. Fysisk LAN mellom to ulike PC-er og ytelse på den andre PC-en registreres først når det faktisk er prøvd. Norsk guide, startfil, manifest og faktiske testresultater følger Windows ZIP-en.
