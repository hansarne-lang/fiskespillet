#include "Fishing/FiskRodCycle.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodLifecycleTest, "Fiskelaget.RodCycle.RestartAndStaleCommands",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodLifecycleTest::RunTest(const FString& Parameters)
{
    FFiskRodCycle Cycle;
    const FGuid Initial;
    TestFalse(TEXT("No action before casting"), Cycle.CanAct(Initial));
    TestTrue(TEXT("First cast"), Cycle.TryStart(Initial, 1000));
    const FGuid First = Cycle.GetId();
    TestTrue(TEXT("Opaque nonempty identity"), First.IsValid());
    TestFalse(TEXT("Repeated initial cast"), Cycle.TryStart(Initial, 1001));
    TestFalse(TEXT("Repeated current cast while open"), Cycle.TryStart(First, 1001));
    TestTrue(TEXT("Current commands accepted"), Cycle.CanAct(First));
    TestTrue(TEXT("Finish landing once"), Cycle.FinishLanding(First, 2000));
    TestFalse(TEXT("Duplicate landing"), Cycle.FinishLanding(First, 2001));
    TestFalse(TEXT("Cannot cancel a landed fish to bypass animation"), Cycle.Withdraw(First));
    TestFalse(TEXT("Last millisecond of landing"), Cycle.TryStart(First, 4999));
    TestTrue(TEXT("Restart at end of animation"), Cycle.TryStart(First, 5000));
    const FGuid Second = Cycle.GetId();
    TestTrue(TEXT("New identity"), First != Second);
    TestFalse(TEXT("Old commands rejected during new fight"), Cycle.CanAct(First));
    TestFalse(TEXT("Old cancel cannot withdraw new fight"), Cycle.Withdraw(First));
    TestFalse(TEXT("Old landing cannot land new fish"), Cycle.FinishLanding(First, 6000));
    TestTrue(TEXT("New command survives stale attempts"), Cycle.CanAct(Second));
    TestTrue(TEXT("Own cancellation"), Cycle.Withdraw(Second));
    TestFalse(TEXT("Duplicate cancellation"), Cycle.Withdraw(Second));
    TestFalse(TEXT("Stale F after cancellation"), Cycle.TryStart(First, 6000));
    TestTrue(TEXT("Restart immediately after cancellation"), Cycle.TryStart(Second, 6000));
    TestTrue(TEXT("Each restart is independent"), Cycle.GetId() != First && Cycle.GetId() != Second);
    const FGuid Yesterday=Cycle.GetId();Cycle.ResetForDay();
    const FGuid Today=Cycle.GetId();
    TestTrue(TEXT("New day rotates identity even before next cast"),Today.IsValid() && Today!=Yesterday);
    TestFalse(TEXT("Yesterday's action denied"),Cycle.CanAct(Yesterday));
    TestFalse(TEXT("Yesterday's cast denied"),Cycle.TryStart(Yesterday,0));
    TestFalse(TEXT("Original empty token denied after day change"),Cycle.TryStart(FGuid(),0));
    TestTrue(TEXT("Today's token starts fresh clock without old landing cooldown"),Cycle.TryStart(Today,0));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodIsolationTest, "Fiskelaget.RodCycle.OwnerIsolation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodIsolationTest::RunTest(const FString& Parameters)
{
    FFiskRodCycle Host, Guest;
    Host.TryStart(FGuid(), 0); Guest.TryStart(FGuid(), 0);
    const FGuid HostId = Host.GetId(), GuestId = Guest.GetId();
    TestFalse(TEXT("Other player cannot act on rod"), Guest.CanAct(HostId));
    TestFalse(TEXT("Other player cannot cancel rod"), Guest.Withdraw(HostId));
    TestTrue(TEXT("Guest cancels own rod"), Guest.Withdraw(GuestId));
    TestTrue(TEXT("Host fight unaffected"), Host.CanAct(HostId));
    TestTrue(TEXT("Host can land"), Host.FinishLanding(HostId, 1000));
    TestFalse(TEXT("Old guest action cannot land host fish"), Host.FinishLanding(GuestId, 1000));
    return true;
}
#endif
