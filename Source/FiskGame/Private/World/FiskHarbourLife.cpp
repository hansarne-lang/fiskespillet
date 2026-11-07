#include "World/FiskHarbourLife.h"
#include "World/FiskMarinaHub.h"
#include "World/FiskSpeciesArt.h"
#include "World/FiskVisualPrimitives.h"
#include "FiskHarbourRules.h"
#include "World/FiskPierScenery.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "EngineUtils.h"
#include "UI/FiskMarinaPanel.h"
#include "DayCycle/FiskDayState.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
UFiskHarbourLife::UFiskHarbourLife()
{
    PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f;People.SetNum(12);
    ConstructorHelpers::FObjectFinder<UStaticMesh> A(TEXT("/Engine/BasicShapes/Cube.Cube")),B(TEXT("/Engine/BasicShapes/Sphere.Sphere")),C(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> M(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));Cube=A.Object;Sphere=B.Object;Cylinder=C.Object;Material=M.Object;
}
void UFiskHarbourLife::BeginPlay()
{
    Super::BeginPlay();FiskPierScenery::Build(GetOwner(),Cube,Cylinder,Material);int32 N=0;
    auto Part=[&](UStaticMesh* Mesh,FVector P,FVector Scale,FLinearColor Color){return FiskVisual::AddPart(GetOwner(),GetOwner()->GetRootComponent(),Mesh,Material,*FString::Printf(TEXT("HarbourLiving%d"),N++),P,Scale,Color,.75f);};
    const FVector Bar(-4200,-1900,0);
    for(int32 I=0;I<12;++I)
    { const FVector P(-5000+(I%6)*300,-2280-(I/6)*380,0);Part(Cylinder,P+FVector(0,0,180),FVector(.35,.35,.9),FLinearColor(.28,.19,.11));Part(Cylinder,P+FVector(0,0,232),FVector(.65,.65,.13),FLinearColor(.76,.51,.22)); }
    for(int32 I=0;I<5;++I)
    { const FVector P=Bar+FVector(-1200+I*600,-1250,0);Part(Cylinder,P+FVector(0,0,210),FVector(.13,.13,2),FLinearColor(.3,.23,.13));Part(Cylinder,P+FVector(0,0,319),FVector(2.5,2.5,.15),FLinearColor(.62,.42,.2));Part(Cylinder,P+FVector(0,0,345),FVector(.15,.15,.5),FLinearColor(.13,.56,.51)); }
    for(int32 I=0;I<7;++I)
    { const FVector P=Bar+FVector(-1600+I*530,950,0);Part(Cylinder,P+FVector(0,0,290),FVector(.35,.35,5.8),FLinearColor(.43,.3,.16));for(int32 S=0;S<6;++S) { auto* Leaf=Part(Sphere,P+FVector(190*FMath::Cos(S*PI/3),190*FMath::Sin(S*PI/3),600),FVector(4.8,.75,.16),FLinearColor(.1,.36,.14));Leaf->SetRelativeRotation(FRotator(-14,S*60,0)); } }
}
FLinearColor UFiskHarbourLife::PersonColor(FGuid Profile) const
{ const auto* H=CastChecked<AFiskMarinaHub>(GetOwner());const int32 I=H->GetResidents().IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Profile==Profile;});return People.IsValidIndex(I)?People[I].ShirtColor:FLinearColor::Black; }
void UFiskHarbourLife::BuildGallery(int32 Species)
{
    for(auto P:GalleryParts) { if(P) { P->DestroyComponent(); } }GalleryParts.Reset();if(GalleryRoot) { GalleryRoot->DestroyComponent(); }
    const auto* Art=FiskSpeciesArt::Find(Species);if(!Art) { BuiltSpecies=-1;return; }++Generation;
    GalleryRoot=NewObject<USceneComponent>(GetOwner(),*FString::Printf(TEXT("SpeciesGallery%d"),Generation));GetOwner()->AddInstanceComponent(GalleryRoot);GalleryRoot->SetupAttachment(GetOwner()->GetRootComponent());GalleryRoot->SetRelativeLocation(FVector(-10000,-6000,650));GalleryRoot->RegisterComponent();
    GalleryParts.Add(FiskMarine::Add(GetOwner(),GalleryRoot,*FString::Printf(TEXT("SpeciesBody%d"),Generation),FiskSpeciesArt::Body(Species),Material,Art->Body,.36f));
    GalleryParts.Add(FiskMarine::Add(GetOwner(),GalleryRoot,*FString::Printf(TEXT("SpeciesFins%d"),Generation),FiskSpeciesArt::Fins(Species),Material,Art->Accent,.42f));
    auto Part=[&](FVector P,FVector Scale,FLinearColor Color){auto* C=FiskVisual::AddPart(GetOwner(),GalleryRoot,Sphere,Material,*FString::Printf(TEXT("GalleryPart%d_%d"),Generation,GalleryParts.Num()),P,Scale,Color,.5f);GalleryParts.Add(C);return C;};
    for(int32 Side:{-1,1}) { const FVector P=FiskSpeciesArt::Eye(Species,Side);Part(P,FVector(Art->Eye/50),FLinearColor(.89,.85,.64));Part(P+FVector(1,Side*Art->Eye*.7,0),FVector(Art->Eye/90),FLinearColor(.01,.02,.025)); }
    if(Species==1 || Species==5 || Species==7 || Species==10)
    { for(int32 I=0;I<8;++I) { const float T=.3f+.055f*I;const float Radius=FMath::Pow(FMath::Sin(T*PI),.7f);for(int32 S:{-1,1}) { Part(FVector((T-.5f)*Art->Length,S*Art->Width*Radius*.94f,Species==5?0:5),FVector(Species==5?.06:.15,.035,Species==5?Art->Height/60:.04),Art->Accent); } } }
    BuiltSpecies=Species;UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SPECIES_GALLERY species=%d bodyVertices=%d finVertices=%d"),Species,FiskSpeciesArt::Body(Species).Vertices.Num(),FiskSpeciesArt::Fins(Species).Vertices.Num());
}
void UFiskHarbourLife::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);const auto* PC=Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());if(!PC || !PC->IsLocalController()) { return; }
    const auto* Hub=CastChecked<AFiskMarinaHub>(GetOwner());const auto* Panel=PC->GetMarinaPanel();const FVector Camera=PC->PlayerCameraManager?PC->PlayerCameraManager->GetCameraLocation():FVector::ZeroVector;
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();const double Now=GetWorld()->GetRealTimeSeconds();VisiblePeople=0;
    for(const auto& M:Hub->GetMessages()) { Greetings.Observe(M.Serial,M.Name,Now); }
    for(int32 I=0;I<12;++I)
    {
        auto& F=People[I];const auto* Resident=Hub->GetResidents().IsValidIndex(I)?&Hub->GetResidents()[I]:nullptr;
        bool Aboard=false;if(Resident) { for(TActorIterator<AFiskStarterBoat> B(GetWorld());B;++B) { if(B->IsAboard(Resident->PlayerId)) { Aboard=true;break; } } }
        const bool Show=Resident && Resident->PlayerId>=0 && Resident->Place>=0 && Resident->Place!=3 && !(Aboard && Resident->Place==0);
        if(!Show) { if(F.Root) { F.Root->SetVisibility(false,true); }continue; }
        if(!F.Root) { const FLinearColor Colors[]={FLinearColor(.18,.56,.61),FLinearColor(.75,.3,.2),FLinearColor(.73,.62,.3),FLinearColor(.25,.55,.27),FLinearColor(.49,.32,.65),FLinearColor(.8,.78,.65)};F.Build(GetOwner(),GetOwner()->GetRootComponent(),*FString::Printf(TEXT("HarbourPerson%d"),I),Sphere,Cube,Cylinder,Material,Colors[Resident->Identity.Color]); }
        static const FLinearColor Colors[]={FLinearColor(.18f,.56f,.61f),FLinearColor(.75f,.3f,.2f),FLinearColor(.73f,.62f,.3f),FLinearColor(.25f,.55f,.27f),FLinearColor(.49f,.32f,.65f),FLinearColor(.8f,.78f,.65f)};
        F.SetShirt(Colors[Resident->Identity.Color]);++VisiblePeople;F.Root->SetVisibility(true,true);
        const FVector Target=Resident->bWalking?Resident->WalkPosition:Resident->Place==1?AFiskMarinaHub::HousePosition(I)+FVector(0,-650,130):FiskHarbour::MeetingPoint(I,Resident->Place);
        F.Root->SetRelativeLocation(Resident->bWalking?FMath::VInterpTo(F.Root->GetRelativeLocation(),Target,Delta,16):Target);F.Root->SetRelativeRotation(FRotator(0,Resident->bWalking?Resident->WalkYaw:Resident->Place==1?-90:Resident->Place==2?-90:0,0));
        F.Pose(Now,0,Greetings.IsWaving(Resident->Identity.DisplayName,Now),Resident->Identity.DisplayName,Camera,Resident->bWalking?Resident->WalkSpeed/220:0);
    }
    const bool Gallery=Panel && Panel->IsGallery();if(Gallery && BuiltSpecies!=Panel->GetGallerySpecies()) { BuildGallery(Panel->GetGallerySpecies()); }
    if(GalleryRoot) { GalleryRoot->SetVisibility(Gallery,true);if(Gallery) { GalleryRoot->SetRelativeRotation(FRotator(0,Panel->GetGalleryYaw(),0));GalleryRoot->SetRelativeLocation(FVector(-10000,-6000,650+FMath::Sin(GetWorld()->GetRealTimeSeconds())*7)); } }
}
