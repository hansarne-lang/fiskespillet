#include "FiskHelm.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include <limits>
namespace
{
    FiskHelm::FState AtSea(FVector Position=FVector(20000,0,0),double Heading=0.0)
    { FiskHelm::FState S;S.Position=Position;S.Heading=Heading;return S; }
    void Run(FiskHelm::FState& S,double Throttle,double Rudder,double Seconds,int32 Kind=0,bool bTowing=false,double Frame=1.0/60)
    { const int32 Frames=FMath::RoundToInt32(Seconds/Frame);for(int32 I=0;I<Frames;++I) { FiskHelm::Advance(S,Throttle,Rudder,Kind,bTowing,Frame); } }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmThrottleTest,"Fiskelaget.Helm.ThrottleSpeedAndStop",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmThrottleTest::RunTest(const FString&)
{
    auto S=AtSea();
    TestTrue(TEXT("Stopped boat at sea can fish"),FiskHelm::CanFish(S));
    Run(S,1,0,2);
    TestTrue(TEXT("Half way up to speed"),S.Speed>FiskHelm::BaseSpeed*.4 && S.Speed<FiskHelm::BaseSpeed*.6);
    TestFalse(TEXT("Moving boat cannot fish"),FiskHelm::CanFish(S));
    Run(S,1,0,4);
    TestTrue(TEXT("Full speed equals the charted speed"),FMath::IsNearlyEqual(S.Speed,FiskHelm::BaseSpeed,.01));
    TestTrue(TEXT("Straight ahead out to sea"),S.Position.X>20000 && FMath::IsNearlyZero(S.Position.Y,.01) && S.Position.Z==0.0);
    Run(S,0,0,3);
    TestTrue(TEXT("Coasts down, not an instant stop"),S.Speed>FiskHelm::StillSpeed && S.Speed<FiskHelm::BaseSpeed);
    Run(S,0,0,4);
    TestTrue(TEXT("Released throttle stops the boat"),S.Speed==0.0 && FiskHelm::CanFish(S));
    const FVector Stopped=S.Position;Run(S,0,1,5);
    TestTrue(TEXT("A stopped boat stays put"),S.Position.Equals(Stopped,.001));
    Run(S,-1,0,10);
    TestTrue(TEXT("Reverse is slow"),FMath::IsNearlyEqual(S.Speed,-FiskHelm::BaseSpeed*FiskHelm::ReverseShare,.01) && S.Position.X<Stopped.X);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmRudderTest,"Fiskelaget.Helm.RudderNeedsSpeed",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmRudderTest::RunTest(const FString&)
{
    auto S=AtSea();Run(S,0,1,5);
    TestEqual(TEXT("No turning at a standstill"),S.Heading,0.0);
    auto Starboard=AtSea();Run(Starboard,1,0,6);Run(Starboard,1,1,2);
    TestTrue(TEXT("Full rudder turns one radian in two seconds at full speed"),FMath::IsNearlyEqual(Starboard.Heading,1.0,.03));
    TestTrue(TEXT("The turn moves the boat sideways"),Starboard.Position.Y>100);
    auto Port=AtSea();Run(Port,1,0,6);Run(Port,1,-1,2);
    TestTrue(TEXT("Port and starboard mirror each other"),FMath::IsNearlyEqual(Port.Heading,-Starboard.Heading,1.e-6) && FMath::IsNearlyEqual(Port.Position.Y,-Starboard.Position.Y,.01));
    auto Circle=AtSea(FVector(80000,-20000,0));Run(Circle,1,0,6);Run(Circle,1,1,60);
    TestTrue(TEXT("Heading stays within one turn"),Circle.Heading>=-PI && Circle.Heading<=PI);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmBoatKindTest,"Fiskelaget.Helm.OffshoreAndTowingSpeed",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmBoatKindTest::RunTest(const FString&)
{
    TestEqual(TEXT("Starter boat"),FiskHelm::MaxSpeed(0,false),FiskHelm::BaseSpeed);
    TestEqual(TEXT("Offshore boat is a quarter faster"),FiskHelm::MaxSpeed(1,false),FiskHelm::BaseSpeed*1.25);
    TestEqual(TEXT("Towing keeps 65 percent"),FiskHelm::MaxSpeed(0,true),FiskHelm::BaseSpeed*.65);
    auto S=AtSea();Run(S,1,0,8,1,true);
    TestTrue(TEXT("Towing offshore boat"),FMath::IsNearlyEqual(S.Speed,FiskHelm::BaseSpeed*1.25*.65,.01));
    auto Slow=AtSea();Run(Slow,.5,0,8);
    TestTrue(TEXT("Half throttle gives half speed"),FMath::IsNearlyEqual(Slow.Speed,FiskHelm::BaseSpeed*.5,.01));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmObstacleTest,"Fiskelaget.Helm.IslandsChartEdgeAndMarina",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmObstacleTest::RunTest(const FString&)
{
    for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I)
    {
        const FVector Island=FiskRouteGeometry::Island(I);
        auto S=AtSea(Island-FVector(FiskRouteGeometry::Radius(I)+8000,0,0));Run(S,1,0,30);
        TestTrue(TEXT("Island stops the boat"),S.Speed<FiskHelm::BaseSpeed*.1 && FiskRouteGeometry::Navigable(S.Position));
        TestTrue(TEXT("Never on land"),FVector::Dist2D(S.Position,Island)>FiskRouteGeometry::Radius(I)+1500);
        const FVector Aground=S.Position;Run(S,-1,0,10);
        TestTrue(TEXT("Reverse gets the boat off"),S.Position.X<Aground.X-500);
    }
    auto Edge=AtSea(FVector(140000,0,0));Run(Edge,1,0,30);
    TestTrue(TEXT("Chart edge stops the boat"),Edge.Position.X<=150000 && Edge.Speed<FiskHelm::BaseSpeed*.1);
    auto Home=AtSea(FVector(3000,0,0),PI);Run(Home,1,0,30);
    TestTrue(TEXT("The boat can reach the marina but not pass behind it"),Home.Position.X>=0 && Home.Position.X<200);
    Run(Home,0,0,10);
    TestFalse(TEXT("No fishing inside the marina"),FiskHelm::CanFish(Home));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmInputTest,"Fiskelaget.Helm.BadInputAndFrameRate",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmInputTest::RunTest(const FString&)
{
    const double Nan=std::numeric_limits<double>::quiet_NaN();
    TestEqual(TEXT("Not a number counts as released"),FiskHelm::Clean(Nan),0.0);
    TestEqual(TEXT("Over-range throttle is clamped"),FiskHelm::Clean(50.0),1.0);
    TestEqual(TEXT("Over-range reverse is clamped"),FiskHelm::Clean(-50.0),-1.0);
    auto Bad=AtSea();Run(Bad,Nan,Nan,5);
    TestTrue(TEXT("Bad input never moves the boat"),Bad.Position.Equals(FVector(20000,0,0)) && Bad.Speed==0.0 && Bad.Heading==0.0);
    auto Cheat=AtSea();Run(Cheat,1000,0,8);
    TestTrue(TEXT("No speed above the limit"),Cheat.Speed<=FiskHelm::BaseSpeed+.01);
    auto Frozen=AtSea();FiskHelm::Advance(Frozen,1,0,0,false,-1);FiskHelm::Advance(Frozen,1,0,0,false,Nan);FiskHelm::Advance(Frozen,1,0,0,false,3600);
    TestTrue(TEXT("A stalled frame is capped"),Frozen.Position.X<20000+FiskHelm::BaseSpeed*FiskHelm::LongestAdvance);
    auto Slow=AtSea(),Fast=AtSea();
    for(int32 I=0;I<300;++I) { FiskHelm::Advance(Slow,1,.5,0,false,1.0/30); }
    for(int32 I=0;I<1200;++I) { FiskHelm::Advance(Fast,1,.5,0,false,1.0/120); }
    TestTrue(TEXT("Same result at 30 and 120 frames per second"),Slow.Position.Equals(Fast.Position,1.0) && FMath::IsNearlyEqual(Slow.Heading,Fast.Heading,1.e-3));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHelmTripTest,"Fiskelaget.Helm.TakeHelmStopAndArrive",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHelmTripTest::RunTest(const FString&)
{
    using namespace FiskHelm;
    TestFalse(TEXT("Only the captain"),CanTakeHelm(false,TripStopped,300000));
    TestFalse(TEXT("Not from the dock before 02:00"),CanTakeHelm(true,TripDock,119999));
    TestTrue(TEXT("From the dock at 02:00"),CanTakeHelm(true,TripDock,120000));
    TestFalse(TEXT("Not from the dock after 17:00"),CanTakeHelm(true,TripDock,1020000));
    TestTrue(TEXT("At sea also after 17:00, to come home"),CanTakeHelm(true,TripUnderHelm,1100000));
    TestTrue(TEXT("Take over an autopilot leg"),CanTakeHelm(true,40,300000) && CanTakeHelm(true,150,300000));
    TestFalse(TEXT("Home in the marina stays home"),CanTakeHelm(true,TripMarina,300000));
    auto S=AtSea();Run(S,1,0,6);
    TestFalse(TEXT("Moving boat does not settle"),Settles(S,0));
    Run(S,0,0,8);
    TestTrue(TEXT("Stopped with the throttle released settles"),Settles(S,0));
    TestFalse(TEXT("Throttle held does not settle"),Settles(S,.2));
    TestFalse(TEXT("No arrival straight from the dock"),Arrived(FVector(0,1250,0),false));
    TestTrue(TEXT("Arrival after being out"),Arrived(FVector(1000,-1000,0),true));
    TestFalse(TEXT("Outside the marina"),Arrived(FVector(3000,0,0),true));
    TestEqual(TEXT("200 m at full speed take 12 s"),TravelMs(20000,0,false),12000);
    TestTrue(TEXT("Towing takes longer"),TravelMs(20000,0,true)>TravelMs(20000,0,false));
    TestEqual(TEXT("Bad length"),TravelMs(std::numeric_limits<double>::quiet_NaN(),0,false),1);
    return true;
}
#endif
