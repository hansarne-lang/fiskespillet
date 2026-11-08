#include "FiskPierGuide.h"
#include "Misc/AutomationTest.h"
#include <limits>
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierFurniture,"Fiskelaget.PierGuide.FurnitureAndBodyClearance",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierFurniture::RunTest(const FString&)
{
    for(int32 I=0;I<FiskPier::StoolCount;++I) { TestFalse(TEXT("Stool blocks"),FiskPier::Inside(FiskPier::Stool(I))); }
    for(int32 I=0;I<FiskPier::TableCount;++I) { TestFalse(TEXT("Table blocks"),FiskPier::Inside(FiskPier::Table(I))); }
    for(int32 I=0;I<FiskPier::PostCount;++I) { TestFalse(TEXT("Post blocks"),FiskPier::Inside(FiskPier::Post(I))); }
    TestFalse(TEXT("Body stays inside edge"),FiskPier::Inside(FVector(-910,1000,130)));TestTrue(TEXT("Clear passage"),FiskPier::Inside(FVector(-1000,0,130)));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierSweep,"Fiskelaget.PierGuide.SweptMovementCannotCrossStools",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierSweep::RunTest(const FString&)
{
    for(int32 I=0;I<12;++I) { const FVector C=FiskPier::Stool(I);FVector P=C+FVector(130,0,0);for(int32 N=0;N<30;++N) { P=FiskPier::Step(P,{-1,0},100,true);TestTrue(TEXT("Every step safe"),FiskPier::Inside(P)); }TestTrue(TEXT("Cannot tunnel through seat"),P.X>C.X+60); }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierGuideDestinations,"Fiskelaget.PierGuide.AllFourteenDestinations",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierGuideDestinations::RunTest(const FString&)
{
    for(int32 I=0;I<14;++I) { const auto G=FiskPier::Destination(I);const auto Path=FiskPier::GuidePath(FiskPier::Spawn(),G);if(Path.Num()<2) { AddError(FString::Printf(TEXT("Unreachable destination %d"),I));continue; }TestTrue(TEXT("Exact start"),Path[0]==FiskPier::Spawn());TestTrue(TEXT("Exact goal"),Path.Last()==G);for(int32 J=1;J<Path.Num();++J) { TestTrue(TEXT("Safe segment"),FiskPier::ClearSegment(Path[J-1],Path[J])); } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierGuidePairs,"Fiskelaget.PierGuide.AllDestinationPairsAreConnected",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierGuidePairs::RunTest(const FString&)
{
    for(int32 I=0;I<14;++I) { for(int32 J=0;J<14;++J) { const auto A=FiskPier::Destination(I),B=FiskPier::Destination(J);const auto Path=FiskPier::GuidePath(A,B);if(Path.Num()<2 || Path.Num()>300) { AddError(FString::Printf(TEXT("Path missing %d to %d"),I,J));return false; }for(int32 K=1;K<Path.Num();++K) { if(!FiskPier::ClearSegment(Path[K-1],Path[K])) { AddError(TEXT("Unsafe path segment"));return false; } } } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierGuideMovement,"Fiskelaget.PierGuide.OrdinaryAxisReachesEveryGoal",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierGuideMovement::RunTest(const FString&)
{
    FVector P=FiskPier::Spawn();for(int32 I=0;I<14;++I) { const auto Path=FiskPier::GuidePath(P,FiskPier::Destination(I));if(Path.Num()<2) { AddError(TEXT("No path"));return false; }int32 K=1;for(int32 N=0;N<15000 && K<Path.Num();++N) { if(FVector::Dist2D(P,Path[K])<14) { ++K;continue; }const FVector V=Path[K]-P;const FVector2D A=FVector2D(V.X,V.Y).GetSafeNormal()*FMath::Min(1.,V.Size2D()/70.);const FVector Q=FiskPier::Step(P,A,.05,true);if(!FiskPier::Inside(Q) || FVector::Dist2D(P,Q)>17.501) { AddError(TEXT("Unsafe or excessive motion"));return false; }P=Q; }TestTrue(TEXT("Goal reached"),FVector::Dist2D(P,FiskPier::Destination(I))<15); }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierGuideInvalid,"Fiskelaget.PierGuide.RejectWaterFurnitureAndNonFinite",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierGuideInvalid::RunTest(const FString&)
{
    for(auto P:{FVector(0,0,130),FiskPier::Stool(0),FVector(-1300,0,0),FVector(std::numeric_limits<double>::infinity(),0,130),FiskPier::Destination(-1),FiskPier::Destination(14)}) { TestTrue(TEXT("Invalid goal"),FiskPier::GuidePath(FiskPier::Spawn(),P).IsEmpty());TestTrue(TEXT("Invalid start"),FiskPier::GuidePath(P,FiskPier::Spawn()).IsEmpty()); }
    TestTrue(TEXT("Deterministic"),FiskPier::GuidePath(FiskPier::Destination(13),FiskPier::Destination(1))==FiskPier::GuidePath(FiskPier::Destination(13),FiskPier::Destination(1)));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierNameFacing,"Fiskelaget.PierGuide.UprightNamesFromEveryCamera",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierNameFacing::RunTest(const FString&)
{
    for(int32 Yaw=0;Yaw<360;Yaw+=15) { for(int32 Pitch=-80;Pitch<=80;Pitch+=20) { const FRotator Camera(Pitch,Yaw,12);const FVector Toward=-Camera.Vector(),Up=Camera.RotateVector(FVector::UpVector);const auto R=FiskPier::NameFacing(Toward,Up);TestTrue(TEXT("Front faces viewer"),FVector::DotProduct(R.Vector(),Toward)>.999);TestTrue(TEXT("Text up matches screen"),FVector::DotProduct(R.RotateVector(FVector::UpVector),Up)>.999); } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPierCamera,"Fiskelaget.PierGuide.ThreeDistinctSafeCameraOffsets",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskPierCamera::RunTest(const FString&)
{ const auto A=FiskPier::WalkCamera(0),B=FiskPier::WalkCamera(1),C=FiskPier::WalkCamera(2);TestTrue(TEXT("Near closer than follow"),B.Size()<A.Size());TestTrue(TEXT("Overview farther"),C.Size()>A.Size());for(int32 I=0;I<3;++I) { TestTrue(TEXT("Above deck"),FiskPier::WalkCamera(I).Z>300); }return true; }
#endif
