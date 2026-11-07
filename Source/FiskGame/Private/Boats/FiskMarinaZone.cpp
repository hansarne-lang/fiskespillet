#include "Boats/FiskMarinaZone.h"
#include "Boats/FiskStarterBoat.h"
#include "World/FiskVisualPrimitives.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
AFiskMarinaZone::AFiskMarinaZone()
{ SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("MarinaMarkers")));PrimaryActorTick.bCanEverTick=false; }
void AFiskMarinaZone::BeginPlay()
{
    Super::BeginPlay();auto* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));auto* Cylinder=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));auto* Material=LoadObject<UMaterialInterface>(nullptr,TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    for(int32 I=1;I<8;++I) { const double A=I*PI/4;const FVector P(FMath::Cos(A)*AFiskStarterBoat::MarinaRadiusCm,FMath::Sin(A)*AFiskStarterBoat::MarinaRadiusCm,0);const FLinearColor Color=I<4?FLinearColor(.1,.45,.2):FLinearColor(.65,.08,.04);
        FiskVisual::AddPart(this,RootComponent,Sphere,Material,*FString::Printf(TEXT("BoundaryBuoy%d"),I),P,FVector(.28,.28,.4),FLinearColor(.92,.86,.68));
        FiskVisual::AddPart(this,RootComponent,Cylinder,Material,*FString::Printf(TEXT("BoundaryBeacon%d"),I),P+FVector(0,0,35),FVector(.08,.08,.5),Color);
    }
}
