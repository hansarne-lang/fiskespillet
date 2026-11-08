#include "World/FiskOceanScene.h"
#include "World/FiskCoastMesh.h"
#include "FiskRouteGeometry.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProceduralMeshComponent.h"
void AFiskOceanScene::BuildCoast()
{
    auto Instances=[&](FName Name,UStaticMesh* Mesh,FLinearColor Color)
    {
        auto* C=NewObject<UInstancedStaticMeshComponent>(this,Name);AddInstanceComponent(C);C->SetupAttachment(RootComponent);C->SetStaticMesh(Mesh);C->SetCollisionEnabled(ECollisionEnabled::NoCollision);C->SetCastShadow(false);
        auto* Tint=UMaterialInstanceDynamic::Create(Material,C);TArray<FMaterialParameterInfo> Params;TArray<FGuid> Ids;Material->GetAllVectorParameterInfo(Params,Ids);for(const auto& P:Params) { Tint->SetVectorParameterValue(P.Name,Color); }C->SetMaterial(0,Tint);C->RegisterComponent();return C;
    };
    auto* Rock=Instances(TEXT("CoastRocks"),Sphere,FLinearColor(.24f,.29f,.26f));auto* LightRock=Instances(TEXT("CoastLimestone"),Sphere,FLinearColor(.53f,.52f,.38f));
    auto* Trunk=Instances(TEXT("CoastPalmTrunks"),Cylinder,FLinearColor(.32f,.23f,.13f));auto* Leaves=Instances(TEXT("CoastPalmFronds"),Sphere,FLinearColor(.07f,.25f,.10f));
    auto Add=[](UInstancedStaticMeshComponent* C,FVector P,FVector S,FRotator Rotation=FRotator::ZeroRotator){C->AddInstance(FTransform(Rotation,P,S));};
    for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I)
    {
        const FLinearColor Colors[]={FLinearColor(.69f,.58f,.36f),FLinearColor(.13f,.28f,.10f),FLinearColor(.23f,.29f,.19f)};
        for(int32 B=0;B<3;++B) { auto* C=FiskMarine::Add(this,RootComponent,*FString::Printf(TEXT("CoastTerrain%d_%d"),I,B),FiskCoast::Terrain(I,B),Material,Colors[B],.9f);C->SetRelativeLocation(FiskRouteGeometry::Island(I)); }
        auto* Foam=FiskMarine::Add(this,RootComponent,*FString::Printf(TEXT("ShoreFoam%d"),I),FiskCoast::Foam(I),Material,FLinearColor(.49f,.72f,.67f),.25f);Foam->SetRelativeLocation(FiskRouteGeometry::Island(I));Foam->SetCastShadow(false);
        for(int32 J=0;J<64;++J) { const FVector P=FiskCoast::Decoration(I,J,.45+(J%7)*.075);const double Size=1.8+(J%5)*.65;Add(J%3?Rock:LightRock,P+FVector(0,0,35),FVector(Size*2,Size,Size*.8),FRotator(J%4*8,J*31,0)); }
        for(int32 J=0;J<16;++J) { const FVector P=FiskCoast::Decoration(I,J+80,.6+(J%3)*.07);Add(Trunk,P+FVector(0,0,330),FVector(.55,.55,6.6),FRotator(7,J*37,0));for(int32 K=0;K<6;++K) { const double A=K*PI/3;Add(Leaves,P+FVector(FMath::Cos(A)*190,FMath::Sin(A)*190,650),FVector(5.2,.8,.18),FRotator(-18,K*60+J*7,0)); } }
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_COAST_BUILT islands=3 bands=9 rocks=%d palms=%d fronds=%d instanceGroups=4"),Rock->GetInstanceCount()+LightRock->GetInstanceCount(),Trunk->GetInstanceCount(),Leaves->GetInstanceCount());
}
