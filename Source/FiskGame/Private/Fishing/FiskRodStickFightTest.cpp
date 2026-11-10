#include "FiskRodStick.h"
#include "Fishing/FiskFightModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodStickFightTest,"Fiskelaget.Fishing.RodStickLandsFish",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodStickFightTest::RunTest(const FString& Parameters)
{
    // Steady up-and-down strokes without holding any button land a fish in the unchanged fight model.
    FFiskFightModel Fight; Fight.Reset(1234, 30.0f, 1, 0);
    FFiskRodStick S; S.Reset(0.2f);
    float Time = 0.0f;
    while (Time < 240.0f && Fight.GetOutcome() == EFiskFightOutcome::Fighting)
    {
        const float Phase = FMath::Fmod(Time, 3.0f);
        const float Speed = Fight.IsSurging() ? -0.15f : Phase < 1.8f ? 0.4f : -0.6f;
        S.Move(Speed / 30); S.Advance(1.0f / 30, Fight.IsSurging());
        Fight.SetRaised(S.WantsRaised()); Fight.SetReeling(S.WantsReeling());
        if (Fight.GetLoadPercent() > 90) { Fight.AdjustDrag(-1); } else if (Fight.GetLoadPercent() < 30) { Fight.AdjustDrag(1); }
        Fight.Advance(1.0 / 30); Time += 1.0f / 30;
    }
    TestEqual(TEXT("Fish ready to land with mouse strokes only"), static_cast<int32>(Fight.GetOutcome()), static_cast<int32>(EFiskFightOutcome::ReadyToLand));
    return true;
}
#endif
