#include "DayCycle/FiskDayState.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDayBoundaryTest, "Fiskelaget.DayCycle.Boundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFiskDayBoundaryTest::RunTest(const FString& Parameters)
{
    constexpr double Start = 42.0;
    const auto Check = [this](double Delta, int32 Seconds, EFiskDayPhase Phase)
    {
        const int32 ActualSeconds = AFiskDayState::CalculateElapsedSeconds(Start + Delta, Start);
        const FString Label = FString::Printf(TEXT("Elapsed %.3f"), Delta);
        TestEqual(Label + TEXT(" display"), ActualSeconds, Seconds);
        TestTrue(Label + TEXT(" phase"), AFiskDayState::GetPhase(ActualSeconds) == Phase);
    };

    Check(-0.001, 0, EFiskDayPhase::Preparation);
    Check(0.0, 0, EFiskDayPhase::Preparation);
    Check(119.999, 119, EFiskDayPhase::Preparation);
    Check(120.0, 120, EFiskDayPhase::FishingAndReturn);
    Check(1019.999, 1019, EFiskDayPhase::FishingAndReturn);
    Check(1020.0, 1020, EFiskDayPhase::Settlement);
    Check(1199.999, 1199, EFiskDayPhase::Settlement);
    Check(1200.0, 1200, EFiskDayPhase::Finished);
    Check(1250.0, 1200, EFiskDayPhase::Finished);
    return true;
}
#endif
