#include "Economy/FiskSessionHistory.h"
bool FFiskSessionHistory::Append(int32 Day, int32 NowMs, const FFiskDayLedger& Ledger)
{
    if (Day <= 0 || NowMs < FFiskDayLedger::DeadlineMs || !Ledger.IsSettled()
        || (!Days.IsEmpty() && Day <= Days.Last().Number)) { return false; }
    FFiskHistoryDay Entry; Entry.Number = Day;
    for (const FFiskDayResult& Result : Ledger.GetResults())
    {
        FFiskHistoryRow Row; Row.PlayerId = Result.PlayerId;
        Row.Delivered = Result.DeliveredFish; Row.Lost = Result.LostFish;
        Row.Income = Result.SessionBalance; Row.DeliveredGrams = Result.DeliveredWeightGrams;
        for (const FFiskCatchResult& Catch : Result.Catches)
        {
            if (Catch.Fish.WeightGrams > Row.BestGrams)
            { Row.BestGrams = Catch.Fish.WeightGrams; Row.BestSpecies = Catch.Fish.Species; }
        }
        Entry.Rows.Add(Row);
    }
    Days.Add(MoveTemp(Entry));
    if (Days.Num() > Capacity) { Days.RemoveAt(0); }
    return true;
}
