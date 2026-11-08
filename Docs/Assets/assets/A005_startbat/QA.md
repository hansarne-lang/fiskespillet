# A005 Kontroll av startbåt

Kontrolldato 4. november 2026. Blender 5.2.2 LTS. **Til kontroll før integrasjon i hovedspillet.**

## Kontrollert leveranse

- Den lagrede Blender-filen er gjenåpnet i en ny økt. 139 redigerbare modellkomponenter er bevart: 117 på båt og innredning, 18 på motoren og 4 på propellen.
- Den målte skroglengden er 7,9248 meter. Selve skrogskallet er omtrent 2,653 meter bredt. Rekke, ramme og detaljer gir en større samlet utstrekning.
- Spillmodellen har 37 276 trekanter på båt og innredning, 6 556 på motoren og 488 på propellen, totalt 44 320. Båtens avstandsmodell har 17 892 trekanter; motor og propell beholdes som separate deler.
- Alle fire kontrollerte modeller har gyldige koordinater og UV-er, uten flater med null areal, løse punkter, åpne kanter eller kanter med flere enn to tilstøtende flater.
- To 2K-bilder er pakket i Blender-filen og lagret separat. Fargekartet bruker sRGB; OpenGL-normaler bruker datafarger. Glassfiber, sklisikkert dekk, setetrekk, stål, glass, gummi og lanterner har egne PBR-materialer.
- De tre synlige spillmodellene er eksportert til både FBX og GLB og importert i hver sin tomme Blender-scene. Kontrollen sammenligner mål, trekanttall, materialplasser og dreiepunkter for hver del.
- Motorens underordnede propell, 14 ankerpunkter og tre lukkede, konvekse FBX-kollisjoner er bevart. Stangankrenes retning følger de to hule rørene i rekka.
- To illustrerende Blender-handlinger for motorstyring og propell er bevart i kildefilen. De er inaktive i hvileposisjonen og er ikke en ferdig spillanimasjon.

Se [validation_report.json](validation_report.json) for målefeil og kontrollresultater. [build_report.json](build_report.json) registrerer navn, geometri og alle ankerposisjoner.

Kontrollen førte til justerte avrundinger på små beslag, langsgående flater på den buede rekka og riktig evaluering av ankertransformer før tilordning til båtens rot. Disse rettelsene inngår i den kontrollerte kildefilen og eksportene.

## Visuell kontroll

Fire forhåndsvisninger er tatt fra den gjenåpnede kildefilen og inspisert. De viser baug og skrog, akterdekk og motor, førerplass og en ekstra visning i nøytralt lys. Vann og lys er et separat Blender-miljø og inngår ikke i eksportfilene. [capture_report.json](capture_report.json) registrerer bildene.

Fargevalg, proporsjoner, skrogforløp, rekker, seter og cockpit vurderes mot prosjektets karibiske referanse. Skjermen viser ingen fangstopplysninger. Den endelige visuelle kontrollen må også gjøres i A002-morgenlyset og fra hovedspillets faste kameraer.

## Gjenstår i hovedspillet

- Unreal-import med separate båt-, motor- og propellmodeller, riktige dreiepunkter og fungerende glassmateriale.
- Knytte motorstyring og propellhastighet til faktisk båtbevegelse. Flyting, trim, motorkraft, autopilot og kjølvann inngår ikke i denne modellen.
- Prøve de to fiskeplassene, stangfestene og landingsankrene sammen med figur, hender, stang, snelle og line. Ingen av de to stangplassene skal gi flere aktive stenger enn båtens samlede kapasitet.
- Tilpasse ombordstigning og kameravinkler til A004-brygga. Bryggas dekk og båtens cockpit har ulike høyder; faktisk spillerposisjon og overgangen må avklares i spillintegrasjonen.
- Kontrollere båtkollisjon mot kyst, brygge og andre båter, inkludert motorens utstrekning. De tre medfølgende skrogformene er et arbeidsgrunnlag.
- Kontrollere LOD-overganger, glass-sortering, materialkostnad og ytelse med samtidige båter og spillere.
- Koble lagerkapasitet, overlast og sleping til spillets systemer. Modellen alene håndhever ingen av disse reglene.

A005 står åpent i assetlisten fram til integrasjonskravene er oppfylt. Alle modeller og teksturer kommer fra prosjektets egen kilde.
