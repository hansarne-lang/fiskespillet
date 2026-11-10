# Strandbarens veddemål

Status: steg 1 av [#22](https://github.com/hansarne-lang/fiskespillet/issues/22). Oppgjørsmodellen finnes og er testet, men er ikke koblet til spillet ennå.

Reglene er godkjent av prosjekteieren og står i [DECISIONS.md](../../DECISIONS.md) (8. november 2026, punkt 1–8). Regneeksemplene står i [kravspesifikasjon v1.1](../Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md).

## Steg 1: oppgjørsmodell (levert)

`Source/FiskGame/Private/Economy/Betting/FiskBetting.{h,cpp}` (`FFiskBetBook`):

- **Registrering:** innsatsen må være $50, $100 eller $250 (punkt 1), og hver spiller får ett veddemål per dag (punkt 2). Samme veddemål sendt på nytt gir `AlreadyRegistered` og skal ikke belastes igjen. Etter `Close()` (17:00) tas det ikke imot nye veddemål.
- **Oppgjør:** deling i forhold til innsatsen (punkt 3), alle med lik største vekt vinner (punkt 4), og alle får innsatsen tilbake når ingen har riktig tips eller ingen fangst teller (punkt 5 og 6).
- **Avrunding:** hele øre, med restøren til største innsats og deretter første registrering (punkt 7). Summen av utbetalingene er alltid lik potten.
- **Kvittering:** veddemålets ID er også kvitterings-ID for utbetalingen (punkt 8). `FFiskCareerProfile::Credit` med samme ID godskriver bare én gang.
- **Penger og personer:** beløp er i øre, som karrierens saldo. Personer er karriere-ID-er, så et veddemål overlever frakobling.

Tester: `Fiskelaget.Economy.BettingExamples` (de fem eksemplene) og `Fiskelaget.Economy.BettingRules`.

## Gjenstår

1. **Hvilke fangster sammenlignes?** Avgjort 10. november 2026 (DECISIONS.md): bare tellende fangster fra fiskere som var ute på sjøen da veddemålene kunne registreres. Spillere som allerede var i land, er ikke med. Serveren må derfor huske hvilke fiskere som var ute ved første registrering, og gi bare deres fangster til `Settle`.
2. **Belastning av innsatsen:** karrieren trenger en idempotent trekk-kvittering for innsats (FiskCore), etter samme mønster som `Buy`/`Credit`.
3. **Server og nettverk:** registrering i strandbaren, kontroll av at spilleren er i land og fiskeren ute, lagring av boka for dagen og utbetaling ved 17:00 sammen med det felles oppgjøret.
4. **Brukerflate:** valg av fisker og innsats i strandbaren (B). Resultatene vises først ved 17:00.
5. **LAN-tester:** frakobling, omstart, sen levering og dobbel forespørsel.
