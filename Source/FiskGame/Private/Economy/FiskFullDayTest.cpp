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
#endif
