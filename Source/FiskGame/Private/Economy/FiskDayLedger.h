#pragma once

#include "CoreMinimal.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Economy/FiskCatchContext.h"
#include "FiskDayLedger.generated.h"

USTRUCT()
struct FFiskCatchResult
{
    GENERATED_BODY()
    UPROPERTY() FGuid CatchId;
    UPROPERTY() FFiskCatchData Fish;
    UPROPERTY() FFiskCatchContext Context;
    UPROPERTY() bool bDelivered = false;
};

USTRUCT()
struct FFiskDayResult
{
    GENERATED_BODY()
    UPROPERTY() int32 PlayerId = INDEX_NONE;
    UPROPERTY() int32 DeliveredFish = 0;
    UPROPERTY() int32 LostFish = 0;
    UPROPERTY() int32 SessionBalance = 0;
    UPROPERTY() int32 DeliveredWeightGrams = 0;
    UPROPERTY() TArray<FFiskCatchResult> Catches;
};

// This ledger remains on the server. Only the final view is replicated.
class FFiskDayLedger
{
public:
    static constexpr int32 DeadlineMs = 1020000;
    static constexpr int32 TestFishPrice = 10;
    bool Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs);
    bool Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs, const FFiskCatchData& Fish, const FFiskCatchContext& Context=FFiskCatchContext(),bool bTransportable=true);
    bool Settle(int32 NowMs, int32 ArrivalMs);
    bool SettleFleet(int32 NowMs,const TMap<FGuid,int32>& Arrivals);
    bool IsSettled() const { return bSettled; }
    const TArray<FFiskDayResult>& GetResults() const { return Results; }
private:
    struct FStoredCatch { int32 PlayerId; FFiskCatchData Fish; FFiskCatchContext Context; bool bTransportable=true; };
    TMap<FGuid, FStoredCatch> Catches;
    TArray<FFiskDayResult> Results;
    bool bSettled = false;
};
