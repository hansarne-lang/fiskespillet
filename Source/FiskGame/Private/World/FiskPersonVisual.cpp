#include "World/FiskPersonVisual.h"
#include "World/FiskVisualPrimitives.h"
#include "FiskPierGuide.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
void FFiskPersonVisual::Build(AActor* Actor,USceneComponent* Parent,FName Name,UStaticMesh* Sphere,UStaticMesh* Cube,UStaticMesh* Cylinder,UMaterialInterface* Material,FLinearColor Shirt)
{
    Root=NewObject<USceneComponent>(Actor,Name);Actor->AddInstanceComponent(Root);Root->SetupAttachment(Parent);Root->RegisterComponent();
    int32 N=0;auto Part=[&](UStaticMesh* Mesh,FVector P,FVector Scale,FLinearColor Color){return FiskVisual::AddPart(Actor,Root,Mesh,Material,*FString::Printf(TEXT("%s_P%d"),*Name.ToString(),N++),P,Scale,Color,.8f);};
    const FLinearColor Skin(.67f,.43f,.29f),Navy(.035f,.095f,.14f);
    ShirtColor=Shirt;Shirts.Add(Part(Sphere,FVector(0,0,117),FVector(.52,.31,.65),Shirt));Part(Cube,FVector(0,0,79),FVector(.45,.3,.26),Navy);
    Part(Cylinder,FVector(0,0,149),FVector(.14,.14,.17),Skin);Part(Sphere,FVector(0,0,167),FVector(.34,.3,.38),Skin);
    Part(Cylinder,FVector(0,0,188),FVector(.48,.44,.04),Navy);Part(Sphere,FVector(0,0,191),FVector(.35,.33,.15),Navy);
    for(int32 S:{-1,1})
    {
        auto* Leg=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Navy);Legs[S<0?0:1]=Leg;FiskVisual::SetCylinder(Leg,FVector(0,S*13,77),FVector(7,S*13,20),8);
        Feet[S<0?0:1]=Part(Cube,FVector(15,S*13,12),FVector(.35,.2,.16),Navy);Part(Sphere,FVector(16,S*9,170),FVector(.045,.04,.04),FLinearColor(.04,.045,.04));
        const int32 I=S<0?0:1;Upper[I]=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Shirt);Shirts.Add(Upper[I]);Lower[I]=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Skin);
    }
    Label=NewObject<UTextRenderComponent>(Actor,*FString::Printf(TEXT("%s_Name"),*Name.ToString()));Actor->AddInstanceComponent(Label);Label->SetupAttachment(Root);Label->SetRelativeLocation(FVector(0,0,225));Label->SetHorizontalAlignment(EHTA_Center);Label->SetWorldSize(23);Label->SetTextRenderColor(FColor(215,245,235));Label->RegisterComponent();
}
void FFiskPersonVisual::SetShirt(FLinearColor Color)
{ if(ShirtColor.Equals(Color)) { return; }ShirtColor=Color;for(auto* P:Shirts) { if(auto* M=Cast<UMaterialInstanceDynamic>(P->GetMaterial(0))) { TArray<FMaterialParameterInfo> Params;TArray<FGuid> Ids;M->GetAllVectorParameterInfo(Params,Ids);for(const auto& V:Params) { M->SetVectorParameterValue(V.Name,Color); } } } }
void FFiskPersonVisual::Pose(double Time,int32 Activity,bool Wave,const FString& Name,FVector Camera,float Walk,FVector CameraUp)
{
    if(!Root) { return; }const float Bob=FMath::Sin(Time*1.7)*1.7;
    for(int32 I=0;I<2;++I)
    {
        const float S=I?1:-1;const bool Salute=Wave && I==1;
        const FVector A(0,S*28,137),B(Salute?FVector(6,S*47,166):FVector(Activity?27:8,S*36,108+Bob)),C(Salute?FVector(11,S*(48+FMath::Sin(Time*8)*13),205):FVector(Activity?48:15,S*26,Activity?129+FMath::Sin(Time*3)*9:82+Bob));
        const float Swing=FMath::Sin(Time*8+I*PI)*FMath::Clamp(Walk,0.f,1.f);
        const FVector Foot(7+Swing*25,S*13,20+FMath::Max(0.f,Swing)*12);
        FiskVisual::SetCylinder(Legs[I],FVector(0,S*13,77),Foot,8);Feet[I]->SetRelativeLocation(Foot+FVector(8,0,-8));
        FiskVisual::SetCylinder(Upper[I],A,B+FVector(!Wave && !Activity?-Swing*18:0,0,0),8);FiskVisual::SetCylinder(Lower[I],B+FVector(!Wave && !Activity?-Swing*18:0,0,0),C+FVector(!Wave && !Activity?-Swing*28:0,0,0),6.5);
    }
    Label->SetText(FText::FromString(Name));Label->SetWorldRotation(FiskPier::NameFacing(Camera-Label->GetComponentLocation(),CameraUp));
}
