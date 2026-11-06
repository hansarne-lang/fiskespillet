#include "Fishing/FiskFightModel.h"
#include "Fishing/FiskCombatFeedback.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

namespace
{
    void Pump(FFiskFightModel& Fight, int32 Tick)
    {
        const bool bLift = !Fight.IsSurging() && (Tick / 40) % 2 == 0;
        Fight.SetRaised(bLift);
        Fight.SetReeling(!Fight.IsSurging() && !bLift);
        Fight.Advance(0.05);
    }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFightLandingTest, "Fiskelaget.Fight.PumpingAndRuns",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskFightLandingTest::RunTest(const FString& Parameters)
{
    for (int32 Seed = 1; Seed <= 100; ++Seed)
    {
        FFiskFightModel Fight; Fight.Reset(Seed);
        bool bRun = false, bLineOutDuringRun = false;
        for (int32 Tick = 0; Tick < 2400 && Fight.GetOutcome() == EFiskFightOutcome::Fighting; ++Tick)
        {
            const float Before = Fight.GetLine();
            Pump(Fight, Tick);
            bRun |= Fight.IsSurging();
            bLineOutDuringRun |= Fight.IsSurging() && Fight.GetLine() > Before;
        }
        TestTrue(TEXT("Pumping lands across random run schedules"), Fight.GetOutcome() == EFiskFightOutcome::ReadyToLand);
        TestTrue(TEXT("Run takes line out"), bRun && bLineOutDuringRun);
        TestTrue(TEXT("Near boat at landing"), Fight.GetLine() <= 2.0f);
    }
    FFiskFightModel NoPump; NoPump.Reset(7); NoPump.SetReeling(true); NoPump.Advance(120);
    TestTrue(TEXT("Holding reel without pumping cannot land"), NoPump.GetOutcome() != EFiskFightOutcome::ReadyToLand);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFightLossTest, "Fiskelaget.Fight.LossAndIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskFightLossTest::RunTest(const FString& Parameters)
{
    for (bool bBreak : {true, false})
    {
        FFiskFightModel Host, Guest; Host.Reset(11); Guest.Reset(22);
        for (int32 I = 0; I < 20; ++I) { Guest.AdjustDrag(bBreak ? 1 : -1); }
        TestEqual(TEXT("Bounded drag"), Guest.GetDragPercent(), bBreak ? 100 : 10);
        Guest.SetRaised(bBreak); Guest.SetReeling(bBreak);
        for (int32 Tick = 0; Tick < 2400 && Host.GetOutcome() == EFiskFightOutcome::Fighting; ++Tick)
        {
            Guest.Advance(0.05); Pump(Host, Tick);
        }
        TestTrue(TEXT("Expected loss reason"), Guest.GetOutcome() == (bBreak ? EFiskFightOutcome::LineBreak : EFiskFightOutcome::Slack));
        TestTrue(TEXT("Other fight can still land"), Host.GetOutcome() == EFiskFightOutcome::ReadyToLand);
    }
    FFiskFightModel Recover; Recover.Reset(4);
    Recover.SetRaised(true); Recover.SetReeling(true);
    for (int32 I = 0; I < 6; ++I) { Recover.AdjustDrag(1); }
    Recover.Advance(0.5); Recover.SetReeling(false); Recover.SetRaised(false);
    for (int32 I = 0; I < 6; ++I) { Recover.AdjustDrag(-1); }
    Recover.Advance(0.5);
    TestTrue(TEXT("Releasing load in time prevents break"), Recover.GetOutcome() == EFiskFightOutcome::Fighting);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFightTimingTest, "Fiskelaget.Fight.RealTimeSteps",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskFightTimingTest::RunTest(const FString& Parameters)
{
    FFiskFightModel One, Many; One.Reset(31); Many.Reset(31);
    One.SetRaised(true); Many.SetRaised(true);
    One.Advance(12.0);
    for (int32 I = 0; I < 1200; ++I) { Many.Advance(0.01); }
    TestEqual(TEXT("Render-independent line"), One.GetLine(), Many.GetLine());
    TestEqual(TEXT("Render-independent load"), One.GetLoadPercent(), Many.GetLoadPercent());
    TestEqual(TEXT("Render-independent runs"), One.IsSurging(), Many.IsSurging());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskReelAudioTest, "Fiskelaget.Fight.ReelSignal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskReelAudioTest::RunTest(const FString& Parameters)
{
    TArray<int16> Quiet, Reel, Run; uint64 Offset = 0;
    FiskReelSound::Fill(Quiet, 2205, false, false, 40, Offset); Offset = 0;
    FiskReelSound::Fill(Reel, 2205, true, false, 50, Offset); Offset = 0;
    FiskReelSound::Fill(Run, 2205, true, true, 90, Offset);
    int32 Energy = 0; bool bDifferent = false;
    for (int32 I = 0; I < Quiet.Num(); ++I)
    {
        if (Quiet[I] != 0) { AddError(TEXT("Idle sound must be silent")); break; }
        Energy += FMath::Abs(static_cast<int32>(Reel[I]));
        bDifferent |= Reel[I] != Run[I];
        if (FMath::Abs(static_cast<int32>(Run[I])) > 7000) { AddError(TEXT("Unexpected audio clipping")); break; }
    }
    TestTrue(TEXT("Audible signal has energy"), Energy > 10000);
    TestTrue(TEXT("Run and reel signals differ"), bDifferent);
    return true;
}
#endif
