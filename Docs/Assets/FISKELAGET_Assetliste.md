# FISKELAGET assetliste og fremdrift

> **Historisk status per 5. november 2026.** Listen og bildene er ikke oppdatert siden. Fra v0.17 lager spillet modeller, hav og fisk i kode (`ProceduralMeshComponent`), og Blender-/FBX-kildene som omtales her, er ikke tatt i bruk i hovedspillet. Hvilke punkter som fortsatt gjelder, avklares i en egen oppgave før listen brukes til planlegging.

Her følger vi arbeidet med modeller, materialer, animasjoner, lyder og grafikk til spillet. Målet er først å få én komplett fiskesituasjon til å fungere, deretter fylle ut LAN-prototypen og senere utvide spillverdenen.

Opprettet: 4. november 2026. Grunnlag: [Kravspesifikasjon v1.1](../Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md) og [idébanken](../Requirements/FISKELAGET_Idebank_2026-11-03.md).

Visuell retning: **fargerik karibisk realisme**. Alle leveranser følger [assetstandarden](FISKELAGET_Assetstandard.md) og [referansebildet](assets/references/fiskelaget_visuell_referanse_v1.jpg).

Dokumentasjon, kontrollrapporter og komprimerte forhåndsbilder er publisert her. Blender-, Unreal-, FBX-, GLB- og teksturkilder som omtales i dokumentasjonen, ligger foreløpig i produksjonsmappen og følger ikke denne dokumentasjonspubliseringen. Se [innhold og kildefiler](README.md).

## Felles statusoversikt

**Sist oppdatert: 5. november 2026.** Dette er prosjektets felles assetliste. Oversikten er offentlig på GitHub. Oppdateringer leveres gjennom pull request. Bruk de samme A-numrene i oppgaver, mapper og overleveringer. [GitHub Issues](https://github.com/hansarne-lang/fiskespillet/issues) er fortsatt autoritativ kilde for utviklingsoppgaver; denne listen registrerer assetbehov og leveranser.

| Status | Antall assetpunkter | Betydning |
| --- | ---: | --- |
| Til kontroll | 15 | Første utgave er laget. Kontroll og integrasjon i hovedspillet gjenstår som angitt under hvert punkt. |
| Åpen uten registrert leveranse | 64 | Vi trenger assetet; en ferdig leveranse er ennå ikke registrert i denne listen. |
| Ferdig i hovedspillet | 0 | Ingen punkter er foreløpig bekreftet mot alle ferdigkravene nedenfor. |
| **Totalt** | **79** | **25 i P1, 31 i P2 og 23 i P3. P3 er videre utvidelser.** |

Antallene gjelder assetpunkter, ikke enkeltmodeller eller prosent ferdig arbeid. Ett punkt kan inneholde flere modeller, materialer eller animasjoner.

### Det som er bygget

Alle leveransene under har status **Til kontroll**, med Codex som ansvarlig for første utgave. Lenken åpner innhold, filer og bruksinformasjon. Den detaljerte listen lenger ned viser gjenstående arbeid for hvert punkt.

| ID | Leveranse | Bygget |
| --- | --- | --- |
| A001 | [Havoverflate](assets/A001_hav/README.md) | Bølger, krusninger og tre vannvarianter i egen Unreal-testscene. |
| A002 | [Himmel og morgenlys](assets/A002_himmel/README.md) | Sol, atmosfære, skyer, dis og omgivelseslys i egen Unreal-testscene. |
| A003 | [Øy og kyst](assets/A003_oy_og_kyst/README.md) | Øy med strand, klipper, palmer, busker og gress, versjon 2. |
| A004 | [Første brygge](assets/A004_brygge/README.md) | T-brygge med fortøyning, fendere og ombordstigningsområde. |
| A005 | [Startbåt](assets/A005_startbat/README.md) | Båt på omtrent 26 fot, to fiskeplasser og separat motor og propell. |
| A006 | [Stangholder](assets/A006_stangholder/README.md) | Holder med monteringsflens og stanganker. |
| A007 | [Fiskestang](assets/A007_fiskestang/README.md) | Stang med føringsringer, rigg og belastningsbøy. |
| A008 | [Snelle](assets/A008_snelle/README.md) | Snelle med separat spole, sveiv, håndtak og bremsespak. |
| A009 | [Line og krok](assets/A009_line_og_krok/README.md) | Redigerbar Blender-line i tre tilstander, krok og svivel. |
| A010 | [Agnfisk](assets/A010_agnfisk/README.md) | Modell, skjellmateriale, svømmerigg og svømmeløkke. |
| A011 | [Landingsutstyr](assets/A011_landingsutstyr/README.md) | Håv og separat sikringstropp med festepunkter. |
| A026 | [Tropisk vegetasjon](assets/A026_vegetasjon/README.md) | Tre palmer, busk og gresstue, brukt i øyscenen. |
| A028 | [Modulær marina](assets/A028_modulaer_marina/README.md) | Tolv moduler, landgang og ombordstigningstrapper; eksempel med fire båtplasser. |
| A029 | [Butikk og fiskemottak](assets/A029_butikk_og_mottak/README.md) | To fasadebygg med veranda, separate dører, mottaksluke og skilt. |
| A030 | [Marinadetaljer](assets/A030_marinadetaljer/README.md) | Ti rekvisitter, blant annet bøye, fender, tau, kasse og redningsring. |

### Det vi trenger videre

**P1 har høyest prioritet:** A012–A015 fiskerfigur, animasjoner og snapper; A016–A021 vanneffekter og lyd; A022–A025 grafikk og symboler under fisking. Samtidig trenger A001–A011 integrasjon og kontroll i hovedspillet.

**P2:** A027 havbunn, A031 felles materialer og A032–A056 flere båter, utstyr, fisker, animasjoner, kart, menyer og lyd. A026 og A028–A030 trenger integrasjon og spillkontroll.

**P3:** A057–A079 strandbar, naust, flere arter og båter, større spillverden og presentasjon. Dette er videre utvidelser som prioriteres etter prototypen.

## Slik følger vi fremgangen

- `[ ]` betyr at leveransen fortsatt er åpen. `[x]` betyr at den er ferdig og kontrollert i spillet.
- Bruk **Pågår** eller **Til kontroll** etter navnet når vi arbeider med en oppgave. Legg til ansvarlig og filsti når det er aktuelt.
- Eksisterende modeller og midlertidige assets kan brukes som utgangspunkt. Åpne punkter betyr at den endelige leveransen ikke er bekreftet her.
- Del opp et punkt hvis flere deler får ulik fremdrift. Behold ID-en på hovedpunktet, og gi delene bokstaver, for eksempel A014a og A014b.
- Oppdater listen når arbeidet starter og når en leveranse er kontrollert. En ferdig modell som venter på animasjon eller innlegging i spillet, står fortsatt åpen.
- **Åpen uten registrert leveranse** er standard for punkter uten annen status. Sett **Pågår** når noen tar oppgaven, **Til kontroll** når filer er levert, og kryss av først når ferdigkravene er oppfylt. Bruk **Blokkert** med konkret årsak hvis arbeidet må vente.
- Ved overlevering: registrer ansvarlig, hva som er laget, lenke til filer og kontroll, hva som gjenstår og dato. Oppdater også oversikten over og arbeidsloggen under. Unngå parallelle assetlister med egne statuser.

Eksempel på oppdatering:

`- [ ] A014 Snapper — Pågår. Ansvarlig: navn. Fil: prosjektets assetsti. Gjenstår: svømmeanimasjon.`

Prioriteringen under er en foreslått produksjonsrekkefølge. Sosiale tillegg og videre utvidelser har eget avsnitt fordi prototypeomfanget for disse fortsatt må avklares.

## P1 Første komplette fisketest

### Hav og omgivelser

- [ ] A001 Havoverflate — **Til kontroll**. Ansvarlig: Codex. [Første utgave](assets/A001_hav/README.md) med bølger, krusninger og tre vannvarianter er laget og kontrollert i eget Unreal-prosjekt. [Forhåndsvisning](assets/A001_hav/A001_hav_preview_v1.jpg). Gjenstår: integrasjon, videre visuell finpuss og kontroll i hovedspillet. Se [kontrollstatus](assets/A001_hav/QA.md).
- [ ] A002 Himmel og morgenlys — **Til kontroll**. Ansvarlig: Codex. [Første utgave](assets/A002_himmel/README.md) med varm sol, blå atmosfære, lyse skyer, omgivelseslys, svak horisontdis og fast eksponering er laget og kontrollert i eget Unreal-prosjekt. [Forhåndsvisning](assets/A002_himmel/A002_morgenlys_v1.jpg). Gjenstår: integrasjon, kontroll fra hovedspillets faste kameraer og ytelsesprøve. Se [kontrollstatus](assets/A002_himmel/QA.md).
- [ ] A003 Enkel øy og kyst — **Til kontroll, versjon 2**. Ansvarlig: Codex. [Øya](assets/A003_oy_og_kyst/README.md) har nå palmer, busker, gresstuer og flere strand- og klippedetaljer. Den lagrede scenen er kontrollert etter gjenåpning i Unreal. [Oversikt v2](assets/A003_oy_og_kyst/A003_oy_oversikt_v2.jpg). Gjenstår: hovedspillintegrasjon, kollisjon, kamerakontroll og ytelsesarbeid. Se [kontrollstatus](assets/A003_oy_og_kyst/QA_v2.md).
- [ ] A004 Første brygge — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [T-brygge med tredekke, turkise stolper, fortøyning, fendere og ombordstigningsområde](assets/A004_brygge/README.md). Redigerbar Blender-fil, FBX, GLB, avstandsmodell, materialer og enkle kollisjonsformer er laget. Kildefilen er gjenåpnet, og eksportene er kontrollert ved ny innlesing. [Forhåndsvisning](assets/A004_brygge/A004_brygge_oversikt_v1.jpg). Gjenstår: Unreal-import, plassering på øya, samspill med båt og spiller, kollisjon og ytelsesprøve. Se [kontrollstatus](assets/A004_brygge/QA.md).

### Båt og utstyr

- [ ] A005 Startbåt — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Startbåt på omtrent 26 fot](assets/A005_startbat/README.md) med midtkonsoll, soltak, seter, to fiskeplasser og separat motor/propell. Redigerbar kilde, FBX/GLB, avstandsmodell, materialer, kollisjon og ankerpunkter er laget og kontrollert etter gjenåpning. [Forhåndsvisning](assets/A005_startbat/A005_startbat_baug_v1.jpg). Gjenstår: Unreal-import, flyting og bevegelse, samspill med spiller, utstyr og brygge, kameraer og ytelse. Se [kontrollstatus](assets/A005_startbat/QA.md).
- [ ] A006 Stangholder — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Gjenbrukbar stangholder](assets/A006_stangholder/README.md) med hult stålrør, monteringsflens, tre innfestinger, bunnpinne og stanganker. Blender-kilde og FBX/GLB er kontrollert ved ny innlesing. [Forhåndsvisning](assets/A006_stangholder/A006_stangholder_v1.jpg). Gjenstår: montering i båtrekka, Unreal-import og samspill med stang, line og kamera. Se [kontrollstatus](assets/A006_stangholder/QA.md).
- [ ] A007 Første fiskestang — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Båtstang på 1,90 meter](assets/A007_fiskestang/README.md) med grep, snellefeste, ni føringsringer og åttebeins rigg for belastningsbøy. Blender-kilde, FBX/GLB, animasjon og ankerpunkter er kontrollert etter gjenåpning og ny import. [Bøyvisning](assets/A007_fiskestang/A007_stang_belastet_v1.jpg). Gjenstår: Unreal-import, kobling til linekraft og pumping, montering i holder, samspill med hender, line, kamera og ytelse. Se [kontrollstatus](assets/A007_fiskestang/QA.md).
- [ ] A008 Første snelle — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Havfiskesnelle](assets/A008_snelle/README.md) med separat spole, sveiv, håndtak og bremsespak. Redigerbar kilde, fem spilldeler, bevegelsesdemonstrasjoner og FBX/GLB er kontrollert. [Forhåndsvisning](assets/A008_snelle/A008_snelle_v1.jpg). Gjenstår: Unreal-import, lineføring, styring av innsveiving og brems, håndgrep, kamera og ytelse. Se [kontrollstatus](assets/A008_snelle/QA.md).
- [ ] A009 Fiskeline og krok — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Line, krok og svivel](assets/A009_line_og_krok/README.md) med redigerbar line som følger A007/A008 i stram, slakk og belastet tilstand. Kilde, tre bakte linemodeller og separate metalldeler er kontrollert etter gjenåpning og FBX/GLB-import. [Forhåndsvisning](assets/A009_line_og_krok/A009_line_boyd_stang_v1.jpg). Gjenstår: bevegelig line i hovedspillet, kraft/slakk/linebrudd, agnfeste, kamera og ytelse. Se [kontrollstatus](assets/A009_line_og_krok/QA.md).
- [ ] A010 Agnfisk — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Sølvfarget agnfisk](assets/A010_agnfisk/README.md) med 2K-skjellkart, finner, firebeins svømmerigg og agnanker. Blender-kilde, FBX/GLB og svømmeløkke er kontrollert. [Forhåndsvisning](assets/A010_agnfisk/A010_agnfisk_v1.jpg). Gjenstår: Unreal-import, krokfeste, agnprogram, dybde, materiale og ytelse. Se [kontrollstatus](assets/A010_agnfisk/QA.md).
- [ ] A011 Landingsutstyr — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Håv og separat sikringstropp](assets/A011_landingsutstyr/README.md) med metallbøyle, modellerte nettsnorer, todelt skaft, grep, klips og ankre. Redigerbar kilde og FBX/GLB er kontrollert. [Forhåndsvisning](assets/A011_landingsutstyr/A011_hav_og_sikringstropp_v1.jpg). Gjenstår: samspill med figur og fangst, landingsanimasjon, sikring, kollisjon, kamera og ytelse. Se [kontrollstatus](assets/A011_landingsutstyr/QA.md).

### Fisker og fisk

- [ ] A012 Grunnfigur for fisker — kropp, hender, klær og rigg som fungerer fra spillerkamera og utvendige båtkameraer.
- [ ] A013 Fiskerens grunnanimasjoner — vente, holde stang, pumpe, sveive, justere brems og lande fisk.
- [ ] A014 Snapper — artsriktig modell, materialer og størrelsesvariasjon.
- [ ] A015 Snapperanimasjoner — svømming, utras, vending, kamp nær båten og landing.

### Effekter og lyd

- [ ] A016 Båtens kjølvann og motorskum — tilpasset startbåtens bevegelse og fart.
- [ ] A017 Grunnleggende vannsprut — fisk ved overflaten og enkel landing.
- [ ] A018 Snellelyder — automatisk første utras, senere utras, innsveiving og justering av brems.
- [ ] A019 Fiskelyder — napp, belastning i utstyret, linebrudd og landing.
- [ ] A020 Startbåtens motorlyder — start, tomgang, akselerasjon, marsjfart og stopp med sammenhengende overganger.
- [ ] A021 Havmiljøets lyd — bølger, vann mot skrog og vind med sømløse løkker.

### Grafikk under fisking

- [ ] A022 Stangkort og statussymboler — valgt stang, agn, dybde, venting, napp og kamp.
- [ ] A023 Kampvisning — linebelastning, brems, line ute og retning på linebevegelsen.
- [ ] A024 Klokke og returvarsel — dagsklokke, frist ved minutt 17 og tydelige varseltilstander.
- [ ] A025 Handlingssymboler — betjening av stang, landing, avbrytelse og retur med lesbare tekster og ikoner.

## P2 Resten av den første LAN prototypen

### Miljø og marina

- [ ] A026 Tropisk vegetasjon — **Til kontroll, første pakke**. Ansvarlig: Codex. [Tre palmevarianter, busk og gresstue](assets/A026_vegetasjon/README.md) er laget og brukt i A003 v2. Modeller, materialer og plasseringer er kontrollert i den gjenåpnede øyscenen. [Palmedetalj](assets/A026_vegetasjon/A026_palme_detalj_v1.jpg). Gjenstår: hovedspillintegrasjon, detaljnivåer, gruppering og kontroll av vind og ytelse. Se [kontrollstatus](assets/A026_vegetasjon/QA.md).
- [ ] A027 Havbunn — sand, stein, korallrev og undervannsskrent med tilhørende materialer.
- [ ] A028 Modulær marina — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Tolv moduler](assets/A028_modulaer_marina/README.md) med brede bryggedeler, smale utliggere, landgang og ombordstigningstrapp. Redigerbar kilde, 24 FBX/GLB-eksporter, koblingspunkter og 59 kollisjonsformer er kontrollert. Eksempel med 33 sammenkoblede moduler og fire startbåter. [Forhåndsvisning](assets/A028_modulaer_marina/A028_marina_fire_batplasser_v1.jpg). Gjenstår: Unreal-import, plassering på øya, spillerbevegelse og ombordstigning, kollisjon med bevegelig båt, kamera, LAN og ytelse. Se [kontrollstatus](assets/A028_modulaer_marina/QA.md).
- [ ] A029 Butikk og fiskemottak — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [To fargerike fasadebygg](assets/A029_butikk_og_mottak/README.md) med veranda, separate dører, mottaksluke, betjeningspunkter og modellerte skilt. Blender-kilde, to FBX/GLB-pakker, delte 2K-materialkart og enkle kollisjonsformer er kontrollert etter gjenåpning. [Forhåndsvisning](assets/A029_butikk_og_mottak/A029_marina_oversikt_v1.jpg). Gjenstår: plassering på øya, spiller-/kamerakontroll, interaksjon, dør-/lukebevegelse, kollisjon og ytelse. Se [kontrollstatus](assets/A029_butikk_og_mottak/QA.md).
- [ ] A030 Marinadetaljer — **Til kontroll, Blender-utgave 1**. Ansvarlig: Codex. [Ti gjenbrukbare rekvisitter](assets/A030_marinadetaljer/README.md): bøye, fender, taukveil, fortøyningsklamp, fiskekasse, kjøleboks, utstyrskoffert, pullert, redningsring og bøtte. Blender-kilde og tjue FBX/GLB-eksporter er kontrollert. [Forhåndsvisning](assets/A030_marinadetaljer/A030_marinadetaljer_v1.jpg). Gjenstår: plassering, kollisjon, bevegelig bruk, materialkontroll, detaljnivåer og ytelse i hovedspillet. Se [kontrollstatus](assets/A030_marinadetaljer/QA.md).
- [ ] A031 Felles materialer — treverk, malt metall, glassfiber, gummi, glass, sand og stein med passende slitasje og våte overflater.

### Flere båter og utstyr

- [ ] A032 Offshorebåt — komplett modell med materialer, cockpit, motorer, stangplasser og fangstlager. Omtrent 36 fot er visuell referanse.
- [ ] A033 Offshorebåtens motorlyder og kjølvann — tilpasset båtens motorer, skrog og fart.
- [ ] A034 Fangstlager og slepeutstyr — synlig lagerløsning, festepunkter og tau til stor fisk.
- [ ] A035 Blekksprutagn — modell, materialer og bevegelse.
- [ ] A036 Metalljigg — modell, materialer og bevegelse.
- [ ] A037 Utstyrsoppgraderinger — nødvendige varianter av stenger, sneller og snøre med gjenkjennelige forskjeller.
- [ ] A038 Utstyrsikoner — stenger, sneller, snøre, kroker, agn, stangholdere og båter til rigging og butikk.

### Flere fisk og bevegelser

- [ ] A039 Amberjack — artsriktig modell, materialer og størrelsesvariasjon.
- [ ] A040 Amberjackanimasjoner — kraftige dykk, utras, vendinger, landing og bevegelse nær båten.
- [ ] A041 Tunfisk — artsriktig modell og materialer med størrelsesvariasjon, inkludert stor slepefisk. Endelig tunfiskvariant må avklares.
- [ ] A042 Tunfiskanimasjoner — lange utras, retningsendringer, landing og sleping.
- [ ] A043 Fiskerens ekstra animasjoner — reaksjon på napp, mistet fisk, hjelp ved landing og sikring av slepefisk.
- [ ] A044 Fiskerens visuelle varianter — klær og farger som gjør medspillere gjenkjennelige.
- [ ] A045 Utvidede vanneffekter — skrogsprut, fiskebevegelser ved overflaten og spor etter slepefisk.

### Kart og grensesnitt

- [ ] A046 Sjøkartets grunnlag — kyst, dybdekoter, sand, stein, rev og skrenter som samsvarer med spillverdenen.
- [ ] A047 Kartmarkører — egen og andres båter, marina, destinasjon, rute og navngitte fiskeplasser.
- [ ] A048 Ekkoloddgrafikk — bunn, struktur, fisketegn, dybdeskala og omtrentlige fiskedybder.
- [ ] A049 Båt og mannskap — båtvalg, ombordstigning, kapteinsmerke, mannskapsoversikt og overføring av kapteinsrollen.
- [ ] A050 Lager og returstatus — normal last, fullt lager, overlast, sleping og beregnet returtid uten å røpe dagens fangstresultater.
- [ ] A051 Rigging og butikk — valg av utstyr, dybde, agnprogram, brems, kjøp og lagrede oppsett.
- [ ] A052 Startmeny og LAN — opprett spill, finn spill, lobby, tilkobling og feilmeldinger.
- [ ] A053 Journal og karriere — fangsthistorikk, filtre, hovedtall og personlige rekorder.
- [ ] A054 Felles avsløring og dagsoppgjør — fangstkort, plasseringer, salg, premier, utgifter og tap ved sen retur.
- [ ] A055 Innstillinger og lesbarhet — tekststiler, knapper, menytilstander, lydvalg og kontrolloversikt.
- [ ] A056 Meny og varsellyder — valg, kjøp, feil, returfrist, avsløring, rekord og oppgjør.

## P3 Sosiale tillegg og videre utvidelser

### Strandbar og naust

- [ ] A057 Strandbarens bygg og materialer — bar med havutsikt og plass til sosialt samvær.
- [ ] A058 Strandbarens innredning — bardisk, bord, stoler, glass, drinker og tropiske dekorasjoner.
- [ ] A059 Resultat og veddemålsvisning — tavle, valg av fisker, innsats, pott og resultat.
- [ ] A060 Strandbarens lyd og musikk — karibisk musikk, bølgeskvulp og fugleliv.
- [ ] A061 Personlig naust — modulært bygg med navneskilt, veranda og besøksmulighet.
- [ ] A062 Naustets utstyrsrom — stangstativer, oppbevaring for sneller og agn og andre rekvisitter.
- [ ] A063 Trofévegg — rammer, plaketter og visning av tidligere bekreftede rekorder.
- [ ] A064 Kosmetikk til naust — varianter av fasade, møbler, dekorasjoner og belysning.

### Levende reiser og større spillverden

- [ ] A065 Flygefisk — modell, materialer og animasjon over og under overflaten.
- [ ] A066 Pelikan — modell, materialer, flygeanimasjon og lyd.
- [ ] A067 Fregattfugl — modell, materialer, flygeanimasjon og lyd.
- [ ] A068 Grouper — modell, materialer og animasjoner når kampstilen er avklart.
- [ ] A069 Mahi mahi — modell, materialer, kampanimasjoner og akrobatikk.
- [ ] A070 Wahoo — modell, materialer og raske utras.
- [ ] A071 Blå marlin — modell, materialer, eksplosive utras, vendinger og hopp.
- [ ] A072 Avanserte hopp og vannsprut — artstilpassede effekter og bevegelse i og over vannet.
- [ ] A073 Sportsfiskebåt — modell, materialer, motorlyder, kjølvann, stangplasser og slepeutstyr.
- [ ] A074 Flere øyer og fiskeområder — landskap, kyst, havbunn og visuelle kjennetegn når de fem områdenes innhold er bestemt.
- [ ] A075 Flere båter og kosmetiske varianter — skrog, farger, detaljer og oppgraderinger etter videre designvalg.

### Spillprofil og presentasjon

- [ ] A076 Spillogo — lesbar tittelgrafikk til hovedmeny og presentasjon.
- [ ] A077 Hovedmenybilde og lasteskjermer — motiver som følger spillets realistiske karibiske uttrykk.
- [ ] A078 Menymusikk — passende stemning og sømløs avspilling.
- [ ] A079 Utvidede presentasjonssekvenser — flere faste reisevinkler og korte sekvenser ved spesielle fangster, landing og avsløring. Grunnleggende faste kameraer brukes allerede i P1.

## Når et punkt kan krysses av

- Assetet har riktig form, størrelse og uttrykk for realistisk 3D i et karibisk miljø.
- Nødvendige materialer, bevegelser og lyder er med, eller er ferdige i egne tilknyttede punkter.
- Assetet er lagt inn i spillet og kontrollert fra de kameraene hvor spilleren ser eller hører det.
- Det fungerer sammen med stang, line, fisk, båt og grensesnitt der dette er relevant.
- Det er prøvd med flere samtidige båter eller spillere når det er relevant for leveransen.
- Bruksrettigheter og kilde er registrert hvis assetet er hentet utenfra.
- Ansvarlig eller den som kontrollerer, legger inn filsti og ferdigdato ved avkryssing.

Dagens bekreftede art, eksakte vekt, lengde, fangstantall, verdi og nye rekorder skal være skjult til minutt 17. Dette gjelder også asseter og visninger til journal, lager, trofévegg, varsler og oppgjør. Synlige fisker og slepefisk skal fortsatt kunne observeres i spillverdenen.

## Arbeidslogg

Før en kort linje her når vi starter, leverer eller reviderer et punkt. Selve avkryssingen over er den gjeldende statusen.

| Dato | Asset ID | Endring | Ansvarlig | Fil eller merknad |
| --- | --- | --- | --- | --- |
| 2026-11-04 | Alle | Assetlisten opprettet med foreslått produksjonsrekkefølge. | Codex | Eksisterende assets avstemmes når punktene tas opp. |
| 2026-11-04 | Referanse | Visuell assetstandard v1 og referansebilde opprettet. | Codex | `FISKELAGET_Assetstandard.md` og `assets/references/`. |
| 2026-11-04 | A001 | Arbeid startet på havmaterialet. | Codex | `assets/A001_hav/`. |
| 2026-11-04 | A001 | Første havmateriale, tre varianter, vanngeometri og testscene laget. Shader og variantinnstillinger kontrollert; forhåndsvisning tatt i Unreal. Status Til kontroll. | Codex | `assets/A001_hav/QA.md`. Integrasjon og visuell finpuss gjenstår. |
| 2026-11-04 | A002 | Himmel og morgenlys laget som gjenbrukbart lysdelnivå. Innstillinger kontrollert i Unreal; horisont- og fargeprøver inspisert. Status Til kontroll. | Codex | `assets/A002_himmel/QA.md`. Integrasjon, kamerakontroll og ytelsesprøve gjenstår. |
| 2026-11-04 | A003 | Øy, kyst, sandbukt og steinvarianter laget. Eksportert geometri og Unreal-ressurser kontrollert; tre forhåndsvisninger inspisert. Status Til kontroll. | Codex | `assets/A003_oy_og_kyst/QA.md`. Foreslåtte marinaankerpunkter registrert for videre arbeid med A004. |
| 2026-11-04 | A003 v2 | Øya detaljert med 42 palmer, 45 busker, 90 gresstuer og 56 nye strand- og klippesteiner. Scenekoblinger rettet og lagringen kontrollert ved gjenåpning. | Codex | `assets/A003_oy_og_kyst/QA_v2.md`. Status Til kontroll. |
| 2026-11-04 | A026 | Første vegetasjonspakke laget: tre palmer, busk, gresstue og egne materialer med svak vind. Geometri, materialer og lagret scene kontrollert. | Codex | `assets/A026_vegetasjon/QA.md`. Status Til kontroll. |
| 2026-11-04 | A004 | Blender-produksjon startet på første T-brygge med fortøyning og ombordstigningsområde. | Codex | `assets/A004_brygge/`. |
| 2026-11-04 | A004 | Blender-utgave 1 levert med 621 redigerbare deler, FBX/GLB, avstandsmodell, tre teksturkart, kollisjon og ankerpunkter. Kildefil og eksportgeometri kontrollert etter gjenåpning. Status Til kontroll. | Codex | `assets/A004_brygge/QA.md`. Hovedspillintegrasjon gjenstår. |
| 2026-11-04 | A005 | Blender-produksjon startet på startbåt med cockpit, to fiskeplasser, motor og bevegelig propell. | Codex | `assets/A005_startbat/`. |
| 2026-11-04 | A005 | Blender-utgave 1 levert med 139 redigerbare komponenter, separat motor/propell, FBX/GLB, avstandsmodell, to 2K-kart, tre kollisjonsformer og 14 ankerpunkter. Geometri, mål og eksporthierarki kontrollert. Fire bilder fra gjenåpnet kilde inspisert. Status Til kontroll. | Codex | `assets/A005_startbat/QA.md`. Hovedspillintegrasjon gjenstår. |
| 2026-11-04 | A006 | Gjenbrukbar stangholder laget i Blender med 768 trekanter, seks kildedeler og stanganker. Kildefil, mål og FBX/GLB kontrollert ved gjenåpning. Status Til kontroll. | Codex | `assets/A006_stangholder/QA.md`. Montering og samspill med utstyret gjenstår. |
| 2026-11-04 | A007, A008 | Blender-produksjon startet på stang med belastningsrigg og snelle med bevegelige deler. | Codex | `assets/A007_fiskestang/` og `assets/A008_snelle/`. |
| 2026-11-04 | A007 | Første stang levert med 85 kildedeler, åtte bein, belastningshandling og 13 ankre. Grep, skinnevekter og deformasjon kontrollert i kilde, FBX og GLB. Tre bilder fra gjenåpnet kilde inspisert. Status Til kontroll. | Codex | `assets/A007_fiskestang/QA.md`. Hovedspillintegrasjon gjenstår. |
| 2026-11-04 | A008 | Snelle levert med 35 kildedeler, fem bevegelige spilldeler, fire demonstrasjonshandlinger og seks ankre. Eksportmål, dreiepunkter og sveiv-/grephierarki kontrollert. Status Til kontroll. | Codex | `assets/A008_snelle/QA.md`. Betjening og samspill med line gjenstår. |
| 2026-11-04 | A009–A011 | Blender-produksjon startet på line/krok, agnfisk og håv med sikringstropp. | Codex | Nye assetmapper under `assets/`. |
| 2026-11-04 | A009 | Levende Blender-line kontrollert mot 26 punkter, ni føringsringer og krokøye i tre tilstander. Krok, svivel og tre bakte linemodeller levert som FBX/GLB. Fire bilder inspisert. Status Til kontroll. | Codex | `assets/A009_line_og_krok/QA.md`. Spillets bevegelige line gjenstår. |
| 2026-11-04 | A010 | Agnfisk levert med 39 kildedeler, 10 320 trekanter, 2K-kart og firebeins svømmeløkke. Form og finner finpusset; kilde, eksport og animasjon kontrollert på nytt. To bilder inspisert. Status Til kontroll. | Codex | `assets/A010_agnfisk/QA.md`. Agnfeste og spillintegrasjon gjenstår. |
| 2026-11-04 | A011 | Håv og sikringstropp levert med 108 kildedeler, fem spillmodeller, to røtter og sju ankre. Modellmål og FBX/GLB kontrollert. Forhåndsvisning inspisert. Status Til kontroll. | Codex | `assets/A011_landingsutstyr/QA.md`. Landing med figur og fangst gjenstår. |
| 2026-11-04 | A029, A030 | Blender-produksjon startet på butikk, fiskemottak og gjenbrukbare marinadetaljer. | Codex | `assets/A029_butikk_og_mottak/` og `assets/A030_marinadetaljer/`. |
| 2026-11-04 | A029 | To bygg levert med 421 kildedeler, fem spillmodeller, tre 2K-kart, åtte ankre og 14 kollisjonsbokser. Hengselpunkter, innganger og FBX/GLB kontrollert. Tre bilder fra gjenåpnet kilde inspisert. Status Til kontroll. | Codex | `assets/A029_butikk_og_mottak/QA.md`. Hovedspillintegrasjon gjenstår. |
| 2026-11-04 | A030 | Ti marinarekvisitter levert med 144 kildedeler, egne festepunkter og én FBX/GLB-pakke per modell. Alle tjue eksportfiler kontrollert ved ny innlesing. Forhåndsvisning inspisert. Status Til kontroll. | Codex | `assets/A030_marinadetaljer/QA.md`. Plassering og spilltilpasning gjenstår. |
| 2026-11-04 | A028 | Blender-produksjon startet på modulær marina med koblingspunkter, landgang og eksempel med fire båtplasser. | Codex | `assets/A028_modulaer_marina/`. |
| 2026-11-05 | A028 | Tolv moduler levert. Lagret kilde og 24 eksporter kontrollert; eksempel med 33 sammenkoblede moduler og fire båter. Status Til kontroll. | Codex | `assets/A028_modulaer_marina/QA.md`. Hovedspillintegrasjon og bevegelig ombordstigning gjenstår. |
| 2026-11-05 | Alle | Felles statusoversikt, oversikt over bygde leveranser og gjenstående behov lagt til. Prosjektforsiden lenker hit. | Codex | 79 punkter: 15 til kontroll og 64 uten registrert leveranse. |
