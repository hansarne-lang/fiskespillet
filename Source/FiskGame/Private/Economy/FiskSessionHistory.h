#pragma once
#include "CoreMinimal.h"
#include "Economy/FiskDayLedger.h"
#include "FiskSessionHistory.generated.h"

USTRUCT()
struct FFiskHistoryRow
{
    GENERATED_BODY()
    UPROPERTY() int32 PlayerId = INDEX_NONE;
    UPROPERTY() int32 Delivered = 0;
    UPROPERTY() int32 Lost = 0;
    UPROPERTY() int32 Income = 0;
    UPROPERTY() int32 DeliveredGrams = 0;
    UPROPERTY() int32 BestGrams = 0;
    UPROPERTY() int32 BestSpecies = -1;
};
USTRUCT()
struct FFiskHistoryDay
{
    GENERATED_BODY()
    UPROPERTY() int32 Number = 0;
    UPROPERTY() TArray<FFiskHistoryRow> Rows;
};
class FFiskSessionHistory
{
public:
    static constexpr int32 Capacity = 32;
    bool Append(int32 Day, int32 NowMs, const FFiskDayLedger& Ledger);
    const TArray<FFiskHistoryDay>& GetDays() const { return Days; }
private:
    TArray<FFiskHistoryDay> Days;
};
