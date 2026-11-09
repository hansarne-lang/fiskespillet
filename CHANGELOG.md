# Endringslogg

Nyeste først. Spillerguide med gjeldende taster: [Docs/PLAYER_GUIDE.md](Docs/PLAYER_GUIDE.md). Detaljer, grenser og testgrunnlag står i versjonsguidene i [Docs/Development](Docs/Development). Datoene er når versjonen ble integrert i `main`. v0.2–v0.8.1 ble integrert samlet 6. november 2026.

## Musstyrt stang – under arbeid

I fiskekampen styres stanga med musa, uten å holde knapper. Musa opp pumper, musa ned sveiver inn av seg selv, og musehjulet er bremsen. Piltastene virker også. Det er standard og kan slås av under Innstillinger, sammen med musfølsomhet. Fiskekampens regler og nettverksprotokollen er uendret. Innstillingsfila får versjon 2. Issue #132.

## SpilLan – 9. november 2026

FISKELAGET kan publiseres til launcheren SpilLan med eget kortbilde, én mappe per versjon og lagring i brukermappa (`-SaveToUserDir`). Spillernavnet fra SpilLan blir standardnavn i naustet. Ingen spillregler er endret. [Oppskrift](Docs/Development/SPILLAN.md), issue #130.

## Testbudsjett – 9. november 2026

`Test-PR.ps1` med tre nivåer (Build, PR, Release), én kort testrapport (`report.py`), kjente feil merket med issue (`known_failures.json`) og cache som hopper over skript som allerede har bestått med samme bygg. Codex er tatt ut av utviklingen. Issue #137.

## Raskere bygg og test – 8. november 2026

Testklokke (`-FiskClockScale`, bare utenfor Shipping), felles port- og klokkeoppsett for testskriptene, parallell kjøring med tidsmåling (`Run-All.ps1`), valg av tester etter endrede filer, inkrementelt byggeskript med tidslogg og hodeløs kjøring av automasjonstestene. Ingen spillregler er endret. Bygget og automasjonstestet på Windows; testresultater i PR #117.

## v0.22 Øyhav og havneguide – 8. november 2026

F10-havneguide til båt, strandbar og tolv naust, med rute på minikartet. Møbel- og kantklaring på bryggene, tre gåkameraer, hilsener mens du går og rikere terreng på de tre øyene. [Guide](Docs/Development/ISLAND_HARBOUR_V022.md), PR #111.

## v0.21 Bryggeliv – 8. november 2026

Gåbart bryggenett (T, W/A/S/D, Shift) med serverstyrt gange, minikart, nærhetsvalg med E, gåanimasjoner og klesfarger. AI-testhjelp med Å, tydelig nappelyd og knappen «Tilbake til marina». [Guide](Docs/Development/PIER_LIFE_V021.md), PR #109.

## v0.20 Levende havn – 7. november 2026

Synlige fiskere med navn og vinking, original havneatmosfære, kamera for bar, havn og følg båt (B, V) og 3D-galleri med 15 artsmodeller i artsboken. [Guide](Docs/Development/LIVING_HARBOUR_V020.md), PR #107.

## v0.19 Fiskeflåten – 7. november 2026

Inntil tolv spillere og tolv uavhengige båter i samme LAN-økt. Hver karriere kan hente sin egen båt (Tab, C) eller dele båt med andre. [Guide](Docs/Development/FISHING_FLEET_V019.md), PR #105.

## v0.18 Naustliv – 7. november 2026

Naustbutikk (H, N) med ni permanente lys-, brygge- og troféoppgraderinger og mer detaljerte naust. Pynt gir ingen fiskebonus. [Guide](Docs/Development/HARBOUR_LIFE_V018.md), PR #103.

## v0.17 Hav og storfangst – 7. november 2026

Formet skrog og bølgende hav i 3D, fysiske marinabøyer, fangstlager (F11), sleping av storfisk og tvungen retur ved full båt. Fotomodus (F9). [Guide](Docs/Development/OCEAN_CARGO_V017.md), PR #101.

## v0.16 Skipperens verktøykasse – 7. november 2026

Turplanlegger (O) for alle 16 atlasmerker med seks turforslag og korteste rundtur, samt valgfri fisketid per stopp med budsjett mot 17:00. [Guide](Docs/Development/SKIPPERTOOLS_V016.md), PR #99.

## v0.15 Dyphavspakken – 7. november 2026

Femte habitat, Dyphavsbanken (400 m bunn), og fire nye arter, til sammen 15. [Guide](Docs/Development/DEEPSEA_V015.md), PR #97.

## v0.14 Fangstjakten – 7. november 2026

Fangstsentral (Z) med 30 samlinger for arter, habitater, agn, programmer og størrelser. Samlingene gir ingen penger eller bonuser. [Guide](Docs/Development/COLLECTIONS_V014.md), PR #95.

## v0.13 Mangroveekspedisjonen – 6. november 2026

Fjerde habitat, Mangrovebanken, fire nye arter og tre nye atlasmerker. [Guide](Docs/Development/MANGROVE_V013.md), PR #93.

## v0.12 Fiskerpakken – 6. november 2026

Fiskeskole fra hovedmenyen (F6), stangsett som byttes samlet og fiskeassistent (Q). [Guide](Docs/Development/ANGLER_V012.md), PR #91.

## v0.11 Skipperpakken – 6. november 2026

Felles turplan med opptil fem stopp, fotomodus og turbok. [Guide](Docs/Development/SKIPPER_V011.md), PR #89.

## v0.10 Havutforskning og sjøpass – 6. november 2026

Hav-atlas med ni navngitte merker (M, A), flere stopp per tur og personlig sjøpass med milepæler (L). [Guide](Docs/Development/EXPLORATION_V010.md), PR #87.

## v0.9 Ekspedisjoner – 6. november 2026

Dagens ekspedisjon (J) med fem mål, klarmarkering og faste meldinger, fiskeforhold og vær, og trygg gjenhenting av ekspedisjonsposter etter frakobling. [Guide](Docs/Development/EXPEDITIONS_V09_GUIDE.md), [plan](Docs/Development/EXPEDITIONS_V09_PLAN.md).

## v0.8.1 Stabilitetsrettelser – 6. november 2026

Gjennomgang av v0.8 med rettelser i lagring og gjenoppretting av naustvalg og i hjelpevinduet. [Gjennomgang](Docs/Development/REVIEW_V08_STABILITY_V081.md).

## v0.8 Marina og artsjakt – november 2026

Sju arter, artsbok (I), marina med naust og strandbar og dagslys som følger serverklokken. [Guide](Docs/Development/MARINA_DISCOVERY_V08.md).

## v0.7 Karriere og offshore – 5. november 2026

Varig karriere med kvitteringskjede og $30 000 i startsaldo, butikk (P) med utstyr og offshorebåt, og inntil fire LAN-spillere. [Guide](Docs/Development/CAREER_OFFSHORE_V07.md), eiergodkjent i DECISIONS.md.

## v0.6 Flere stenger, artsprofiler og kameraer – 5. november 2026

To stenger på startbåten, egne kampprofiler per art og flere kameravinkler. [Guide](Docs/Development/FISHING_V06.md).

## v0.5 Ekkolodd, favoritter, HUD og LAN-tavle – november 2026

Ekkolodd (F7), lagrede kartfavoritter, ny HUD og LAN-tavle (F8). [Guide](Docs/Development/NAVIGATION_V05.md).

## v0.4 Fangstjournal og artsrekorder – november 2026

Lokal fangstjournal (F4) med sortering, filtre og artsrekorder, som først oppdateres etter avsløringen 17:00. [Guide](Docs/Development/JOURNAL_V04.md).

## v0.3 Flere fiskedager i samme økt – november 2026

Verten starter neste dag med N etter 20:00. Mannskap, kaptein og rigg beholdes, og øktens oppgjør kan leses med F3. [Guide](Docs/Development/SESSION_V03.md).

## v0.2 Mer fiskeinnhold – november 2026

Arter per fiskeplass, skjult fangstgenerering på serveren, detaljert oppgjør ved 17:00 og lagrede oppsett. [Guide](Docs/Development/CONTENT_V02.md).

## v0.1 Spillbar prototype – 3.–5. november 2026

Første LAN-fiskedag med felles klokke (PR #29), ombordstigning, båttur, stang, fiskekamp med brems og belastning, gjentatt fiske, stangoppsett (G), sjøkart (M), kapteinsoverføring (K), retur til marinasonen, enkel 3D, hovedmeny og spill alene. [Full dag](Docs/Development/FULL_DAY.md), [fiskekamp](Docs/Development/FISH_COMBAT.md), [hovedmeny](Docs/Development/MAIN_MENU.md), [spill alene](Docs/Development/SOLO_PLAY.md).

## Unreal-grunnstamme – 3. november 2026

Minimale moduler FiskCore og FiskGame, Game- og Editor-target og teknisk oppstart (PR #27). [Bootstrap](Docs/Development/BOOTSTRAP.md).

## Kravspesifikasjon v1.1 – 3. november 2026

Samler de fem nye ideene og vedtatt felles avsløring, salg og utbetaling ved minutt 17. Oppdaterer hovedlenker, beslutningsgrunnlag, klientfiltrering og planlagte akseptanseprøver. Kravene finnes som Markdown og Word; v1.0 beholdes som historikk. Nye funksjoner og åpne eierbeslutninger spores i det eksisterende GitHub Project.

Dette er en dokumentrevisjon, ikke en spillversjon. Ingen spillmekanikk, nye moduler eller utførte UE-tester inngår.

## Utviklingsgrunnlag 0.0.1 – 3. november 2026

Etablerer offentlig prosjektgrunnlag med godkjent arkitektur, modulkontrakter, kravavskrift, bidrags-/agentregler, Git LFS-mønstre, Issue-/PR-maler og en lesende repositorykontroll i GitHub Actions.

GitHub Issues er prosjektets oppgaveliste. Projects, grenregler og øvrig etableringsstatus dokumenteres med faktiske bevis i de tilhørende issuene. Ingen Unreal-bygg eller spillmekanikk er levert av dette grunnoppsettet.
