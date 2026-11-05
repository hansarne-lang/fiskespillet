#include "Fishing/FiskRodSelection.h"
#include "Fishing/FiskRodCycle.h"
#include "Fishing/FiskRigSettings.h"
#include "Fishing/FiskFightModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodAllocationTest,"Fiskelaget.MultiRod.CapacityAndSelection",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodAllocationTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Empty boat"),FFiskRodSelection::Capacity(0),0);
    TestEqual(TEXT("Solo two holders"),FFiskRodSelection::Capacity(1),2);
    TestEqual(TEXT("Two crew one each"),FFiskRodSelection::Capacity(2),1);
    TestEqual(TEXT("Invalid crew rejected"),FFiskRodSelection::Capacity(3),0);
    FFiskRodSelection S;S.Reset(1);const FFiskRodSelection Initial=S;
    TestFalse(TEXT("No allocation before boarding"),S.Select(S.Revision,1,0,0));
    TestFalse(TEXT("Third rod not fitted"),S.Select(S.Revision,1,2,2));
    TestTrue(TEXT("Solo chooses second rod"),S.Select(S.Revision,1,1,2));
    TestFalse(TEXT("Old selection no longer matches"),S.Matches(Initial));
    TestFalse(TEXT("Old revision cannot choose another rod"),S.Select(Initial.Revision,1,0,2));
    TestFalse(TEXT("Another day rejected"),S.Select(S.Revision,2,0,2));
    TestTrue(TEXT("Return to first"),S.Select(S.Revision,1,0,2));
    TestFalse(TEXT("Passenger's extra slot denied"),S.Select(S.Revision,1,1,1));
    const FFiskRodSelection BeforeDay=S;S.Reset(2);
    TestFalse(TEXT("Prior day commands rejected"),S.Matches(BeforeDay));TestEqual(TEXT("New morning starts on first rod"),S.Slot,0);
    TestTrue(TEXT("Disconnect releases second holder"),S.Select(S.Revision,2,1,FFiskRodSelection::Capacity(1)));
    S.Revision=MAX_int32;TestFalse(TEXT("Selection revision saturates closed"),S.Select(MAX_int32,2,0,2));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodIndependenceTest,"Fiskelaget.MultiRod.IndependentCyclesAndRigs",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodIndependenceTest::RunTest(const FString& Parameters)
{
    FFiskRodCycle A,B;A.ResetForDay();B.ResetForDay();
    TestTrue(TEXT("Separate initial IDs"),A.GetId()!=B.GetId());
    TestTrue(TEXT("First cast"),A.TryStart(A.GetId(),160000));TestTrue(TEXT("Second cast"),B.TryStart(B.GetId(),160000));
    const FGuid Other=B.GetId();
    TestFalse(TEXT("Wrong rod ID rejected"),A.CanAct(Other));
    TestTrue(TEXT("Withdraw only first"),A.Withdraw(A.GetId()));TestTrue(TEXT("Second remains open"),B.CanAct(Other));
    TestTrue(TEXT("Second landing accepted"),B.FinishLanding(Other,200000));TestFalse(TEXT("No repeated landing"),B.FinishLanding(Other,200001));
    FFiskRigSettings R1,R2;
    TestTrue(TEXT("Change first rig"),FFiskRigRules::TryApply(R1,0,1,40,1));
    TestEqual(TEXT("Other bait unchanged"),R2.Bait,0);TestEqual(TEXT("Other depth unchanged"),R2.Depth,30);
    TestTrue(TEXT("Configure second independently"),FFiskRigRules::TryApply(R2,0,2,20,2));
    TestEqual(TEXT("First depth retained"),R1.Depth,40);
    A.ResetForDay();B.ResetForDay();TestFalse(TEXT("Old cast removed next day"),B.CanAct(Other));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSpeciesFightTest,"Fiskelaget.Fight.SpeciesProfilesAndLanding",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSpeciesFightTest::RunTest(const FString& Parameters)
{
    double Total[3]={0,0,0};
    for(int32 Species=0;Species<3;++Species)
    {
        for(int32 Seed=1;Seed<=100;++Seed)
        {
            FFiskFightModel Fight;Fight.Reset(Seed,60,Species);int32 Tick=0;
            for(;Tick<12000 && Fight.GetOutcome()==EFiskFightOutcome::Fighting;++Tick)
            {
                const bool Lift=!Fight.IsSurging() && (Tick/40)%2==0;
                Fight.SetRaised(Lift);Fight.SetReeling(!Fight.IsSurging() && !Lift);Fight.Advance(.05);
            }
            TestTrue(TEXT("Every profile can be landed with correct pumping"),Fight.GetOutcome()==EFiskFightOutcome::ReadyToLand);
            Total[Species]+=Tick*.05;
        }
    }
    TestTrue(TEXT("Snapper shorter than amberjack on average"),Total[0]<Total[1]);
    TestTrue(TEXT("Tuna longer than amberjack on average"),Total[2]>Total[1]);
    for(int32 Species=0;Species<3;++Species)
    {
        FFiskFightModel One,Many;One.Reset(31,60,Species);Many.Reset(31,60,Species);One.SetRaised(true);Many.SetRaised(true);
        One.Advance(12);for(int32 I=0;I<1200;++I) { Many.Advance(.01); }
        TestEqual(TEXT("All profiles use identical fixed-step timing"),One.GetLine(),Many.GetLine());
        TestEqual(TEXT("All profiles preserve load timing"),One.GetLoadPercent(),Many.GetLoadPercent());
    }
    FFiskFightModel First,Second;First.Reset(2);Second.Reset(3);First.SetRaised(true);First.SetReeling(true);
    First.SetRaised(false);First.SetReeling(false);Second.SetRaised(true);
    First.Advance(1);Second.Advance(1);
    TestTrue(TEXT("Released previous input reduces previous load"),First.GetLoadPercent()<Second.GetLoadPercent());
    return true;
}
#endif
