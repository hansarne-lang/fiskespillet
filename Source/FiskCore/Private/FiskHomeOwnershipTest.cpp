#include "FiskCareerProfile.h"
#include "FiskMarinaTypes.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeCatalogTest,"Fiskelaget.Home.CatalogAndConservation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeCatalogTest::RunTest(const FString& Parameters)
{
    auto P=FFiskCareerProfile::Create(TEXT("NAUST"));const auto Old=P;int64 Sum=0;
    for(int32 I=4;I<13;++I) { Sum+=P.Price(I);TestTrue(TEXT("Unique premium purchase"),P.Buy(FGuid::NewGuid(),P.Revision,I));TestTrue(TEXT("Entitled after debit"),P.Has(I));TestTrue(TEXT("Full chain validates"),P.IsValid()); }
    TestEqual(TEXT("Nine test prices"),Sum,970000LL);TestEqual(TEXT("Money conserved"),P.Cash,2030000LL);TestEqual(TEXT("Nine debits only"),P.Revision,10);TestTrue(TEXT("Extends old v1 grant"),P.Extends(Old));
    TestFalse(TEXT("No equipment granted by cosmetics"),P.Has(0));TestFalse(TEXT("No boat granted"),P.Has(3));TestFalse(TEXT("Insufficient balance denies offshore"),P.Buy(FGuid::NewGuid(),P.Revision,3));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeEntitlementTest,"Fiskelaget.Home.EntitlementAndForgery",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeEntitlementTest::RunTest(const FString& Parameters)
{
    auto P=FFiskCareerProfile::Create(TEXT("NAUST"));FFiskMarinaIdentity V;TestTrue(TEXT("Free choices remain free"),V.IsOwnedBy(P));V.Lighting=3;TestFalse(TEXT("Unpaid activation rejected"),V.IsOwnedBy(P));
    P.Buy(FGuid::NewGuid(),P.Revision,6);TestTrue(TEXT("Top tier may be bought directly"),V.IsOwnedBy(P));V.Pier=1;TestFalse(TEXT("Lighting does not unlock pier"),V.IsOwnedBy(P));
    auto Forged=P;Forged.Owned|=1<<7;TestFalse(TEXT("Ownership without debit invalid"),Forged.IsValid());Forged=P;Forged.Receipts.Last().Amount=0;TestFalse(TEXT("Free fake purchase invalid"),Forged.IsValid());
    Forged=P;Forged.Receipts.Last().Item=31;TestFalse(TEXT("Out-of-range shift never accepted"),Forged.IsValid());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeReplayTest,"Fiskelaget.Home.ReplayRevisionAndLimits",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeReplayTest::RunTest(const FString& Parameters)
{
    auto P=FFiskCareerProfile::Create(TEXT("NAUST"));const auto Cmd=FGuid::NewGuid();TestTrue(TEXT("Purchase"),P.Buy(Cmd,1,4));
    TestTrue(TEXT("Reliable retry is idempotent"),P.Buy(Cmd,1,4));TestEqual(TEXT("One debit"),P.Cash,2975000LL);TestEqual(TEXT("One receipt"),P.Revision,2);
    TestFalse(TEXT("Conflicting retry"),P.Buy(Cmd,2,5));TestFalse(TEXT("Stale revision"),P.Buy(FGuid::NewGuid(),1,5));TestFalse(TEXT("Rebuy denied"),P.Buy(FGuid::NewGuid(),2,4));
    TestFalse(TEXT("Unknown category"),P.Buy(FGuid::NewGuid(),2,13));TestFalse(TEXT("Negative item"),P.Buy(FGuid::NewGuid(),2,-1));
    while(P.Revision<P.MaxReceipts) { if(!P.Credit(FGuid::NewGuid(),0)) { return false; } }
    TestFalse(TEXT("Capacity denies new debit"),P.Buy(FGuid::NewGuid(),P.Revision,5));TestTrue(TEXT("Original retry still works"),P.Buy(Cmd,1,4));return true;
}
#endif
