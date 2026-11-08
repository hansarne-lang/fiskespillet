# A001 kontroll av første utgave

Kontrollert 4. november 2026 i Unreal Engine 5.8.4, i det separate arbeidsprosjektet som følger assetet.

## Gjennomført

- Hovedmaterialet er opprettet som en faktisk Unreal-ressurs og kompilert. Siste kontroll returnerte ingen shaderfeil.
- Alle tre materialvariantene er lagret. Foreldremateriale, bølgehøyde, fart, ruhet, krusningsstyrke, lysspredning og absorpsjon er lest tilbake og kontrollert mot innstillingene.
- Importert vanngeometri er målt til 6400 × 6400 centimeter med vannrett overflate før bølgeforskyvning.
- Testscenen er lagret med vannruter, en enklere fjernflate, skrånende sandbunn, sol, himmel og kamera.
- En forhåndsvisning på 1600 × 900 piksler er tatt direkte i Unreal og inspisert. Bildet viser turkist vann, synlig dybdeovergang og bølge-/normaldetaljer.
- Krusningene er dempet med kameraavstanden og filtrert etter pikselstørrelse etter at første bilde viste for mye gjentakelse.

Maskinlesbare resultater: [build_report.json](build_report.json). Bilde: [A001_hav_preview_v1.png](A001_hav_preview_v1.jpg).

## Gjenstår før A001 er ferdig

- Integrer materialet med hovedspillets havflate, lys og faste kameraer.
- Finpuss bølgevariasjon, farger og refleksjoner sammen med båt og ferdig havbunn til uttrykket ligger nærmere den visuelle referansen.
- Kontroller vannets bevegelser i spill, skjøter mellom ruter, overgangen til fjernflaten og synlighet av fiskeline og fisk.
- Prøv ytelse og visning med flere båter og spillere. Dette er ikke målt i denne leveransen.

A001 står som **Til kontroll** og er ikke krysset av som ferdig. Referansebildets fulle visuelle kvalitet er et mål for den samlede spillscenen, ikke et bestått kriterium i denne første materialutgaven.
