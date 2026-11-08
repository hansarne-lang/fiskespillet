# A028 Kontroll av modulær marina

Leveranse registrert 5. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Bestått kontroll

- Lagret kilde gjenåpnet: tolv moduler, 838 redigerbare kildedeler og 62 336 trekanter i modulbiblioteket, summert én av hver spillmodell.
- Alle spillmodeller har gyldige koordinater, ingen nullflater, åpne kanter, ikke-manifoldkanter eller løse punkter. Modulrøttene står ved lokal origo.
- 59 enkle kollisjonsformer er lukkede og har positivt volum. Hver form har åtte hjørner.
- Alle 24 eksportfiler er lest inn i tomme scener. Navn, mål, trekanttall, materialplasser og koblingspunkter er bevart. FBX inneholder kollisjonsformene; GLB utelater dem.
- Eksempelets 33 moduler har 32 kompatible koblinger. Alle har koblingsvei til land; største avvik mellom koblingssentre er under 0,001 mm.
- Fire A005-båter, inkludert motor og propell, ligger innenfor sine båtplasser uten overlapp mellom båtenes målgrenser. Alle fire trapper har koblingsvei til land.
- Avstanden mellom trappeanker og båtens inngangsanker er 25 cm horisontalt og 23 cm i høyden. Den minste statiske avstanden mellom trapp/fender og båtens målgrenser er omtrent 5 mm; dette er ikke en margin for bevegelig båt.
- Oversiktsbildet fra den gjenåpnede kilden er inspisert: sammenhengende brygge, fire båter, landgang, tredekke, turkise stolper og fasader er synlige.
- Moduloversikten og nærbildet av ombordstigning er inspisert. Modulutvalget er innenfor bildet, og trinn, rekkverk, tredekke og båtens side vises i nærbildet. De tre bildene er registrert i `capture_report.json`.

Detaljer finnes i [validation_report.json](validation_report.json) og [build_report.json](build_report.json). Koblingskontrollen gjelder ankerpunkter og modelloppstilling, ikke spillerens navigasjon.

## Gjenstår i hovedspillet

Unreal-import og plassering på A003, tilpasning av stolper til havbunn og vannstand, kollisjon, spillerpassasje, trapp og ombordstigning med faktiske figurer må prøves. Kontroller skrog, fendere og trapp ved båtbevegelse. Materialer og synlighet må vurderes fra spillkameraene. Avstandsmodeller, instansering, ytelse med flere båter og spillere og LAN-kontroll gjenstår.

A028 står åpent i den felles assetlisten til ferdigkravene er oppfylt.
