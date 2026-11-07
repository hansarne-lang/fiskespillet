# v0.19 – Fiskeflåten

Spillbar Windows-utviklingspakke fra #104. Inntil tolv spillere kan være tilkoblet samtidig. Hver karriere kan hente én egen aktiv båt i samme økt; flåten har maksimalt tolv båter. Du kan fortsatt spille alene eller dele båt med venner.

## Kom i gang

1. Start samme v0.19-pakke på alle PC-ene. Velg Spill på verten, og koble til vertens LAN-adresse på de andre.
2. E går om bord i den valgte båten. Tab åpner flåten. Piler/klikk velger båt, Enter går om bord, C henter og går om bord i din egen båt. L går i land ved startbrygga. Tab/Escape lukker oversikten.
3. Startbåten har to plasser og er tilgjengelig uten nytt kjøp. Du kan bruke allerede kjøpt offshorebåt fra P-butikken når du eier båten ved startbrygga. Den har fire plasser/seks stangholdere. Tidligere priser og inntil tre egne stenger gjelder fortsatt.
4. O viser turplanen til båten du er om bord i. J viser dens ekspedisjon. K viser mannskapet og kapteinen. Bare kapteinen velger reisemål og starter R fra 02:00.
5. F starter fisket ved ankomst. F11 viser din båts last/slep. Båtene kan fiske, flytte seg og vende hjem uavhengig av hverandre.

## Egne reiser, felles døgn

Alle bruker samme normale 20-minuttersdøgn. Hver båt har egen kaptein, stangfordeling, rute med inntil fem stopp, ekspedisjonsmål, avreise, reisestemmer, fangstlager og hjemreise. En full eller slepende båt tvinger bare sitt eget mannskap hjem.

Før 17:00 er nøyaktig fangst, vekt og verdi skjult. Flåtetavlen viser bare offentlig eier, båttype, mannskap og reisestatus. Ved 17:00 avgjøres hver fangst etter om båten som tok imot den krysset marinaen innen fristen. En sen båt påvirker ikke inntekten til en tidlig båt. Bare egen kvalifisert fangst gir inntekt.

Bytt båt eller mannskap ved startbrygga før avreise. Etter at båten har reist, er mannskapet bundet til turen til neste døgn, også etter hjemkomst. Naust og strandbar kan brukes etter hjemkomst. Frakobling fjerner spilleren fra mannskapet; båt, last og oppgjørsgrunnlag blir hos verten. En gjenoppkoblet karriere beholder eierskap. Har profilen reist i dette døgnet, kan den ikke gå om bord i en annen båt før neste dag, heller ikke via E eller ved å hente egen båt. Dette hindrer motstridende ekspedisjoner/oppgjør for samme profil og døgn. Bekreftet journal/inntekt hentes fra vertens arkiv. Uoppgjort last overlever fortsatt ikke vertskrasj.

Verten starter neste døgn med N etter 20:00 og ferdig lagret oppgjør for tilkoblede spillere. Alle båter, lager og ekspedisjoner nullstilles samtidig; utstyr, valgte båttyper, mannskap og ruteoppsett beholdes.

## En større marina

Marinaen har tolv adskilte naustplasser i tre rekker. H åpner eget naust; PgUp/PgDn besøker alle tolv, mens 1–4 beholder direkte besøk til de første fire. N inne i naustet viser de ni permanente pyntevarene fra v0.18. Egne lys, brygger, utstillinger og artsrekorder følger profilen.

B åpner strandbaren ved marinaen. Inntil tolv navn og offentlig status vises mens andre båter er ute. Fem faste hilsener og tavlen etter 17:00 følger med. Dette er fortsatt en forenklet sosial marina, ikke fri gange eller et komplett sosialt fullspill.

## Tester og grenser

Pakken inneholder TESTRESULTATER.json med faktisk testet kilde og EXE-kontrollsum. Utviklingsprøven bruker tolv native profiler på én fysisk PC, tre forskjellige fiskebåter og normal tid, med katalogfisk satt til 150 kg for å kontrollere separat last/slep og sen retur. Napp, kamp, reise og døgn er ikke spolt. Dette er ikke et bevis på naturlig fangstfordeling eller fysisk tolv-PC-ytelse.

Begge PC-ene må ha v0.19/protokoll 19. Eldre klienter avvises. Lagrede karrierer, naust, ruter, journal og fotovinkler bruker eksisterende formater. Lagringer er lokale; kopier din egen profil ved bytte av PC. GitHub synkroniserer kildekode, ikke lagringer. Båtplasser/eierskap i den aktive flåten gjelder økten og er ikke en egen permanent båtliste.

Fysisk LAN/ytelse på to eller flere PC-er (#9), ferdig realistisk grafikk, full sosial strandbar, avklarte veddemål, lisensbeslutning og Unreal-bygg i CI gjenstår. Ingen nye priser, kreditter, betalte tjenester eller offentlig binærrelease er lagt til.
