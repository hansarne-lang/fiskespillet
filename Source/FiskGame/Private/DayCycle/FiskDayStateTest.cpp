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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDayClockScaleTest, "Fiskelaget.DayCycle.TestClockScale",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FFiskDayClockScaleTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Normal day is wall time"), AFiskDayState::ScaledElapsedMilliseconds(61.5, 1), 61500);
    TestEqual(TEXT("Scale multiplies elapsed time"), AFiskDayState::ScaledElapsedMilliseconds(15.0, 4), 60000);
    TestEqual(TEXT("Scaled day still ends at 20:00"), AFiskDayState::ScaledElapsedMilliseconds(400.0, 4), AFiskDayState::DayEnd * 1000);
    TestEqual(TEXT("Negative time clamps"), AFiskDayState::ScaledElapsedMilliseconds(-1.0, 4), 0);
    TestEqual(TEXT("Invalid scale is normal"), AFiskDayState::ScaledElapsedMilliseconds(10.0, 0), 10000);
    TestEqual(TEXT("Scale is capped"), AFiskDayState::ScaledElapsedMilliseconds(10.0, 1000), 200000);
    TestEqual(TEXT("No flag"), AFiskDayState::ParseTestClockScale(TEXT("-unattended")), 1);
    TestEqual(TEXT("Null command line"), AFiskDayState::ParseTestClockScale(nullptr), 1);
#if !UE_BUILD_SHIPPING
    TestEqual(TEXT("Flag read"), AFiskDayState::ParseTestClockScale(TEXT("-unattended -FiskClockScale=4")), 4);
    TestEqual(TEXT("Flag capped"), AFiskDayState::ParseTestClockScale(TEXT("-FiskClockScale=99")), AFiskDayState::MaxTestClockScale);
    TestEqual(TEXT("Flag floor"), AFiskDayState::ParseTestClockScale(TEXT("-FiskClockScale=0")), 1);
#endif
    return true;
}
#endif
