# A026 kontroll av første vegetasjonspakke

Kontrollert 4. november 2026 i Unreal Engine 5.8.4, i A003-arbeidsprosjektet.

## Gjennomført

- Tre palmevarianter, en busk og en gresstue er laget som redigerbare 3D-modeller og importert som native Unreal-ressurser.
- Kildegeometrien er kontrollert for gyldige koordinater og normaler, gyldige indeksreferanser og trekanter med areal større enn null. Stammer og kokosnøtter har lukkede flater. Bladkanter er åpne som tilsiktet.
- Høydene på de importerte modellene er lest tilbake og kontrollert mot eksporten.
- Materialplassene for bark, blader og kokosnøtter er kontrollert på alle modeller.
- Alle tre materialene er kompilert uten shaderfeil. Bladmaterialet er tosidig og bruker motorens løvverksmodell med lysgjennomgang.
- Vindparameteren er lest tilbake til 7 centimeter i materialene. Videre vurdering av bevegelsen i faktisk spilling gjenstår.
- Øyscenen har 42 palmer, 45 busker og 90 gresstuer. Rotpunktene står utenfor det reserverte marinaområdet.
- Kollisjon er eksplisitt slått av på detaljene, og dette er kontrollert etter gjenåpning av prosjektet.
- Den lagrede scenen er åpnet i en ny Editor-prosess og kontrollert på nytt. Øya, morgenlyset og detaljnivået lastes sammen fra riktige ressurser.
- Nærbildet av palmen og de nye øybildene er tatt fra den gjenåpnede scenen og inspisert.

Resultater: [geometry_validation.json](geometry_validation.json), [validation_report.json](validation_report.json) og [build_report.json](build_report.json).

## Gjenstående arbeid

- Integrer og vurder vegetasjonen fra hovedspillets faste kameraer.
- Tilpass detaljnivåer og gruppering av gjentatte planter, og mål ytelse på de aktuelle PC-ene.
- Kontroller vind og små blader under bevegelse og ved ulike avstander. Instansering trenger egen kontroll av vindens rotpunkt.
- Avklar og bygg eventuell enkel kollisjon for stammer ved integrasjon.
- Finpuss bark, bladdekning og fargevariasjon sammen med resten av den ferdige marinaen.

A026 står som **Til kontroll**. Første vegetasjonspakke er produsert og kontrollert; den er ikke krysset av som ferdig integrert i spillet.
