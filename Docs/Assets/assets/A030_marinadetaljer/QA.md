# A030 Kontroll av marinadetaljer

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. Ti spillmodeller, 144 redigerbare deler og ti navngitte ankerpunkter er bevart.
- Alle ti modeller er kontrollert uten nullflater, åpne kanter, løse punkter eller kanter med flere enn to tilstøtende flater. Alle koordinater er gyldige. De synlige åpningene i kasse, bøtte og redningsring er mellom lukkede overflater.
- Alle ti modeller har dreiepunkt ved lokal origo. De tjue eksportene, én FBX og én GLB per modell, er lest inn i tomme Blender-scener. Modellnavn, mål, trekanttall, materialplasser og ankerpunkter er bevart.
- Forhåndsvisningen er inspisert. Den viser hele objektene, inkludert fenderens feste, kasseåpninger, bøttehåndtak, tvinnet tau og redningsringens lyse felt.
- Pakken har egne modeller med original geometri og materialfarger, uten eksterne modeller eller teksturer.

Se [validation_report.json](validation_report.json).

## Gjenstår

Plassering i brygge og marina, kollisjon, materialkontroll i hovedspillets lys og synlighet fra kamera må prøves. Flyting, svingende fendere, fortøyning, gripefunksjoner og eventuelle lokkanimasjoner må kobles til spillfunksjonene der de trengs. Avstandsmodeller, instansering og ytelse med mange samtidige rekvisitter gjenstår.

A030 står åpent i assetlisten til integrasjonskravene er oppfylt.
