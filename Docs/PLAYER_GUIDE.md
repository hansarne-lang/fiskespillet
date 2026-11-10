# FISKELAGET – spillerguide

Gjelder v0.22. Detaljene for hver versjon står i [versjonsguidene](#versjonsguider). Hva som er nytt i hver versjon, står i [endringsloggen](../CHANGELOG.md).

## Starte spillet

1. Installer og start FISKELAGET fra SpilLan, eller pakk ut hele Windows-pakken og start `Start-FISKELAGET.bat`. Unreal Editor trengs ikke. Fra SpilLan brukes spillernavnet ditt som standardnavn i naustet.
2. I hovedmenyen velger du med mus, piltaster eller Tab, Enter aktiverer og Escape går tilbake.
   - **Spill** starter en dag. Du kan spille alene, og venner kan koble seg til underveis.
   - **Bli med venner** kobler til vertens LAN-adresse. Standardporten er 7777. På samme PC brukes `127.0.0.1`.
   - **Innstillinger** og **Avslutt**.
3. Alle på LAN må bruke samme versjon. Inntil tolv spillere kan være med.

Lagringer (karriere, journal, naust, favoritter, oppsett) ligger lokalt på hver PC. GitHub synkroniserer dem ikke. Fra SpilLan ligger de i `%LOCALAPPDATA%\FiskELaget\Saved`. Slik flytter du lagringer fra en eldre pakke: se [SpilLan-oppskriften](Development/SPILLAN.md#lagring).

## Fiskedagen

En dag varer 20 minutter i sanntid. Klokken stopper ikke når menyer er åpne.

| Spillklokke | Hva skjer |
|---|---|
| 00:00–02:00 | Forberedelse ved brygga: gå om bord, velg plass, rigg stengene |
| 02:00 | Kapteinen kan reise ut med R |
| 17:00 | Absolutt frist: fangsten må ha krysset marinasonen. Felles avsløring og oppgjør for alle |
| 20:00 | Dagen er slutt. Verten starter neste dag med N |

Fangstresultater (art, vekt, verdi, rekorder) er skjult for alle, også for deg selv, til 17:00. Penger fra dagens fangst kommer først ved oppgjøret.

## Båt og reise

| Tast | Funksjon |
|---|---|
| E | Gå om bord, eller bruk naust, bar eller båt i nærheten |
| Tab | Flåten: velg båt med piler/klikk og Enter. C henter din egen båt, L går i land |
| K | Mannskap. Kapteinen kan gi rollen til en annen om bord |
| M | Sjøkart. A åpner hav-atlaset med 16 merker, Tab viser favoritter, F2 gir en favoritt navn |
| O | Turplan med inntil fem stopp. A legger til, Backspace fjerner siste, Delete tømmer, Enter velger neste stopp. T viser turforslag, L bruker forslaget, G finner korteste rundtur |
| R | Kapteinen: reis ut fra 02:00, eller dra hjem fra fiskeplassen. Knappen «Tilbake til marina» gjør det samme |
| J | Dagens ekspedisjon. 1–5 velger mål ved brygga, T markerer deg klar, 6–8 sender faste meldinger |
| F7 | Ekkolodd ved fiskeplassen |
| F11 | Fangstlageret. Full båt eller slep tvinger mannskapet hjem |
| F9 / F12 | Fotomodus / ta bilde |

Lukk kart og paneler før du trykker R.

## Fiske

| Tast | Funksjon |
|---|---|
| G | Stangoppsett: agn, dybde og program for ledig stang |
| Q | Fiskeassistent og lagrede stangsett |
| 1 / 2 / 3 | Velg din stang (opptil tre, avhengig av båt og mannskap) |
| F | Sett ut valgt stang |
| X | Trekk inn stanga eller avbryt kampen |
| Musa opp | Løft stanga (pump). Stanga blir stående der du slipper musa |
| Musa ned | Senk stanga. Snella sveiver inn av seg selv, men slurer under utras |
| Pil opp / pil ned | Løft eller senk stanga trinnvis uten mus |
| Venstre / høyre mus (hold) | Gammel knappestyring: løft og sveiv. Et klikk gir knappene kontrollen til du flytter musa igjen |
| Musehjul | Stram eller løsne bremsen |
| Mellomrom | Land fisken når den er nær og under kontroll |
| Å | AI-testhjelp: forsøker én vanlig kamp og landing etter napp. Å eller manuell kontroll tar over igjen. Finnes i Development-pakken, ikke i Shipping-bygg |

Napp kommer etter noen minutter og varsles med en tydelig lyd. Pump ved å føre musa rolig opp, og før den så ned mens snella sveiver inn. Du trenger ikke holde noen knapp. Måleren til høyre i kampfeltet viser stangvinkelen. Over gullstreken holder stanga spenningen når du lar musa ligge. Musstyrt stang og musfølsomhet (lav, middels, høy) velges under Innstillinger. Slår du av musstyringen, gjelder knappene som før. Stram ikke bremsen for hardt under utras, ellers ryker lina. Slakk line for lenge mister fisken. F6 i hovedmenyen åpner fiskeskolen, der du kan øve uten at det påvirker karrieren.

## Resultater og samlinger

| Tast | Funksjon |
|---|---|
| F1 | Kontroller |
| F2 | Dagens oppgjør (låst til 17:00) |
| F3 | Tidligere oppgjør i økten |
| F4 | Fangstjournal. F6 veksler mellom journal og artsrekorder |
| F8 | LAN-tavle |
| Y | Din varige fiskedagbok |
| L | Sjøpass med milepæler |
| I | Artsbok. Ved marinaen viser V 3D-galleriet |
| Z | Fangstsentralen med 30 samlinger |
| P | Karriere og butikk (utstyr og offshorebåt) |

## Marina og brygger

| Tast | Funksjon |
|---|---|
| H | Ditt naust. N åpner naustbutikken, der V forhåndsviser hele naustet. U viser lagrede stangoppsett |
| B | Strandbaren. 1–5 sender hilsener, V bytter mellom bar, havn og følg båt |
| T | Bryggeliv i land: W/A/S/D går, Shift løper, piltaster dreier kameraet, musehjul zoomer, Home nullstiller |
| F10 | I bryggeliv (trykk T først): havneguide til båt, strandbar eller ett av tolv naust |

Fra en båt ved marinaen går du i land med Tab, L, Tab.

## Generelt

- Escape lukker først det øverste panelet. Neste Escape åpner øktmenyen.
- Inne i et panel kan samme tast bety noe annet. Panelet viser hvilke taster som gjelder.
- Modeller og figurer er stiliserte. Ferdig realistisk grafikk er ikke påstått.

## Versjonsguider

Detaljer, grenser og testgrunnlag for hver versjon:

| Versjon | Guide |
|---|---|
| v0.22 | [Øyhav og havneguide](Development/ISLAND_HARBOUR_V022.md) |
| v0.21 | [Bryggeliv](Development/PIER_LIFE_V021.md) |
| v0.20 | [Levende havn](Development/LIVING_HARBOUR_V020.md) |
| v0.19 | [Fiskeflåten](Development/FISHING_FLEET_V019.md) |
| v0.18 | [Naustliv](Development/HARBOUR_LIFE_V018.md) |
| v0.17 | [Hav og storfangst](Development/OCEAN_CARGO_V017.md) |
| v0.16 | [Skipperens verktøykasse](Development/SKIPPERTOOLS_V016.md) |
| v0.15 | [Dyphavspakken](Development/DEEPSEA_V015.md) |
| v0.14 | [Fangstjakten](Development/COLLECTIONS_V014.md) |
| v0.13 | [Mangroveekspedisjonen](Development/MANGROVE_V013.md) |
| v0.12 | [Fiskerpakken](Development/ANGLER_V012.md) |
| v0.11 | [Skipperpakken](Development/SKIPPER_V011.md) |
| v0.10 | [Havutforskning og sjøpass](Development/EXPLORATION_V010.md) |
| v0.9 | [Ekspedisjoner](Development/EXPEDITIONS_V09_GUIDE.md) |
| v0.8 / v0.8.1 | [Marina og artsjakt](Development/MARINA_DISCOVERY_V08.md), [gjennomgang og rettelser](Development/REVIEW_V08_STABILITY_V081.md) |
| v0.7 | [Karriere og offshore](Development/CAREER_OFFSHORE_V07.md) |
| v0.6 | [Flere stenger, artsprofiler og kameraer](Development/FISHING_V06.md) |
| v0.5 | [Ekkolodd, favoritter, HUD og LAN-tavle](Development/NAVIGATION_V05.md) |
| v0.4 | [Fangstjournal og artsrekorder](Development/JOURNAL_V04.md) |
| v0.3 | [Flere fiskedager i samme økt](Development/SESSION_V03.md) |
| v0.2 | [Mer fiskeinnhold](Development/CONTENT_V02.md) |
| v0.1 | [Hovedmeny](Development/MAIN_MENU.md), [spill alene](Development/SOLO_PLAY.md), [full fiskedag](Development/FULL_DAY.md) |
