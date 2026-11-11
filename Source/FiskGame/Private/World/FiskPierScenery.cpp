#include "World/FiskPierScenery.h"
#include "FiskPierRules.h"
#include "World/FiskVisualPrimitives.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
void FiskPierScenery::Build(AActor* O,UStaticMesh* Cube,UStaticMesh* Cylinder,UMaterialInterface* Material)
{
    int32 N=0;const FLinearColor Wood(.29f,.19f,.1f),Dark(.11f,.08f,.04f),Rope(.68f,.58f,.32f);
    auto Part=[&](UStaticMesh* Mesh,FVector P,FVector S,FLinearColor C){return FiskVisual::AddPart(O,O->GetRootComponent(),Mesh,Material,*FString::Printf(TEXT("PierLife%d"),N++),P,S,C,.8f);};
    for(const auto& A:FiskPier::Areas)
    {
        Part(Cube,FVector((A.X0+A.X1)/2,(A.Y0+A.Y1)/2,112),FVector((A.X1-A.X0)/100,(A.Y1-A.Y0)/100,.36),Wood);
        const bool Long=A.Y1-A.Y0>A.X1-A.X0;
        for(double V=Long?A.Y0:A.X0;V<(Long?A.Y1:A.X1);V+=240) { Part(Cube,Long?FVector((A.X0+A.X1)/2,V,130.3):FVector(V,(A.Y0+A.Y1)/2,130.3),Long?FVector((A.X1-A.X0)/100,.025,.012):FVector(.025,(A.Y1-A.Y0)/100,.012),Dark); }
    }
    for(int32 I=0;I<FiskPier::PostCount;++I)
    {
        const FVector P=FiskPier::Post(I);Part(Cylinder,P+FVector(0,0,-60),FVector(.22,.22,2.8),Wood);
        if(I%2==0) { Part(Cylinder,P+FVector(0,0,165),FVector(.06,.06,3.3),Dark);Part(Cube,P+FVector(0,0,338),FVector(.45,.45,.4),FLinearColor(.94f,.73f,.31f));
            auto* L=NewObject<UPointLightComponent>(O,*FString::Printf(TEXT("PierLamp%d"),I));O->AddInstanceComponent(L);L->SetupAttachment(O->GetRootComponent());L->SetRelativeLocation(P+FVector(0,0,330));L->SetIntensity(450);L->SetAttenuationRadius(500);L->SetCastShadows(false);L->SetLightColor(FLinearColor(1,.75f,.4f));L->RegisterComponent(); }
    }
    auto Sign=[&](const TCHAR* Label,FVector P){Part(Cylinder,P+FVector(0,0,130),FVector(.1,.1,2.6),Dark);Part(Cube,P+FVector(0,0,270),FVector(2.7,.14,.7),Wood);auto* S=NewObject<UTextRenderComponent>(O,*FString::Printf(TEXT("PierSign%d"),N++));O->AddInstanceComponent(S);S->SetupAttachment(O->GetRootComponent());S->SetRelativeLocation(P+FVector(0,-9,280));S->SetRelativeRotation(FRotator(0,-90,0));S->SetHorizontalAlignment(EHTA_Center);S->SetWorldSize(24);S->SetText(FText::FromString(Label));S->RegisterComponent();};
    Sign(TEXT("OMBORD / E"),FVector(-1050,300,130));Sign(TEXT("STRANDBAR"),FVector(-1550,-2700,130));
    for(int32 I=0;I<3;++I) { Sign(*FString::Printf(TEXT("NAUST %d–%d"),I*4+1,I*4+4),FVector(-1550,3500+I*2300,130)); }
    // Beach by the boardwalk (#18): sun loungers with umbrellas on the sand north of the platform. They stand
    // outside the walkable deck, so they need no clearance.
    Sign(TEXT("STRANDA"),FVector(-5200,-2500,130));
    for(int32 I=0;I<4;++I)
    {
        const FVector P(-7300+I*260,-1650,30);
        Part(Cube,P+FVector(0,0,18),FVector(.7,1.9,.12),FLinearColor(.93f,.9f,.82f));
        Part(Cube,P+FVector(0,-80,40),FVector(.7,.35,.4),FLinearColor(.93f,.9f,.82f));
        if(I%2==0) { Part(Cylinder,P+FVector(130,40,110),FVector(.05,.05,2.2),Dark);Part(Cylinder,P+FVector(130,40,220),FVector(2.2,2.2,.06),I==0?FLinearColor(.86f,.24f,.2f):FLinearColor(.16f,.5f,.72f)); }
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_SCENERY parts=%d"),N);
}
