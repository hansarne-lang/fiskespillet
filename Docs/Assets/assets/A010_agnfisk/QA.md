# A010 Kontroll av agnfisk

Kontrolldato 4. november 2026. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. Spillmodellen har 10 320 trekanter, fire bein, normaliserte vekter og gyldige UV-er. Et pakket 2K-fargekart og de redigerbare kildedelene er bevart.
- Geometrien har ingen nullflater, åpne kanter, løse punkter eller kanter med flere enn to tilstøtende flater. Alle koordinater er gyldige.
- Halen beveger seg til begge sider, mens hodets agnfeste står i ro. Første og siste pose i svømmeløkken stemmer.
- FBX og GLB er importert i tomme Blender-scener. Modellnavn, mål, trekanttall, materialplasser, fire bein og animasjon er bevart. En svømmepose er sammenlignet ved samme relative tidspunkt i begge eksportformatene.
- Bilder i hvile- og svømmepose er inspisert etter finpuss av kroppsprofil, øyne, gjellekant og finnestråler. Kontroll av den justerte geometrien og eksportene er bestått.

Se [validation_report.json](validation_report.json).

## Gjenstår

Unreal-import, materialkontroll i undervannslys, agnfeste på A009-kroken og synlighet fra spillerkamera må prøves. Rolig, pulserende og aktivt agnprogram må styre hele agnets bevegelse og dybde. Svømmeløkken skal tilpasses denne bevegelsen. Eventuell artsbestemmelse, størrelsesvarianter, detaljnivåer og ytelse gjenstår.

Den generelle agnmodellen er et arbeidsgrunnlag og står åpent i assetlisten til integrasjonskravene er oppfylt.
