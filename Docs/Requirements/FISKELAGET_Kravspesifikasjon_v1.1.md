# FISKELAGET

Samlet kravspesifikasjon

Dokumentversjon 1.1 • Designgrunnlag gjennom v4.1 med godkjente tillegg • 3. november 2026

### Formål og spillopplevelse

FISKELAGET er et realistisk, sosialt fiskespill for PC med opptil 12 spillere på lokalt nettverk (LAN). Spillet foregår i en fiktiv karibisk øygruppe med tropiske øyer, korallrev, dyphav, solskinn og store sportsfisker. Avslappet, passiv fisking avbrytes av intense fiskekamper. Spillere kan eie egne båter eller fiske sammen på venners båter.

Kravene beskriver både den planlagte spillopplevelsen og en avgrenset første spillbar LAN-prototype. Realistisk 3D er valgt fra første prototype. Spillet skal fungere uten internett og uten sentral brukerkonto. Dagens fangstresultater skal avsløres samlet ved minutt 17. Salg, fangstpremier og veddemålsgevinster skal utbetales først da.

### Kravstatus og bruk

- Faste krav er uttrykt med «skal». De omfatter brukerens valg og videreførte designbeslutninger.

- Prototypeomfanget i kapittel 2 er avgrensningen foreslått i v4.0. Funksjoner som utsettes, beholdes i planen for det ferdige spillet. De nye ideene er del av det samlede designet; omfanget i første prototype må avklares særskilt.

- Testverdier, kontrollforslag og kameraforslag er foreløpige og skal prøves før de låses.

- Ved motstrid gjelder den nyeste uttrykkelige brukerbeslutningen. Realistisk 3D erstatter anbefalingen om enklere 3D i v4.0. Faste kameravinkler om bord, hemmelige dagsresultater og oppgjør ved minutt 17 erstatter tidligere motstridende forslag.

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

Dette er krav til spillopplevelsen. Unreal Engine 5 er valgt i den senere godkjente arkitekturen. Teknisk gjennomføring beskrives i arkitekturdokumentene.

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

### Offshore gir større muligheter og risiko

IDÉ-003 er valgt som prinsipp. Jo lenger fra marinaen spillerne fisker, desto større skal muligheten være for enorme og verdifulle fisker. Lengre reisetid gir mindre fisketid, og stor fisk kan kreve lengre kamp og sleping med redusert fart. Returfristen ved minutt 17 gjelder fortsatt uten unntak.

Bedre båter skal gi fordeler gjennom fart, lastekapasitet og slepeevne. Små båter skal fortsatt kunne prøve seg langt offshore. Konkrete avstander og sannsynligheter er balanseringsvalg; lang reise garanterer ingen storfangst.

## 3 Fiskedag og LAN-samspill

### Felles dagsrytme

| Virkelig tid | Fase |
|---|---|
| 00:00–02:00 | Forberedelser i marinaen |
| 02:00–17:00 | Transport, fiske og retur |
| 17:00–20:00 | Felles avsløring og oppgjør ved 17:00, deretter handel og oppgraderinger |

Én fiskedag skal vare nøyaktig 20 minutter. Alle spillere skal følge samme serverstyrte klokke. Båtene skal være tilbake i marinaen senest ved minutt 17. Fristen er absolutt og gjelder også ved pågående kamp eller sleping.

Båter som kommer for sent, skal miste hele dagens fangst og få null i salgsinntekt for denne. Permanent utstyr og tidligere opptjente penger beholdes. Personlige rekorder fra bekreftede landinger beholdes, men dagens nye rekorder offentliggjøres først ved felles avsløring. Ved minutt 17 skal serveren avslutte sjøfasen, kontrollere levering innen fristen og gjennomføre avsløring og oppgjør. Fiskedagen varer fortsatt til minutt 20.

### Automatisk navigasjon

- Kapteinen skal velge et gyldig sjøpunkt på kartet. Autopiloten skal håndtere reisen, posisjonsholding, dorging, kollisjonsunngåelse og retur.

- Båteieren skal være kaptein som standard. Kapteinsrollen skal kunne overføres.

- Kapteinen skal bestemme fiskeplass, dorgemønster og frivillig retur. Andre spillere skal ikke kunne blokkere en returbeslutning.

- Beregnet reisetid og returrisiko skal støtte kapteinens vurdering. Returfristen skal være synlig også under kamp.

### Levende båtreise

IDÉ-002 er fastsatt som designvalg. Reisen fra brygga til fiskeplassen skal vises som en sammenhengende 3D-reise: morgenstemning, motorstart, automatisk utkjøring, akselerasjon, kjølvann, bølger og motorlyd. Flygefisk, pelikaner, fregattfugler og andre båter skal gi levende omgivelser.

Om bord skal spilleren bruke faste kameravinkler, uten fri bevegelse. Reisescenen skal kunne hoppes over bare når alle om bord er enige. Serverens reisetid og båtens fysiske ferd fortsetter uendret; andre spillere ser fortsatt båten reise. Hoppet gir ingen ekstra fisketid. Kapteinen kan fortsatt starte retur uten mannskapets samtykke; avstemning om å hoppe over visningen gjelder bare presentasjonen.

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

Fisken skal ha skjult styrke, hurtighet, utholdenhet, aggressivitet og artsatferd. Bekreftet art, eksakt vekt og lengde skal holdes skjult for alle til felles avsløring ved minutt 17, også etter landing og levering. Utholdenhet skal ikke vises som helsebar.

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

Ved ankomst skal serveren registrere levert fangst og om båten er innen returfristen. All kvalifisert levert fisk skal selges automatisk ved minutt 17. Salgsinntekt, eventuelle fangst- og rekordpremier og veddemålsgevinster skal krediteres først ved dette oppgjøret. Ingen forskuddsbetaling eller foreløpig økning i disponibel saldo skal røpe fangstverdien.

| Vare | Foreslått testpris |
|---|---|
| Naturlig agn | $20 |
| Kunstig blekksprut | $100 |
| Bedre snøre | $250 |
| Bedre snelle | $1 000 |
| Bedre stang | $1 500 |
| Ekstra stangholder | $500 |
| Offshorebåt | $25 000 |

Før minutt 17 kan spilleren handle og satse med tidligere opptjente, disponible penger. Kjøp og veddemålsinnsatser belastes når de gjennomføres. Dagens fangstinntekt kan brukes etter oppgjøret i perioden 17:00–20:00 på agn, stenger, sneller, snøre, stangholdere, båter, oppgraderinger og kosmetikk til naustet. Mindre oppgraderinger skal kunne nås tidlig i karrieren, mens større båter er langsiktige mål.

Prisene er foreløpige fra v4.0. Mengde per kjøp, artspriser, premier og øvrige priser er ikke låst. Antall fiskedager til oppgraderinger skal vurderes i spilltest.

### Fangstjournal og enkel statistikk

Hver landet fisk skal få en unik fangst-ID. Journalen skal registrere art, vekt, lengde, sted, tidspunkt, agn, fiskedybde, program, båt, utstyr, kampvarighet og økonomisk resultat. Statusene skal være «Landet», «Solgt» og «Tapt ved retur». Journalen skal kunne sorteres etter dato, art og vekt; statusfilter er foreslått. Dagens poster og resultater skal holdes skjult i spillerens journal fram til minutt 17; serveren registrerer dem fortløpende.

De fire hovedtallene skal være antall fisk landet, samlet landet fangstvekt, total salgsinntekt og største fisk. Bekreftede landinger teller selv om fangsten senere tapes ved retur. Salgsinntekt teller bare gjennomførte salg. Dagens endringer i statistikk, personlige artsrekorder, rekordvarsler og trofévegg skal først vises ved avsløringen ved minutt 17. Tidligere dagers bekreftede statistikk og rekorder skal fortsatt være tilgjengelige.

### Lagring og integritet

Én lokal karriere skal fungere på alle LAN-servere. Penger, båter, utstyr, agn, rekorder, journal, statistikk, fiskeplassmarkeringer, lagrede oppsett, naust og kjøpte kosmetiske naustoppgraderinger skal bevares. Dagens usolgte fangst og uavsluttede økonomiske oppgjør er midlertidige fram til serverens bekreftede oppgjør.

Serverbekreftelser, unike ID-er, transaksjonslogg, automatisk lagring og sikkerhetskopier skal redusere risikoen for tap og utilsiktet duplisering. Frakobling eller krasj skal ikke gjøre det mulig å selge samme fisk to ganger. Full beskyttelse mot bevisst profilkopiering mellom uavhengige offline-servere kan ikke garanteres.

### Strandbarens veddemål

IDÉ-005 har godkjent hovedregel. Etter egen ankomst til marinaen skal spilleren kunne vedde fiskepenger i strandbaren på hvem av fiskerne som fortsatt er ute, som leverer den største enkeltfisken. Bare fiskere som fortsatt er ute ved registrering, er aktuelle.

Innsatsen skal trekkes én gang fra disponibel saldo. Vinnerne skal dele hele potten. Ved minutt 17 skal nye veddemål stenges, serveren skal fastslå hvilke fangster som er levert innen fristen, og gevinster skal utbetales som del av felles oppgjør. Bare levert fisk innen fristen teller. LAN-serveren skal registrere veddemål og sikre at samme oppgjør ikke utbetales flere ganger, heller ikke etter frakobling eller omstart.

Særreglene for innsats, pottfordeling, lik størstevekt, ingen riktige tips og ingen tellende fangst er avklart 8. november 2026. Se «Avklaring 8. november 2026 — veddemål» nederst og DECISIONS.md. Fisken tilhører stangeieren, også på delte båter.

## 7 Kart, ekkolodd og grensesnitt

### Sjøkart og fiskeplasser

Sjøkartet skal vise bunndybde og dybdekoter, sand, stein, korallrev og skrenter, generelle fiskemuligheter, andre båter, egne markeringer, beregnet reisetid og returrisiko. Det skal støtte fritt valg av gyldige sjøpunkter og permanent lagring av navngitte fiskeplasser. Kartet skal ikke avsløre eksakt art eller fiskestørrelse.

Ekkoloddet skal vise bunndybde, bunnstruktur, fisketegn og omtrentlige fiskedybder. Det skal kunne brukes under fiske og hjelpe spilleren å velge og justere fiskeplass og dybde.

### HUD under venting og kamp

Under vanlig fiske skal HUD-en være diskret og vise aktive stenger, agn, dybde, båtstatus, operativ lagerstatus, dagsklokke og returtid. Lagerstatus skal kunne varsle fullt lager, overlast og sleping uten å vise eksakt fangstvekt, fangstantall eller dagens resultat. Båt og mannskap skal kunne identifiseres, og kapteinen markeres. V4.1-plansjen viser stangkort langs venstre side, klokke og frist oppe til høyre og diskret kompass.

Under kamp skal linebelastning, snellebrems, line ute i meter, om fisken tar eller gir line, valgt stang og returfrist med sikkerhetsmargin framheves. Plansjen bruker en farget belastningsmåler nederst i bildet; måleren skal gjelde linebelastning og ikke avsløre skjult fiskevekt eller fungere som helsebar.

Stangkortene skal vise oppsett og stangstatus. Artsnavn i eksempeloppsett i plansjen identifiserer ikke den krokede fisken. Bekreftet fangstart, eksakt vekt, lengde og verdi skal først vises ved felles avsløring ved minutt 17.

### Marina og dagsoppgjør

Marinaen skal tilby båtvalg, ombordstigning, kjøp, mottak av fangst, strandbar, personlige naust, dagsoppsummering og oppgraderinger. Den skal være kompakt og ha lett forståelige menyer. Levering registreres ved ankomst; automatisk fangstsalg og utbetaling skjer ved minutt 17.

Ved minutt 17 skal dagsoppsummeringen vise landede fisker, samlet vekt, største fisk, fiskesalg, eventuelle rekordpremier, veddemålsresultat, innsatser, gevinster, øvrige utgifter og nettoresultat. Ved for sen retur skal fangsttap og null salgsinntekt komme tydelig fram.

En enkel rekordtavle foreslås med fanene «Min karriere», «Dagens resultater» og «LAN-rekorder». Min karriere viser hovedtall og personlige rekorder; LAN-rekorder gjelder den aktuelle serveren. Visningene skal gi oversikt og sosial sammenligning uten avanserte analyser av fangstrate, agneffektivitet eller inntekt per fiskeplass.

Ved tidlig retur skal spilleren kunne bruke strandbaren, besøke naust, lese tidligere dagers journal og rekorder, velge lagret fiskeplass og handle til neste dag med disponible penger. Dagens oppgjør og fangstresultater blir tilgjengelige først ved minutt 17. Statistikk skal ikke gi direkte fiskefordeler.

### Felles avsløring av dagens fangster

Alle dagens bekreftede fangstresultater skal være hemmelige for alle spillere fram til minutt 17, også for fiskeren selv og mannskapet. Det omfatter bekreftet art, eksakt vekt og lengde, fangstantall, samlet fangstvekt, verdi, dagsplasseringer, journalopplysninger og nye rekorder. Journal, statistikk, rekordvarsler, trofévegg, saldo og resultattavler skal følge samme regel.

Serveren skal registrere og kontrollere fangster, lager og levering fortløpende. Båter, fiskekamper, synlige fisker og slepefisk skal fortsatt kunne observeres, slik at spillerne kan gjette på utfallet. Tidlig levering skal ikke utløse resultatvisning, salg eller utbetaling. Først ved minutt 17 skal dagens resultater avsløres samlet for alle.

### Den tropiske strandbaren

IDÉ-001 er valgt. Spillere som kommer tidlig tilbake, skal kunne slappe av i en tropisk strandbar med karibisk musikk, drinker, bølgeskvulp, fugleliv og utsikt over havet. Baren skal være et sosialt samlingspunkt for spillere som er ferdige med fiskingen.

Tilleggsidéene er å se andre båter komme inn i sanntid, se store fisker som slepes inn og ha en resultattavle med dagens fangster. En slik tavle skal holde resultatene skjult fram til minutt 17. Strandbarens veddemål følger reglene i kapittel 6.

### Personlige naust

IDÉ-004 er godkjent. Hver spiller skal få et eget naust langs brygga med spillerens navn i store bokstaver over inngangen. Alle starter med et enkelt naust som er spillerens personlige hjem.

Naustet skal ha utstyrsrom for stenger, sneller og agn, trofévegg med personlige rekorder og veranda med havutsikt. Spillere skal kunne besøke venners naust og kjøpe kosmetiske oppgraderinger med opptjente fiskepenger. Naustets progresjon skal følge den permanente karrieren mellom LAN-servere. Dagens nye rekorder skal først vises på troféveggen ved felles avsløring.

## 8 Realistisk 3D og animasjon

### Visuell retning og miljø

Første prototype skal ha realistisk 3D med solfylt karibisk miljø og troverdige båter, fisker, stenger, sneller, vann og lys. Havet skal ha troverdige bølger, refleksjoner, kjølvann og vannsprut. Fargen skal variere fra turkis på grunt vann til mørkeblått offshore.

Båtene skal ha realistiske proporsjoner og tydelige stangholdere, motorer, cockpit og landingsutstyr. Fiskemodellene skal ha artsriktige kroppsformer, størrelsesforskjeller og bevegelsesmønstre. Marinaen skal ha et kompakt, tropisk uttrykk.

### Kamera om bord

| Kamera | Visning og bruk |
|---|---|
| Spillerkamera | Foreslått fast vinkel over skulderen med oversikt over stang, snelle og vann |
| Båtkamera | Faste utvendige kameravinkler som viser båt, medspillere og hav |
| Kinematisk visning | Korte, kontrollerte sekvenser ved spesielle fangster, landing, hopp og rekorder |

Kameraet skal bevare muligheten til å følge linebelastning og kontrollere fiskekampen. Kinematikk skal ikke gjøre sentrale kampsignaler vanskelige å oppfatte. Faste kameravinkler og ingen fri bevegelse om bord er nå fastsatt. Valg av konkrete vinkler prøves i prototypen. Reisepresentasjonen skal følge kapittel 3.

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

AK01–AK10 gjelder den planlagte LAN-prototypen med de oppdaterte spillreglene. AK11–AK16 dekker de nye designvalgene; prototypeomfanget for disse avklares før utviklingsoppgaver opprettes. Kriteriene dokumenterer ikke ferdig utviklede eller beståtte funksjoner.

| ID | Akseptansekriterium |
|---|---|
| AK01 | Minst to PC-er kan spille sammen på LAN uten internett eller sentral konto; senere prøves opptil 12 spillere. |
| AK02 | Spillere kan fiske fra samme og forskjellige båter. Kapteinen kan velge fritt gyldig sjøpunkt, og båten navigerer automatisk. |
| AK03 | Rigging, båtens samlede stangkapasitet og grensen på tre aktive stenger per spiller håndheves. |
| AK04 | Stenger fisker passivt, napp og kroking skjer automatisk, og første utras krever ingen handling. |
| AK05 | Spilleren kan pumpe, sveive, justere brems, miste fisk og lande med én handling. Artsatferd er merkbart forskjellig. |
| AK06 | Fullt lager, tillatt overlast og sleping utløser retur. Kamper avsluttes og stenger trekkes inn; sleping gir lavere fart. |
| AK07 | 20-minuttersklokken er felles. Levering innen minutt 17 registreres; for sen retur taper hele dagens fangst uten tap av tidligere verdier. Felles avsløring og oppgjør skjer ved minutt 17. |
| AK08 | Ingen fangst selges og ingen fangstpremie eller veddemålsgevinst utbetales før minutt 17. Ved oppgjøret selges kvalifisert fangst én gang, og inntekten kan brukes. Gjentatte oppgjør gir ingen dobbel kreditering. |
| AK09 | Karriere, oppsett, journal, rekorder og markeringer bevares etter omstart og ved bruk på en annen LAN-server. |
| AK10 | Realistisk 3D, stangbøy, line, fiskebevegelse og lyd gir sammenhengende signaler. HUD viser ingen utholdenhetsbar og røper ikke dagens skjulte fangstresultater. |
| AK11 | Tidlig tilbakekomst gir tilgang til strandbaren mens andre fortsatt fisker. Resultattavlen skjuler dagens resultater til minutt 17. |
| AK12 | Én manglende tilslutning hindrer reisehopp. Ved enighet får båten samme serverstyrte ankomsttid, og andre spillere ser den fysiske reisen. Kapteinen kan starte retur uavhengig av avstemningen. |
| AK13 | En startbåt kan velge et gyldig offshorepunkt. Reiseprognosen gjenspeiler avstand og redusert slepefart. Muligheten for storfangst prøves mot valgte balanseverdier. |
| AK14 | Spillere har hvert sitt navngitte naust, kan besøke hverandre og beholder naustets kosmetiske progresjon ved omstart og bytte av LAN-server. |
| AK15 | En spiller i land kan satse på en fisker som fortsatt er ute. Innsatsen trekkes én gang. Bare fangst levert innen fristen teller. Pottfordeling og særtilfeller prøves etter avklaring. |
| AK16 | En tidlig returnert fisker og mannskapet får ikke dagens bekreftede fangstresultater via HUD, journal, statistikk, trofévegg, varsler eller saldo. Ved minutt 17 får alle felles avsløring. Kjøp før oppgjøret bruker bare tidligere disponible penger. |

### Testrekkefølge og observasjoner

Test først én spillers kamp, deretter en komplett fiskedag, så LAN med 2–4 spillere og til slutt belastning med 8–12. Prøv flere samtidige kamper, flere båter på samme plass, kapteinens retur under andres kamp, fullt lager og sleping med knapp tidsmargin.

Registrer napp, landinger, tap, returresultater, nettoinntekt og tid brukt på faktisk fiske. Undersøk om spillerne vil starte en ny dag. Kontroller også frakobling og krasj rundt oppgjør, gjenoppretting fra sikkerhetskopi og journal med mange fangster.

Prøv dessuten tidlig levering uten saldoøkning, skjulte dagsdata i alle visninger, samordnet avsløring ved minutt 17, korrekt rekkefølge mellom fristkontroll og oppgjør og gjenopptakelse etter krasj uten dobbel utbetaling. Ved reisehopp sammenlignes serverens ankomsttid og en observatørs visning. Test også naustprogresjon på en annen LAN-server når denne funksjonen inngår i omfanget.

## 10 Åpne avklaringer og kilder

### Valg som må avklares før de låses

- Unreal Engine 5 er valgt i den godkjente arkitekturen. Minstekrav til maskinvare, bildefrekvens og øvrige ytelsesgrenser må fortsatt fastsettes.

- Endelige tastetildelinger. Den oppgitte musekontrollen og mellomrom for landing er forslag; v4.1-plansjens mellomrom for pumping er en motstridende skisseetikett.

- Båtpriser, artspriser per kilo, rekordpremier, startøkonomi, agnforbruk og andre driftsutgifter. Testpriser skal ikke behandles som ferdig balanse.

- Kampvarigheter, artsvekter og endelig variant av tunfisk. Plansjen viser snapper 2–20 kg, amberjack 10–50 kg og tunfisk 30–120 kg som foreløpige referanser.

- Overlast for sportsfiskebåt, reduksjon i fart under sleping, hvordan slepefisk kombineres med last, og utfallet for fisk som ikke kan fraktes.

- Hvordan grenseankomst ved minutt 17 registreres, og hvilken sone som regnes som marina. Selve returfristen er fast.

- Inntektsfordeling, fiskens eierskap og stangplassfordeling på delte båter; detaljert håndtering ved frakobling.

- Hvordan kjøpt ekstra stangholder påvirker båtens kapasitet. Oppgitt stangkapasitet skal være tydelig for spilleren.

- Navn og innhold for de fem områdene i fullversjonen, grouperens kampstil og detaljert omfang etter prototypen.

- Omfang av grunnleggende hopp, kinematikk og vibrasjon i første prototype; avanserte hopp og effekter er utsatt.

- ~~Innsatsgrenser, pottfordeling ved ulike innsatser, lik størstevekt, ingen riktige tips og ingen tellende fangst.~~ Avklart 8. november 2026, se nederst.

- Konkrete avstander og sannsynligheter for storfangst offshore.

- Plassering av personlige naust i ulike serveres marinaer, priser på kosmetikk og betjening av besøk og bevegelse i marinaen.

- Omfang og prioritering av strandbar, levende reiser, offshorebalanse, naust og veddemål i første prototype. Faste kameraer, hemmelige dagsresultater og oppgjør ved minutt 17 er godkjente regler.

### Endringer i versjon 1.1

Fem ideer er innarbeidet: strandbar, levende båtreiser, større muligheter og risiko offshore, personlige naust og veddemål. Dagens bekreftede fangstopplysninger, journal, rekorder og resultater skjules for alle til minutt 17. Levering registreres ved ankomst, mens automatisk salg og alle fangstrelaterte utbetalinger venter til felles oppgjør. Tidligere opptjente penger kan brukes før oppgjøret.

### Kildegrunnlag og samordning

K1: Samtalen «Planlegg multiplayer fiskespill», v3.8. Enkel statistikk, journal, rekorder, marina og permanent karriere.
K2: Samme samtale, v4.0. Første LAN-prototype, foreløpige testverdier, avgrensning og testplan.
K3: Samme samtale, brukerens valg av realistisk 3D og bildeplansjen «v4.1 Visuell designspesifikasjon – Realistisk 3D prototype».
K4: Brukerens konsoliderte rekonstruksjon av tidligere krav og v4.1, gitt i denne oppgaven 3. november 2026.
K5: Prosjekteierens samlede idébank IDÉ-001–IDÉ-005, 3. november 2026.
K6: Prosjekteierens avklaring om å skjule alle dagens fangstresultater til felles avsløring ved minutt 17.
K7: Prosjekteierens beslutning om å vente med salg og utbetaling til minutt 17, slik at saldoen ikke røper fangsten.
K8: Den senere godkjente arkitekturen og masterpromptens valg av Unreal Engine 5.


Kapittel 2 og testprisene bygger på K2. Statistikkdetaljene bygger på K1, med utsatt resultatvisning etter K6. Den visuelle retningen bygger på K3 og K4. K5–K7 styrer de nye sosiale funksjonene, faste kameraer, hemmelige resultater og utsatt oppgjør. K8 bekrefter motorvalget. Øvrige krav er videreført fra v1.0.

Samordning: Dagens bekreftede art, eksakte mål, verdi og resultater skal skjules til minutt 17, også etter landing og tidlig levering. Salg, fangstpremier og veddemålsgevinster venter til samme tidspunkt. Dette erstatter v1.0s tidligere resultatvisning og umiddelbare salg. Statistikk skal fortsatt være enkel, journalen detaljert og den første prototypen i realistisk 3D.


## Avklaring 4. november 2026 — ankomst i marinaen

Ankomst registreres av LAN-serveren når båtens midtpunkt krysser grensen til den synlige marinasonen. Ankomsttid mindre enn eller lik 1 020 000 ms fra fiskedagens start gir levering innen fristen. Serveren behandler kryssingen før oppgjøret, også når begge behandles i samme ramme. Klientens melding eller nettverksforsinkelse endrer ikke ankomsttidspunktet. Sonens størrelse er en testverdi som balanseres senere.

## Avklaring 8. november 2026 — veddemål

Prosjekteieren godkjente særreglene i [#15](https://github.com/hansarne-lang/fiskespillet/issues/15). Reglene står i DECISIONS.md. Kort: fast innsats på $50, $100 eller $250, ett veddemål per spiller per dag, deling i forhold til innsatsen, deling ved lik størstevekt, tilbakebetaling når ingen har riktig tips eller ingen tellende fangst finnes, avrunding i øre med restøren til størst innsats og deretter første registrering, og utbetaling som egen kvittering også etter frakobling.

Regneeksempler. Alle bevarer hele potten:

| # | Situasjon | Utfall |
|---|---|---|
| 1 | A $250 og B $50 på X, C $100 på Y. Pott $400. X leverer største fisk | A 250/300 × $400 = $333,33 + restøre $0,01 = **$333,34**. B 50/300 × $400 = **$66,66**. C $0. Sum $400,00 |
| 2 | A $100 på X, C $100 på Y, B $100 på Z. X og Y leverer begge 28 200 g, som er største vekt. Pott $300 | A **$150**, C **$150**, B $0. Sum $300 |
| 3 | Alle har satset på X eller Y, men Z leverer største fisk | Alle får innsatsen tilbake |
| 4 | X ankommer 17:00:05 | X sin fangst teller ikke. Største av de andre fangstene som er levert i tide, avgjør. Leverte ingen i tide, får alle innsatsen tilbake |
| 5 | Oppgjøret kjøres på nytt etter omstart | Samme kvitterings-ID gir ingen ny utbetaling |

Word-utgaven av kravspesifikasjonen er ikke oppdatert med denne avklaringen. Markdown-utgaven og DECISIONS.md gjelder.

