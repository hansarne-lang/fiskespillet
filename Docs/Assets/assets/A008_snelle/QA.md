# A008 Kontroll av snelle

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. Fem spilldeler og 35 redigerbare kildedeler er bevart. Spilldelene har totalt 7 428 trekanter.
- Alle spilldeler er kontrollert uten nullflater, åpne kanter, løse punkter eller kanter med flere enn to tilstøtende flater. Alle koordinater er gyldige.
- Spole, sveiv, grep og bremsespak er prøvd med sine egne Blender-handlinger. Aksene er lokale X-akser. Kraftgrepet er underordnet sveiva.
- Seks ankre stemmer med kildeposisjonene. Det pakkede 1K-kartet for spolens linepakke er bevart.
- FBX og GLB er importert i hver sin tomme Blender-scene. Fem deler, mål, trekanttall, materialplasser, dreiepunkter, ankre og sveiv-/grephierarki er bevart.
- A007-kilden inneholder en visningskopi ved stangas snellefeste. Denne kopien er ikke med i stangeksporten.
- Eget visningsbilde og montering på A007 er inspisert. Bildet av hele snella inkluderer kraftgrep og monteringsfot.

Se [validation_report.json](validation_report.json). Visningsbildene viser modellen i Blender; endelig materiale vurderes også i spillets morgenlys.

## Gjenstår

Unreal-import, håndgrep, kameraklaring og plassering må prøves sammen med stanga og figuren. Rotasjonshastigheter, utveksling, brems, line ute og utras skal styres av den faktiske fiskesituasjonen. Lyder følger A018. A009 må knytte en bevegelig line til spole/utgang og føringsringer. Detaljnivåer og ytelse med flere samtidige sneller må kontrolleres i hovedspillet.

De statiske eksportdelene og Blender-demonstrasjonene bekrefter mekaniske bevegelsesmuligheter, mens endelig betjening og fiskelogikk gjenstår. A008 står åpent i assetlisten.
