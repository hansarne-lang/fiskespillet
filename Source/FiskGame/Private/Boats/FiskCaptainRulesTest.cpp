#include "Boats/FiskStarterBoat.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCaptainAuthorityTest,"Fiskelaget.Captain.AuthorityAndOldCommands",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCaptainAuthorityTest::RunTest(const FString& Parameters)
{
    TArray<int32> Crew={256,257};FFiskRouteView V;V.RefreshCaptain(Crew,256,true);
    TestEqual(TEXT("Owner defaults to captain"),V.CaptainId,256);TestEqual(TEXT("Initial revision remains zero"),V.Revision,0);
    TestFalse(TEXT("Passenger cannot transfer"),V.Transfer(257,0,256,Crew));
    TestFalse(TEXT("Cannot choose unknown player"),V.Transfer(256,0,999,Crew));
    TestFalse(TEXT("Cannot transfer to self"),V.Transfer(256,0,256,Crew));
    TestFalse(TEXT("Stale revision rejected"),V.Transfer(256,1,257,Crew));
    TestEqual(TEXT("Rejections preserve revision"),V.Revision,0);
    TestTrue(TEXT("Captain can transfer"),V.Transfer(256,0,257,Crew));
    TestFalse(TEXT("Old captain loses rights"),V.Authorizes(256,1,Crew));
    TestTrue(TEXT("New captain has rights"),V.Authorizes(257,1,Crew));
    TestTrue(TEXT("Can give role back"),V.Transfer(257,1,256,Crew));
    TestFalse(TEXT("Old command rejected after role returns"),V.Authorizes(256,0,Crew));
    TestTrue(TEXT("Current command accepted"),V.Authorizes(256,2,Crew));
    V.Revision=MAX_int32;TestFalse(TEXT("Exhausted revision fails closed"),V.Authorizes(256,MAX_int32,Crew));
    TestFalse(TEXT("No revision wrap"),V.Transfer(256,MAX_int32,257,Crew));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCaptainFallbackTest,"Fiskelaget.Captain.DefaultAndDisconnectFallback",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCaptainFallbackTest::RunTest(const FString& Parameters)
{
    FFiskRouteView V;TArray<int32> Crew={257};V.RefreshCaptain(Crew,256,true);
    TestEqual(TEXT("First crew when owner absent"),V.CaptainId,257);
    Crew.Add(256);V.RefreshCaptain(Crew,256,true);TestEqual(TEXT("Owner becomes default when boarding"),V.CaptainId,256);
    TestTrue(TEXT("Explicit choice"),V.Transfer(256,V.Revision,257,Crew));
    const int32 ChosenRevision=V.Revision;V.RefreshCaptain(Crew,256,false);
    TestEqual(TEXT("Owner does not steal explicit role"),V.CaptainId,257);TestEqual(TEXT("No spurious revision"),V.Revision,ChosenRevision);
    Crew.Remove(257);V.RefreshCaptain(Crew,256,true);
    TestEqual(TEXT("Remaining owner takes over"),V.CaptainId,256);TestTrue(TEXT("Disconnect invalidates old epoch"),V.Revision>ChosenRevision);
    Crew.Reset();V.RefreshCaptain(Crew,256,true);TestEqual(TEXT("Empty boat has no captain"),V.CaptainId,INDEX_NONE);
    TestFalse(TEXT("No empty-boat rights"),V.Authorizes(256,V.Revision,Crew));
    return true;
}
#endif
