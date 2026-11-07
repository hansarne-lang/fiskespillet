# FISKELAGET v0.17 – Hav og storfangst

Windows-pakken starter fra BAT uten Unreal Editor. Alle LAN-deltakere må bruke v0.17.0 / protokoll17. Spill alene eller opptil fire sammen. E går om bord, M velger plass, R starter fra 02:00, F setter ut stang. F11 åpner fangstlageret. K åpner fortsatt mannskap/kapteinsbytte.

## Bedre 3D-presentasjon

Båtene har et formet skrog med smal baug og bred akterende, dekk med plankeskjøter, buede rekker, fendere, instrumentpanel og fiskeluke. Havet rundt båten har en bevegelig bølgeflate med overflatenormaler og værstyrt bølgehøyde. Fysiske bøyer markerer marinaen. En stor fisk på slep vises bak båten med slepetau og bevegelse i sjøen. Dette er fortsatt forenklet 3D, ikke ferdig realistisk grafikk.

F9 åpner fotomodus om bord. Piler, PgUp/PgDn og musehjul styrer bildet. F5/F8 lagrer/henter én av tre egne fotovinkler valgt med1/2/3. G viser tredelingsrutenett; H skjuler både rutenett og tekst; F12 tar PNG. Spillklokken går videre.

## Felles fangstlager

Lageret deles av hele båtens besetning, mens fisken fortsatt tilhører spilleren som landet den. Serveren registrerer hver unik fangst én gang.

| Båt | Vanlig lager | Tillatt overlast | Største enkeltfisk på slep |
|---|---:|---:|---:|
| Startbåt | 100 kg | 10 kg | 200 kg |
| Offshorebåt | 300 kg | 30 kg | 500 kg |

Dette er foreløpige testkapasiteter fra spillplanen. Offshorebåtens slepegrense er over dagens største katalogfisk; den beholdes som kapasitet for videre innhold.

En fisk som får plass, sikres i lageret. Når lageret når vanlig kapasitet eller tillatt overlast, avsluttes fiskingen for alle om bord og båten starter normal hjemtur. En enkelt fisk som er større enn vanlig lager pluss overlast, men innen slepegrensen, sikres på slep. Tidligere last beholdes. Sleping avslutter fiskingen og starter hjemtur med65 prosent av vanlig båtfart. Avrunding av returtiden er konservativ.

Alle aktive stenger og kamper trekkes inn ved tvungen retur. Allerede registrerte landinger beholdes. Nye landinger kan ikke legges til etter at tvungen retur er bestemt.

En fisk som ikke får plass og ikke kan slepes, registreres som landet, men ikke solgt. Har båten allerede last, starter normal hjemtur med denne lasten. Er båten tom og fisken overstiger slepegrensen, kan fiskingen fortsette etter at fisken er oppgitt. Dette er uttrykkelige testregler for tidligere uavklarte tilfeller.

## Hemmelighold, retur og oppgjør

F11 viser kjent båtkapasitet og lastestatus, men ikke eksakt fangstvekt, art eller verdi før17:00. Slepefisken har en generell silhuett, ikke en bekreftet artsmodell eller en nøyaktig lengde. En synlig stor fisk kan fortsatt gi grunnlag for å gjette.

Marinagrensen og fristen er uendret: båtens midtpunkt må krysse innen1020000ms. Redusert slepefart påvirker faktisk hjemreisetid og returmargin. Sen retur taper både lagret fisk og slepefisken. Inntekt utbetales bare for fraktbar fangst levert innen fristen, én gang ved17:00. Ingen forskuddsbetaling.

Journalens «Ikke solgt» omfatter både fisk tapt ved sen retur og fisk som ikke kunne fraktes. Landede fisk teller fortsatt i landet vekt og landingsstatistikk; inntekt teller bare solgt fisk. Gamle journal- og karriereformater beholdes. Fangstlageret gjelder den aktuelle fiskedagen og nullstilles ved nytt døgn; uoppgjort last overlever ikke vertskrasj.

## Øvrige funksjoner og testing

Alle15 arter, fem habitater,36 fangstmål, fiskeskole, butikk, naust, ekspedisjoner og turplanlegger følger med. O har16 atlasmerker, seks turforslag, korteste rundtur og tidsbudsjett. Y viser egen bekreftet dagbok og sammenligning med HTML-eksport. Z åpner fangstjakten.

Pakkens TESTRESULTATER.json dokumenterer faktisk testet kilde, motor, bygg og kjøretidsprøver. Lastprøvene bruker tydelig merkede katalogfisk med faste vekter for å nå grensene; nappklokker, fiskekamp, landing, reise og oppgjør kjører i vanlig tid. Fysisk LAN mellom to PC-er og ytelsen på den andre PC-en skal bare bekreftes etter faktisk prøving.
