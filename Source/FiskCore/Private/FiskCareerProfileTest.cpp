#include "FiskCareerProfile.h"
#include "FiskBoatLayout.h"
#include "FiskRouteGeometry.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCareerLedgerTest,"Fiskelaget.Career.ReceiptConservationAndReplay",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCareerLedgerTest::RunTest(const FString& Parameters)
{
    FFiskCareerProfile P=FFiskCareerProfile::Create(TEXT("FISKER"));const FFiskCareerProfile Initial=P;
    TestTrue(TEXT("Initial grant valid"),P.IsValid());TestEqual(TEXT("Test grant"),P.Cash,3000000LL);
    const FGuid Buy=FGuid::NewGuid(),Day=FGuid::NewGuid();
    TestFalse(TEXT("Stale purchase denied"),P.Buy(Buy,0,0));TestFalse(TEXT("Unknown item denied"),P.Buy(Buy,1,99));
    TestTrue(TEXT("Buy offshore"),P.Buy(Buy,1,3));TestEqual(TEXT("Price deducted"),P.Cash,500000LL);
    TestTrue(TEXT("Replay same command"),P.Buy(Buy,1,3));TestEqual(TEXT("No duplicate debit"),P.Revision,2);
    TestFalse(TEXT("Same id other item rejected"),P.Buy(Buy,2,0));
    TestFalse(TEXT("Negative income denied"),P.Credit(Day,-1));TestTrue(TEXT("Settlement credit"),P.Credit(Day,47200));
    TestTrue(TEXT("Credit replay"),P.Credit(Day,47200));TestEqual(TEXT("Credit exactly once"),P.Cash,547200LL);
    TestFalse(TEXT("Conflicting replay rejected"),P.Credit(Day,100));
    TestTrue(TEXT("Chain extends initial"),P.Extends(Initial));TestTrue(TEXT("Conservation validates"),P.IsValid());
    FFiskCareerProfile Forged=P;++Forged.Cash;TestFalse(TEXT("Unexplained money rejected"),Forged.IsValid());
    Forged=P;Forged.Owned=15;TestFalse(TEXT("Unpaid equipment rejected"),Forged.IsValid());
    Forged=P;Forged.Receipts.Add(Forged.Receipts.Last());++Forged.Revision;TestFalse(TEXT("Duplicate receipt rejected"),Forged.IsValid());
    FFiskCareerProfile Branch=Initial;TestTrue(TEXT("Other valid branch"),Branch.Buy(FGuid::NewGuid(),1,0));TestFalse(TEXT("Divergent branch is not imported over known history"),P.Extends(Branch));
    while(P.Revision<FFiskCareerProfile::MaxReceipts) { if(!P.Credit(FGuid::NewGuid(),0)) { AddError(TEXT("Could not fill receipt log"));return false; } }
    TestFalse(TEXT("Full ledger fails closed"),P.Credit(FGuid::NewGuid(),0));TestTrue(TEXT("Old receipt still idempotent at capacity"),P.Credit(Day,47200));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskOffshoreLayoutTest,"Fiskelaget.Boats.OffshoreRodAllocation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskOffshoreLayoutTest::RunTest(const FString& Parameters)
{
    for(int32 Kind=0;Kind<2;++Kind) { for(int32 Crew=1;Crew<=FiskBoatLayout::Seats(Kind);++Crew)
    {
        TSet<int32> Holders;
        for(int32 Seat=0;Seat<Crew;++Seat) { const int32 Count=FiskBoatLayout::Rods(Kind,Crew,Seat);TestTrue(TEXT("One to three owned rods"),Count>=1 && Count<=3);
            for(int32 Slot=0;Slot<Count;++Slot) { const int32 Holder=FiskBoatLayout::FirstHolder(Kind,Crew,Seat)+Slot;TestFalse(TEXT("No shared holder"),Holders.Contains(Holder));Holders.Add(Holder); } }
        TestTrue(TEXT("Within physical capacity"),Holders.Num()<=FiskBoatLayout::Holders(Kind));
    } }
    TestEqual(TEXT("Offshore solo cap"),FiskBoatLayout::Rods(1,1,0),3);
    TestEqual(TEXT("Fourth gets one"),FiskBoatLayout::Rods(1,4,3),1);
    TestEqual(TEXT("Invalid crew rejected"),FiskBoatLayout::Rods(1,5,0),0);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSeaGeometryTest,"Fiskelaget.Navigation.FreeSeaAndIslandAvoidance",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSeaGeometryTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("Marina not a fishing destination"),FiskRouteGeometry::Sea(FVector::ZeroVector));
    TestFalse(TEXT("Island rejected"),FiskRouteGeometry::Sea(FiskRouteGeometry::Island(0)));
    TestFalse(TEXT("Outside chart rejected"),FiskRouteGeometry::Sea(FVector(160000,0,0)));
    for(int32 X=10000;X<=150000;X+=10000) { for(int32 Y=-100000;Y<=100000;Y+=10000)
    {
        const FVector Destination(X,Y,0);if(!FiskRouteGeometry::Sea(Destination)) { continue; }
        const TArray<FVector> P=FiskRouteGeometry::Path(Destination);TestTrue(TEXT("Every valid point reachable"),P.Num()>=2);
        if(P.Num()<2) { continue; }TestTrue(TEXT("Ends at exact point"),P.Last().Equals(Destination));
        for(int32 I=1;I<P.Num();++I) { TestTrue(TEXT("Segments clear of land"),FiskRouteGeometry::Clear(P[I-1],P[I])); }
        TestTrue(TEXT("End interpolation"),FiskRouteGeometry::Position(P,1).Equals(Destination));
    } }
    TestTrue(TEXT("Route behind island detours"),FiskRouteGeometry::Path(FVector(70000,85000,0)).Num()>2);return true;
}
#endif
