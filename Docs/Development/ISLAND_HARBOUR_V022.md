# v0.22 – Øyhav og havneguide

T åpner bryggeliv når du er i land. Fra en båt ved marinaen går du i land med Tab, L, Tab. WASD går, Shift løper og E bruker naust, strandbar eller ombordstigning i nærheten. Klokken og det vanlige oppgjøret klokken 17:00 fortsetter.

F10 åpner havneguiden. Velg båt, strandbar eller ett av tolv naust med opp/ned og trykk Enter. Guiden følger gangveier rundt møbler og viser ruten i gull på minikartet. Ledige naustplasser er merket. Figuren løper med vanlig, serverbegrenset fart og stopper ved målet. E brukes fortsatt for å besøke eller gå om bord; guiden velger ikke mannskap, kjøper ikke noe og starter ingen båttur.

WASD overtar gangen straks. E avbryter guiden før nærhetsvalget. En annen meny stopper gåhjelpen, og den starter ikke igjen når menyen lukkes. F10 stopper en pågående rute og åpner målvalget. Escape lukker først målvalget; uten målvalg avslutter den bryggeliv. Om en hindring stopper fremdriften, gir guiden beskjed etter tre sekunder. Du kan gå selv eller velge målet på nytt.

C veksler mellom følgkamera, nærkamera og oversikt. Venstre/høyre pil dreier kameraet, musehjul justerer avstand og Home nullstiller både kameratype, vinkel og avstand. Faste hilsener på 1–5 kan sendes mens du går. De følger den samme serverkontrollen og pausen på to sekunder som i strandbaren. Minikartet nummererer naustene, og offentlige hilsener vises kort i bildet. Navn følger skjermens oppretning også fra skrå kameraer og på vippende båter.

Gange tar nå hensyn til figurens radius på 30 cm. Tolv barkrakker, fem bord og tretten bryggestolper deler gulvplan mellom grafikk og serverens sperrer. Korte bevegelsessteg hindrer at figuren hopper gjennom møbler ved lave bildefrekvenser. Modellen er fortsatt et flatt, avgrenset bryggenett; dette er ikke generell karakterfysikk, hopping, svømming eller fri gange på øyene.

De tre eksisterende øyene har egne terrengflater med strand, grønne skråninger, rygger og uregelmessig kystlinje. 192 steiner, 48 palmer og 288 palmeblader er samlet i fire instansgrupper. Kystskum og sjøfugler med to bevegelige vinger gir flere detaljer. Terreng og dekorasjoner ligger innenfor øyenes eksisterende navigasjonssperrer. Sjøreiser, fiskeplasser, værregler, fangst og økonomi bruker de samme reglene som før. Grafikken er fortsatt stilisert.

Testhjelpen fra v0.21 er med: Å etter napp lar en AI forsøke én vanlig fiskekamp. Å igjen eller manuell kamp gir deg kontrollen tilbake. Napp varsles med en original klokketone på egne stenger. R eller den klikkbare returknappen sender kapteinens båt hjem med vanlig reisetid. Ingen fangst eller inntekt offentliggjøres før 17:00. Testhjelpen er avslått i Shipping. Se også [v0.21-guiden](PIER_LIFE_V021.md) for detaljer om fiskehjelp og returknapp.

Alle PC-er må bruke v0.22/protokoll22. Eldre spillklienter avvises. Karriere, journal og naust beholder lagringsformatene; midlertidig gangposisjon, rute og kameravalg lagres ikke. Profiler ligger lokalt og synkroniseres ikke gjennom GitHub. Fysisk LAN og ytelse på en annen PC må fortsatt prøves. Bare bekreftede oppgjør kan hentes tilbake etter et vertskrasj.

Utviklingskontroll: `Tools/Testing/Run-HarbourGuide.ps1` kjører den pakkede EXE-en mot 14 mål, manuell overtakelse, menyavbrudd, kameravalg, vanlig ombordstigning, to nettverksspillere og tre ferske profilinnlastinger. Den tar separate skjermbilder av tre øyer med et rent testkamera; spillposisjon eller sjøreiser flyttes ikke av dette kameraet. Resultater må leses fra faktisk kjøring, ikke fra testens eksistens.
