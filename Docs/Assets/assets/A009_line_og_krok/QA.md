# A009 Kontroll av line og krok

Kontrolldato 4. november 2026. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Den lagrede Blender-filen er gjenåpnet. Alle 26 linekontroller stemmer med de registrerte posisjonene i tre tilstander.
- Den evaluerte lineoverflaten ligger 0,5 mm fra styringspunktene, som tilsvarer modellens radius. Dette er målt ved snelleutgang, alle ni føringsringer og punktene langs det frie strekket.
- Midtpunktets sig er målt til 2 cm, 35 cm og 0 cm i henholdsvis stram, slakk og belastet tilstand. Krokøyet følger endepunktet i alle tre tilstander.
- Minste radielle klaring mellom linen og føringsringenes åpninger er 2,65 mm ved sentrum. Endelig lineføring og kontakt under fysisk belastning må fortsatt prøves i spillet.
- Krok, svivel og de tre bakte linemodellene har ingen nullflater, åpne kanter, løse punkter eller kanter med flere enn to tilstøtende flater.
- Begge eksportpakkene er prøvd som FBX og GLB i tomme Blender-scener. Modellnavn, mål, trekanttall og materialplasser er bevart. Referansestang og referansesnelle er ikke med i eksportene.
- Bildene av stram line, slakk, bøyd stang og løst metallutstyr er inspisert. Linen går gjennom føringsringene, og krokens øye og innovervendte spiss er synlige i nærbildet.

Se [validation_report.json](validation_report.json) og [build_report.json](build_report.json).

## Gjenstår

Spillets line må oppdateres fra den faktiske stanga, snella og kroken, med sammenhengende slakk, stramming og linelengde. Blender-demonstrasjonen har ingen kraftberegning og eksporterer ikke et ferdig Unreal-linesystem. Kobling til A010, agnfeste, knute, napp, linebrudd, kameraavhengig synlighet, materiale og ytelse med flere stenger må prøves i hovedspillet.

A009 står åpent i assetlisten til disse integrasjonskravene er oppfylt.
