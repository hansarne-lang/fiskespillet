#include "FiskRodStick.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
namespace
{
    // Moves the rod at a steady speed (angle per second) for a while in 60 Hz frames.
    void Sweep(FFiskRodStick& S, float Speed, float Seconds, bool bSurging = false)
    { for (float T = 0; T < Seconds; T += 1.0f / 60) { S.Move(Speed / 60); S.Advance(1.0f / 60, bSurging); } }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodStickTest,"Fiskelaget.Fishing.RodStickInputs",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodStickTest::RunTest(const FString& Parameters)
{
    FFiskRodStick S; S.Reset(0.3f);
    S.Advance(1.0f / 60, false);
    TestFalse(TEXT("Low still rod rests"), S.WantsRaised() || S.WantsReeling());
    Sweep(S, 0.8f, 0.4f);
    TestTrue(TEXT("Moving up lifts"), S.WantsRaised() && !S.WantsReeling() && S.GetDirection() == 1);
    Sweep(S, 0.0f, 0.5f);
    TestTrue(TEXT("Still high rod keeps tension"), S.GetAngle() >= FFiskRodStick::HoldHigh && S.WantsRaised() && !S.WantsReeling());
    Sweep(S, -0.8f, 0.3f);
    TestTrue(TEXT("Moving down reels"), S.WantsReeling() && !S.WantsRaised() && S.GetDirection() == -1);
    Sweep(S, -0.8f, 0.2f, true);
    TestTrue(TEXT("Run: reel slips while lowering"), !S.WantsReeling() && !S.WantsRaised());
    Sweep(S, -0.8f, 2.0f);
    TestEqual(TEXT("Clamped at bottom"), S.GetAngle(), 0.0f);
    TestFalse(TEXT("Pushing past the bottom does not keep reeling"), S.WantsReeling());
    S.Reset(0.3f);
    for (int32 I = 0; I < 120; ++I) { S.Move((I % 2 ? -1 : 1) * 0.002f); S.Advance(1.0f / 60, false); }
    TestEqual(TEXT("Hand tremor is ignored"), S.GetDirection(), 0);
    S.Reset(0.9f); Sweep(S, 1.0f, 1.0f);
    TestEqual(TEXT("Clamped at top"), S.GetAngle(), 1.0f);
    TestEqual(TEXT("Pushing past the top is still"), S.GetDirection(), 0);
    S.Reset(0.3f); S.Move(FFiskRodStick::KeyStep); S.Advance(1.0f / 60, false);
    TestTrue(TEXT("One arrow key step lifts"), S.WantsRaised());
    TestTrue(TEXT("Sensitivity range"), FFiskRodStick::ValidSensitivity(1) && FFiskRodStick::ValidSensitivity(3) && !FFiskRodStick::ValidSensitivity(0) && !FFiskRodStick::ValidSensitivity(4));
    TestTrue(TEXT("Higher sensitivity needs less travel"), FFiskRodStick::TravelForSensitivity(3) < FFiskRodStick::TravelForSensitivity(2) && FFiskRodStick::TravelForSensitivity(2) < FFiskRodStick::TravelForSensitivity(1));
    return true;
}
#endif
