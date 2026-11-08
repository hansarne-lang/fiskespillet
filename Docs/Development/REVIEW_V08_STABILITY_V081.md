# Gjennomgang av v0.8 og rettelser i v0.8.1

Bestilt 6. november 2026, [oppgave 76](https://github.com/hansarne-lang/fiskespillet/issues/76). Gjennomgått [PR 75](https://github.com/hansarne-lang/fiskespillet/pull/75), grunnlag `8a85cdf3dbdfaa63c553b3bc711df7a539a096a2` til `cb57a51d26b7e47204d4c5fdaa2d0ed187c3f18a`. Én agent gjennomfører egen kodegjennomgang og feilretting; dette er ikke en uavhengig ekstern godkjenning.

## Funn og rettelser

| Prioritet | Utløsende situasjon og konsekvens i v0.8.0 | Rettelse |
| --- | --- | --- |
| P2 | Naustets hovedfil eller backup har et stort revisjonsnummer, men er ellers ugyldig. `Read` endrer ut-parameteren før hele filen er validert, og `Save` bruker tallet fra den ugyldige filen. En gyldig backup kan derfor ikke reparere hovedfilen; ved maks int kan inkrementet også flyte over. | Revisjonen publiseres først etter full validering; bare gyldige filer bestemmer neste revisjon. Grensen kontrolleres før inkrement. Regresjonsprøven bruker skadet hovedfil og backup med flere ugyldige revisjoner. |
| P2 | Verten har bekreftet nye naustvalg, men den lokale skrivingen feiler midlertidig. `Identity` beholdes på gamle verdier til en senere vellykket skriving. Lukking og gjenåpning viser de gamle valgene, som brukeren deretter kan lagre over de nye. En automatisk bekreftelse kan også overskrive et usendt utkast. | Serverbekreftet identitet oppdateres straks, lokal ventende skriving spores separat, og usendte utkast beholdes. Faktisk blokkert midlertidig fil og nye forsøk testes gjennom vanlig H-input og RPC. |
| P2 | Hjelpevinduet har tolv linjer etter marinautvidelsen. Siste linje står på y=610 og overlapper bunnteksten på y=620 ved 1280×720. | Redusert linjeavstand gir egen plass til alle kontrollene og bunnteksten. Kontrolleres med grafisk skjermbilde. |

Teststøtte: Marinaens skjermbildefunksjon venter kort etter et kameraskifte. Dette unngår at et naustbilde fra oppgjørsprøven viser forrige båtvinkel selv om riktig panel er åpnet.

## Gjennomgått omfang

Alle 27 endrede filer i PR75 er vurdert. Moduler og berøringspunkter:

- FiskCore: små repliserte marinafelt, tekst-/palettgrenser og stabile arts-ID-er.
- Lagring: profilbinding, hovedfil/backup, ugyldige og framtidige formater, revisjon og midlertidig filbytte.
- Controller og UI: eierskap, bryggekrav, meldingsrate, tastaturprioritet, navn, utkast, lagringsbekreftelser, kamera og menylukking.
- Marina: fire naustplasser, gjenbruk etter frakobling, lokal geometri fra replisert tilstand og maksimalt sju trofeer.
- Fiske: sju arter, habitatfordeling som summerer til 100, gamle arts-ID-er/priser, størrelses-/lengdevalidering og kamper med alle eksisterende utstyrsbonuser.
- Journal, mannskap, bootstrap, dagslys og nettverksversjon: koblingene til dagens avsløring, karriere, kart, stenger og eksisterende menyer.
- Tester, README og utviklingsdokumentasjon: kontroller samsvarer med den faktiske prototypen, og lokale nettverksprosesser skilles fra fysisk LAN.

Ingen andre konkrete feil er bekreftet i de gjennomgåtte endringene. Dette er ingen garanti for at spillet er feilfritt. Faktiske bygg, testresultater, kilde-SHA og arkivsjekker føres i den lokale leveranserapporten og oppgave76 etter gjennomført kjøring.

## Versjon og kompatibilitet

Produktversjon er 0.8.1. Nettverksfamilien beholder prosjektidentitet 0.8.0 og protokoll8 fordi rettelsene ikke endrer RPC-er, repliserte felt eller økonomiregler. Kompatibilitet med den faktiske v0.8.0-pakken skal prøves; eldre v0.7 skal fortsatt avvises. Konfigurasjonsversjon1 for naust, karriere og journal beholdes. v0.8.0-arkivet erstattes ikke.

## Kjente begrensninger videreføres

Fysisk to-PC-prøve er ikke del av denne bestillingen. Manglende journal fra et oppgjør mottatt mens spilleren var frakoblet etterfylles fortsatt ikke automatisk. Ubekreftet dagsfiske overlever ikke serverkrasj. Karriere har 512 kvitteringer; journal har 4096 poster / 4 MiB. Historiske importerte trofeer er kosmetiske, og har ingen global autentisering. Kosmetisk backup kan være forrige husutgave. Enkle kodebygde modeller, én aktiv felles båt og opptil fire LAN-spillere beholdes.

Neste innholdsforslag står i [v0.9-planen](EXPEDITIONS_V09_PLAN.md); det er ikke implementert av v0.8.1.
