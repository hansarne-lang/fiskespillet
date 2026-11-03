# FISKELAGET – Modulkontrakter

Status: godkjent kontraktgrunnlag v1.0 for videre implementasjon. Dette er dokumenterte datakontrakter, ikke implementert spillkode.

## Typer og felles regler

`Id` = UUID/FGuid, `Int32` = fortegnet 32-bit heltall, `Int64` = fortegnet 64-bit heltall, `Bool` = boolsk, `String` = begrenset UTF-8-tekst, `Enum` = lukket navngitt sett, `List<T>` = liste, `Optional<T>` = eksplisitt valgfri verdi. Vekt/last uttrykkes som `Int64` gram, lengde og avstand som `Int64` millimeter og pengesummer som `Int64` minste valutaenhet. Enhetsvalg er teknisk forslag og krever ingen endring av spillbalansen. Ingen flyttallsbasert pengeføring.

Alle hendelser har `EventId:Id`, `SchemaVersion:String` (starter 1.0), `SessionId:Id`, `DayId:Id`, `CorrelationId:Id`, `CausationId:Optional<Id>`, `ServerSequence:Int64`, `OccurredAtServerMs:Int64` og `Producer:Enum<modul>`. Tid er autoritativ, monoton server-/sesjonstid; systemklokke i UTC brukes bare i diagnose og journal. Sekvens er unik innen sesjonen. Hendelses-ID beholdes ved gjentatt levering.

Eiermodulen er eneste tillatte produsent etter servervalidering. Oppførte konsumenter abonnerer på immutable verdier; de skriver kun egne data. Klienter kan sende kommandoer, aldri utstede disse hendelsene som autoritative fakta. Privat serverpayload filtreres til klientlesemodeller. Dobbeltlevering av samme ID må være uten ekstra virkning; varige forbrukere fører mottakskvittering. Levering kan gjentas og er ikke «exactly once».

Alle radene arver felles felter, autoritet, duplikatregler og versjonering. Valideringsfeil gir avvist kommando med årsak og ingen hendelse. Forbigående leveringsfeil forsøkes begrenset på nytt; varige feil gir karantene, diagnostikk og kontrollert gjenoppretting. Oppgjør skal ikke rapporteres ferdig ved uavklart lagringsfeil.

## Hendelser

### FishHooked

- Eier/formål: Fishing; bekrefte automatisk kroking og første utras.
- Felter: `FightId:Id`, `RodId:Id`, `PlayerId:Id`, `BoatId:Id`, `HookedAtServerMs:Int64`.
- Avsender: serverens Fishing etter gyldig napp. Konsumenter: Multiplayer og UI gjennom filtrert kampmodell.
- Autoritet/feil: serveren kontrollerer aktiv stang, eier og gjeldende dag. Ukjent/allerede opptatt stang avvises; samme FightId oppretter ikke ny kamp. Ingen art/eksakt vekt i offentlig modell.
- Versjonering: felles 1.0-regler; ny påkrevd kampinformasjon krever major-versjon.

### FishFightStarted

- Eier/formål: Fishing; første automatiske utras er over og aktiv kamp kan begynne.
- Felter: `FightId:Id`, `RodId:Id`, `PlayerId:Id`, `BoatId:Id`, `ActiveAtServerMs:Int64`.
- Avsender: serverens Fishing. Konsumenter: Multiplayer og UI.
- Autoritet/feil: krever aktiv, kroket fisk og gyldig tilstandsovergang. Ugyldig rekkefølge settes til side for resynkronisering, ikke en ny FishHooked.
- Versjonering: 1.0; endring av overgangsbetydning krever major.

### CatchLanded

- Eier/formål: Fishing; landing bekreftet, og faktisk art/vekt kan registreres.
- Felter: `CatchId:Id`, `FightId:Id`, `PlayerId:Id`, `BoatId:Id`, `SpeciesId:Id`, `WeightGrams:Int64`, `LengthMm:Int64`, `LocationId:Id`, `PositionMm:Vector3<Int64>`, `DepthMm:Int64`, `BaitId:Id`, `ProgramId:Id`, `EquipmentSnapshot:EquipmentRecord`, `FightDurationMs:Int64`, `LandedAtServerMs:Int64`.
- `EquipmentRecord` har `RodId:Id`, `ReelId:Id`, `LineId:Id`, `HookId:Id` og `SetupRevision:Int32`.
- Avsender: serverens Fishing. Konsumenter: Boats, Career, Multiplayer og UI.
- Autoritet/feil: bare én CatchId per fullført FightId; ikke negativ vekt/lengde. Landing gir rekord/journalgrunnlag, men ingen salg før Boats har godkjent transport. Lagringsfeil beholder pending leveranse med samme ID.
- Versjonering: 1.0; art/vektenheter og eierskapssemantikk endres kun ved major.

### CatchStored

- Eier/formål: Boats; bekrefte at fangsten tilhører båtens godkjente transportmanifest.
- Felter: `CatchId:Id`, `BoatId:Id`, `PlayerId:Id`, `ManifestRevision:Int64`, `TransportMode:Enum<Onboard,Towed>`, `TotalOnboardGrams:Int64`, `CapacityGrams:Int64`, `OverloadGrams:Int64`.
- Avsender: serverens Boats etter validert CatchLanded. Konsumenter: Economy, Career, Multiplayer og UI.
- Autoritet/feil: én manifestpost per CatchId; manglende landing avventes/resynkroniseres. Kan ikke brukes til å legge samme fisk på flere båter. Utfallet for utransportabel fisk må følge godkjent designavklaring.
- Versjonering: 1.0; ny transportmodus krever kontrollert major-endring eller eksplisitt kompatibel utvidelse i alle konsumenter.

### TowingStarted

- Eier/formål: Boats; storfisk er sikret for sleping og retur må starte.
- Felter: `BoatId:Id`, `CatchId:Id`, `TowingWeightGrams:Int64`, `TowingLimitGrams:Int64`, `SpeedScalePermille:Int32` (1–999), `ReturnId:Id`.
- Avsender: serverens Boats. Konsumenter: integrasjonslaget, Multiplayer, UI og World-visning via adapter.
- Autoritet/feil: slepegrense kontrolleres; kapasitet og fart må komme fra godkjente data. En repetisjon starter ikke enda en returreise. Fishing stenges gjennom returkommandoen, ikke direkte dataskriving.
- Versjonering: 1.0; fartsskalaens enhet/endret transportbetydning krever major.

### ForcedReturnStarted

- Eier/formål: Boats; automatisk retur ved fullt lager, tillatt overlast eller sleping.
- Felter: `ReturnId:Id`, `BoatId:Id`, `Reason:Enum<FullHold,AllowedOverload,Towing>`, `TriggeredByCatchId:Id`, `ManifestRevision:Int64`, `StartedAtServerMs:Int64`.
- Avsender: serverens Boats. Konsumenter: Fishing, Multiplayer og UI.
- Autoritet/feil: godkjent lager-/slepetilstand kreves. Fishing avslutter kamper og trekker inn stenger idempotent. Navigasjonsfeil varsles; den absolutte fristen endres ikke.
- Versjonering: 1.0; ny grunn må godkjennes som spillregel og kontraktendring.

### VoluntaryReturnStarted

- Eier/formål: Boats; kapteinens godkjente returbeslutning.
- Felter: `ReturnId:Id`, `BoatId:Id`, `RequestedByPlayerId:Id`, `CaptainRevision:Int64`, `StartedAtServerMs:Int64`.
- Avsender: serverens Boats etter RequestVoluntaryReturn. Konsumenter: Fishing, Multiplayer og UI.
- Autoritet/feil: kapteinsrolle kontrolleres ved behandlingstidspunktet. Annen spiller avvises med `NotCaptain`. Foreldet rolleversjon krever ny lesing. Eksisterende returreise returneres ved gjentakelse.
- Versjonering: 1.0; endret kapteinsmyndighet krever eiergodkjenning og major.

### BoatArrived

- Eier/formål: Boats; registrere båtens faktiske ankomst til marina.
- Felter: `BoatId:Id`, `ReturnId:Id`, `MarinaId:Id`, `ArrivedAtServerMs:Int64`, `ManifestRevision:Int64`, `CatchIds:List<Id>`.
- Avsender: serverens Boats. Konsumenter: Economy, DayCycle, Career, Multiplayer og UI.
- Autoritet/feil: marinasone og ankomsttid kontrolleres på serveren. Klientens posisjon/klokke er ikke nok. Economy avgjør oppgjør med dagsfrist og manifest som samlet grunnlag; tilfeldig hendelsesrekkefølge gir aldri salgsrett.
- Versjonering: 1.0; ankomstsonen er et separat godkjent designvalg.

### DayDeadlineReached

- Eier/formål: DayCycle; absolutt returfrist ved dagminutt 17.
- Felter: `DeadlineAtServerMs:Int64`, `DayElapsedMs:Int64` (=1 020 000), `SettlementId:Id`, `BoatArrivalSnapshotRevision:Int64`.
- Avsender: serverens DayCycle, én logisk gang per DayId. Konsumenter: Boats, Economy, Career, Multiplayer og UI.
- Autoritet/feil: oppgjørsbarrieren fryser relevant ankomst-/manifestgrunnlag. Tidsmåleravvik varsles og gjenopprettes fra serverens tidsgrunnlag. Ingen unntak eller klientbuffer.
- Versjonering: 1.0; endret frist er spilldesign og kan ikke innføres som teknisk reparasjon.

### CatchSold

- Eier/formål: Economy; bekrefte en bokført salgsutbetaling.
- Felter: `CatchId:Id`, `BoatId:Id`, `PlayerId:Id`, `TransactionId:Id`, `SettlementId:Id`, `PriceRevision:Int32`, `GrossAmountMinor:Int64`, `CurrencyId:String`.
- Avsender: serverens Economy etter validert ankomst/frist/manifest. Konsumenter: Career, Multiplayer og UI.
- Autoritet/feil: unik salgsnøkkel per CatchId; gjenkall gir samme transaksjon. Fangst med tapt oppgjør kan ikke selges. Bokføring og outbox lagres samlet før bekreftelse. Feil betales ikke ut optimistisk.
- Versjonering: 1.0; valuta-/beløpssemantikk krever major. Rekordpremie er egen bokført transaksjon, ikke skjult dobbeltsalg.

### CatchLost

- Eier/formål: Economy som oppgjørseier; bekrefte usolgt fangst tapt ved for sen retur.
- Felter: `CatchId:Id`, `BoatId:Id`, `PlayerId:Id`, `SettlementId:Id`, `Reason:Enum<LateReturn>`, `LostAtServerMs:Int64`.
- Avsender: serverens Economy etter DayCycle-/Boats-grunnlaget. Konsumenter: Boats, Career, Multiplayer og UI.
- Autoritet/feil: solgt fangst kan ikke også tapes. Idempotent overgang fra usolgt til tapt. Personlig rekord og landet-statistikk beholdes; tidligere penger røres ikke. Andre tapsgrunner krever designavklaring.
- Versjonering: 1.0; tapsgrunner og effekt kan ikke endres uten kontrollert versjonering.

### CareerUpdated

- Eier/formål: Career; profilprojeksjon er varig lagret etter serverbekreftet endring.
- Felter: `PlayerId:Id`, `ProfileId:Id`, `ProfileRevision:Int64`, `AppliedEventIds:List<Id>`, `AppliedTransactionIds:List<Id>`, `SaveFormatVersion:Int32`, `SavedAtUtc:String` (RFC3339).
- Avsender: Career gjennom servergodkjent lagringsflyt. Konsumenter: Multiplayer og UI; Economy kan lese kvittering, men ikke godkjenne nye penger fra klientpayload.
- Autoritet/feil: monotone revisjoner; avvist lagring gir ikke CareerUpdated. Atomisk filbytte og sikkerhetskopi skal kunne gjenopprette profil. Ingen garanti mot bevisst kopiering mellom uavhengige offline-servere.
- Versjonering: hendelse 1.0; lagringsformat versjoneres separat med migrering og gjenopprettingstest.

## Kommandoer og svar

Felles kommando: `CommandId:Id`, `PlayerId:Id`, `SessionId:Id`, `DayId:Id`, `ExpectedRevision:Optional<Int64>`, `Payload:<type>`. PlayerId bindes til autentisert nettverkstilkobling og kan ikke velges fritt av klienten. Svar har `CommandId`, `Accepted:Bool`, `ReasonCode:Enum`, `ResultingRevision:Optional<Int64>`.

| Kommando | Mottaker | Tilleggsfelter | Tillatelse og feilhåndtering |
|---|---|---|---|
| SetRodSetup | Fishing | RodId, EquipmentRecord, BaitId, ProgramId, DepthMm:Int64, DragPermille:Int32 | Eier, utstyr, felles stangplasser og maks tre kontrolleres; ugyldig oppsett avvises |
| RequestVoluntaryReturn | Boats | BoatId:Id, CaptainRevision:Int64 | Bare nåværende kaptein; stale/not-captain avvises |
| StoreCatch | Boats, kun serverintegrasjon | CatchId:Id, BoatId:Id, LandingEventId:Id | Krever autoritativ landing; duplikat returnerer tidligere utfall |
| SellManifest | Economy, kun serverintegrasjon | BoatId:Id, ManifestRevision:Int64, SettlementId:Id | Frist/ankomst og unik fangst/transaksjon; klient kan ikke utstede salgsrett |
| ApplyCareerEvents | Career, kun serverintegrasjon | PlayerId:Id, EventIds:List<Id>, ExpectedRevision:Int64 | Kun bekreftede hendelser; stale revisjon gir resynkronisering |

Velg `ReasonCode` fra minst NotOwner, NotCaptain, WrongPhase, StaleRevision, CapacityExceeded, Duplicate, InvalidPayload, UnknownEntity og PersistenceUnavailable. Duplikat kan være vellykket gjenlevering av tidligere svar; skal ikke opprette ny effekt.

## Versjonering og kompatibilitet

Valgfrie felt med entydig standard kan legges til som minor. Fjerning, ny betydning, nye påkrevde felt eller nye enumverdier som gamle mottakere ikke håndterer, krever major. Ukjent major avvises med eksplisitt inkompatibilitet. Eventversjon, nettverksprotokollversjon, profilformat og spillversjon skal være separate verdier.

Modulene skal kunne testes med kontraktfakes uten å instansiere hverandres private objekter. Test duplikater, feil rekkefølge, manglende referanser, gjenstart og oppgjørsgrensen. Kontraktendringer krever integrasjonsreview, og designendringer krever prosjekteieren.
