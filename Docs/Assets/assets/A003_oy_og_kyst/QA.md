# A003 kontroll av første utgave

Kontrollert 4. november 2026 i Unreal Engine 5.8.4, i det separate prosjektet `FiskelagetIslandAsset`.

Dette gjelder grunngeometrien i v1. Seneste kontroll av øya med palmer og andre detaljer står i [QA_v2.md](QA_v2.md).

## Gjennomført

- En sammenhengende øy med strand, bukt, høydedrag, klippekyst og undervannssokkel er laget som faktisk 3D-geometri. Tre kyststeinvarianter følger med, og ti steiner er plassert i delnivået.
- Alle fire eksporterte modeller er kontrollert for lukkede flater, konsekvent utvendig flateretning, gyldige normaler, endelige koordinater og trekanter med areal større enn null. Kontrollen bestod.
- Hovedgeometriens mål er lest tilbake i Unreal og kontrollert mot eksporten: 760 × 760 meter inkludert undervannssokkel. Høyeste punkt er omtrent 50 meter over havnivå.
- Den foreslåtte landplassen ved marinaen er kontrollert i kildegeometrien til 2,2 meter over vannflaten. Vannpunktet utenfor ligger under havnivå.
- Retningen er rettet for OBJ-importen og kontrollert visuelt i Unreal. Sandbukta ligger på planlagt side av øya.
- Hovedmaterialet er kompilert uten shaderfeil. Tilordning av materialer til øy og steiner, steinvariantens overstyring og fargeparametere er kontrollert.
- Det er kontrollert én øy, ti plasserte kyststeiner, tre steinvarianter og tre kameraer i den samlede testscenen.
- Kollisjon er eksplisitt satt til `NoCollision` på terreng og steiner; dette er kontrollert i Unreal.
- Tre bilder på 1600 × 900 piksler er tatt direkte i Unreal etter siste geometriendring og inspisert: oversikt, innseiling og marinabukt.
- Repeterende terrengrygger er erstattet med jevnere variasjon. Marinaovergangen og strandlinjen er myket opp før siste kontroll.

Resultater: [geometry_validation.json](geometry_validation.json), [validation_report.json](validation_report.json), [build_report.json](build_report.json) og [capture_report.json](capture_report.json).

## Gjenstår før A003 er ferdig

- Migrer øya til hovedspillet og prøv plasseringen sammen med faktiske båtstørrelser, reisetider og faste kameraer.
- Lag og test kystkollisjon og rutevalg rundt øya og inn i bukta. Den visuelle øya er ikke en ferdig navigasjons- eller kollisjonsløsning.
- Finpuss overflater og klippedetaljer ved nærbilder. Bakkefargen erstatter ikke palmer, busker og annen vegetasjon fra A026.
- Kontroller strand og landgang sammen med A004-brygga. Ankerpunktene er forslag, og kan justeres ved denne prøven.
- Tilpass detaljnivå og mål ytelse ved relevante avstander og flere båter. Det er ikke gjennomført en ytelsesprøve i denne leveransen.

A003 står som **Til kontroll** og er ikke krysset av som ferdig. Geometri, materialer og testscene er produsert og kontrollert; hovedspillintegrasjon gjenstår.
