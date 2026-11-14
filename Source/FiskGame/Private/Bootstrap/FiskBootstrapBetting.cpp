// Beach bar bets on the server (#22 step 2). Rules: DECISIONS.md 2026-11-08 and 2026-11-10.
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"

bool AFiskBootstrapGameMode::IsAtSea(int32 Player) const
{
    for(const AFiskStarterBoat* Boat:Fleet)
    {
        if(Boat && Boat->IsAboard(Player)) { return Boat->GetTripProgress()!=AFiskStarterBoat::AtDock && Boat->GetTripProgress()!=AFiskStarterBoat::AtMarina; }
    }
    return false;
}
bool AFiskBootstrapGameMode::AreBetsOpen() const
{
    const AFiskDayState* Day=GetGameState<AFiskDayState>();
    return Day && Day->HasStarted() && Day->GetElapsedMilliseconds()<FFiskDayLedger::DeadlineMs && !bBetsSettled && !BetBook.IsClosed() && !IsWeighInStarted();
}
EFiskBetResult AFiskBootstrapGameMode::PlaceBet(int32 Player,const FGuid& Bettor,const FGuid& Bet,const FGuid& Fisher,int64 Stake,TFunctionRef<bool()> Charge)
{
    const bool Known=BetBook.GetBets().ContainsByPredicate([&](const FFiskBet& B){return B.Id==Bet;});
    if(!Known)
    {
        // A retry of a registered bet is answered by the book below, even after 17:00.
        if(!AreBetsOpen()) { return EFiskBetResult::Closed; }
        if(IsAtSea(Player)) { return EFiskBetResult::NotAshore; }
        bool FisherOut=false;
        for(const TPair<int32,FGuid>& Bound:DayProfiles) { if(Bound.Value==Fisher && Bound.Key!=Player && IsAtSea(Bound.Key)) { FisherOut=true; } }
        if(!FisherOut && Fisher!=Bettor) { return EFiskBetResult::FisherNotAtSea; }
    }
    FFiskBetBook Next=BetBook;
    const EFiskBetResult Result=Next.Register(Bet,Bettor,Fisher,Stake);
    if(Result!=EFiskBetResult::Registered) { return Result; }
    // The book only changes once the stake receipt is saved, so a failed save leaves no bet behind.
    if(!Charge()) { return EFiskBetResult::NotCharged; }
    BetBook=MoveTemp(Next);
    for(const TPair<int32,FGuid>& Bound:DayProfiles) { if(IsAtSea(Bound.Key)) { BetFishers.Add(Bound.Value); } }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BET_REGISTERED player=%d stake=%lld bets=%d pot=%lld fishers=%d"),Player,static_cast<long long>(Stake),BetBook.GetBets().Num(),static_cast<long long>(BetBook.Pot()),BetFishers.Num());
    return Result;
}
void AFiskBootstrapGameMode::SettleBets()
{
    if(bBetsSettled || !Settlement || !Settlement->GetView().bRevealed) { return; }
    BetBook.Close();bBetsSettled=true;
    const TArray<FFiskBetCatch> Counting=FFiskBetBook::CountingCatches(Settlement->GetView().Results,DayProfiles,BetFishers);
    int32 Winners=0;bool Refund=false;const TArray<FFiskBetPayout> Payouts=BetBook.Settle(Counting);
    BetSummary=FFiskBetBook::Summarize(BetBook.GetBets(),Payouts,Counting);
    for(const FFiskBetPayout& Payout:Payouts)
    { UnclaimedBets.Add(Payout.Receipt,Payout);BetResults.Add(Payout.Bettor,Payout.Amount);Winners+=Payout.Amount>0 && !Payout.bRefund?1:0;Refund|=Payout.bRefund; }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_BETS_SETTLED bets=%d pot=%lld counting=%d winners=%d refund=%d"),BetBook.GetBets().Num(),static_cast<long long>(BetBook.Pot()),Counting.Num(),Winners,Refund);
}
TArray<FFiskBetPayout> AFiskBootstrapGameMode::UnclaimedBetPayouts(const FGuid& Bettor) const
{
    TArray<FFiskBetPayout> Out;
    for(const TPair<FGuid,FFiskBetPayout>& Pair:UnclaimedBets) { if(Pair.Value.Bettor==Bettor) { Out.Add(Pair.Value); } }
    return Out;
}
bool AFiskBootstrapGameMode::IsBetPending(const FGuid& Bet) const
{ return BetBook.GetBets().ContainsByPredicate([&](const FFiskBet& B){return B.Id==Bet;}) || UnclaimedBets.Contains(FFiskCareerProfile::BetPayoutId(Bet)); }
void AFiskBootstrapGameMode::BetBoard(const FGuid& Bettor,FGuid& Fisher,int64& Stake,int64& Payout) const
{
    Fisher=FGuid();Stake=0;Payout=-1;
    if(const FFiskBet* Own=BetBook.GetBets().FindByPredicate([&](const FFiskBet& B){return B.Bettor==Bettor;})) { Fisher=Own->Fisher;Stake=Own->Stake; }
    if(const int64* Paid=BetResults.Find(Bettor)) { Payout=*Paid; }
}
void AFiskBootstrapGameMode::ResetBetsForDay()
{ BetBook=FFiskBetBook();BetFishers.Reset();BetResults.Reset();bBetsSettled=false;BetSummary=FFiskBetSummary(); }
