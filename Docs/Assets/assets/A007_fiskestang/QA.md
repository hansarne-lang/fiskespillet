# A007 Kontroll av fiskestang

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. Spillmodellen har 46 060 trekanter, åtte bein og normaliserte vekter. De 85 kildedelene er bevart.
- Geometrien har ingen nullflater, åpne kanter, løse punkter eller kanter med mer enn to tilstøtende flater. Alle koordinater er gyldige.
- Grepenes største bevegelsesavvik i den store belastningsposen er 0 meter. Tuppens posisjon endres fra omtrent Y 0,006, Z 1,900 til Y 0,426, Z 1,750 meter.
- Ni føringsankre og fire monterings-/håndankre stemmer med kildeposisjonene i hvile. Føringsankrene er underordnet riggen.
- Målt diameter på nedre grep er 34,4 mm mot holderens 54 mm. Monteringshøyde og kontakt med bunnpinnen gjenstår som en egen sammensettingskontroll.
- FBX og GLB er importert i tomme Blender-scener. Trekanttall, materialplasser, åtte bein, animasjon og ankerpunkter er bevart. Den store belastningsposen er sammenlignet ved samme relative tidspunkt i animasjonen, også når importscenen har en annen bildefrekvens.
- Det pakkede grafittkartet er bevart i kildefilen. Snelle og presentasjonsmiljø er utelatt fra stangeksporten.
- Oversikt, nærbilde og belastningspose er inspisert. Den bøyde stanga og føringsringene følger riggen, mens grep og visningssnelle står i ro.

Se [validation_report.json](validation_report.json) for konkrete avvik etter eksport og [capture_report.json](capture_report.json) for visningsfilene.

## Gjenstår

Unreal-import, endelig materiale og kontroll fra spillerkamera må gjøres i hovedspillet. Bøyen må kobles til linebelastning og pumping. Håndplassering, snellefeste, faktisk innstikksdybde i A006 og lineføring gjennom bøyde ringer skal prøves med A008, A009 og spillerfiguren. Detaljnivåer, materialkostnad og ytelse med flere stenger er ikke ferdigprøvd.

Belastningshandlingen er en visuell demonstrasjon uten ferdige stivhets- eller styrkeverdier. A007 står åpent i assetlisten til integrasjonskravene er oppfylt.
