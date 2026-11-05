#include "Economy/FiskDayLedger.h"
#include "Boats/FiskStarterBoat.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSettlementTest, "Fiskelaget.FullDay.Settlement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFiskSettlementTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;
    const FGuid First = FGuid::NewGuid();
    TestTrue(TEXT("First catch"), Ledger.Record(First, 256, 200000));
    TestFalse(TEXT("Duplicate catch under another player"), Ledger.Record(First, 257, 200000));
    TestFalse(TEXT("Invalid catch ID"), Ledger.Record(FGuid(), 256, 200000));
    TestFalse(TEXT("Catch at settlement barrier"), Ledger.Record(FGuid::NewGuid(), 256, 1020000));
    TestTrue(TEXT("Other player's catch"), Ledger.Record(FGuid::NewGuid(), 257, 200000));
    TestFalse(TEXT("No early settlement"), Ledger.Settle(1019999, 500000));
    TestEqual(TEXT("No result leak"), Ledger.GetResults().Num(), 0);
    TestTrue(TEXT("Settle at 17:00"), Ledger.Settle(1020000, 1020000));
    TestEqual(TEXT("Two players"), Ledger.GetResults().Num(), 2);
    for (const FFiskDayResult& Row : Ledger.GetResults())
    {
        TestEqual(TEXT("One delivered fish"), Row.DeliveredFish, 1);
        TestEqual(TEXT("Only one payment"), Row.SessionBalance, 10);
    }
    TestFalse(TEXT("Repeated settlement"), Ledger.Settle(1020001, 1020000));
    TestFalse(TEXT("Catch after settlement"), Ledger.Record(FGuid::NewGuid(), 256, 100));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskArrivalTest, "Fiskelaget.FullDay.ArrivalBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFiskArrivalTest::RunTest(const FString& Parameters)
{
    for (int32 Arrival : {1019999, 1020000, 1020001, -1})
    {
        FFiskDayLedger Ledger;
        Ledger.Record(FGuid::NewGuid(), 256, 200000);
        Ledger.Record(FGuid::NewGuid(), 256, 300000);
        Ledger.Settle(1020010, Arrival);
        const bool bOnTime = Arrival >= 0 && Arrival <= FFiskDayLedger::DeadlineMs;
        TestEqual(TEXT("Entire catch delivery"), Ledger.GetResults()[0].DeliveredFish, bOnTime ? 2 : 0);
        TestEqual(TEXT("Entire catch loss"), Ledger.GetResults()[0].LostFish, bOnTime ? 0 : 2);
        TestEqual(TEXT("Correct payment"), Ledger.GetResults()[0].SessionBalance, bOnTime ? 20 : 0);
    }
    TestEqual(TEXT("Circle crossing at deadline"), AFiskStarterBoat::CalculateCrossingMs(1008300), 1020000);
    TestEqual(TEXT("Circle crossing one ms late"), AFiskStarterBoat::CalculateCrossingMs(1008301), 1020001);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMultipleCatchesTest, "Fiskelaget.FullDay.MultipleCatches",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMultipleCatchesTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;
    for (int32 Player : {256, 257})
    {
        for (int32 Fish = 0; Fish < 2; ++Fish)
        {
            const FGuid Id = FGuid::NewGuid();
            TestTrue(TEXT("Unique landing"), Ledger.Record(Id, Player, 300000 + Fish * 180000));
            TestFalse(TEXT("Duplicate cannot change owner or value"), Ledger.Record(Id, Player + 1, 600000));
        }
    }
    TestFalse(TEXT("No early payment"), Ledger.Settle(1019999, 900000));
    TestEqual(TEXT("No partial result exposure"), Ledger.GetResults().Num(), 0);
    TestTrue(TEXT("One shared settlement"), Ledger.Settle(1020000, 900000));
    TestEqual(TEXT("Both players present"), Ledger.GetResults().Num(), 2);
    for (const FFiskDayResult& Row : Ledger.GetResults())
    {
        TestEqual(TEXT("Two fish per player"), Row.DeliveredFish, 2);
        TestEqual(TEXT("No delivered fish lost"), Row.LostFish, 0);
        TestEqual(TEXT("One payment per fish"), Row.SessionBalance, 20);
    }
    TestFalse(TEXT("No double payout"), Ledger.Settle(1200000, 900000));
    TestFalse(TEXT("No catches after barrier"), Ledger.Record(FGuid::NewGuid(), 256, 900000));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCatchDetailsTest,"Fiskelaget.Content.PrivateDetailedSettlement",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCatchDetailsTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;const FGuid Id=FGuid::NewGuid();
    const FFiskCatchData First=FiskCatches::Roll(127,0),Second=FiskCatches::Roll(51,2);
    TestTrue(TEXT("Store first server-generated fish"),Ledger.Record(Id,256,300000,First));
    TestFalse(TEXT("Duplicate cannot replace fish or owner"),Ledger.Record(Id,257,400000,Second));
    TestTrue(TEXT("Store another species"),Ledger.Record(FGuid::NewGuid(),256,500000,Second));
    TestEqual(TEXT("No details exposed during day"),Ledger.GetResults().Num(),0);
    TestFalse(TEXT("No details one ms early"),Ledger.Settle(1019999,900000));
    TestTrue(TEXT("Reveal exactly at barrier"),Ledger.Settle(1020000,1020000));
    TestEqual(TEXT("One player row"),Ledger.GetResults().Num(),1);
    if(Ledger.GetResults().Num()!=1) { return false; }
    const FFiskDayResult& Row=Ledger.GetResults()[0];
    TestEqual(TEXT("Every fish appears"),Row.Catches.Num(),2);
    TestEqual(TEXT("Exact integer sum"),Row.SessionBalance,First.Value+Second.Value);
    TestEqual(TEXT("Exact weight sum"),Row.DeliveredWeightGrams,First.WeightGrams+Second.WeightGrams);
    const FFiskCatchResult* Detail=Row.Catches.FindByPredicate([&](const FFiskCatchResult& C){return C.CatchId==Id;});
    TestTrue(TEXT("Immutable original catch survives"),Detail && Detail->Fish.WeightGrams==First.WeightGrams && Detail->Fish.Species==0 && Detail->bDelivered);
    TestFalse(TEXT("Payout cannot repeat"),Ledger.Settle(1200000,900000));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskLostDetailsTest,"Fiskelaget.Content.LateReturnAndInvalidCatch",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskLostDetailsTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;FFiskCatchData Fish=FiskCatches::Roll(42,1),Bad=Fish;Bad.Value=-1;
    TestFalse(TEXT("Bad value cannot enter ledger"),Ledger.Record(FGuid::NewGuid(),256,400000,Bad));
    TestTrue(TEXT("Valid fish enters ledger"),Ledger.Record(FGuid::NewGuid(),256,400000,Fish));
    TestTrue(TEXT("Late arrival settles once"),Ledger.Settle(1020000,1020001));
    const FFiskDayResult& Row=Ledger.GetResults()[0];
    TestEqual(TEXT("No revenue from lost fish"),Row.SessionBalance,0);
    TestEqual(TEXT("No delivered weight"),Row.DeliveredWeightGrams,0);
    TestEqual(TEXT("Lost count"),Row.LostFish,1);
    TestTrue(TEXT("Details revealed as lost"),Row.Catches.Num()==1 && !Row.Catches[0].bDelivered && Row.Catches[0].Fish.Species==1);
    return true;
}
#endif
