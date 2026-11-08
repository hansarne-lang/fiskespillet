#include "DayCycle/FiskSessionRules.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskNextDayRulesTest,"Fiskelaget.Session.NextDayGuards",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskNextDayRulesTest::RunTest(const FString& Parameters)
{
    TestFalse(TEXT("Last ms of day cannot advance"),FiskSession::CanAdvance(true,1,1,1199999,true));
    TestTrue(TEXT("Host after exact end and settlement"),FiskSession::CanAdvance(true,1,1,1200000,true));
    TestFalse(TEXT("Remote captain cannot advance"),FiskSession::CanAdvance(false,1,1,1200000,true));
    TestFalse(TEXT("Settlement required"),FiskSession::CanAdvance(true,1,1,1200000,false));
    TestFalse(TEXT("Duplicate old day request"),FiskSession::CanAdvance(true,2,1,1200000,true));
    TestFalse(TEXT("Future day request"),FiskSession::CanAdvance(true,2,3,1200000,true));
    TestFalse(TEXT("No overflow after max day"),FiskSession::CanAdvance(true,FiskSession::MaxDay,FiskSession::MaxDay,1200000,true));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskReturnWarningTest,"Fiskelaget.Session.ReturnWarningBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskReturnWarningTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Ordinary margin"),FiskSession::ReturnWarning(60001),0);
    TestEqual(TEXT("Last minute"),FiskSession::ReturnWarning(60000),1);
    TestEqual(TEXT("Exact arrival still in time"),FiskSession::ReturnWarning(0),1);
    TestEqual(TEXT("One ms late"),FiskSession::ReturnWarning(-1),2);
    return true;
}
#endif
