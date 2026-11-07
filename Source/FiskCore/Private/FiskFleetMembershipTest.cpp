#include "FiskFleetMembership.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetReconnectLock,"Fiskelaget.Fleet.ReconnectPreservesVoyageLock",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetReconnectLock::RunTest(const FString&)
{
    FFiskFleetMembership Fleet;const auto Captain=FGuid::NewGuid(),Passenger=FGuid::NewGuid(),Shore=FGuid::NewGuid();
    TestTrue(TEXT("Captain can join before departure"),Fleet.CanJoin(Captain));TestTrue(TEXT("Passenger can join before departure"),Fleet.CanJoin(Passenger));
    Fleet.Depart({Captain,Passenger});const FGuid ReconnectedProfile=Passenger;
    TestFalse(TEXT("Owner cannot start another voyage"),Fleet.CanJoin(Captain));TestFalse(TEXT("A new connection with the same profile cannot change boats"),Fleet.CanJoin(ReconnectedProfile));TestTrue(TEXT("An ashore profile remains free"),Fleet.CanJoin(Shore));
    Fleet.ResetForDay();TestTrue(TEXT("Next day unlocks captain"),Fleet.CanJoin(Captain));TestTrue(TEXT("Next day unlocks reconnected passenger"),Fleet.CanJoin(ReconnectedProfile));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFleetProfileGuard,"Fiskelaget.Fleet.ValidProfileAndIndependentCrews",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskFleetProfileGuard::RunTest(const FString&)
{
    FFiskFleetMembership Fleet;const auto A=FGuid::NewGuid(),B=FGuid::NewGuid();TestFalse(TEXT("Unbound connections cannot board"),Fleet.CanJoin(FGuid()));Fleet.Depart({A,A,FGuid()});TestFalse(TEXT("Repeated departure does not unlock A"),Fleet.CanJoin(A));TestTrue(TEXT("Other crew still free"),Fleet.CanJoin(B));Fleet.Depart({B});TestFalse(TEXT("Both boats now locked independently"),Fleet.CanJoin(B));Fleet.ResetForDay();TestFalse(TEXT("Reset never authorizes an invalid profile"),Fleet.CanJoin(FGuid()));return true;
}
#endif
