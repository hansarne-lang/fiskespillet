#include "Boats/FiskCrewFigures.h"
#include "Boats/FiskStarterBoat.h"
#include "World/FiskMarinaHub.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Components/SceneComponent.h"
#include "UObject/ConstructorHelpers.h"
UFiskCrewFigures::UFiskCrewFigures()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f;Figures.SetNum(4);
    ConstructorHelpers::FObjectFinder<UStaticMesh> A(TEXT("/Engine/BasicShapes/Cube.Cube")),B(TEXT("/Engine/BasicShapes/Sphere.Sphere")),C(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> M(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));Cube=A.Object;Sphere=B.Object;Cylinder=C.Object;Material=M.Object;
}
void UFiskCrewFigures::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);auto* Boat=CastChecked<AFiskStarterBoat>(GetOwner());const auto* PC=GetWorld()->GetFirstPlayerController();if(!PC || !PC->IsLocalController()) { return; }
    TActorIterator<AFiskMarinaHub> Hub(GetWorld());const FVector Camera=PC->PlayerCameraManager?PC->PlayerCameraManager->GetCameraLocation():FVector::ZeroVector;
    USceneComponent* Parent=Boat->GetRootComponent();for(USceneComponent* Child:Parent->GetAttachChildren()) { if(Child && Child->GetName()==TEXT("BoatVisualRoot")) { Parent=Child;break; } }
    for(int32 Seat=0;Seat<4;++Seat)
    {
        const int32 Id=Boat->GetCrewMemberId(Seat);const auto* Resident=Hub?Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.PlayerId==Id && Id>=0;}):nullptr;
        auto& F=Figures[Seat];if(Id<0 || (Resident && (Resident->Place==1 || Resident->Place==2))) { if(F.Root) { F.Root->SetVisibility(false,true); }continue; }
        if(!F.Root) { const FLinearColor Colors[]={FLinearColor(.16,.54,.59),FLinearColor(.72,.27,.17),FLinearColor(.73,.6,.24),FLinearColor(.21,.48,.25),FLinearColor(.47,.29,.66),FLinearColor(.77,.76,.6)};F.Build(Boat,Parent,*FString::Printf(TEXT("CrewFigure%d"),Seat),Sphere,Cube,Cylinder,Material,Colors[Resident?Resident->Identity.Color:Seat]); }
        static const FLinearColor Colors[]={FLinearColor(.16f,.54f,.59f),FLinearColor(.72f,.27f,.17f),FLinearColor(.73f,.6f,.24f),FLinearColor(.21f,.48f,.25f),FLinearColor(.47f,.29f,.66f),FLinearColor(.77f,.76f,.6f)};F.SetShirt(Colors[Resident?Resident->Identity.Color:Seat]);
        F.Root->SetVisibility(true,true);F.Root->SetRelativeLocation(FVector(-175+(Seat/2)*130,(Seat%2?1:-1)*58,93));F.Root->SetRelativeRotation(FRotator(0,180,0));F.Root->SetRelativeScale3D(FVector(.78));
        const int32 Holder=Boat->GetHolderFor(Id,0);F.Pose(GetWorld()->GetRealTimeSeconds(),Holder>=0?Boat->GetRodActivity(Holder):0,false,Resident?Resident->Identity.DisplayName:FString::Printf(TEXT("Fisker %d"),Id),Camera);
    }
    if(!bReported && Boat->GetCrewCount()>0) { bReported=true;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CREW_FIGURES boat=%d crew=%d"),Boat->GetFleetIndex(),Boat->GetCrewCount()); }
}
