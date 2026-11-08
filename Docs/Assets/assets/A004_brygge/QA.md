# A004 Kontroll av første brygge

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Status Til kontroll før integrasjon i hovedspillet.**

## Kontrollert i Blender

- Den lagrede `.blend`-filen er gjenåpnet i en ny bakgrunnsøkt. 621 modellkomponenter er bevart som redigerbare objekter.
- Hovedmodellen har 70 844 trekanter og avstandsmodellen 35 422. Begge er kontrollert uten flater med null areal, ugyldige koordinater, løse punkter, åpne kanter eller kanter med flere enn to tilstøtende flater.
- Omregning til spillmodell bevarer UV-koordinater og 11 materialplasser. Trefargens kart bruker sRGB; ruhet og OpenGL-normaler bruker datafarger. Tre sømløse 1K-bilder er pakket i kildefilen.
- Lokale modellmål inkluderer detaljer: X −6,0725 til 6,0725 meter, Y omtrent 0 til 21,0725 meter og Z −2,4 til 2,597 meter. Toppflaten på dekket ligger 1,35 meter over modellens havnivå.
- FBX og GLB er importert i hver sin tomme Blender-scene. Trekanttall og antall materialplasser stemmer. Største målfeil er omtrent 0,000002 meter for FBX og 0 for GLB.
- FBX-importen bevarer to lukkede, konvekse gangkollisjonsbokser og fem navngitte ankerpunkter. Presentasjonsvann, sand og lys inngår ikke i spillmodellene.

Geometrikontrollen førte til enklere, lukkede skruehoder og sammensveising av konverterte tauenders kantpunkter. Endringene er med i de kontrollerte eksportfilene. Se [validation_report.json](validation_report.json) og [build_report.json](build_report.json).

## Visuell vurdering

Oversikt og nærbilde viser bearbeidede plankekanter, langsgående trefiber, stålbeslag, turkise malte detaljer, hengende fendere og ryddige gangflater. T-hodet har plass rundt redningsringen. Gule kantmarkeringer viser ombordstigningsområdet. Blender-bildene er arbeidsvisninger av den faktiske modellen.

Oversikt, nærbilde og [visning i nøytralt lys](A004_brygge_noytralt_lys_v1.jpg) er inspisert. Endelig samsvar med havet, øya og morgenlyset vurderes etter innlegging i Unreal.

## Gjenstår i hovedspillet

- Import og kontroll av materialkoblinger, normalretning og avstandsmodell i Unreal.
- Plassering på A003 med landforbindelse, riktig havdybde og tilpasset stolpelengde. Brygga er foreløpig ikke plassert på øya.
- Kontroll av dekkhøyde og ombordstigning sammen med A005-startbåten, spilleren og de faste kameraene.
- Utprøving av gangkollisjon, båtkollisjon, tilgjengelig vann ved T-formen og eventuell navigasjon.
- Kontroll av detaljnivåoverganger, fender-/taubevegelse dersom det trengs, og ytelse med samtidige båter.

A004 står åpent i assetlisten til disse integrasjonskontrollene er ferdige. Modellen er laget fra medfølgende kilde uten eksterne modeller eller teksturer.
