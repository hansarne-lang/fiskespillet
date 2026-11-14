#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Economy/FiskDayLedger.h"
#include "Economy/FiskSessionHistory.h"
#include "FiskDaySettlement.generated.h"

// One fish that has been on the scale in the shared weigh-in (#183). Only weighed fish are replicated.
USTRUCT()
struct FFiskWeighEntry
{
    GENERATED_BODY()
    UPROPERTY() int32 PlayerId = INDEX_NONE;
    UPROPERTY() FFiskCatchData Fish;
};
// Public from the start of the weigh-in: how many fish each participant landed, has had weighed and lost.
USTRUCT()
struct FFiskWeighTally
{
    GENERATED_BODY()
    UPROPERTY() int32 PlayerId = INDEX_NONE;
    UPROPERTY() int32 Caught = 0;
    UPROPERTY() int32 Weighed = 0;
    UPROPERTY() int32 Lost = 0;
};
USTRUCT()
struct FFiskWeighInView
{
    GENERATED_BODY()
    UPROPERTY() bool bStarted = false;
    UPROPERTY() bool bEarly = false;      // Started because everyone was home before 17:00
    UPROPERTY() int32 StartedMs = -1;     // Day clock at the start
    UPROPERTY() int32 Total = 0;          // Fish to weigh
    UPROPERTY() TArray<FFiskWeighEntry> Weighed;   // Smallest first; the last one is on the scale
    UPROPERTY() TArray<FFiskWeighTally> Tally;
};

USTRUCT()
struct FFiskSettlementView
{
    GENERATED_BODY()
    UPROPERTY() int32 DayNumber = 1;
    UPROPERTY() FGuid DayId;
    UPROPERTY() bool bRevealed = false;   // The weigh-in is over: results, income, journal and bets follow
    UPROPERTY() TArray<FFiskDayResult> Results;
    UPROPERTY() FFiskWeighInView WeighIn;
};

UCLASS()
class AFiskDaySettlement final : public AInfo
{
    GENERATED_BODY()
public:
    AFiskDaySettlement();
    bool RegisterCatch(const FGuid& CatchId, int32 PlayerId, const FFiskCatchData& Fish,const FFiskCatchContext& Context=FFiskCatchContext());
    virtual void BeginPlay() override;
    const FFiskSettlementView& GetView() const { return View; }
    const TArray<FFiskHistoryDay>& GetHistory() const { return HistoryView; }
    void ResetForDay(int32 DayNumber);
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    // The shared weigh-in has started (#183): no more departures and no more bets today.
    bool IsWeighInStarted() const { return View.WeighIn.bStarted || View.bRevealed; }
private:
    TArray<FFiskWeighEntry> WeighQueue;   // Server only: the order on the scale, never replicated in full
    double WeighStartedAt = 0;
    void StartWeighIn(int32 NowMs,bool bEarly);
    void AdvanceWeighIn(int32 NowMs);
    void Reveal(int32 NowMs);
    TMap<FGuid,TWeakObjectPtr<class AFiskStarterBoat>> CatchBoats;
    FFiskDayLedger Ledger;
    FFiskSessionHistory History;
    UPROPERTY(Replicated) TArray<FFiskHistoryDay> HistoryView;
    UPROPERTY(ReplicatedUsing = OnRep_View)
    FFiskSettlementView View;
    UFUNCTION() void OnRep_View();
};
