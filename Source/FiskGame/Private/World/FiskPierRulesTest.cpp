#include "FiskPierRules.h"
#include "FiskGreetingClock.h"
#include "Misc/AutomationTest.h"
#include <limits>
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierSpine,"Fiskelaget.Pier.SpawnAndSpine",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierSpine::RunTest(const FString&) { TestTrue(TEXT("Spawn on deck"),FiskPier::Inside(FiskPier::Spawn()));for(int32 Y=-3100;Y<10200;Y+=100) { TestTrue(TEXT("Continuous spine"),FiskPier::Inside(FVector(-1300,Y,130))); }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierHouses,"Fiskelaget.Pier.TwelveConnectedHouseDoors",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierHouses::RunTest(const FString&) { for(int32 I=0;I<12;++I) { auto P=FiskPier::HouseDoor(I);TestTrue(TEXT("House door on deck"),FiskPier::Inside(P));while(P.X<-1300) { P.X=FMath::Min(-1300.,P.X+100);TestTrue(TEXT("Row connects to spine"),FiskPier::Inside(P)); } }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierConnections,"Fiskelaget.Pier.BarAndBoatConnections",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierConnections::RunTest(const FString&) { for(int32 X=-1300;X>=-4200;X-=100) { TestTrue(TEXT("Bar path"),FiskPier::Inside(FVector(X,-2900,130))); }for(int32 X=-1300;X<=-500;X+=100) { TestTrue(TEXT("Boarding path"),FiskPier::Inside(FVector(X,0,130))); }TestTrue(TEXT("Bar interact"),FiskPier::NearBar(FVector(-4200,-2900,130)));TestTrue(TEXT("Boat interact"),FiskPier::NearBoat(FVector(-550,0,130)));return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierVoid,"Fiskelaget.Pier.RejectWaterAndWrongHeight",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierVoid::RunTest(const FString&) { for(auto P:{FVector(10000,0,130),FVector(-5000,0,130),FVector(-1300,0,-100),FVector(-1300,20000,130)}) { TestFalse(TEXT("Unsafe point"),FiskPier::Inside(P)); }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierSpeed,"Fiskelaget.Pier.ServerStepBounded",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierSpeed::RunTest(const FString&) { const auto P=FiskPier::Spawn();TestTrue(TEXT("Walk 22cm max"),FVector::Dist(P,FiskPier::Step(P,{0,1},30,false))<=22.001);TestTrue(TEXT("Run 35cm max"),FVector::Dist(P,FiskPier::Step(P,{0,1},30,true))<=35.001);return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierDiagonal,"Fiskelaget.Pier.NoDiagonalSpeedBoost",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierDiagonal::RunTest(const FString&) { const auto P=FiskPier::Spawn();TestTrue(TEXT("Normalized"),FVector::Dist(P,FiskPier::Step(P,{1,1},.1,true))<=35.001);return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierEdges,"Fiskelaget.Pier.CannotWalkOffEdges",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierEdges::RunTest(const FString&) { for(FVector2D A:{FVector2D(1,0),FVector2D(-1,0),FVector2D(0,1),FVector2D(0,-1),FVector2D(1,1)}) { auto P=FiskPier::Spawn();for(int32 I=0;I<10000;++I) { P=FiskPier::Step(P,A,.1,true);if(!FiskPier::Inside(P)) { AddError(TEXT("Walk escaped safe deck"));return false; } } }return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierInvalid,"Fiskelaget.Pier.NonFiniteInputRejected",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierInvalid::RunTest(const FString&) { const double NaN=std::numeric_limits<double>::quiet_NaN(),Inf=std::numeric_limits<double>::infinity();TestFalse(TEXT("NaN"),FiskPier::ValidAxis({NaN,0}));TestFalse(TEXT("Infinity"),FiskPier::ValidAxis({Inf,0}));TestFalse(TEXT("Oversized axis"),FiskPier::ValidAxis({100,0}));TestTrue(TEXT("NaN delta stationary"),FiskPier::Step(FiskPier::Spawn(),{0,1},NaN,true)==FiskPier::Spawn());return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierAuthority,"Fiskelaget.Pier.StartRequiresOwnShoreCareer",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierAuthority::RunTest(const FString&) { TestTrue(TEXT("Own shore"),FiskPier::CanStart(true,true,true,false));TestFalse(TEXT("Foreign"),FiskPier::CanStart(true,false,true,false));TestFalse(TEXT("At sea"),FiskPier::CanStart(true,true,false,true));TestFalse(TEXT("Aboard docked boat"),FiskPier::CanStart(true,true,true,true));TestFalse(TEXT("Unready"),FiskPier::CanStart(false,true,true,false));return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierReplay,"Fiskelaget.Pier.SessionAndSerialRejectReplay",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierReplay::RunTest(const FString&) { TestTrue(TEXT("Fresh"),FiskPier::AcceptInput(true,true,3,3,5,4,{0,1}));TestFalse(TEXT("Old session"),FiskPier::AcceptInput(true,true,2,3,5,4,{0,1}));TestFalse(TEXT("Replay"),FiskPier::AcceptInput(true,true,3,3,4,4,{0,1}));TestFalse(TEXT("Stopped"),FiskPier::AcceptInput(false,true,3,3,5,4,{0,1}));TestFalse(TEXT("Foreign"),FiskPier::AcceptInput(true,false,3,3,5,4,{0,1}));return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGreetingDays,"Fiskelaget.Pier.GreetingDoesNotRepeatOnNewDay",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskGreetingDays::RunTest(const FString&) { FFiskGreetingClock C;C.Observe(1,TEXT("A"),267);TestTrue(TEXT("Fresh wave"),C.IsWaving(TEXT("A"),268));TestFalse(TEXT("Expires"),C.IsWaving(TEXT("A"),272));C.Observe(1,TEXT("A"),1467);TestFalse(TEXT("Old message does not retrigger"),C.IsWaving(TEXT("A"),1468));return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGreetingOwners,"Fiskelaget.Pier.GreetingsSeparatePeople",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskGreetingOwners::RunTest(const FString&) { FFiskGreetingClock C;C.Observe(1,TEXT("A"),1);C.Observe(2,TEXT("B"),3);TestFalse(TEXT("A expired"),C.IsWaving(TEXT("A"),5));TestTrue(TEXT("B active"),C.IsWaving(TEXT("B"),5));TestFalse(TEXT("Other name"),C.IsWaving(TEXT("C"),5));return true; }
#endif
