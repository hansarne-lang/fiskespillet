# A029 Kontroll av marinabygg

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. To bygg med 421 redigerbare deler og fem spillmodeller er bevart.
- Butikk og dør har samlet 47 988 trekanter. Fiskemottak, dør og mottaksluke har samlet 43 896. Geometrien har ingen nullflater, åpne kanter, løse punkter eller kanter med flere enn to tilstøtende flater.
- De tre egne 2K-kartene er pakket i Blender-filen. Ruhet og normaler bruker datafarger. Presentasjonens ressurser fra A004, A026 og A030 er holdt utenfor byggenes eksportfiler.
- Begge dører er prøvd med rotasjon rundt eget hengsel. Dreiepunktet står stille, og døren svinger ut av dørfeltet. Den frie bredden mellom sideomrammingene er 0,91 meter.
- Hvert bygg har sju lukkede, konvekse kollisjonsbokser. Et punkt midt i dørfeltet er kontrollert uten treff i de faste veggvolumene.
- FBX og GLB er importert i tomme Blender-scener. Modellnavn, mål, trekanttall, materialplasser, dreiepunkter og fire ankerpunkter per bygg er bevart. FBX beholder de sju kollisjonsformene per bygg.
- Leveringspunktet er flyttet fram fra disken. Skiltgeometrien inneholder bare faste funksjonsnavn og viser ingen dagsresultater.
- Oversikt, nærbilde av mottaket og nøytral lysvisning er inspisert. Materialkoblingen på presentasjonspalmene er rettet, og rekvisitter ved dører og betjeningsområde er flyttet til ryddigere plasseringer.

Se [validation_report.json](validation_report.json) og [build_report.json](build_report.json).

## Gjenstår

Byggene må plasseres på A003 med riktig gulv-, terreng- og bryggehøyde. Spillerens trinn, dører, disk, kollisjon, kameraføring og navigasjon må prøves samlet. Mottakslukens eventuelle bevegelse, innredning ved behov, glassmateriale, detaljnivåer og ytelse gjenstår.

Interaksjonspunktene må kobles til butikk, levering og serverens oppgjør. Verken mottaksbygget eller rekvisittene skal røpe dagens fangstopplysninger før minutt 17. A029 står åpent i assetlisten til integrasjonskravene er oppfylt.
