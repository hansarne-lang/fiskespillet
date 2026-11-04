#include "FiskFishingGround.h"
#include "Fishing/FiskRigSettings.h"
#include "Boats/FiskStarterBoat.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGroundCatalogTest,"Fiskelaget.Grounds.CatalogAndRigDepth",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskGroundCatalogTest::RunTest(const FString& Parameters)
{
    TestNull(TEXT("Negative location"),FiskGrounds::Find(-1));TestNull(TEXT("Unknown location"),FiskGrounds::Find(3));
    for(int32 Id=0;Id<FiskGrounds::Count;++Id)
    {
        const FFiskFishingGround* G=FiskGrounds::Find(Id);
        if(!TestNotNull(TEXT("Known location"),G)) { return false; }
        TestTrue(TEXT("Sea point beyond marina"),G->Position.X>0 && G->Position.Size()>AFiskStarterBoat::MarinaRadiusCm);
        TestTrue(TEXT("Fish signs above bottom"),G->FishDepthMin>=10 && G->FishDepthMax<=G->BottomDepth);
        FFiskRigSettings R;R.Depth=G->FishDepthMin;
        TestEqual(TEXT("Suitable depth and movement"),FFiskRigRules::Quality(R,G),2);
        TestTrue(TEXT("Can apply selected depth"),FFiskRigRules::TryApply(R,0,0,R.Depth,0,G->BottomDepth));
        TestFalse(TEXT("No fishing below bottom"),FFiskRigRules::IsValid(0,G->BottomDepth+10,0,G->BottomDepth));
        FRandomStream Random(55);const double Wait=FFiskRigRules::BiteDelay(R,Random,G);
        TestTrue(TEXT("Ground uses good interval"),Wait>=120 && Wait<=180);
    }
    FFiskRigSettings Deep;Deep.Depth=90;
    TestTrue(TEXT("Deep setup at drop-off"),FFiskRigRules::IsValid(0,90,0,FiskGrounds::Find(1)->BottomDepth));
    TestFalse(TEXT("Same rig invalid at reef"),FFiskRigRules::IsValid(0,90,0,FiskGrounds::Find(0)->BottomDepth));
    TestEqual(TEXT("Old default preserved"),FFiskRigRules::Quality(FFiskRigSettings(),FiskGrounds::Find(0)),2);
    TestTrue(TEXT("Distinct destinations"),FiskGrounds::Find(1)->Position!=FiskGrounds::Find(2)->Position);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGroundTimingTest,"Fiskelaget.Grounds.ReturnDeadlineAndMargin",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskGroundTimingTest::RunTest(const FString& Parameters)
{
    for(int32 Id=0;Id<FiskGrounds::Count;++Id)
    {
        const FFiskFishingGround& G=*FiskGrounds::Find(Id);
        const int32 Leg=FiskGrounds::CrossingMs(0,Id);
        TestTrue(TEXT("Circle before dock center"),Leg>0 && Leg<G.TravelMs);
        TestEqual(TEXT("Exact deadline crossing"),AFiskStarterBoat::CalculateCrossingMs(1020000-Leg,Id),1020000);
        TestEqual(TEXT("One millisecond late"),AFiskStarterBoat::CalculateCrossingMs(1020001-Leg,Id),1020001);
        TestEqual(TEXT("One millisecond early"),AFiskStarterBoat::CalculateCrossingMs(1019999-Leg,Id),1019999);
        TestEqual(TEXT("Preparation includes waiting"),FiskGrounds::ReturnMarginMs(0,-1,Id),900000-2*G.TravelMs);
        TestEqual(TEXT("Return margin at spot"),FiskGrounds::ReturnMarginMs(900000,100,Id),120000-G.TravelMs);
        TestEqual(TEXT("Negative margin"),FiskGrounds::ReturnMarginMs(1020000,100,Id),-G.TravelMs);
        TestEqual(TEXT("First return percent stays conservative"),FiskGrounds::ReturnMarginMs(900000,101,Id),120000-G.TravelMs);
        TestEqual(TEXT("Halfway home"),FiskGrounds::ReturnMarginMs(900000,150,Id),120000-G.TravelMs/2);
    }
    TestEqual(TEXT("Legacy reef boundary unchanged"),AFiskStarterBoat::CalculateCrossingMs(1008300),1020000);
    TestEqual(TEXT("Invalid route fails closed"),FiskGrounds::CrossingMs(0,99),-1);
    return true;
}
#endif
