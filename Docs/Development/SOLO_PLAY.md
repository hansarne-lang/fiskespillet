# Spill alene

Oppgave #59 er bestilt av prosjekteieren 5. november 2026. Spill fra hovedmenyen starter nå fiskedagen med første spiller. Spilleren kan være alene hele økten. Den samme listen-serveren brukes for solo og LAN, og Bli med venner virker fortsatt. En gjest som kommer senere følger vertens pågående dag; `StartDay` nullstiller ikke en aktiv dag. Hovedmenyen starter fortsatt ingen dag.

Velg Spill, trykk E for å gå om bord, og vent til 02:00. Lukk kart og menyer og trykk R for avreise. M velger fiskeplass ved brygga; G endrer ledig stang. F setter ut stanga ved ankomst. Kapteinen kan returnere med R. Frist 17:00 og dagsslutt 20:00 er uendret.

`Tools/Testing/Run-SoloPlay.ps1` starter én grafisk prosess fra det pakkede spillet med isolert brukerområde. Prøven bruker menyens og kontrollerens faktiske inputrute, og venter på bekreftet ombordstigning før den kontrollerer resultatet. Den kontrollerer at hovedmenyen ikke starter dagen, at én vert starter den, at E gir kapteinsrolle, at tidlig R avvises, at menyen ikke pauser, og at vanlig klokke når 02:00 før R starter turen. Deretter settes stanga ut og båten returnerer til marinaen. Fire skjermbilder kreves. Ingen akselerert klokke eller ekstra klient brukes. Eksisterende tospiller-menyscenario og Unreal-testene er regresjonsgrunnlag. Resultater registreres på testet commit i issue/PR; denne prosedyren alene er ikke et bestått testresultat.

Kjent kapteins-smoke-race fra #9 er utenfor denne endringen. Permanent karriere og lagring mellom PC-er inngår ikke.
