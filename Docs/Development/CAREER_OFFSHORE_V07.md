# v0.7 – Karriere og offshore

Eiergodkjent omfang og autoritativ arbeidsstatus: [GitHub #72](https://github.com/hansarne-lang/fiskespillet/issues/72). Bygger på v0.6 / b87a2c6. Dette dokumentet beskriver kontrakten; faktiske testresultater registreres i GitHub og leveransebevisene.

## Økonomi og varig profil

FiskCore eier `FFiskCareerProfile` og den begrensede kvitteringskjeden. Career eier filformat og atomisk lagring. Controller/GameMode er integrasjonsadaptere for nettverk og eksisterende oppgjør; UI får bare eierens bekreftede profil. Ingen ekstra fysisk UE-modul eller nettjeneste opprettes.

Ny profil får en UUID, navn og én engangskvittering på $30 000. Int64-beløp er i hundredeler, med maksimal saldo 10 milliarder dollar. Kjedens maksimum er512 kvitteringer inkludert startbonus; full kjede stanser endringer og viser feil, uten å kaste gamle duplikatvern. Hver post har unik ID, type, vare og fortegnet beløp. Revidert saldo og eide varer må kunne rekonstrueres nøyaktig fra hele kjeden. En vare kan kjøpes én gang. Prisene er $250/$1 000/$1 500/$25 000 for snøre/snelle/stang/offshore. Bonusene er15% større bruddgrense,15% raskere innsveiving og10% pumping, kombinert én gang.

Klienten laster sin lokale profil med backup og sender den på egen eierforbindelse. Serveren validerer kjeden og sammenholder den med `Authority/<profile-id>.ini`. Bare identiske kjeder eller en lengre kjede med identisk prefiks kan velges; forgrening blokkeres. Samme profil kan ikke være aktiv på to forbindelser. Importert offline-historikk kan ikke autentiseres kryptografisk mot en uavhengig LAN-server; dette er ingen sentral anti-juks-løsning.

Kjøp bruker kommando-ID, forventet revisjon og vare-ID. Serveren kontrollerer saldo, eierskap og at spilleren er i marinaen. En kopi endres og lagres hos verten før den publiseres. Klienten lagrer og kvitterer den nøyaktige profil-ID/revisjonen; ved lokal lagringsfeil beholdes ventende snapshot og retry. UI skiller lagringsfeil fra bekreftelse. Ny dag krever ferdig oppgjørskvittering og lokal lagringsbekreftelse fra hver aktiv profil.

Før17:00 finnes ingen ny fangstinntekt i profilpayload. Serveren binder spillers økt-ID til vedvarende profil-ID før fisking tillates. Ved avsløring summeres bare profilens egne kvalifiserte fangster. Dags-ID er kvitterings-ID: gjentakelse krediterer ikke på nytt, og konflikt avvises. Bindingen beholdes gjennom frakobling til neste dag, slik at gjenoppkobling i samme økt kan få riktig inntekt. Serverkrasj før oppgjør bevarer ikke uavsluttet dagsfiske; tidligere varige kjøp og oppgjør beholdes. Eksisterende journal etterbetales aldri som penger.

Lagring bruker formatversjon1 i egen `FiskelagetCareer.ini`, validerer både hovedfil og backup under lås og skriver validert midlertidig fil før atomisk flytting. Ugyldige/fremtidige filer og motstridende kjeder overskrives ikke. Eksisterende journal/favoritter/oppsett beholdes som egne filer. Journalens valgfrie båt-/utstyrsfelt har standard0 ved eldre poster. En eldre spillversjon kan ikke bruke den nye karriereøkonomien.

## Båter og stenger

Én aktiv båt per økt. Verten/eieren velger en eid modell ved brygga med plass til mannskapet. Startbåt2plasser/2holdere; offshore4plasser/6holdere og25% høyere fart (80% av reisetiden). Gjester har egen økonomi og kan bruke sin kjøpte båt som vert i en senere økt. Ingen flere samtidige båter, nytt fangstlager, overlast eller sleping inngår.

Offshorefordeling etter stabil mannskapsrekkefølge: én=3, to=3+3, tre=2+2+2, fire=2+2+1+1. Startbåtens v0.6-fordeling beholdes. Holderindeks er summen av tidligere mannskapsmedlemmers kapasitet pluss egen stangindeks. Hver stang har separat utsetting, rigg og kamp. Bytte slipper løft/sveiving; én kamp kan styres aktivt om gangen. Alle stenger nullstilles ved ny dag med riggene beholdt. 1/2/3 velger stang utenfor menyer; i G velges fortsatt lagringsplass.

## Sjøkart og miljø

Sjøpunkter bruker centimeter i et avgrenset kart. Land, marina, ugyldige koordinater og øyer med sikkerhetsavstand avvises. To synlige øyer og samme delte geometri brukes av kart, miljø og serverens ruteplanlegger. En liten synlighetsgraf velger korteste kollisjonsfrie rute rundt øyene. Båten følger polyline i virkelig tid; ingen reisehopp eller tidsbuffer innføres.

Nærmeste eksisterende habitat gir dybde og fiskemuligheter uten ny artsfordeling. Lengde/hastighet kalibreres mot de tre opprinnelige12/40/75sekundersrutene; nye punkter har4–150sekunder som testgrense før offshorefaktoren. Returmargin og marinasonekryssing bruker samme faktiske rutelengde/tid. Favorittformat2 lagrer koordinat og navn og migrerer format1-områder til deres gamle koordinat. Fremtidige formatversjoner avvises.

Miljøet utvider den kildebygde prototypen med marina, tropiske øyer, vannfarger, større båt, seks synlige holdere, fugler, flygefisk og prosedural motor-/hav-/fuglelyd. Ingen eksterne betalte ressurser eller produksjonsferdige realistiske modeller inngår. Skjult fangstart/vekt og utholdenhet avsløres ikke gjennom nye data. Faste kameraer og normal20minuttersdag/17:00-frist beholdes.

## Bruk og verifisering

P åpner karriere/butikk. Piler/klikk velger vare eller båt; Enter utfører, F5 prøver lagring igjen, Escape/P lukker. M viser kart og støtter klikk i sjøen; Tab åpner favoritter og F5 lagrer valgt koordinat. Eksisterende hjelp, oppgjør, journal, ekkolodd og tavle beholdes.

Modellprøver dekker kvitteringsbevaring, duplikater, kjøp, konflikter, penger, kapasitet, utstyr og øyomkjøring. Virkelig Editor-/spillbygg og grafiske solo-/fire-spiller-/normaldagsprøver kreves i tillegg. Ingen test påstås bestått bare fordi testkode er skrevet. Endelig guide og testresultater følger ZIP/BAT-leveransen; PR forblir til review/integrasjon.
