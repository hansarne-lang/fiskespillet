# FISKELAGET minimal Unreal foundation

Kildeforberedelsen i #26 oppretter FiskCore/FiskGame, Game-/Editor-target og en oppstart med FISKELAGET-tittel. Første kompilering, pakking og faktisk Windows-oppstart kvalifiseres i #3/#5. Kilde- og strukturkontroll er ikke bevis på at C++-prosjektet bygger.

## Innhold og grenser

FiskCore bruker bare Unreal Core. FiskGame laster FiskCore og inneholder en minimal oppstartsklasse og visning av tittel. Ingen av de ni logiske spillområdene har fått mekanikker. Den eksisterende Engine Entry-scenen brukes til teknisk oppstart; dette er ikke den visuelle spillprototypen eller et ferdig havmiljø. Ingen engine-ressurser kopieres inn i repositoryet.

## Kontrollert bygg

Bruk installert Unreal 5.8.4 og en verktøykjede som godtas av motorens Windows_SDK.json. Motor- og kompilatorstier holdes lokale; prosjektet angir ingen personlig eller absolutt motorsti.

Kjør motorens Build.bat med target FiskELagetEditor, plattform Win64, konfigurasjon Development og prosjektfilen FiskELaget.uproject. Bygg deretter Game-target FiskELaget. Editor og bygg skal ikke kjøre samtidig på denne maskinen.

Pakk med RunUAT BuildCookRun for Win64 Development, prosjektets Engine Entry-kart, build/cook/stage/pak/archive og en lokal arkivmappe utenfor repositoryet. Ikke legg Binaries, Intermediate, Saved, DerivedDataCache eller pakkede bygg i Git. Et spillbygg publiseres ikke som release av denne kildeleveransen.

## Oppstartsprøve

Det pakkede Windows-programmet skal starte uten Unreal Editor. Normal oppstart skal vise FISKELAGET og «Teknisk grunnstamme». Loggen skal inneholde FISKCORE_MODULE_LOADED og FISKELAGET_BOOTSTRAP_READY etter at verden er startet. En prøve skal registrere eksakt commit, motor-/kompilator-/SDK-versjon, exitkode og loggresultat før den rapporteres Passed.

Ved denne kildeforberedelsen er C++-bygg, pakking og Windows-oppstart Not Run fordi Microsoft-installasjonen venter på godkjent Windows UAC. Faktisk fremdrift og testbevis følger GitHub Issues #3, #5 og #26.
