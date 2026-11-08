#include "World/FiskCoastMesh.h"
#include "FiskRouteGeometry.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCoastMeshes,"Fiskelaget.Coast.ValidBoundedTerrainAndFoam",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskCoastMeshes::RunTest(const FString&)
{
    for(int32 I=0;I<3;++I) { for(int32 B=0;B<4;++B) { const auto M=B==3?FiskCoast::Foam(I):FiskCoast::Terrain(I,B);TestTrue(TEXT("Valid geometry"),M.Valid());TestTrue(TEXT("Bounded vertices"),M.Vertices.Num()<=585);for(const auto& V:M.Vertices) { if(V.Size2D()>FiskRouteGeometry::Radius(I) || V.Z>2500 || V.Z<-40) { AddError(TEXT("Terrain left exclusion bounds"));return false; } }for(int32 T=0;T<M.Triangles.Num();T+=3) { const auto A=M.Vertices[M.Triangles[T]],C=M.Vertices[M.Triangles[T+1]],D=M.Vertices[M.Triangles[T+2]];if(FVector::CrossProduct(C-A,D-A).Z<=0) { AddError(TEXT("Winding or degenerate face"));return false; } } } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCoastDecoration,"Fiskelaget.Coast.AllDecorationsRemainInsideExcludedIslands",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskCoastDecoration::RunTest(const FString&)
{
    for(int32 I=0;I<3;++I) { for(int32 J=0;J<128;++J) { const auto P=FiskCoast::Decoration(I,J,static_cast<double>(J%17)/16);TestTrue(TEXT("Finite decoration"),!P.ContainsNaN());TestTrue(TEXT("Room for full foliage"),FVector::Dist2D(P,FiskRouteGeometry::Island(I))+500<FiskRouteGeometry::Radius(I));TestFalse(TEXT("Does not occupy navigable sea"),FiskRouteGeometry::Navigable(FVector(P.X,P.Y,0))); } }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCoastDeterminism,"Fiskelaget.Coast.DeterministicAndRejectInvalidIslands",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FFiskCoastDeterminism::RunTest(const FString&)
{ TestTrue(TEXT("Same deterministic mesh"),FiskCoast::Terrain(1,2).Vertices==FiskCoast::Terrain(1,2).Vertices);TestFalse(TEXT("Invalid island"),FiskCoast::Terrain(3,0).Valid());TestFalse(TEXT("Invalid band"),FiskCoast::Terrain(0,3).Valid());TestFalse(TEXT("Negative island"),FiskCoast::Foam(-1).Valid());return true; }
#endif
