#include "Economy/FiskDayLedger.h"

bool FFiskDayLedger::Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs)
{
    return Record(CatchId,PlayerId,LandedAtMs,FFiskCatchData());
}
bool FFiskDayLedger::Record(const FGuid& CatchId, int32 PlayerId, int32 LandedAtMs,const FFiskCatchData& Fish,const FFiskCatchContext& Context)
{
    if (bSettled || !CatchId.IsValid() || PlayerId < 0 || LandedAtMs < 0
        || LandedAtMs >= DeadlineMs || Catches.Contains(CatchId) || !FiskCatches::IsValid(Fish)
        || Catches.Num()>=10000 || !Context.IsValid(false)) { return false; }
    FFiskCatchContext Stored=Context;Stored.LandedAtMs=LandedAtMs;
    if(!Stored.IsValid(false)) { return false; }
    Catches.Add(CatchId, FStoredCatch{PlayerId,Fish,Stored});
    return true;
}

bool FFiskDayLedger::Settle(int32 NowMs, int32 ArrivalMs)
{
    if (bSettled || NowMs < DeadlineMs) { return false; }
    const bool bOnTime = ArrivalMs >= 0 && ArrivalMs <= DeadlineMs;
    for (const auto& Catch : Catches)
    {
        FFiskDayResult* Row = Results.FindByPredicate([&Catch](const FFiskDayResult& Item) { return Item.PlayerId == Catch.Value.PlayerId; });
        if (!Row)
        {
            FFiskDayResult NewRow;
            NewRow.PlayerId = Catch.Value.PlayerId;
            Results.Add(NewRow);
            Row = &Results.Last();
        }
        if (bOnTime)
        {
            ++Row->DeliveredFish;
            Row->SessionBalance += Catch.Value.Fish.Value;
            Row->DeliveredWeightGrams += Catch.Value.Fish.WeightGrams;
        }
        else { ++Row->LostFish; }
        FFiskCatchResult Detail;Detail.CatchId=Catch.Key;Detail.Fish=Catch.Value.Fish;Detail.bDelivered=bOnTime;
        Detail.Context=Catch.Value.Context;
        Row->Catches.Add(Detail);
    }
    for(FFiskDayResult& Row:Results)
    {
        Row.Catches.Sort([](const FFiskCatchResult& A,const FFiskCatchResult& B)
        { return A.CatchId.ToString()<B.CatchId.ToString(); });
    }
    Results.Sort([](const FFiskDayResult& A, const FFiskDayResult& B) { return A.PlayerId < B.PlayerId; });
    bSettled = true;
    return true;
}
