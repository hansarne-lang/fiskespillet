# FISKELAGET – Samlet idébank

Status per 3. november 2026 · 5 hovedideer

Idébanken utvider designgrunnlaget gjennom v4.1 med sosialt liv i marinaen, levende båtreiser, større risiko offshore, personlige naust og veddemål i strandbaren. Statusene nedenfor gjengir prosjekteierens valg. Valg av hvilke funksjoner som inngår i første prototype, er fortsatt åpent.

Dette er et tillegg til kravspesifikasjonen v1.0. Ved motstrid gjelder den nyeste uttrykkelige brukerbeslutningen. GitHub Issues er fortsatt den autoritative listen over utviklingsoppgaver; idébanken beskriver spilldesign.

Ideene og de senere avklaringene er innarbeidet i [kravspesifikasjon v1.1](FISKELAGET_Kravspesifikasjon_v1.1.md). Ved minutt 17 skjer felles avsløring, automatisk salg og utbetaling av fangstpremier og veddemålsgevinster. Før dette brukes bare tidligere disponible penger til handel og innsatser.

## IDÉ-001 – Den tropiske strandbaren

**Status: Valgt**

Spillere som kommer tidlig tilbake til marinaen, skal kunne slappe av i en tropisk strandbar. Karibisk musikk, drinker, bølgeskvulp, fugleliv og havutsikt skal skape stemningen. Baren skal være et sosialt samlingspunkt for spillere som er ferdige med fiskingen.

### Tilleggsidéer

- Se andre spilleres båter komme inn i marinaen i sanntid.
- Se store fisker som slepes inn.
- Se en resultattavle med dagens fangster.

Resultattavlen skal avsløre dagens fangstresultater samlet ved minutt 17, som beskrevet i IDÉ-005. Tidlig levering skal ikke avsløre resultater.

## IDÉ-002 – Den levende båtreisen

**Status: Designvalg fastsatt**

Reisen fra brygga til fiskeplassen skal oppleves som en sammenhengende 3D-reise med levende omgivelser.

- Morgenstemning på brygga og motorstart.
- Automatisk utkjøring fra marinaen.
- Akselerasjon, kjølvann, bølger og motorlyd.
- Flygefisk, pelikaner, fregattfugler og andre båter.
- Faste kameravinkler og ingen fri bevegelse om bord.

Reisescenen skal kunne hoppes over dersom alle om bord er enige. Hoppet skal bare endre presentasjonen: Serverens reisetid fortsetter uendret, og hoppet gir ingen ekstra fisketid. Andre spillere skal fortsatt se båten fysisk reise.

Dette presiserer tidligere kameraforslag: Fri kameravisning om bord erstattes av faste kameravinkler. Kapteinens rett til å starte retur gjelder fortsatt; enighet om å hoppe over reisescenen er et separat valg.

## IDÉ-003 – Langt ut, stor gevinst

**Status: Prinsipp valgt**

Jo lenger fra marinaen spillerne fisker, desto større skal muligheten være for enorme og verdifulle fisker. Lengre avstand skal samtidig innebære større risiko.

- Lengre reisetid gir mindre tilgjengelig fisketid.
- Større fisk kan gi lengre og vanskeligere kamper.
- Sleping kan redusere hjemreisefarten.
- Returfristen ved minutt 17 er absolutt.
- Store fangster kan gi betydelig økonomisk gevinst.
- Bedre båter gir fordeler gjennom fart, lastekapasitet og slepeevne.
- Små båter skal fortsatt kunne prøve seg langt offshore.

Konkrete sannsynligheter og avstander skal balanseres. Prinsippet gir økte muligheter, uten å garantere storfangst ved lang reise. Reglene om tap av hele dagens fangst ved for sen retur gjelder fortsatt.

## IDÉ-004 – Personlige naust

**Status: Godkjent**

Hver spiller skal få sitt eget naust langs brygga, med spillerens navn i store bokstaver over inngangen. Alle skal starte med et enkelt naust som fungerer som spillerens personlige hjem.

Naustet skal ha:

- Utstyrsrom for stenger, sneller og agn.
- Trofévegg med personlige rekorder.
- Veranda med utsikt mot havet.
- Mulighet for besøk fra venner.
- Kosmetiske oppgraderinger kjøpt med opptjente fiskepenger.

Naustets progresjon skal følge den permanente karrieren mellom LAN-servere. Detaljene for plassering av naust i den aktuelle serverens marina er ikke bestemt.

## IDÉ-005 – Strandbarens veddemål

**Status: Hovedregel godkjent**

Etter egen ankomst til marinaen skal spilleren kunne vedde fiskepenger i strandbaren på hvem av fiskerne som fortsatt er ute, som leverer den største enkeltfisken.

- Bare fiskere som fortsatt er ute, er aktuelle ved registrering av veddemålet.
- Innsatsen skal trekkes fra spillerens fiskepenger.
- Vinnerne skal dele hele potten.
- Oppgjøret skal avgjøres ved minutt 17.
- Bare fisk levert innen returfristen skal telle.
- Dagens fangstresultater skal holdes skjult for alle fram til felles avsløring ved minutt 17, også for fiskeren selv og mannskapet.
- LAN-serveren skal registrere veddemål og forhindre dobbelt utbetaling.

### Godkjent felles avsløring

Prosjekteierens avklaring 3. november 2026 er at alt av fangstresultater skal skjules fram til slutten av sjøfasen. Minutt 17 er tidspunktet for felles avsløring og oppgjør; fiskedagen avsluttes fortsatt ved minutt 20.

Bekreftet art, eksakt vekt og lengde, fangstverdi, dagsplasseringer og nye fangstrekorder skal først vises ved avsløringen. Det gjelder også fangstjournal, resultattavle, rekordvarsler, trofévegg og andre visninger som ellers ville røpet dagens resultater. Tidligere dagers bekreftede karrieredata kan fortsatt vises.

Serveren skal registrere fangstene og kontrollere levering og frist fortløpende. Båter, fiskekamper, synlige fisker og sleping kan fortsatt observeres som del av den levende spillverdenen. Slike synsinntrykk kan gi spillerne grunnlag for å gjette, mens de bekreftede fangstresultatene forblir skjult.

Denne beslutningen erstatter de tidligere reglene om bekreftelse av art og eksakt vekt ved landing, offentliggjøring av vekt ved levering og umiddelbare varsler om dagens fangstrekorder. Bare fangster levert innen fristen kan telle i veddemålsoppgjøret.

Prosjekteierens neste avklaring fastsetter også at salg og utbetaling skal vente til minutt 17. Levering registreres ved ankomst, men gir ingen forskuddsbetaling eller foreløpig økning i disponibel saldo. Fangstinntekt, fangst- og rekordpremier og veddemålsgevinster blir disponible ved oppgjøret. Tidligere opptjente penger kan brukes før dette; kjøp og innsatser trekkes når de gjennomføres.

### Åpne beslutninger

- Innsatsgrenser.
- Fordelingsmodell når vinnerne har satset ulike beløp.
- Hva som skjer hvis ingen tipper riktig.
- Hvordan lik størstevekt og tilfeller uten tellende fangst håndteres.
- Hvem som regnes som fiskens eier når flere deler båt eller hjelper ved landing.

De tre første punktene er uttrykkelig uavklarte i idégrunnlaget. De øvrige følger av samordningen med fangst- og LAN-reglene og er spørsmål til senere avklaring.

## Samordning med tidligere krav

Den felles 20-minuttersdagen, automatisk navigasjon, permanent lokal karriere og serverens autoritet over økonomi og fangster gjelder fortsatt.

| Område | Betydning for kravspesifikasjonen |
| --- | --- |
| Dagsrytme | Tidlig tilbakekomst gir tilgang til strandbaren. Minutt 17 forblir fristen for levering og tidspunktet for veddemålsoppgjør. |
| Kamera og reise | Faste kameravinkler erstatter forslaget om fri visning om bord. Reisehopp endrer ikke serverens klokke eller båtens fysiske ferd. |
| Offshorefiske | Avstand skal påvirke muligheten for storfangst og risikoen ved retur. Eksisterende reisetider er fortsatt testverdier. |
| Marina | Strandbar og personlige naust utvider marinaens funksjoner. Hvordan besøk og bevegelse i marinaen betjenes, må konkretiseres. |
| Karriere | Naust og kosmetiske oppgraderinger skal følge karrieren mellom LAN-servere. |
| Fangstopplysninger | Dagens bekreftede fangstresultater skjules for alle til felles avsløring ved minutt 17. Personlige landingsrekorder alene kvalifiserer ikke en fangst til veddemålsoppgjør; levering innen fristen kreves. |
| Økonomi | Innsatser, pott og utbetalinger inngår i serverens transaksjonskontroll. Levering registreres ved ankomst, mens salg og utbetaling venter til minutt 17. Tidligere disponible penger kan brukes før oppgjøret. |
| Prototype | Idéenes status avgjør designretningen, men utvider ikke automatisk omfanget for første spillbare prototype. |

## Foreslåtte akseptansekriterier

Kriteriene er forslag til senere utviklingsoppgaver. De beskriver ønsket oppførsel, ikke gjennomførte tester.

| Idé | Forslag til demonstrasjon |
| --- | --- |
| IDÉ-001 | En spiller som har kommet tilbake tidlig, kan bruke strandbaren mens andre spillere fortsatt fisker. Resultattavlen avslører dagens fangstresultater først ved minutt 17. |
| IDÉ-002 | Med flere spillere om bord hindrer én manglende tilslutning at reisescenen hoppes over. Ved enighet får båten samme serverstyrte ankomsttid som ved vanlig visning, og en annen spiller ser båten reise. |
| IDÉ-003 | Kart og returprognose viser konsekvensen av lengre avstand og redusert slepefart. En startbåt kan velge et gyldig offshorepunkt. Sannsynligheter testes etter at balanseverdier er valgt. |
| IDÉ-004 | To spillere har hvert sitt navngitte naust og kan besøke hverandre. Naustets kosmetiske progresjon bevares ved omstart og bruk av karrieren på en annen LAN-server. |
| IDÉ-005 | En spiller i land kan satse på en fisker som fortsatt er ute. Før minutt 17 får heller ikke fangstens eier eller mannskap se dagens bekreftede fangstresultater, journalopplysninger eller rekordvarsler. Tidlig levering gir ingen fangstinntekt på saldoen. Ved minutt 17 skjer felles avsløring, salg og utbetaling av premier og veddemålsgevinster. Innsatsen trekkes én gang, for sent levert fisk teller ikke, og gjentatt behandling av samme oppgjør gir ingen ekstra utbetaling. Fordeling og særtilfeller testes etter avklaring. |

## Visuelle referanser fra idégrunnlaget

Lenkene nedenfor er beholdt som inspirasjon fra innspillet.

- IDÉ-002: [TOURNAMENTS | Walker's Cay](https://images.openai.com/static-rsc-4/WXxPzih1DkXPRRiLSXEoQA1krHipys-S3NPJcatoqrHa6jvNPYXlVo6hcQRUKLdd6vZTyqdvpM6qHqPU7DIdii3PgXZ5hRMg4npjSKLSgxkBXUWfScDeFZEVt4sJI6skl1SuDg-q3FMDpjwuKDPKPbxNSqS8JHm6B9LVRQBNOfY?purpose=inline)
- IDÉ-003: [shadcnui](https://images.openai.com/static-rsc-4/Xi4-eqq58H-FRxLJg9Fx6ClWxK9qk6vben3-JukgwIEM8YZHqRMb_Jrjm_BdtMziqpb0b_WIFZBIYJSRlOVsGxbX9vCsiE8dQHX_xgvDi-HNbyyvc2DLL8_1yl818OkZDpEtUofdH_IC15VXyF3XaLrvvX4x8298NhhzNOD52Tc?purpose=inline)
- IDÉ-004: [TOP 10 Things To Do in Placencia, Belize](https://images.openai.com/static-rsc-4/uwegRNz7bMNPq0Mx6Vp-qKMdhfAxUt9Uo0nPHFAiLk47hwskY59qifrW4QqEVY5Jxg2TqDlqM1bMXGiKOzsdWbN_X1o_CMrlSv7T_7SSq-dDsDUbNPSYJQn-PKdW_jQENGYmLcfZc7-AkbfIa92sgUDVPZMMAwUgK6stAI1tmCo?purpose=inline)
- IDÉ-005: [Karibik: St. Martin - Trauminsel mit zwei guten Seiten | YACHT](https://images.openai.com/static-rsc-4/vsFV1t_BhOegLjr7a4ZPVmaCnsgrz_IwV_YOqZ82r1OvYGOrfjFvgqC42OeHpWXZ9QjWZiR4bI81jQhuq3GeTtHTGbnt1AeJ8UsWiw8_hLB3R2p3FkOkA-PwntumX8uFc8B8vy-iiB6On_BWw0-Pe1Efb7PNLPA20rSQbqPILrU?purpose=inline)
