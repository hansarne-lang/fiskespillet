# v0.9 – Ekspedisjoner og fiskeforhold

Bestilt implementert av prosjekteieren 6. november 2026 gjennom «gjør 2 og 3», [GitHub77](https://github.com/hansarne-lang/fiskespillet/issues/77). Utviklingen følges i #85 og deloppgavene #80–#84. Dokumentet beskriver omfang og kontrakter; ferdigstatus og faktisk verifikasjon føres i leveransens testresultater. Eksisterende idébank og kravspesifikasjon erstattes ikke.

## Målet for spilleren

Hver fiskedag får et tydelig ekspedisjonsmål og mer variasjon i havmiljøet. Mannskapet planlegger reisen sammen, bruker egne stangoppsett og møtes ved oppgjøret. Samtidig skal forbindelsesbrudd gi bedre mulighet til å hente igjen bekreftede journaldata.

## Anbefalt rekkefølge

| Del | Konkret innhold | Når delen er ferdig |
| --- | --- | --- |
| 1. Bekreftet journal etter brudd | Verten beholder bekreftede fangstoppføringer per profil. Ved ny tilkobling kan spilleren hente manglende poster, deduplisert på fangst-ID. Pengekvitteringer behandles fortsatt separat. | Frakobling før/under/etter17:00 gir samme journal ved gjenoppkobling; gjentatt henting lager verken ekstra fisk eller penger. Gyldig lokal journal og backup bevares. |
| 2. Varierende havmiljø | Tre servervalgte forhold: rolig sjø, passatvind og tropisk regn. Synlig bølge-/regn-/lysvariasjon, værkort på kartet og tilpasset havlyd. Første del endrer bare presentasjonen. | Alle spillere ser samme dagsforhold. Tilgjengelig fisketid, båtfart, reisetid og17:00-frist er uendret. Grafikk-/lydvalg virker også på svakere PC. |
| 3. Ekspedisjonsmål | Fem oppdragstyper: art fra valgt habitat, ulike arter, felles fangst fra flere fiskere, ny personlig rekord og levering fra et valgt sjøpunkt. Mål velges ved brygga og evalueres fra serverens bekreftede oppgjør. | Dagens art, eksakte mål og resultat skjules til17:00. Mislykkede og sent leverte fangster håndteres tydelig. Samme dag kan ikke fullføres flere ganger. |
| 4. Personlig utstyrsrom | Naustpanelet får oversikt over eid utstyr, båter, artsfunn og tre navngitte stangoppsett. Oppsett kan hentes til valgt egen stang med eksisterende servervalidering. | Omstart, båtskifte og gjenoppkobling bevarer oppsettene. Ingen kan endre en annens stang. Kjøp følger fortsatt eksisterende priser og kvitteringer. |
| 5. Felles avreiseplan | Mannskapet ser valgt sjøpunkt, ekspedisjonsmål, returmargin og hvem som har markert seg klar. Enkle forhåndsdefinerte meldinger for avreise/retur. | Kapteinen beholder retten til å starte og returnere. Klokken pauses ikke, og ingen blir flyttet eller gitt ekstra fisketid ved klar-markering. |

Del1 bør ferdigstilles først fordi den lukker en kjent begrensning i v0.8. Del2–5 gir den nye innholdspakken. Hver del får egen underoppgave og en demonstrerbar leveranse før neste integreres.

## Fastlagte valg i denne pakken

- Oppdrag gir først medaljer og historikk. Et eventuelt pengebonussystem må få konkrete beløp, grenser og kvitteringsregler før det bygges. Forslag: ingen nye pengebonuser i første ekspedisjonspakke.
- Vær endrer først bare presentasjonen. Eventuell påvirkning på napp, kamp eller båtfart vurderes som en separat balanseendring; ingen skjulte multiplikatorer.
- Ett aktivt felles ekspedisjonsmål per dag, valgt av kapteinen ved brygga. Solospill bruker mål som kan fullføres alene; flerspillermål krever flere fiskere bare når de deltar.
- Journalgjenhenting krever en avgrenset vertskopi og migreringsplan. Lagringsgrense, kvitteringsgrense og sletting av gamle data skal være uttrykkelige; eksisterende filer skal ikke tømmes automatisk.

Konkrete kontrakter: journalarkiv per profil har maks 4096 fangster / 4 MiB; medaljehistorikk per profil har maks 512 ekspedisjoner / 1 MiB. Eksisterende karrieregrense på 512 kvitteringer beholdes. Ingen automatisk sletting eller utrulling av gamle data. Når arkivering ikke kan fullføres, venter neste dag. Ukjente nyere filformater og to ugyldige filkopier bevares. Journalformat 1 beholder gamle poster uten sjøpunkt; nye poster kan ha valgfrie koordinater. Stangoppsettformat 1 får valgfrie navn og leser eldre navnløse oppsett. Medaljefilen har format 1 og er bundet til profilens GUID.

Det finnes ett mål per dag. Habitatmålet krever én levert fisk av arten med høyest habitatandel ved valgt plass. Artsmålet krever to ulike leverte arter. Mannskapsmålet krever fangst fra to ulike profiler når flere dro ut, ellers én. Rekordmålet sammenligner levert fisk med samme profils tidligere bekreftede artsrekorder i denne vertens arkiv, fanget ved avreise. Første bekreftede fangst kan bli rekord. Sjøpunktmålet krever levert fisk landet innen fem meter fra valgt sjøpunkt. Avreisemannskapet får samme medalje ved fullføring; ingen ny pengebonus. Dagens faktiske fangstart, størrelse og resultat vises først ved 17:00; selve oppdragsteksten er synlig før avreise.

Været velges av verten og går rolig sjø → passatvind → tropisk regn over dager. Bare bølger, regn, lys, dis og lyd påvirkes. 30 FPS-valget halverer regnelementene. Klar-markering er frivillig og endrer ikke avreiserett, klokke eller fisketid. J åpner plan, T markerer klar, 6/7/8 sender faste meldinger med to sekunders mellomrom. U åpner eget utstyrsrom; 1/2/3 velger oppsett, F2 navngir, F5 lagrer bekreftet rigg og F9 ber verten bruke den på valgt egen stang. Lukk panelet før R.

Nettverksfamilie 9 brukes fordi nye replikerte strukturer og meldinger inngår. Alle i samme økt må bruke v0.9; v0.8.0/v0.8.1 og eldre skal avvises, mens lagrede data kan videreføres. Verten importerer aldri klientjournaler som grunnlag for betaling eller medaljer. Gjenhenting sendes i maksimalt 16 poster om gangen og kvitteres først etter varig lokal lagring.

## Kontroller og leveranse

Modell-/lagringstester for deduplisering, profilbinding, migrering og alle fem måltyper. Grafisk solo med én spiller og LAN med to/fire, inkludert frakobling ved17:00. Minst én full normaldag med hver værpresentasjon før ferdig pakke. Test at manglende/ødelagte data bevares, at menyer aldri stopper klokken, og at mål ikke røper dagens fangst før oppgjøret.

Windows ZIP/BAT, norsk guide, kilde-SHA, sjekksummer og faktiske testresultater følger leveransen. Fysisk LAN og ytelse på den andre PC-en føres som egne resultater når de faktisk er prøvd. Ingen dato eller ferdigstatus loves før delene er gjennomført.

## Avgrensning og forhold til idébanken

Planen viderefører sosial marina og personlige naust fra idé18/21 og reisen fra idé19, med samme faste kameraer og serverstyrte klokke. Reisescenehopp, betalte husoppgraderinger, nye båter, flåte, last/overlast, sleping, tolv spillere og strandbarveddemål inngår ikke i denne pakken. Disse trenger egne avklaringer og bør ikke blandes inn i journalgjenhenting eller oppdragsøkonomi. Eksisterende17:00-avsløring og20:00-dagsslutt beholdes.
