#include "Economy/FiskDaySettlement.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "FiskWeighIn.h"
#include "Algo/StableSort.h"

AFiskDaySettlement::AFiskDaySettlement()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void AFiskDaySettlement::BeginPlay()
{
    Super::BeginPlay();
    if(HasAuthority()) { View.DayId=FGuid::NewGuid();ForceNetUpdate(); }
}
bool AFiskDaySettlement::RegisterCatch(const FGuid& CatchId, int32 PlayerId,const FFiskCatchData& Fish,const FFiskCatchContext& Context)
{
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();auto* Boat=Mode?Mode->GetBoatForPlayer(PlayerId):nullptr;
    if(!HasAuthority() || !Day || !Day->HasStarted() || !Boat || !Boat->IsAboard(PlayerId) || Boat->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { return false; }
    auto Candidate=Boat->GetCargo();const auto Result=Candidate.Add(Boat->GetKind(),CatchId,Fish.WeightGrams);
    if(Result==EFiskCargoResult::Rejected || !Ledger.Record(CatchId,PlayerId,Day->GetElapsedMilliseconds(),Fish,Context,Result!=EFiskCargoResult::Uncarryable)) { return false; }
    CatchBoats.Add(CatchId,Boat);Boat->AcceptCargo(Candidate);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CARGO_ACCEPT result=%d mode=%d"),static_cast<int32>(Result),static_cast<int32>(Candidate.Mode));return true;
}

void AFiskDaySettlement::ResetForDay(int32 DayNumber)
{
    if (!HasAuthority()) { return; }
    CatchBoats.Reset();Ledger = FFiskDayLedger(); View = FFiskSettlementView(); View.DayNumber = DayNumber;WeighQueue.Reset();WeighStartedAt=0;
    View.DayId=FGuid::NewGuid();
    SetActorTickEnabled(true); ForceNetUpdate();
}

void AFiskDaySettlement::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority()) { return; }
    if(View.bRevealed) { if(AFiskBootstrapGameMode* ReadyMode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();ReadyMode && ReadyMode->ArchiveConfirmedDay()) { SetActorTickEnabled(false); }return; }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (!Day || !Day->HasStarted() || !Mode || !Mode->GetStarterBoat()) { return; }
    const int32 NowMs = Day->GetElapsedMilliseconds();
    if(View.WeighIn.bStarted) { AdvanceWeighIn(NowMs);return; }
    // The shared weigh-in starts when every boat that went out is home again, or at 17:00 (#183).
    bool bAway=false,bHome=false;
    for(const AFiskStarterBoat* B:Mode->GetFleet())
    {
        if(!B) { continue; }const int32 Trip=B->GetTripProgress();
        bAway|=Trip!=AFiskStarterBoat::AtDock && Trip!=AFiskStarterBoat::AtMarina;bHome|=Trip==AFiskStarterBoat::AtMarina;
    }
    if(!FiskWeighIn::ShouldStart(NowMs,FFiskDayLedger::DeadlineMs,bAway,bHome)) { return; }
    const bool bEarly=NowMs<FFiskDayLedger::DeadlineMs;
    // Calculate crossing before settlement, even when both occur in the same frame.
    TMap<FGuid,int32> Arrivals;for(const auto& Pair:CatchBoats) { Arrivals.Add(Pair.Key,Pair.Value.IsValid()?Pair.Value->GetMarinaArrivalMs(NowMs):-1); }
    if(Ledger.SettleFleet(NowMs,Arrivals,bEarly)) { StartWeighIn(NowMs,bEarly);AdvanceWeighIn(NowMs); }
}

void AFiskDaySettlement::StartWeighIn(int32 NowMs,bool bEarly)
{
    // The ledger is settled on the server; the results stay private. Delivered fish go on the scale
    // smallest first; lost fish (late, or no room in the hold) are only counted.
    TArray<FFiskWeighEntry> Fish;TArray<int32> Landed;
    View.WeighIn=FFiskWeighInView();
    for(const FFiskDayResult& Row:Ledger.GetResults())
    {
        FFiskWeighTally Tally;Tally.PlayerId=Row.PlayerId;Tally.Caught=Row.Catches.Num();Tally.Lost=Row.LostFish;View.WeighIn.Tally.Add(Tally);
        for(const FFiskCatchResult& Catch:Row.Catches)
        { if(Catch.bDelivered) { FFiskWeighEntry E;E.PlayerId=Row.PlayerId;E.Fish=Catch.Fish;Fish.Add(E);Landed.Add(Catch.Context.LandedAtMs); } }
    }
    // Equal weights in landing order: sort by landing time first, then the stable weigh-in order.
    TArray<int32> ByLanding;for(int32 I=0;I<Fish.Num();++I) { ByLanding.Add(I); }
    Algo::StableSort(ByLanding,[&Landed](int32 A,int32 B){return Landed[A]<Landed[B];});
    TArray<int32> Grams;for(const int32 I:ByLanding) { Grams.Add(Fish[I].Fish.WeightGrams); }
    WeighQueue.Reset();for(const int32 I:FiskWeighIn::Order(Grams)) { WeighQueue.Add(Fish[ByLanding[I]]); }
    View.WeighIn.bStarted=true;View.WeighIn.bEarly=bEarly;View.WeighIn.StartedMs=NowMs;View.WeighIn.Total=WeighQueue.Num();
    WeighStartedAt=GetWorld()->GetRealTimeSeconds();
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_WEIGHIN_STARTED total=%d early=%d ms=%d"),View.WeighIn.Total,bEarly,NowMs);
    OnRep_View();ForceNetUpdate();
}

void AFiskDaySettlement::AdvanceWeighIn(int32 NowMs)
{
    const double Elapsed=GetWorld()->GetRealTimeSeconds()-WeighStartedAt;const int32 Count=WeighQueue.Num();
    // The day's end never waits for the scale: whatever is left is weighed at once.
    const bool bEndOfDay=NowMs>=AFiskDayState::DayEnd*1000;
    const int32 Target=bEndOfDay?Count:FiskWeighIn::OnScale(Elapsed,Count);bool bChanged=false;
    while(View.WeighIn.Weighed.Num()<Target)
    {
        const FFiskWeighEntry& E=WeighQueue[View.WeighIn.Weighed.Num()];View.WeighIn.Weighed.Add(E);bChanged=true;
        if(FFiskWeighTally* T=View.WeighIn.Tally.FindByPredicate([&E](const FFiskWeighTally& X){return X.PlayerId==E.PlayerId;})) { ++T->Weighed; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_WEIGHED index=%d player=%d species=%d grams=%d value=%d"),View.WeighIn.Weighed.Num(),E.PlayerId,E.Fish.Species,E.Fish.WeightGrams,E.Fish.Value);
    }
    if(bEndOfDay || FiskWeighIn::Finished(Elapsed,Count)) { Reveal(NowMs);return; }
    if(bChanged) { OnRep_View();ForceNetUpdate(); }
}

void AFiskDaySettlement::Reveal(int32 NowMs)
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>()) { for(auto B:Mode->GetFleet()) { B->RevealCargo(); } }
    View.Results = Ledger.GetResults();
    if(Day) { View.DayNumber = Day->GetDayNumber(); }
    if (History.Append(View.DayNumber, NowMs, Ledger)) { HistoryView = History.GetDays(); }
    View.bRevealed = true;
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_WEIGHIN_FINISHED weighed=%d ms=%d"),View.WeighIn.Weighed.Num(),NowMs);
    OnRep_View();
    ForceNetUpdate();
    if(AFiskBootstrapGameMode* ArchiveMode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();ArchiveMode && ArchiveMode->ArchiveConfirmedDay()) { SetActorTickEnabled(false); }
}

void AFiskDaySettlement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDaySettlement, View);
    DOREPLIFETIME(AFiskDaySettlement, HistoryView);
}

void AFiskDaySettlement::OnRep_View()
{
    if (!View.bRevealed)
    {
        if(View.WeighIn.bStarted) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_WEIGHIN_VIEW weighed=%d total=%d"),View.WeighIn.Weighed.Num(),View.WeighIn.Total); }
        return;
    }
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTLEMENT rows=%d"), View.Results.Num());
    for (const FFiskDayResult& Row : View.Results)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RESULT player=%d delivered=%d lost=%d balance=%d"),
            Row.PlayerId, Row.DeliveredFish, Row.LostFish, Row.SessionBalance);
        for(const FFiskCatchResult& Catch:Row.Catches)
        {
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_REVEALED_CATCH player=%d species=%d grams=%d mm=%d value=%d delivered=%d"),
                Row.PlayerId,Catch.Fish.Species,Catch.Fish.WeightGrams,Catch.Fish.LengthMm,Catch.bDelivered?Catch.Fish.Value:0,Catch.bDelivered);
        }
    }
}
