# Strandbarens veddemål

Status: steg 2 av [#22](https://github.com/hansarne-lang/fiskespillet/issues/22). Veddemål kan settes i strandbaren og gjøres opp ved 17:00. LAN-tester (punkt 5 under) gjenstår.

Reglene er godkjent av prosjekteieren og står i [DECISIONS.md](../../DECISIONS.md) (8. november 2026, punkt 1–8). Regneeksemplene står i [kravspesifikasjon v1.1](../Requirements/FISKELAGET_Kravspesifikasjon_v1.1.md).

## Steg 1: oppgjørsmodell (levert)

`Source/FiskGame/Private/Economy/Betting/FiskBetting.{h,cpp}` (`FFiskBetBook`):

- **Registrering:** innsatsen må være $50, $100 eller $250 (punkt 1), og hver spiller får ett veddemål per dag (punkt 2). Samme veddemål sendt på nytt gir `AlreadyRegistered` og skal ikke belastes igjen. Etter `Close()` (17:00) tas det ikke imot nye veddemål.
- **Oppgjør:** deling i forhold til innsatsen (punkt 3), alle med lik største vekt vinner (punkt 4), og alle får innsatsen tilbake når ingen har riktig tips eller ingen fangst teller (punkt 5 og 6).
- **Avrunding:** hele øre, med restøren til største innsats og deretter første registrering (punkt 7). Summen av utbetalingene er alltid lik potten.
- **Kvittering:** utbetalingens kvitterings-ID avledes av veddemålets ID (`FFiskCareerProfile::BetPayoutId`, punkt 8). `FFiskCareerProfile::Credit` med samme ID godskriver bare én gang.
- **Penger og personer:** beløp er i øre, som karrierens saldo. Personer er karriere-ID-er, så et veddemål overlever frakobling.

Tester: `Fiskelaget.Economy.BettingExamples` (de fem eksemplene) og `Fiskelaget.Economy.BettingRules`.

## Steg 2: spillet (levert)

- **Innsats (FiskCore):** `FFiskCareerProfile::Stake` trekker innsatsen under veddemålets ID som kvitteringstype 3 (`KindBet`, beløpet er minus innsatsen). Samme ID på nytt trekker ikke igjen. Det må være plass til utbetalingskvitteringen etterpå. Eldre spillversjoner godtar ikke en karriere med slik kvittering.
- **Server (`Bootstrap/FiskBootstrapBetting.cpp`):** spillmodusen eier dagens bok. Et veddemål godtas bare før 17:00, når spilleren er i land (ikke om bord i en båt som er ute) og fiskeren er om bord i en båt som er ute. Boka endres først når innsatsen er lagret i karrieren. Ved hver registrering husker serveren alle fiskere som er ute; det er dem som sammenlignes (beslutning 10. november 2026).
- **Oppgjør:** når dagens oppgjør er avslørt ved 17:00, lukkes boka. `FFiskBetBook::CountingCatches` finner største fisk levert innen fristen per fisker (fisken er bokført på stangeieren i dagboka), og `Settle` deler potten. Hver utbetaling, også tap (0) og tilbakebetaling, blir en egen oppgjørskvittering. Utbetalinger venter hos serveren til spilleren er tilkoblet og kvitteringen er lagret, også etter dagsskifte.
- **Foreldreløse innsatser:** finner verten ved tilkobling en innsats som den ikke kjenner (verten har startet på nytt, eller veddemålet ble satt hos en annen vert), betales innsatsen tilbake under utbetalingens kvitterings-ID. En senere utbetaling av samme veddemål avvises da.
- **Strandbaren (B):** F velger fisker blant dem som er ute, S velger innsats, Enter setter. Baren viser eget veddemål, potten og antall veddemål. Ditt resultat vises først etter 17:00.
- **Logglinjer:** `FISKELAGET_BET_REGISTERED`, `FISKELAGET_BET_REPLY`, `FISKELAGET_BETS_SETTLED`, `FISKELAGET_BET_PAID`, `FISKELAGET_BET_ORPHAN_REFUND`.

Tester: `Fiskelaget.Career.BetStakeReceipts` og `Fiskelaget.Economy.BettingCountingCatches`.

## Gjenstår

1. **LAN-tester:** veddemål mellom to spillere, frakobling og ny tilkobling før og etter 17:00, omstart av verten, sen levering og dobbel forespørsel. Krever to PC-er eller en røyktest med to klienter.
2. **Begrensning:** ubetalte utbetalinger lagres bare i minnet hos verten. Starter verten på nytt før en frakoblet spiller kommer tilbake, får spilleren innsatsen tilbake i stedet for en eventuell gevinst.
