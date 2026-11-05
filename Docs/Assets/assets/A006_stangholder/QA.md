# A006 Kontroll av stangholder

Kontrolldato 4. november 2026. **Til kontroll før integrasjon i hovedspillet.**

## Kontrollert

- Lagret Blender-fil er gjenåpnet. Seks komponenter er bevart: rør, flens, tre skruehoder og en tverrpinne.
- Spillmodellen har 768 trekanter, ett materiale og stanganker. Ingen flater har null areal, ingen kanter er åpne, og ingen kanter har flere enn to tilstøtende flater. Koordinatene er endelige tall.
- Målt utstrekning er 132 × 132 × 305 mm. Røret er hult, og støtten ligger under den øvre åpningen.
- FBX og GLB er importert i tomme Blender-scener. Trekanttall, mål, materialplass og stanganker er bevart. Største målfeil i FBX er under 0,000001 meter; GLB avviker ikke i målingen.
- Materialet er gjenbrukt fra prosjektets egen A005-modell. Ingen eksterne modeller eller teksturer inngår.
- Blender-forhåndsvisningen er inspisert; røråpning, flens og innfestinger er synlige. Tverrpinnen ligger lenger nede i røret og inngår i geometri- og eksportkontrollen.

Se [validation_report.json](validation_report.json).

## Gjenstår

Den selvstendige modellen må erstatte A005s innebygde rør dersom den tas i bruk. Montering, nødvendig utsparing i båtrekka og ankerretning kontrolleres i Unreal. Stanghåndtak og innstikkslengde må prøves mot A007, sammen med lineføring, belastningsbøy og de faste kameraene. Materialets glans og ytelse vurderes i hovedspillets morgenlys.

Et fungerende statisk rør bekrefter ikke spillfunksjonene for automatisk fiske eller aktiv stangkapasitet. A006 blir derfor stående åpent i assetlisten.
