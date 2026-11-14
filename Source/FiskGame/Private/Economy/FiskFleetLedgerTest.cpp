#include "Economy/FiskDayLedger.h"
#include "World/FiskMarinaHub.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetDeadline,"Fiskelaget.Fleet.IndependentDeadlines",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetDeadline::RunTest(const FString&)
{
    FFiskDayLedger L;const FGuid A=FGuid::NewGuid(),B=FGuid::NewGuid(),C=FGuid::NewGuid();
    TestTrue(TEXT("Same fisher can have multiple transport records"),L.Record(A,1,200000));L.Record(B,1,300000);L.Record(C,2,400000);
    TMap<FGuid,int32> Arrivals;Arrivals.Add(A,1020000);Arrivals.Add(B,1020001);Arrivals.Add(C,900000);
    TestFalse(TEXT("All boats remain secret before 17"),L.SettleFleet(1019999,Arrivals));TestTrue(TEXT("One shared settlement"),L.SettleFleet(1020000,Arrivals));
    TestEqual(TEXT("Two owners"),L.GetResults().Num(),2);TestEqual(TEXT("First owner's early catch delivered"),L.GetResults()[0].DeliveredFish,1);TestEqual(TEXT("First owner's late catch lost"),L.GetResults()[0].LostFish,1);TestEqual(TEXT("Other boat independent"),L.GetResults()[1].DeliveredFish,1);TestFalse(TEXT("No repeat credit"),L.SettleFleet(1020000,Arrivals));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetMissing,"Fiskelaget.Fleet.MissingAndDiscardedCargo",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetMissing::RunTest(const FString&)
{
    FFiskDayLedger L;const auto A=FGuid::NewGuid(),B=FGuid::NewGuid(),C=FGuid::NewGuid();L.Record(A,1,1);L.Record(B,2,1);L.Record(C,3,1,FFiskCatchData(),FFiskCatchContext(),false);
    TMap<FGuid,int32> Arrivals;Arrivals.Add(B,-1);Arrivals.Add(C,500000);L.SettleFleet(1020000,Arrivals);
    for(const auto& Row:L.GetResults()) { TestEqual(TEXT("No arrival or uncarryable never pays"),Row.SessionBalance,0);TestEqual(TEXT("Loss preserved"),Row.LostFish,1); }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetTwelve,"Fiskelaget.Fleet.TwelveIndependentOwners",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetTwelve::RunTest(const FString&)
{
    FFiskDayLedger L;TMap<FGuid,int32> Arrivals;
    for(int32 I=0;I<12;++I) { const auto C=FGuid::NewGuid();L.Record(C,I,200000+I);Arrivals.Add(C,I%2?1020001:1020000); }
    L.SettleFleet(1020000,Arrivals);TestEqual(TEXT("Twelve rows"),L.GetResults().Num(),12);
    for(int32 I=0;I<12;++I) { TestEqual(TEXT("Own eligibility only"),L.GetResults()[I].DeliveredFish,I%2?0:1); }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetSlips,"Fiskelaget.Fleet.TwelveDistinctNausts",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetSlips::RunTest(const FString&)
{
    for(int32 I=0;I<12;++I) { for(int32 J=0;J<I;++J) { TestTrue(TEXT("Separate houses"),FVector::Dist(AFiskMarinaHub::HousePosition(I),AFiskMarinaHub::HousePosition(J))>=1700); } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskEarlyWeighInLedgerTest,"Fiskelaget.WeighIn.LedgerSettlesWhenEveryoneIsHome",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskEarlyWeighInLedgerTest::RunTest(const FString&)
{
    // #183: the weigh-in may settle the day before 17:00 once every boat that went out is home.
    const FFiskCatchData Fish=FiskCatches::Roll(42,0);const FGuid A=FGuid::NewGuid();
    FFiskDayLedger L;L.Record(A,1,300000,Fish);TMap<FGuid,int32> Home;Home.Add(A,350000);const TMap<FGuid,int32> None;
    TestFalse(TEXT("Not early while someone is out"),L.SettleFleet(400000,Home));
    TestTrue(TEXT("Early when everyone is home"),L.SettleFleet(400000,Home,true));
    TestEqual(TEXT("Delivered"),L.GetResults()[0].DeliveredFish,1);
    TestFalse(TEXT("No catch after the weigh-in has settled the day"),L.Record(FGuid::NewGuid(),1,500000,Fish));
    TestFalse(TEXT("Settled once"),L.SettleFleet(1020000,Home));
    FFiskDayLedger Bad;TestFalse(TEXT("Invalid time"),Bad.SettleFleet(-1,None,true));
    return true;
}
#endif
