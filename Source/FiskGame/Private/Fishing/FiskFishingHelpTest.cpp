#include "Fishing/FiskTestPilot.h"
#include "Fishing/FiskFightModel.h"
#include "Fishing/FiskBiteAlert.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPilotSpeciesTest,"Fiskelaget.TestHelp.AllSpeciesWithNormalPhysics",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPilotSpeciesTest::RunTest(const FString&)
{
    for(int32 Species=0;Species<15;++Species)
    for(int32 Gear:{0,7})
    for(int32 Seed=0;Seed<32;++Seed)
    {
        FFiskFightModel Fight;Fight.Reset(Seed,250,Species,Gear);
        FFiskTestPilot Pilot;
        // Begin with a badly tightened brake to exercise safe recovery too.
        for(int32 I=0;I<6;++I) { Fight.AdjustDrag(1); }
        for(int32 Step=0;Step<3600 && Fight.GetOutcome()==EFiskFightOutcome::Fighting;++Step)
        {
            const auto Intent=Pilot.Step(.1,Fight.IsSurging(),Fight.GetLoadPercent(),Fight.GetDragPercent());
            Fight.SetRaised(Intent.bRaise);Fight.SetReeling(Intent.bReel);Fight.AdjustDrag(Intent.DragStep);
            // AdjustDrag deliberately takes only nonzero steps in the actual controller.
            Fight.Advance(.1);
        }
        if(Fight.GetOutcome()!=EFiskFightOutcome::ReadyToLand)
        { AddError(FString::Printf(TEXT("Pilot lost or timed out: species=%d gear=%d seed=%d outcome=%d line=%.1f"),Species,Gear,Seed,static_cast<int32>(Fight.GetOutcome()),Fight.GetLine()));return false; }
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPilotSafetyTest,"Fiskelaget.TestHelp.SurgeAndLoadRelease",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPilotSafetyTest::RunTest(const FString&)
{
    FFiskTestPilot Pilot;
    for(int32 I=0;I<100;++I) { const auto A=Pilot.Step(.1,false,55,70);TestFalse(TEXT("Never lift and reel together"),A.bRaise && A.bReel); }
    for(bool Run:{false,true})
    {
        const auto A=Pilot.Step(.1,Run,120,100);
        TestFalse(TEXT("Release lift under danger"),A.bRaise);TestFalse(TEXT("Release reel under danger"),A.bReel);TestEqual(TEXT("Loosen brake"),A.DragStep,-1);
    }
    const auto Rest=Pilot.Step(.1,true,74,40);TestEqual(TEXT("Keep a safe run brake"),Rest.DragStep,0);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAssistKeyboardTest,"Fiskelaget.TestHelp.NorwegianAaKey",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAssistKeyboardTest::RunTest(const FString&)
{
    TestTrue(TEXT("Norwegian uppercase key name"),FiskQuickControls::IsAssistKey(FKey(FName(TEXT("Å")))));
    TestTrue(TEXT("Norwegian lowercase key name"),FiskQuickControls::IsAssistKey(FKey(FName(TEXT("å")))));
    TestTrue(TEXT("Engine character mapping uppercase"),FiskQuickControls::IsAssistKey(FInputKeyManager::Get().GetKeyFromCodes(0,0x00c5)));
    TestTrue(TEXT("Engine character mapping lowercase"),FiskQuickControls::IsAssistKey(FInputKeyManager::Get().GetKeyFromCodes(0,0x00e5)));
    TestFalse(TEXT("A does not enable test help"),FiskQuickControls::IsAssistKey(EKeys::A));
    TestFalse(TEXT("Arrow does not enable test help"),FiskQuickControls::IsAssistKey(EKeys::Left));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBiteBellTest,"Fiskelaget.TestHelp.BiteBellSignal",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBiteBellTest::RunTest(const FString&)
{
    int64 Difference=0;
    for(int32 Slot=0;Slot<3;++Slot)
    {
        double Energy=0;int32 Peak=0;
        for(uint64 I=0;I<static_cast<uint64>(FiskBiteSound::SampleRate*FiskBiteSound::Duration);++I)
        {
            const int32 Sample=FiskBiteSound::Sample(I,Slot);Energy+=static_cast<double>(Sample)*Sample;Peak=FMath::Max(Peak,FMath::Abs(Sample));
            if(Slot>0) { Difference+=FMath::Abs(Sample-FiskBiteSound::Sample(I,0)); }
        }
        TestTrue(TEXT("Bell is audible PCM, not silence"),Energy>1000000000.0);
        TestTrue(TEXT("Bell peak is bounded"),Peak>1000 && Peak<30000);
        TestEqual(TEXT("Bell ends in silence"),FiskBiteSound::Sample(22050,Slot),static_cast<int16>(0));
    }
    TestTrue(TEXT("Own rods have different pitches"),Difference>1000000);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskReturnGuardTest,"Fiskelaget.TestHelp.ReturnDirectionAndButtons",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskReturnGuardTest::RunTest(const FString&)
{
    for(int32 P:{0,25,99,100}) { TestTrue(TEXT("Captain may return from outward leg or spot"),FiskQuickControls::CanReturn(true,true,true,true,true,true,P)); }
    for(int32 P:{-2,-1,101,199,200}) { TestFalse(TEXT("Return can never depart at dock or reverse a homeward leg"),FiskQuickControls::CanReturn(true,true,true,true,true,true,P)); }
    for(int32 Missing=0;Missing<6;++Missing)
    {
        bool V[]={true,true,true,true,true,true};V[Missing]=false;
        TestFalse(TEXT("Ready, aboard, captain, day, revision and leg all required"),FiskQuickControls::CanReturn(V[0],V[1],V[2],V[3],V[4],V[5],100));
    }
    for(float S:{.5f,1.f,1.5f,2.f})
    {
        TestTrue(TEXT("Return button scales"),FiskQuickControls::ReturnBox(S).IsInside(FVector2D(1080,195)*S));
        TestTrue(TEXT("Help button scales"),FiskQuickControls::AssistBox(S).IsInside(FVector2D(1080,270)*S));
        TestFalse(TEXT("Return and help buttons do not overlap"),FiskQuickControls::ReturnBox(S).Intersect(FiskQuickControls::AssistBox(S)));
    }
    return true;
}
