#include "Economy/Betting/FiskBetting.h"
#include "FiskCareerProfile.h"

EFiskBetResult FFiskBetBook::Register(const FGuid& Id, const FGuid& Bettor, const FGuid& Fisher, int64 Stake)
{
    if (const FFiskBet* Same = Bets.FindByPredicate([&](const FFiskBet& B) { return B.Id == Id; }))
    {
        return Same->Bettor == Bettor && Same->Fisher == Fisher && Same->Stake == Stake ? EFiskBetResult::AlreadyRegistered : EFiskBetResult::Conflict;
    }
    if (bClosed) { return EFiskBetResult::Closed; }
    if (!Id.IsValid() || !Bettor.IsValid() || !Fisher.IsValid() || !ValidStake(Stake)) { return EFiskBetResult::InvalidStake; }
    if (Bettor == Fisher) { return EFiskBetResult::SelfBet; }
    if (Bets.ContainsByPredicate([&](const FFiskBet& B) { return B.Bettor == Bettor; })) { return EFiskBetResult::AlreadyBetToday; }
    FFiskBet Bet; Bet.Id = Id; Bet.Bettor = Bettor; Bet.Fisher = Fisher; Bet.Stake = Stake; Bet.Order = Bets.Num();
    Bets.Add(Bet);
    return EFiskBetResult::Registered;
}

int64 FFiskBetBook::Pot() const
{
    int64 Sum = 0;
    for (const FFiskBet& Bet : Bets) { Sum += Bet.Stake; }
    return Sum;
}

TArray<FFiskBetPayout> FFiskBetBook::Settle(const TArray<FFiskBetCatch>& Counting) const
{
    TArray<FFiskBetPayout> Out;
    int32 Best = 0;
    for (const FFiskBetCatch& Catch : Counting) { Best = FMath::Max(Best, Catch.Grams); }
    // Rule 4: every fisher with the largest weight (same grams) is a winning tip.
    auto Wins = [&](const FFiskBet& Bet)
    { return Best > 0 && Counting.ContainsByPredicate([&](const FFiskBetCatch& C) { return C.Fisher == Bet.Fisher && C.Grams == Best; }); };
    int64 WinningStake = 0;
    for (const FFiskBet& Bet : Bets) { if (Wins(Bet)) { WinningStake += Bet.Stake; } }
    // Rules 5 and 6: no correct tip, or no counting catch, gives everyone their stake back.
    const bool bRefund = WinningStake == 0;
    const int64 Total = Pot();
    int64 Paid = 0;
    int32 RemainderTo = INDEX_NONE;
    for (const FFiskBet& Bet : Bets)
    {
        FFiskBetPayout Payout; Payout.Receipt = FFiskCareerProfile::BetPayoutId(Bet.Id); Payout.Bettor = Bet.Bettor; Payout.bRefund = bRefund;
        if (bRefund) { Payout.Amount = Bet.Stake; }
        else if (Wins(Bet))
        {
            // Rule 3 and 7: share by stake, rounded down to whole cents.
            Payout.Amount = Total * Bet.Stake / WinningStake;
            if (RemainderTo == INDEX_NONE || Bet.Stake > Bets[RemainderTo].Stake) { RemainderTo = Out.Num(); }
        }
        Paid += Payout.Amount;
        Out.Add(Payout);
    }
    // Rule 7: the leftover cents go to the largest winning stake; on a tie, the earliest bet (Bets is in
    // registration order and only a strictly larger stake moves RemainderTo).
    if (!bRefund && RemainderTo != INDEX_NONE) { Out[RemainderTo].Amount += Total - Paid; }
    return Out;
}

TArray<FFiskBetCatch> FFiskBetBook::CountingCatches(const TArray<FFiskDayResult>& Rows, const TMap<int32, FGuid>& Careers, const TSet<FGuid>& OutAtSea)
{
    TMap<FGuid, int32> Largest;
    for (const FFiskDayResult& Row : Rows)
    {
        const FGuid* Career = Careers.Find(Row.PlayerId);
        if (!Career || !OutAtSea.Contains(*Career)) { continue; }
        for (const FFiskCatchResult& Catch : Row.Catches)
        {
            if (!Catch.bDelivered || Catch.Fish.WeightGrams <= 0) { continue; }
            int32& Best = Largest.FindOrAdd(*Career, 0);
            Best = FMath::Max(Best, Catch.Fish.WeightGrams);
        }
    }
    TArray<FFiskBetCatch> Out;
    for (const TPair<FGuid, int32>& Pair : Largest) { FFiskBetCatch Catch; Catch.Fisher = Pair.Key; Catch.Grams = Pair.Value; Out.Add(Catch); }
    return Out;
}

FFiskBetSummary FFiskBetBook::Summarize(const TArray<FFiskBet>& Bets, const TArray<FFiskBetPayout>& Payouts, const TArray<FFiskBetCatch>& Counting)
{
    FFiskBetSummary S; S.Bets = Bets.Num();
    for (const FFiskBet& Bet : Bets) { S.Pot += Bet.Stake; }
    for (const FFiskBetPayout& Payout : Payouts)
    {
        S.bRefund |= Payout.bRefund;
        if (Payout.bRefund || Payout.Amount <= 0) { continue; }
        ++S.Winners;
        const FFiskBet* Bet = Bets.FindByPredicate([&](const FFiskBet& B) { return FFiskCareerProfile::BetPayoutId(B.Id) == Payout.Receipt; });
        if (Bet) { S.WinningFishers.AddUnique(Bet->Fisher); }
    }
    for (const FFiskBetCatch& Catch : Counting) { if (S.WinningFishers.Contains(Catch.Fisher)) { S.WinningGrams = FMath::Max(S.WinningGrams, Catch.Grams); } }
    return S;
}
