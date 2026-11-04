#pragma once

#include "CoreMinimal.h"
#include "FiskDayLedger.generated.h"

USTRUCT()
struct FFiskDayResult
{
    GENERATED_BODY()
    UPROPERTY() int32 PlayerId = INDEX_NONE;
    UPROPERTY() int32 DeliveredFish = 0;
    UPROPERTY() int32 LostFish = 0;
    UPROPERTY() int32 SessionBalance = 0;
};

// This ledger remains on the server. Only the final view is replicated.
class FFiskDayLedger
{
public:
    static constexpr int32 DeadlineMs = 1020000;
    static constexpr int32 TestFishPrice = 10;
    bool Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs);
    bool Settle(int32 NowMs, int32 ArrivalMs);
    bool IsSettled() const { return bSettled; }
    const TArray<FFiskDayResult>& GetResults() const { return Results; }
private:
    TMap<FGuid, int32> Catches;
    TArray<FFiskDayResult> Results;
    bool bSettled = false;
};
