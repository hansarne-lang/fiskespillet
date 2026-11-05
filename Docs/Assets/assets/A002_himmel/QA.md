# A002 kontroll av første utgave

Kontrollert 4. november 2026 i Unreal Engine 5.8.4, i det separate prosjektet `FiskelagetSkyAsset`.

## Gjennomført

- Et eget lysdelnivå er lagret med én sol, én atmosfære, ett himmellys, ett skylag, én svak horisontdis og én global eksponering.
- En separat testscene kombinerer lysdelnivået med kopier av A001-vannet og egne fargeprøver. To kameraer viser horisont og belysning av overflater.
- Solretning, høyde på 32 grader, lysstyrke på 65 000 lux, temperatur på 6000 kelvin og lyskildens vinkel er lest tilbake og kontrollert mot preset-filen.
- Atmosfærens spredning og himmelfarge, himmellysets styrke, skyenes høyde, tetthet og dekning samt disinnstillingene er kontrollert.
- Eksponeringen er kontrollert som manuell og fast. Det er ingen aktiv eksponeringsjustering ved kamerabytte. Linseglimt er slått av.
- To bilder på 1600 × 900 piksler er tatt direkte i Unreal etter de siste lysendringene og inspisert.
- Den visuelle prøven viser blå himmel med lyse skyer, turkis vann og tydelige forskjeller mellom grått, turkis, grønt, gult, korall og elfenben. Fargenes skyggesider beholder detaljer.
- Himmellyset er oppdatert med et nytt himmelopptak etter justeringene, slik at tidligere varme refleksjoner ikke blir liggende igjen.

Maskinlesbar kontroll: [validation_report.json](validation_report.json). Bygginformasjon: [build_report.json](build_report.json). Bildeinformasjon: [capture_report.json](capture_report.json).

## Gjenstår før A002 er ferdig

- Migrer lysdelnivået til hovedspillet og samordne det med scenens eksisterende lys og eksponering.
- Kontroller uttrykket med ferdig båt, fisker, naust og vegetasjon fra de faste kameraene. Testscenens fargeprøver erstatter ikke denne vurderingen.
- Kontroller at glans og solglitter lar fiskeline, fisk og kampvarsler være lesbare.
- Mål kostnaden for volumetriske skyer og skygger på de aktuelle PC-ene, også sammen med flere båter. Dette er ikke målt i denne leveransen.
- Finpuss skydekning og solretning i den ferdige spillscenen.

A002 står som **Til kontroll** og er ikke krysset av som ferdig. Første utgave er produsert og kontrollert i testprosjektet; integrasjon og kontroll i hovedspillet gjenstår.
