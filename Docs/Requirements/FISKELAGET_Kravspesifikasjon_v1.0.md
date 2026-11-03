# FISKELAGET

Samlet kravspesifikasjon

Dokumentversjon 1.0 • Designgrunnlag gjennom v4.1 • 3. november 2026

### Formål og spillopplevelse

FISKELAGET er et realistisk, sosialt fiskespill for PC med opptil 12 spillere på lokalt nettverk (LAN). Spillet foregår i en fiktiv karibisk øygruppe med tropiske øyer, korallrev, dyphav, solskinn og store sportsfisker. Avslappet, passiv fisking avbrytes av intense fiskekamper. Spillere kan eie egne båter eller fiske sammen på venners båter.

Kravene beskriver både den planlagte spillopplevelsen og en avgrenset første spillbar LAN-prototype. Realistisk 3D er valgt fra første prototype. Spillet skal fungere uten internett og uten sentral brukerkonto.

### Kravstatus og bruk

- Faste krav er uttrykt med «skal». De omfatter brukerens valg og videreførte designbeslutninger.

- Prototypeomfanget i kapittel 2 er avgrensningen foreslått i v4.0. Funksjoner som utsettes, beholdes i planen for det ferdige spillet.

- Testverdier, kontrollforslag og kameraforslag er foreløpige og skal prøves før de låses.

- Ved motstrid gjelder den nyeste uttrykkelige brukerbeslutningen. Realistisk 3D erstatter anbefalingen om enklere 3D i v4.0.

### Dokumentets innhold

1. Formål og kravstatus (denne siden)
2. Prototype og spillverden
3. Fiskedag, navigasjon og LAN-samspill
4. Stenger, napp og fiskekamp
5. Fisk, landing, lager og retur
6. Økonomi, journal og permanent karriere
7. Kart, ekkolodd og grensesnitt
8. Realistisk 3D, kamera, lyd og animasjon
9. Akseptanse og testplan
10. Åpne avklaringer og kilder

Dokumentet er en designleveranse. Det inneholder ingen spillkode og beslutter ikke spillmotor eller teknisk implementasjon.

## 2 Prototype og spillverden

Prototypen skal bevise at rigging og passiv venting er engasjerende, at automatiske utras og aktiv kamp fungerer, at returfristen gir interessante valg, at delte og egne båter fungerer sosialt, og at salg og oppgraderinger gir lyst til å spille en ny fiskedag.

| Innhold | Første prototype |
|---|---|
| Spillere | 2–4 i første LAN-test; dimensjonering og senere testmål for 12 |
| Verden | Én liten karibisk øy med kompakt marina |
| Fiskeområder | Korallrev, undervannsskrent og åpent hav |
| Båter | Startbåt og offshorebåt |
| Arter | Snapper, amberjack og tunfisk |
| Utstyr | Inntil tre aktive stenger per spiller, innenfor båtens kapasitet |
| Agn og programmer | Naturlig agnfisk, blekksprut og metalljigg; rolig, pulserende og aktivt program |
| Metoder | Stilleliggende fiske og enkel dorging |
| Spillflyt | En hel 20-minutters fiskedag med retur, salg, kjøp og permanent progresjon |
| Visuell retning | Realistisk 3D fra starten |

### Områder og foreslåtte reisetider

| Område | Fra marinaen | Prototypeart |
|---|---|---|
| Korallrevet | 20–30 sekunder | Snapper |
| Undervannsskrenten | 40–60 sekunder | Amberjack |
| Åpent hav | 60–90 sekunder | Tunfisk |

Tidene er testmål. Alle tre områdene foreslås tilgjengelige fra starten. Spilleren skal kunne velge ethvert gyldig sjøpunkt, ikke bare markerte fiskeplasser.

### Videre spillomfang

Den fulle øygruppen, alle fem planlagte fiskeområder, flere båter, alle arter og legendariske marliner utsettes. Det samme gjelder avanserte dorgemønstre, mange kosmetiske oppgraderinger, omfattende rekordtavler, valgfri skylagring og komplisert anti-juks. Enkel statistikk og permanent lokal karriere inngår allerede.

Storfisk og sleping prøves først med stor tunfisk. Avanserte hopp og vannspruteffekter kan bygges ut senere; grunnleggende realistisk vann og lesbare fiskebevegelser inngår i den visuelle retningen.

## 3 Fiskedag og LAN-samspill

### Felles dagsrytme

| Virkelig tid | Fase |
|---|---|
| 00:00–02:00 | Forberedelser i marinaen |
| 02:00–17:00 | Transport, fiske og retur |
| 17:00–20:00 | Dagsoppgjør, handel og oppgraderinger |

Én fiskedag skal vare nøyaktig 20 minutter. Alle spillere skal følge samme serverstyrte klokke. Båtene skal være tilbake i marinaen senest ved minutt 17. Fristen er absolutt og gjelder også ved pågående kamp eller sleping.

Båter som kommer for sent, skal miste hele dagens fangst og få null i salgsinntekt for denne. Permanent utstyr og tidligere opptjente penger beholdes. Personlige rekorder fra bekreftede landinger beholdes. Dagsresultatet skal vises ved slutten av sjøfasen.

### Automatisk navigasjon

- Kapteinen skal velge et gyldig sjøpunkt på kartet. Autopiloten skal håndtere reisen, posisjonsholding, dorging, kollisjonsunngåelse og retur.

- Båteieren skal være kaptein som standard. Kapteinsrollen skal kunne overføres.

- Kapteinen skal bestemme fiskeplass, dorgemønster og frivillig retur. Andre spillere skal ikke kunne blokkere en returbeslutning.

- Beregnet reisetid og returrisiko skal støtte kapteinens vurdering. Returfristen skal være synlig også under kamp.

### Egne og delte båter

Hver spiller skal kunne eie flere båter og gå om bord på andres båter. Spillere skal styre sine egne stenger og fiskekamper. En medspiller skal kunne hjelpe ved landing; hjelp er aldri obligatorisk. Overtakelse av en stang skal kreve stangeierens tillatelse.

En fisker skal kunne avbryte sin egen kamp. Kapteinen skal kunne avslutte båtens fisketur og starte retur. Ved tvungen retur skal alle pågående kamper på båten avsluttes og alle stenger trekkes inn.

### Autoritet i økten

LAN-serveren skal bekrefte fangster, kjøp, salg og premier. Spillerens PC skal lagre den bekreftede karrieren. Opplevelsen skal støtte både flere fiskere på samme båt, flere båter på samme plass og samtidige kamper uten at andre spillere reduserer nappfrekvensen.

## 4 Stenger, napp og fiskekamp

### Rigging og automatisk fiske

Hver spiller skal kunne bruke maksimalt tre aktive stenger, begrenset av båtens samlede antall stangplasser. Spilleren skal velge stang, snelle, snøre, krok, agn, ønsket fiskedybde i meter, agnprogram og snellebrems. Oppsett skal kunne lagres og brukes på nytt.

| Program | Automatisk agnbevegelse |
|---|---|
| Rolig | Små bevegelser med lange pauser |
| Pulserende | Korte løft og pauser |
| Aktiv | Raskere og større bevegelser |

Alle stangholdere skal bevege agnet automatisk. Etter utsetting skal stanga fiske uten spillerhandling fram til napp. Dorging er en egen metode hvor båtens fart og agnets egenskaper har betydning.

### Napp og første utras

Hver stang skal ha egen, uforutsigbar nappberegning. Fiskeplass, habitat, agn, faktisk fiskedybde, program og eventuell dorgefart skal påvirke nappfrekvensen. Andre båter eller flere stenger på samme sted skal ikke redusere den.

| Oppsett | Gjennomsnitt mellom napp per stang |
|---|---|
| Godt | 2–3 minutter |
| Middels | 3–5 minutter |
| Dårlig | 5–8 minutter |

Dette er testmål for gjennomsnitt, ikke faste tidsintervaller. Ved napp skal fisken kroke seg automatisk. Første utras skal skje uten spillerhandling, med line som trekkes ut og tydelig snellelyd. Aktiv kamp begynner når utraset stopper.

### Aktiv kamp

Spilleren skal pumpe med stanga, sveive inn line og justere bremsen. Fisken skal kunne gjøre nye utras. Høy linebelastning kan gi linebrudd; slakk line kan løsne kroken. Én spiller skal bare kunne kjempe aktivt med én fisk om gangen, og flere samtidige napp kan derfor gi tapte fisker.

Fisken skal ha skjult styrke, hurtighet, utholdenhet, aggressivitet og artsatferd. Art og eksakt vekt skal bekreftes først ved landing. Utholdenhet skal ikke vises som helsebar.

Kontrollforslag: venstre museknapp løfter stanga, slipp senker den, høyre museknapp sveiver, musehjulet justerer bremsen, og mellomrom lander når fisken er klar. Dette er foreløpig; endelig tastetildeling avklares før spilltesten.

## 5 Fisk, fangstlager og retur

| Planlagt art | Kampstil |
|---|---|
| Snapper | Kortere og enklere kamp |
| Grouper | Planlagt art; egen kampstil må konkretiseres |
| Amberjack | Kraftige dykk og hard motstand |
| Mahi-mahi | Aktiv og akrobatisk |
| Wahoo | Svært raske, kortere utras |
| Gulfinnet tunfisk | Lange utras og høy utholdenhet |
| Blå marlin | Eksplosive utras, retningsendringer og hopp |

Fisken skal normalt være skjult under vann og kunne bli synlig ved hopp, overflatebrudd eller nær båten. Landing skal skje med én enkel handling når fisken er nær nok og under kontroll. Alle fisker skal tas med til marinaen og selges; frivillig gjenutsetting inngår ikke.

### Foreløpige båtkapasiteter

| Båt | Spillere | Stangplasser | Lager | Overlast | Slepefisk |
|---|---|---|---|---|---|
| Startbåt | 2 | 2 | 100 kg | 10 kg | 200 kg |
| Offshorebåt | 4 | 6 | 300 kg | 30 kg | 500 kg |
| Sportsfiskebåt | 6 | 12 | 800 kg | Uavklart | 1 000 kg |

Kapasiteter og slepegrenser er testverdier. V4.1-plansjen viser startbåt på 26 fot og offshorebåt på 36 fot som visuelle modellreferanser. Båtens stangplasser gjelder samlet for alle om bord.

### Utfall ved landing

- Fangsten får plass innenfor normal kapasitet: fiskingen kan fortsette.

- Lageret fylles, eller en siste mindre fisk gir tillatt overlast: automatisk retur med normal fart.

- En stor fisk må slepes og er innenfor slepegrensen: fiskingen avsluttes umiddelbart og automatisk retur starter med redusert fart.

- Fisken kan ikke fraktes: den gir ingen salgsinntekt. Videre håndtering må avklares.

Ved tvungen retur skal alle kamper avsluttes og stenger trekkes inn. For sen ankomst skal gi tap av hele dagens fangst, også ved sleping. Stor fisk skal kunne sikres visuelt for sleping. Slepingens fartsreduksjon og reglene ved allerede lastet båt må balanseres og presiseres.

## 6 Økonomi og permanent karriere

### Salg og oppgraderinger

All levert fisk skal selges automatisk ved ankomst innen returfristen. Salgsverdi skal avhenge av art og vekt; rekordpremier kan komme i tillegg. Pengene skal kunne brukes umiddelbart på agn, stenger, sneller, snøre, stangholdere, båter og oppgraderinger. Mindre oppgraderinger skal kunne nås tidlig, mens større båter er langsiktige mål.

| Vare | Foreslått testpris |
|---|---|
| Naturlig agn | $20 |
| Kunstig blekksprut | $100 |
| Bedre snøre | $250 |
| Bedre snelle | $1 000 |
| Bedre stang | $1 500 |
| Ekstra stangholder | $500 |
| Offshorebåt | $25 000 |

Prisene er foreløpige fra v4.0. Mengde per kjøp, artspriser, premier og øvrige priser er ikke låst. Antall fiskedager til oppgraderinger skal vurderes i spilltest.

### Fangstjournal og enkel statistikk

Hver landet fisk skal få en unik fangst-ID. Journalen skal registrere art, vekt, lengde, sted, tidspunkt, agn, fiskedybde, program, båt, utstyr, kampvarighet og økonomisk resultat. Statusene skal være «Landet», «Solgt» og «Tapt ved retur». Journalen skal kunne sorteres etter dato, art og vekt; statusfilter er foreslått.

De fire hovedtallene skal være antall fisk landet, samlet landet fangstvekt, total salgsinntekt og største fisk. Bekreftede landinger teller selv om fangsten senere tapes ved retur. Salgsinntekt teller bare gjennomførte salg. Personlige artsrekorder skal bevares og varsles kort ved ny rekord.

### Lagring og integritet

Én lokal karriere skal fungere på alle LAN-servere. Penger, båter, utstyr, agn, rekorder, journal, statistikk, fiskeplassmarkeringer og lagrede oppsett skal bevares. Dagens usolgte fangst er midlertidig fram til oppgjør.

Serverbekreftelser, unike ID-er, transaksjonslogg, automatisk lagring og sikkerhetskopier skal redusere risikoen for tap og utilsiktet duplisering. Frakobling eller krasj skal ikke gjøre det mulig å selge samme fisk to ganger. Full beskyttelse mot bevisst profilkopiering mellom uavhengige offline-servere kan ikke garanteres.

## 7 Kart, ekkolodd og grensesnitt

### Sjøkart og fiskeplasser

Sjøkartet skal vise bunndybde og dybdekoter, sand, stein, korallrev og skrenter, generelle fiskemuligheter, andre båter, egne markeringer, beregnet reisetid og returrisiko. Det skal støtte fritt valg av gyldige sjøpunkter og permanent lagring av navngitte fiskeplasser. Kartet skal ikke avsløre eksakt art eller fiskestørrelse.

Ekkoloddet skal vise bunndybde, bunnstruktur, fisketegn og omtrentlige fiskedybder. Det skal kunne brukes under fiske og hjelpe spilleren å velge og justere fiskeplass og dybde.

### HUD under venting og kamp

Under vanlig fiske skal HUD-en være diskret og vise aktive stenger, agn, dybde, båtstatus, fangstlager, dagsklokke og returtid. Båt og mannskap skal kunne identifiseres, og kapteinen markeres. V4.1-plansjen viser stangkort langs venstre side, klokke og frist oppe til høyre og diskret kompass.

Under kamp skal linebelastning, snellebrems, line ute i meter, om fisken tar eller gir line, valgt stang og returfrist med sikkerhetsmargin framheves. Plansjen bruker en farget belastningsmåler nederst i bildet; måleren skal gjelde linebelastning og ikke avsløre skjult fiskevekt eller fungere som helsebar.

Stangkortene skal vise oppsett og status. Artsnavn i eksempeloppsett i plansjen skal ikke tolkes som at kroket fisk er identifisert før landing. Faktisk art og eksakt vekt bekreftes ved landing.

### Marina og dagsoppgjør

Marinaen skal tilby båtvalg, ombordstigning, kjøp, automatisk fangstsalg, dagsoppsummering og oppgraderinger. Den skal være kompakt og betjenes med lett forståelige menyer.

Dagsoppsummeringen skal vise landede fisker, samlet vekt, største fisk, fiskesalg, eventuelle rekordpremier, utgifter og nettoresultat. Ved for sen retur skal fangsttap og null salgsinntekt komme tydelig fram.

En enkel rekordtavle foreslås med fanene «Min karriere», «Dagens resultater» og «LAN-rekorder». Min karriere viser hovedtall og personlige rekorder; LAN-rekorder gjelder den aktuelle serveren. Visningene skal gi oversikt og sosial sammenligning uten avanserte analyser av fangstrate, agneffektivitet eller inntekt per fiskeplass.

Ved tidlig retur skal spilleren kunne se oppgjør og journal, sammenligne rekorder, velge lagret fiskeplass og handle til neste dag. Statistikk skal ikke gi direkte fiskefordeler.

## 8 Realistisk 3D og animasjon

### Visuell retning og miljø

Første prototype skal ha realistisk 3D med solfylt karibisk miljø og troverdige båter, fisker, stenger, sneller, vann og lys. Havet skal ha troverdige bølger, refleksjoner, kjølvann og vannsprut. Fargen skal variere fra turkis på grunt vann til mørkeblått offshore.

Båtene skal ha realistiske proporsjoner og tydelige stangholdere, motorer, cockpit og landingsutstyr. Fiskemodellene skal ha artsriktige kroppsformer, størrelsesforskjeller og bevegelsesmønstre. Marinaen skal ha et kompakt, tropisk uttrykk.

### Foreslåtte kameramoduser

| Kamera | Visning og bruk |
|---|---|
| Spillerkamera | Tredjeperson over skulderen som standard; god oversikt over stang, snelle og vann |
| Båtkamera | Utvendig og fri visning av båten, medspillere og havområdet |
| Kinematisk visning | Korte, kontrollerte sekvenser ved spesielle fangster, landing, hopp og rekorder |

Kameraet skal bevare muligheten til å følge linebelastning og kontrollere fiskekampen. Kinematikk skal ikke gjøre sentrale kampsignaler vanskelige å oppfatte.

### Bevegelse og lyd

- Napp og automatisk første utras: tydelig stangbøy, snelle som slipper line og hørbar snellelyd.

- Aktiv kamp: pumping, sveiving og kroppens og stangas reaksjon på belastning.

- Nye utras: synkronisert fiskebevegelse, line som trekkes ut og endret snellelyd.

- Synlig fisk: hopp, overflatebrudd, vannsprut og innsvømming mot båten etter artens atferd.

- Landing og transport: enkel landing og sikring av stor fisk for sleping.

Fisk, line, stang, lyd og HUD skal formidle samme fysiske hendelse. Den visuelle plansjen viser også vibrasjon ved nye utras; nødvendig enhetsstøtte er ikke avklart.

### Prioritering i første prototype

Prioriter troverdig hav og dagslys, lesbar stangbøy og linebevegelse, artsbestemte bevegelser, tydelige napp- og snellelyder, enkel HUD og realistiske båter med kompakt marina. Avanserte effekter og akrobatikk kan utvides etter at grunnopplevelsen er testet.

## 9 Akseptanse og testplan

Kriteriene nedenfor beskriver hva som skal kunne demonstreres i prototypen. De dokumenterer ikke ferdig utviklede eller beståtte funksjoner.

| ID | Akseptansekriterium |
|---|---|
| AK01 | Minst to PC-er kan spille sammen på LAN uten internett eller sentral konto; senere prøves opptil 12 spillere. |
| AK02 | Spillere kan fiske fra samme og forskjellige båter. Kapteinen kan velge fritt gyldig sjøpunkt, og båten navigerer automatisk. |
| AK03 | Rigging, båtens samlede stangkapasitet og grensen på tre aktive stenger per spiller håndheves. |
| AK04 | Stenger fisker passivt, napp og kroking skjer automatisk, og første utras krever ingen handling. |
| AK05 | Spilleren kan pumpe, sveive, justere brems, miste fisk og lande med én handling. Artsatferd er merkbart forskjellig. |
| AK06 | Fullt lager, tillatt overlast og sleping utløser retur. Kamper avsluttes og stenger trekkes inn; sleping gir lavere fart. |
| AK07 | 20-minuttersklokken er felles. Båter innen fristen ved minutt 17 kan selge; for sen retur taper hele dagens fangst uten å miste tidligere verdier. |
| AK08 | Fangst selges én gang, penger kan brukes umiddelbart, og dagsoppgjøret skiller salg, premier, utgifter og fangsttap. |
| AK09 | Karriere, oppsett, journal, rekorder og markeringer bevares etter omstart og ved bruk på en annen LAN-server. |
| AK10 | Realistisk 3D, stangbøy, line, fiskebevegelse og lyd gir sammenhengende tilbakemelding. HUD viser ingen utholdenhetsbar eller eksakt vekt før landing. |

### Testrekkefølge og observasjoner

Test først én spillers kamp, deretter en komplett fiskedag, så LAN med 2–4 spillere og til slutt belastning med 8–12. Prøv flere samtidige kamper, flere båter på samme plass, kapteinens retur under andres kamp, fullt lager og sleping med knapp tidsmargin.

Registrer napp, landinger, tap, returresultater, nettoinntekt og tid brukt på faktisk fiske. Undersøk om spillerne vil starte en ny dag. Kontroller også frakobling og krasj rundt oppgjør, gjenoppretting fra sikkerhetskopi og journal med mange fangster.

## 10 Åpne avklaringer og kilder

### Valg som må avklares før de låses

- Spillmotor, maskinvarekrav, bildefrekvens og øvrige ytelsesgrenser. Unreal Engine 5 er tidligere anbefalt, men er ikke valgt.

- Endelige tastetildelinger. Den oppgitte musekontrollen og mellomrom for landing er forslag; v4.1-plansjens mellomrom for pumping er en motstridende skisseetikett.

- Båtpriser, artspriser per kilo, rekordpremier, startøkonomi, agnforbruk og andre driftsutgifter. Testpriser skal ikke behandles som ferdig balanse.

- Kampvarigheter, artsvekter og endelig variant av tunfisk. Plansjen viser snapper 2–20 kg, amberjack 10–50 kg og tunfisk 30–120 kg som foreløpige referanser.

- Overlast for sportsfiskebåt, reduksjon i fart under sleping, hvordan slepefisk kombineres med last, og utfallet for fisk som ikke kan fraktes.

- Hvordan grenseankomst ved minutt 17 registreres, og hvilken sone som regnes som marina. Selve returfristen er fast.

- Inntektsfordeling, fiskens eierskap og stangplassfordeling på delte båter; detaljert håndtering ved frakobling.

- Hvordan kjøpt ekstra stangholder påvirker båtens kapasitet. Oppgitt stangkapasitet skal være tydelig for spilleren.

- Navn og innhold for de fem områdene i fullversjonen, grouperens kampstil og detaljert omfang etter prototypen.

- Omfang av grunnleggende hopp, kinematikk og vibrasjon i første prototype; avanserte hopp og effekter er utsatt.
