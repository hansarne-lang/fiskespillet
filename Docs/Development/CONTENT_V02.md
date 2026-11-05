# v0.2 – mer fiskeinnhold og bedre spillflyt

Oppgave #61 bygger på enspillerversjonen i PR #60. Prosjekteieren har valgt denne pakken og godkjent manuell issue-tildeling mens privat Project er utilgjengelig. Review og tester før integrasjon beholdes.

## Fangstinnhold og hemmelighold

Korallrevet har snapper som hovedart og amberjack som alternativ; undervannsskrenten har amberjack og snapper, og åpent hav gulfinnet tunfisk og amberjack. Foreløpig fordeling er 80/20 prosent. Fiskeplassen antyder dermed en art, men bekrefter den ikke. Serveren genererer data én gang per utsetting fra en egen privat GUID som aldri replikeres; den synlige utsettings-ID-en brukes ikke som frø for fangstdata. Sekvensen er også adskilt fra nappberegningen. Art, vekt, lengde og verdi er et privat C++-medlem på serverens stang og inngår ikke i replikerte stang-/riggdata. En avbrutt utsetting gir ingen fangst. Landingen bruker samme data og unik fangst-ID; gjentakelser kan ikke erstatte fisk eller eier.

| Art | Foreløpig vekt | Foreløpig lengde | Testpris per kg |
|---|---|---|---|
| Snapper | 2–20 kg | 30–90 cm | 5 fiskepenger |
| Amberjack | 10–50 kg | 60–160 cm | 8 fiskepenger |
| Gulfinnet tunfisk | 30–120 kg | 110–220 cm | 12 fiskepenger |

Dette er spilltestparametre, ikke biologiske måledata eller ferdig økonomibalanse. Lengden interpoleres lineært etter vekten; salgssummen rundes til nærmeste hele fiskepenge. Vekt trekkes i 100-grams trinn. Mellomregning er int64, og opptil 10 000 gyldige fangster per økt holder summer innenfor eksisterende int32-prototypefelt. Det generiske fiskeutseendet og kampmodellen beholdes; bekreftet art og eksakte mål røpes ikke visuelt før oppgjør.

Ved 17:00 avgjør eksisterende marinasonekryssing om fangstene leveres eller tapes. Hele resultatlisten replikeres først da. F2 viser en låst forklaring før avsløringen og deretter fangstdetaljer, levert/tapt, levert vekt og samlet øktsaldo. Oppgjøret åpnes automatisk når andre paneler er lukket. Piler/PgUp/PgDn blar i lange lister. Sen retur gir null salgsinntekt, men viser de landede fiskene som tapt i oppgjøret. Ingen permanent karriere, premier eller butikk er lagt til. Den gamle Record-overloaden brukes fortsatt som minimums-snapper-fixture (2 kg, 10 fiskepenger) i eldre modelltester; runtime sender alltid genererte data.

## Lagrede oppsett

G åpner stangoppsettet. 1/2/3 velger plass, F5 lagrer gjeldende serverbekreftede valg, og F9 sender et lagret oppsett til vanlig servervalidering. Mens en UI-endring venter på bekreftelse, sperres lagring og flere endringer. For dypt oppsett avvises på grunnere steder. Lagrede revisjonsnumre brukes ikke i en ny økt.

I v0.2 ble FiskelagetRigPresets.ini i pakket Development-bygg skrevet relativt til arbeidsmappen fordi motorens brukerinnstillingssti var tom. v0.3 bruker en fast brukersti og kan lese eldre oppsett som fallback; se SESSION_V03.md. Filen skrives via en midlertidig fil før erstatning; mislykket lagring endrer ikke det bekreftede minneoppsettet. Manglende, ufullstendige, ugyldige eller fremtidige skjemaer lastes som tomme plasser. Lagrede agnvalg gir ingen fangstresultater eller eierskapsrettigheter.

## Hjelp og tilbakemelding

F1 viser kontroller. F2 viser oppgjør. Escape lukker disse panelene. De sperrer lokale spillkontroller, men pauser verken klokke eller pågående kamp. Åpning slipper stang-/sveiveinput. Avreise-R gir nå forklaring ved manglende ombordstigning, feil kaptein, ventetid til 02:00, oppdatert båtstatus, pågående transport eller avsluttet tur. Åpen meny/kart ber spilleren lukke panelet før R.

## Verifisering

Nye Unreal-tester dekker katalog/determinisme, artsgrenser og validering, detaljert oppgjør/duplikater/17:00, sen retur og lagring/lasting/ødelagte oppsett. `Tools/Testing/Run-ContentRelease.ps1` lagrer og henter et oppsett i separate spillprosesser, og kjører deretter to grafiske spillere gjennom en normal 20-minuttersdag: hjelp, låste resultater, avreisemelding, valgt undervannsskrent, ulike dybder, uavhengige napp/kamper/landinger, retur og felles avsløring ved 17:00. Oppgjøret kontrolleres hos begge spillere mot faktiske fangstdata, og tidspunkt måles mot vertens logg. Ingen klokkeakselerasjon brukes. Prøven krever skjermbilder av alle sentrale faser. Faktiske resultater registreres i issue og PR på testet kilde-SHA.

Eksisterende solo- og LAN-flyt beholdes. Test på to fysiske PC-er gjenstår hvis bare localhost er tilgjengelig. Kjent timingfeil i kapteinsbyttets gamle smoke-test er utenfor denne pakken og må fortsatt oppgis.
