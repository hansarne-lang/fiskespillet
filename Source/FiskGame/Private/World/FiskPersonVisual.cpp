#include "World/FiskPersonVisual.h"
#include "World/FiskVisualPrimitives.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
void FFiskPersonVisual::Build(AActor* Actor,USceneComponent* Parent,FName Name,UStaticMesh* Sphere,UStaticMesh* Cube,UStaticMesh* Cylinder,UMaterialInterface* Material,FLinearColor Shirt)
{
    Root=NewObject<USceneComponent>(Actor,Name);Actor->AddInstanceComponent(Root);Root->SetupAttachment(Parent);Root->RegisterComponent();
    int32 N=0;auto Part=[&](UStaticMesh* Mesh,FVector P,FVector Scale,FLinearColor Color){return FiskVisual::AddPart(Actor,Root,Mesh,Material,*FString::Printf(TEXT("%s_P%d"),*Name.ToString(),N++),P,Scale,Color,.8f);};
    const FLinearColor Skin(.67f,.43f,.29f),Navy(.035f,.095f,.14f);
    Part(Sphere,FVector(0,0,117),FVector(.52,.31,.65),Shirt);Part(Cube,FVector(0,0,79),FVector(.45,.3,.26),Navy);
    Part(Cylinder,FVector(0,0,149),FVector(.14,.14,.17),Skin);Part(Sphere,FVector(0,0,167),FVector(.34,.3,.38),Skin);
    Part(Cylinder,FVector(0,0,188),FVector(.48,.44,.04),Navy);Part(Sphere,FVector(0,0,191),FVector(.35,.33,.15),Navy);
    for(int32 S:{-1,1})
    {
        auto* Leg=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Navy);FiskVisual::SetCylinder(Leg,FVector(0,S*13,77),FVector(7,S*13,20),8);
        Part(Cube,FVector(15,S*13,12),FVector(.35,.2,.16),Navy);Part(Sphere,FVector(16,S*9,170),FVector(.045,.04,.04),FLinearColor(.04,.045,.04));
        const int32 I=S<0?0:1;Upper[I]=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Shirt);Lower[I]=Part(Cylinder,FVector::ZeroVector,FVector::OneVector,Skin);
    }
    Label=NewObject<UTextRenderComponent>(Actor,*FString::Printf(TEXT("%s_Name"),*Name.ToString()));Actor->AddInstanceComponent(Label);Label->SetupAttachment(Root);Label->SetRelativeLocation(FVector(0,0,225));Label->SetHorizontalAlignment(EHTA_Center);Label->SetWorldSize(23);Label->SetTextRenderColor(FColor(215,245,235));Label->RegisterComponent();
}
void FFiskPersonVisual::Pose(double Time,int32 Activity,bool Wave,const FString& Name,FVector Camera)
{
    if(!Root) { return; }const float Bob=FMath::Sin(Time*1.7)*1.7;
    for(int32 I=0;I<2;++I)
    {
        const float S=I?1:-1;const bool Salute=Wave && I==1;
        const FVector A(0,S*28,137),B(Salute?FVector(6,S*47,166):FVector(Activity?27:8,S*36,108+Bob)),C(Salute?FVector(11,S*(48+FMath::Sin(Time*8)*13),205):FVector(Activity?48:15,S*26,Activity?129+FMath::Sin(Time*3)*9:82+Bob));
        FiskVisual::SetCylinder(Upper[I],A,B,8);FiskVisual::SetCylinder(Lower[I],B,C,6.5);
    }
    Label->SetText(FText::FromString(Name));Label->SetWorldRotation((Camera-Label->GetComponentLocation()).Rotation());
}
