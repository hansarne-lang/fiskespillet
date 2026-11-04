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

bool AFiskDaySettlement::RegisterCatch(const FGuid& CatchId, int32 PlayerId)
{
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    return HasAuthority() && Day && Day->HasStarted()
        && Ledger.Record(CatchId, PlayerId, Day->GetElapsedMilliseconds());
}

void AFiskDaySettlement::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || View.bRevealed) { return; }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (!Day || !Day->HasStarted() || !Mode || !Mode->GetStarterBoat()) { return; }
    const int32 NowMs = Day->GetElapsedMilliseconds();
    // Calculate crossing before settlement, even when both occur in the same frame.
    const int32 ArrivalMs = Mode->GetStarterBoat()->GetMarinaArrivalMs(NowMs);
    if (Ledger.Settle(NowMs, ArrivalMs))
    {
        View.Results = Ledger.GetResults();
        View.bRevealed = true;
        OnRep_View();
        ForceNetUpdate();
        SetActorTickEnabled(false);
    }
}

void AFiskDaySettlement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDaySettlement, View);
}

void AFiskDaySettlement::OnRep_View()
{
    if (!View.bRevealed) { return; }
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_SETTLEMENT rows=%d"), View.Results.Num());
    for (const FFiskDayResult& Row : View.Results)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RESULT player=%d delivered=%d lost=%d balance=%d"),
            Row.PlayerId, Row.DeliveredFish, Row.LostFish, Row.SessionBalance);
    }
}
