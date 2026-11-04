#include "Economy/FiskDayLedger.h"

bool FFiskDayLedger::Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs)
{
    if (bSettled || !CatchId.IsValid() || PlayerId < 0 || LandedAtMs < 0
        || LandedAtMs >= DeadlineMs || Catches.Contains(CatchId)) { return false; }
    Catches.Add(CatchId, PlayerId);
    return true;
}

bool FFiskDayLedger::Settle(int32 NowMs, int32 ArrivalMs)
{
    if (bSettled || NowMs < DeadlineMs) { return false; }
    const bool bOnTime = ArrivalMs >= 0 && ArrivalMs <= DeadlineMs;
    for (const auto& Catch : Catches)
    {
        FFiskDayResult* Row = Results.FindByPredicate([&Catch](const FFiskDayResult& Item) { return Item.PlayerId == Catch.Value; });
        if (!Row)
        {
            FFiskDayResult NewRow;
            NewRow.PlayerId = Catch.Value;
            Results.Add(NewRow);
            Row = &Results.Last();
        }
        if (bOnTime)
        {
            ++Row->DeliveredFish;
            Row->SessionBalance += TestFishPrice;
        }
        else { ++Row->LostFish; }
    }
    Results.Sort([](const FFiskDayResult& A, const FFiskDayResult& B) { return A.PlayerId < B.PlayerId; });
    bSettled = true;
    return true;
}
