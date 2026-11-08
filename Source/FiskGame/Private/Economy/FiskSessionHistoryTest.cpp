#include "Economy/FiskSessionHistory.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHistoryPrivacyTest,"Fiskelaget.Session.HistoryPrivacyAndIdempotence",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHistoryPrivacyTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;FFiskSessionHistory History;
    const FFiskCatchData Fish=FiskCatches::Roll(42,0);
    Ledger.Record(FGuid::NewGuid(),7,1000,Fish);
    TestFalse(TEXT("No unpublished ledger"),History.Append(1,1020000,Ledger));
    TestTrue(TEXT("Settle at deadline"),Ledger.Settle(1020000,900000));
    TestFalse(TEXT("No early history even with settled input"),History.Append(1,1019999,Ledger));
    TestTrue(TEXT("Publish once at deadline"),History.Append(1,1020000,Ledger));
    TestFalse(TEXT("Duplicate day cannot double income"),History.Append(1,1200000,Ledger));
    const FFiskHistoryRow& Row=History.GetDays()[0].Rows[0];
    TestEqual(TEXT("Delivered once"),Row.Delivered,1);TestEqual(TEXT("Exact income"),Row.Income,Fish.Value);
    TestEqual(TEXT("Largest landed record"),Row.BestGrams,Fish.WeightGrams);
    TestEqual(TEXT("Confirmed species"),Row.BestSpecies,Fish.Species);
    FFiskDayLedger Next;Next.Record(FGuid::NewGuid(),7,2000,FiskCatches::Roll(88,2));
    TestFalse(TEXT("Next day's private catches cannot enter history"),History.Append(2,2000,Next));
    TestEqual(TEXT("Old day retained without new data"),History.GetDays().Num(),1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHistoryBoundTest,"Fiskelaget.Session.HistoryBoundAndLateRecord",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHistoryBoundTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Late;FFiskSessionHistory History;
    const FFiskCatchData Fish=FiskCatches::Roll(91,2);
    Late.Record(FGuid::NewGuid(),9,1000,Fish);Late.Settle(1020000,1020001);
    History.Append(1,1020000,Late);
    const FFiskHistoryRow& Row=History.GetDays()[0].Rows[0];
    TestEqual(TEXT("Late fish lost"),Row.Lost,1);TestEqual(TEXT("No late income"),Row.Income,0);
    TestEqual(TEXT("Late landed record retained"),Row.BestGrams,Fish.WeightGrams);
    FFiskDayLedger Empty;Empty.Settle(1020000,-1);
    for(int32 Day=2;Day<=35;++Day) { TestTrue(TEXT("Append empty settled day"),History.Append(Day,1020000,Empty)); }
    TestEqual(TEXT("Bounded history"),History.GetDays().Num(),32);
    TestEqual(TEXT("Oldest evicted"),History.GetDays()[0].Number,4);
    TestEqual(TEXT("Newest retained"),History.GetDays().Last().Number,35);
    TestFalse(TEXT("Old eviction cannot be replayed"),History.Append(1,1020000,Late));
    return true;
}
#endif
