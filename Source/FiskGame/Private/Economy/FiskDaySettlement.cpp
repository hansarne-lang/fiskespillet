#include "Economy/FiskDaySettlement.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

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
    return HasAuthority() && Day && Day->HasStarted()
        && Ledger.Record(CatchId, PlayerId, Day->GetElapsedMilliseconds(),Fish,Context);
}

void AFiskDaySettlement::ResetForDay(int32 DayNumber)
{
    if (!HasAuthority()) { return; }
    Ledger = FFiskDayLedger(); View = FFiskSettlementView(); View.DayNumber = DayNumber;
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
    // Calculate crossing before settlement, even when both occur in the same frame.
    const int32 ArrivalMs = Mode->GetStarterBoat()->GetMarinaArrivalMs(NowMs);
    if (Ledger.Settle(NowMs, ArrivalMs))
    {
        View.Results = Ledger.GetResults();
        View.DayNumber = Day->GetDayNumber();
        if (History.Append(View.DayNumber, NowMs, Ledger)) { HistoryView = History.GetDays(); }
        View.bRevealed = true;
        OnRep_View();
        ForceNetUpdate();
        if(AFiskBootstrapGameMode* ArchiveMode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();ArchiveMode && ArchiveMode->ArchiveConfirmedDay()) { SetActorTickEnabled(false); }
    }
}

void AFiskDaySettlement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDaySettlement, View);
    DOREPLIFETIME(AFiskDaySettlement, HistoryView);
}

void AFiskDaySettlement::OnRep_View()
{
    if (!View.bRevealed) { return; }
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
