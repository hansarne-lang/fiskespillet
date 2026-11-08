# v0.8 – Marina og artsjakt

Utvikling i [#74](https://github.com/hansarne-lang/fiskespillet/issues/74), fra v0.7 /8a85cdf. Prosjekteieren har bedt om en ny stor pakke og opphevet de tidligere AGENTS.md-instruksjonene i aktiv samtale. Denne pakken beholdes som lokal Windows-testleveranse med egen gren og PR.

## Spillinnhold

Sju arter med stabile ID-er: snapper0, amberjack1, gulfinnet tunfisk2, grouper3, mahi-mahi4, wahoo5, blå marlin6. De tre gamle artenes størrelse/prisregler beholdes slik at gamle journaler validerer. Nye foreløpige intervaller/priser: grouper4–40kg/$9 perkg, mahi3–30kg/$10, wahoo8–70kg/$15, marlin60–250kg/$25. Art bestemmes av serverens private tilfeldighet og habitat. Revfordeling i ID-rekkefølge:50/10/2/28/8/2/0%; skrent12/40/10/15/12/10/1%; åpent hav3/15/42/2/12/21/5%. Ingen rigg garanterer art. Agn/dybde/program påvirker fortsatt napp gjennom eksisterende habitatmodell.

Nye kampmønstre bruker ulike utrasfarter/-lengder, pumping og skjult utholdenhet. Ingen ny fangstlager-/slepebegrensning. Artsboken på I viser generelle tips, utvalg og tidligere bekreftede oppdagelser/rekorder. F4/F6 og artsfilteret støtter alle sju arter med sider. Bekreftet fangstidentitet, mål og verdi skjules fortsatt til17:00.

## Naust og strandbar

H viser eget naust og lar spilleren besøke opptil fire lokale naust med1–4. Fast kamera, ingen fri gange. Egen bygning har navn/skilting, seks farger, tre tak-/verandastiler, tre dekorvalg og en trofévegg. Alle54 kosmetiske kombinasjoner er gratis i v0.8; ingen ny økonomipost eller endring av v0.7-kjøp. Piler/klikk/Enter redigerer og lagrer. Tekstfelt eier bokstav-/talltastene, også H/B/I/P/R. Spillernavnet vises i mannskapslisten og strandbaren.

B viser strandbaren med eget fast kamera, marinamodell, mannskapsstatus, dagens tavle først17:00 og fem forhåndsdefinerte LAN-meldinger. Serveren kontrollerer innsender, bryggeposisjon og to sekunders pause; meldingshistorikken har maksimalt12 poster. Ingen veddemål eller andre pengehandlinger. H/B stenges hvis båten reiser. Artsboken kan brukes på sjøen; menyer stopper aldri klokke/kamp.

Dagslyset følger den eksisterende serverklokken fra varmt morgenlys via lys dag til varm kveld. Ingen endring i20minuttersdag,17:00-frist, reisetid eller kapteinsrettigheter.

## Data og nettverk

`FFiskMarinaIdentity`, trofeer, innbyggere og meldinger er små delte USTRUCT-er i FiskCore. World bygger lokale 3D-detaljer fra repliserte naustdata; controlleren integrerer validerte RPC-er med karriere, båt, journal og UI. Ett UE-prosjekt, samme to runtime-moduler.

Kosmetikk lagres separat i `FiskelagetMarina-<karriere-id>.ini` med profilbinding, revisjon, atomisk midlertidig filbytte og backup. To ugyldige filer, feil profil eller framtidig format blokkerer overskriving. Skadet hovedfil kan leses fra validert backup. Penger/utstyr/kvitteringskjede og eksisterende journalformater endres ikke; ingen ny startbonus ved oppgradering av en kopiert v0.7-karriere.

Serveren binder navne-/naustendringer til eierforbindelsens karriere-ID og marina, validerer tekstlengder/palett/stil/dekor og repliserer høyst fire naust. Besøkende har ingen redigeringskommando for andre eiere. Frakoblede naust beholdes til plassen trengs av en ny profil.

Troféveggen materialiserer personlig bekreftet journal, én største fisk per art. Serveren validerer arts-/målgrenser, maksimalt sju unike arter og avviser dagens dags-ID før17:00. Historiske, lokalt importerte rekorder er kosmetiske, ikke et kryptografisk anti-juks-bevis, og kan aldri betale penger. Oppgjør, saldo og offentlig dagstavle kommer fra eksisterende autoritativt oppgjør. Manglende historiske journalposter fra frakoblet oppgjør etterfylles fortsatt ikke automatisk.

Nettverksprosjektversjon0.8.0 og spillprotokoll8 settes før tilkobling; andre versjoner må avvises. Bruk samme pakke på alle PC-er. Ta backup og kopier hele Saved/Config/Fiskelaget fra forrige pakke ved oppgradering. Åpne ikke nye artsjournaler i eldre spillversjoner. Det er ingen automatisk PC-synkronisering.

## Faktiske kontroller

Modell-/lagringstester dekker alle sju arter, deterministisk fordeling, alle utstyrsbonuser/kamper, kosmetiske felt, omstart, profilbinding, backup og bevaring av skadede/framtidige filer. Grafiske prøver bruker reelle input, butikk/naustfiler, meldinger, besøk og avvist fremmed redigering/dagens trofé. Full normaldag kontrollerer før/etter17:00 og eksisterende firemannsflyt. Resultater publiseres først etter faktisk kjøring og følger leveransen. To fysiske PC-er må skilles fra flere lokale spillprosesser.
