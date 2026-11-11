#include "World/FiskMarinaHub.h"
#include "World/FiskHarbourLife.h"
#include "World/FiskHarbourSound.h"
#include "Boats/FiskStarterBoat.h"
#include "FiskHarbourRules.h"
#include "FiskPierRules.h"
#include "FiskPierGuide.h"
#include "FiskRouteGeometry.h"
#include "World/FiskPierWalk.h"
#include "EngineUtils.h"
#include "World/FiskVisualPrimitives.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskMarinaPanel.h"
#include "Camera/CameraTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
AFiskMarinaHub::AFiskMarinaHub()
{
    bReplicates=true;bAlwaysRelevant=true;PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.5f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("MarinaRoot")));
    CreateDefaultSubobject<UFiskHarbourLife>(TEXT("HarbourLife"));
    CreateDefaultSubobject<UFiskHarbourSound>(TEXT("HarbourSound"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> A(TEXT("/Engine/BasicShapes/Cube.Cube")),B(TEXT("/Engine/BasicShapes/Sphere.Sphere")),C(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> M(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));Cube=A.Object;Sphere=B.Object;Cylinder=C.Object;Material=M.Object;
}
void AFiskMarinaHub::BeginPlay() { Super::BeginPlay();Rebuild(); }
void AFiskMarinaHub::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const { Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(AFiskMarinaHub,Residents);DOREPLIFETIME(AFiskMarinaHub,Messages); }
const TCHAR* AFiskMarinaHub::Phrase(int32 Code)
{
    static const TCHAR* Lines[]={TEXT("God tur og skitt fiske!"),TEXT("Jeg er klar ved brygga."),TEXT("Besøk gjerne naustet mitt!"),TEXT("Bra fangst – vi ses ved oppgjøret!"),TEXT("Jeg tar en pause i strandbaren."),TEXT("Skål!"),TEXT("Gratulerer med fangsten!")};
    static_assert(UE_ARRAY_COUNT(Lines)==PhraseCount,"One line per phrase code");
    return Code>=0 && Code<PhraseCount?Lines[Code]:TEXT("");
}
bool AFiskMarinaHub::Upsert(const FGuid& Profile,int32 Player,const FFiskMarinaIdentity& Identity,const TArray<FFiskMarinaTrophy>& Trophies)
{
    if(!HasAuthority() || !Profile.IsValid() || Player<0 || !Identity.IsValid() || Trophies.Num()>FiskMarina::SpeciesCount) { return false; }
    int32 Index=Residents.IndexOfByPredicate([&](const FFiskMarinaResident& R){return R.Profile==Profile;});
    if(Index<0)
    {
        if(Residents.Num()<12) { Index=Residents.AddDefaulted(); }
        else { Index=Residents.IndexOfByPredicate([](const FFiskMarinaResident& R){return R.PlayerId<0;});if(Index<0) { return false; } }
    }
    FFiskMarinaResident& R=Residents[Index];
    if(R.Profile==Profile && R.PlayerId==Player && R.Identity==Identity && R.Trophies==Trophies) { return true; }
    R.Profile=Profile;R.PlayerId=Player;R.Identity=Identity;R.Trophies=Trophies;OnRep_Residents();ForceNetUpdate();return true;
}
bool AFiskMarinaHub::Say(int32 Player,int32 Code,int32 AtMs)
{
    const FFiskMarinaResident* R=Residents.FindByPredicate([&](const FFiskMarinaResident& V){return V.PlayerId==Player;});
    if(!HasAuthority() || !R || Code<0 || Code>=PhraseCount) { return false; }
    FFiskMarinaMessage M;M.Serial=++MessageSerial;M.Name=R->Identity.DisplayName;M.Code=Code;M.AtMs=AtMs;Messages.Add(M);if(Messages.Num()>12) { Messages.RemoveAt(0); }ForceNetUpdate();return true;
}
void AFiskMarinaHub::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);if(!HasAuthority()) { return; }TSet<int32> Active;
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It) { const APlayerController* PC=It->Get();if(PC && PC->PlayerState) { Active.Add(PC->PlayerState->GetPlayerId()); } }
    bool Changed=false;for(FFiskMarinaResident& R:Residents)
    {
        if(R.PlayerId>=0 && !Active.Contains(R.PlayerId)) { PlaceChoices.Remove(R.PlayerId);R.PlayerId=-1;R.bWalking=false;Changed=true; }
        bool Sea=false;for(TActorIterator<AFiskStarterBoat> B(GetWorld());B;++B) { if(R.PlayerId>=0 && B->IsAboard(R.PlayerId) && B->GetTripProgress()!=AFiskStarterBoat::AtDock && B->GetTripProgress()!=AFiskStarterBoat::AtMarina) { Sea=true;break; } }
        const int32 Place=FiskHarbour::PublicPlace(R.PlayerId>=0,Sea,PlaceChoices.FindRef(R.PlayerId));if(R.Place!=Place) { R.Place=Place;Changed=true; }
    }
    if(Changed) { OnRep_Residents();ForceNetUpdate(); }
}
bool AFiskMarinaHub::SetPlace(int32 Player,int32 Place)
{
    if(!HasAuthority() || Player<0 || Place<0 || Place>2 || !Residents.ContainsByPredicate([&](const FFiskMarinaResident& R){return R.PlayerId==Player;})) { return false; }
    PlaceChoices.Add(Player,Place);return true;
}
bool AFiskMarinaHub::SetWalking(int32 Player,bool Enabled,FVector Position,float Yaw,float Speed)
{
    if(!HasAuthority() || !FiskPier::Inside(Position) || !FMath::IsFinite(Yaw) || !FMath::IsFinite(Speed)) { return false; }
    auto* R=Residents.FindByPredicate([&](const FFiskMarinaResident& V){return V.PlayerId==Player && Player>=0;});if(!R) { return false; }
    const bool Changed=R->bWalking!=Enabled || !R->WalkPosition.Equals(Position,.1) || !FMath::IsNearlyEqual(R->WalkYaw,Yaw,.1f) || !FMath::IsNearlyEqual(R->WalkSpeed,Speed,1.f);
    R->bWalking=Enabled;R->WalkPosition=Position;R->WalkYaw=Yaw;R->WalkSpeed=Speed;if(Changed) { ForceNetUpdate(); }return true;
}
void AFiskMarinaHub::OnRep_Residents()
{
    if(!HasActorBegunPlay()) { return; }bool Build=BuiltResidents.Num()!=Residents.Num();
    for(int32 I=0;!Build && I<Residents.Num();++I) { Build=BuiltResidents[I].Profile!=Residents[I].Profile || !(BuiltResidents[I].Identity==Residents[I].Identity) || BuiltResidents[I].Trophies!=Residents[I].Trophies; }
    if(Build) { Rebuild(); }
}
void AFiskMarinaHub::CalcCamera(float DeltaTime,FMinimalViewInfo& Out)
{
    Super::CalcCamera(DeltaTime,Out);const AFiskBoardingController* PC=Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());
    const UFiskMarinaPanel* Panel=PC?PC->GetMarinaPanel():nullptr;const bool Bar=Panel && Panel->GetPage()==2;
#if WITH_DEV_AUTOMATION_TESTS
    if(PC && PC->GetPierWalk()->CoastTestView()>=0) { const int32 I=PC->GetPierWalk()->CoastTestView();const FVector Focus=FiskRouteGeometry::Island(I)+FVector(0,0,350);Out.Location=Focus+FVector(-1.7,-1.9,1.05)*FiskRouteGeometry::Radius(I);Out.Rotation=(Focus-Out.Location).Rotation();Out.FOV=55;return; }
#endif
    if(PC && PC->GetPierWalk()->IsEnabled() && Panel && !Panel->IsVisible())
    { const auto* W=PC->GetPierWalk();const FVector Focus=W->GetPosition()+FVector(0,0,110);Out.Location=Focus+FRotator(0,W->GetCameraYaw(),0).RotateVector(FiskPier::WalkCamera(W->GetCameraMode()))*W->GetZoom();Out.Rotation=(Focus-Out.Location).Rotation();Out.FOV=60;return; }
    if(Panel && Panel->IsGallery())
    {
        const FVector Focus(-10000,-6000,650);Out.Location=Focus+FVector(450,-1800*Panel->GetGalleryZoom(),180);Out.Rotation=(Focus+FVector(450,0,0)-Out.Location).Rotation();Out.FOV=55;return;
    }
    if(Bar && Panel->GetHarbourView()>0)
    {
        const auto* Follow=Panel->FollowBoat();const bool Track=Panel->GetHarbourView()==2 && Follow;
        const FVector Focus=Track?Follow->GetActorTransform().TransformPosition(FVector(450,-650,160)):FVector(2000,2200,150);
        Out.Location=Track?Follow->GetActorTransform().TransformPosition(FVector(-2600,-3100,1700)):FVector(-3600,-5200,1800);Out.Rotation=(Focus-Out.Location).Rotation();Out.FOV=65;return;
    }
    const FVector Target=Bar?FVector(-4200,-1900,230):HousePosition(Panel?Panel->GetHouseIndex():0)+FVector(0,0,300);
    Out.Location=Target+(Bar?FVector(1500,-2500,1100):FVector(350,-1600,550));Out.Rotation=(Target+FVector(650,0,0)-Out.Location).Rotation();Out.FOV=65;
    if(Panel && Panel->GetPage()==5) { const FVector P=HousePosition(Panel->GetHouseIndex());Out.Location=P+(Panel->IsHomePreview()?FVector(1450,-3200,2050):FVector(350,-2350,1000));Out.Rotation=(P+(Panel->IsHomePreview()?FVector(0,-450,220):FVector(850,-200,330))-Out.Location).Rotation();Out.FOV=Panel->IsHomePreview()?70:65; }
}
void AFiskMarinaHub::Rebuild()
{
    BuiltResidents=Residents;
    for(USceneComponent* P:DetailParts) { if(P) { P->DestroyComponent(); } }DetailParts.Reset();
    for(UStaticMeshComponent* P:Parts) { if(P) { P->DestroyComponent(); } }for(UTextRenderComponent* S:Signs) { if(S) { S->DestroyComponent(); } }Parts.Reset();Signs.Reset();++BuildRevision;
    auto Part=[&](UStaticMesh* Mesh,FVector P,FVector Scale,FLinearColor Color)
    { UStaticMeshComponent* V=FiskVisual::AddPart(this,RootComponent,Mesh,Material,*FString::Printf(TEXT("Hub%d_%d"),BuildRevision,Parts.Num()),P,Scale,Color);Parts.Add(V);return V; };
    auto Text=[&](const FString& Label,FVector P,float Size,FColor Color)
    { UTextRenderComponent* S=NewObject<UTextRenderComponent>(this,*FString::Printf(TEXT("Sign%d_%d"),BuildRevision,Signs.Num()));AddInstanceComponent(S);S->SetupAttachment(RootComponent);S->SetRelativeLocation(P);S->SetRelativeRotation(FRotator(0,-90,0));S->SetHorizontalAlignment(EHTA_Center);S->SetWorldSize(Size);S->SetTextRenderColor(Color);S->SetText(FText::FromString(Label));S->RegisterComponent();Signs.Add(S); };
    const FLinearColor Colors[]={FLinearColor(.27f,.62f,.66f),FLinearColor(.72f,.34f,.22f),FLinearColor(.66f,.57f,.29f),FLinearColor(.35f,.53f,.28f),FLinearColor(.52f,.38f,.66f),FLinearColor(.76f,.75f,.62f)};
    for(int32 Slot=0;Slot<12;++Slot)
    {
        const FFiskMarinaResident* R=Residents.IsValidIndex(Slot)?&Residents[Slot]:nullptr;const FFiskMarinaIdentity V=R?R->Identity:FFiskMarinaIdentity();const FVector P=HousePosition(Slot);
        Part(Cube,P+FVector(0,0,100),FVector(13,12,.4),FLinearColor(.34f,.22f,.12f));
        Part(Cube,P+FVector(0,260,365),FVector(10,.35,5),Colors[V.Color]);Part(Cube,P+FVector(-490,0,365),FVector(.35,6,5),Colors[V.Color]);Part(Cube,P+FVector(490,0,365),FVector(.35,6,5),Colors[V.Color]);
        Part(Cube,P+FVector(0,0,630),FVector(11,8,.3),FLinearColor(.12f,.22f,.27f));
        if(V.Style==1) { for(int32 Side:{-1,1}) { Part(Cube,P+FVector(Side*250,0,710),FVector(5.8,8.5,.25),FLinearColor(.28f,.19f,.13f))->SetRelativeRotation(FRotator(-Side*22,0,0)); } }
        if(V.Style==2) { Part(Cube,P+FVector(0,-390,560),FVector(12,5,.2),FLinearColor(.82f,.65f,.37f));for(int32 Side:{-1,1}) { Part(Cylinder,P+FVector(Side*520,-580,330),FVector(.2,.2,4.5),FLinearColor(.75f,.7f,.55f)); } }
        for(int32 Side:{-1,1}) { Part(Cube,P+FVector(Side*530,-520,170),FVector(.2,1.2,1),FLinearColor(.25f,.17f,.1f)); }
        Text(R?V.HouseName:TEXT("LEDIG NAUST"),P+FVector(0,-310,590),48,FColor(255,232,170));
        Text(R?V.DisplayName:TEXT("VELKOMMEN"),P+FVector(0,-315,525),33,FColor::White);
        Part(Cube,P+FVector(0,225,365),FVector(8,.1,2.6),FLinearColor(.14f,.11f,.08f));
        if(R) { for(int32 I=0;I<R->Trophies.Num();++I) { const FFiskMarinaTrophy& T=R->Trophies[I];const FVector Q=P+FVector(-300+(I%4)*200,195,500-(I/4)*105);Part(Sphere,Q,FVector(1.1,.12,.35),FLinearColor(.9f,.65f,.17f));Text(FString::Printf(TEXT("%s %.1f kg"),FiskMarina::SpeciesName(T.Species),T.Grams/1000.0),Q+FVector(0,-10,-35),14,FColor::White); } }
        if(V.Decor==1) { for(int32 Side:{-1,1}) { Part(Cylinder,P+FVector(Side*520,-430,185),FVector(.8,.8,1),FLinearColor(.37f,.16f,.08f));Part(Sphere,P+FVector(Side*520,-430,290),FVector(1.3,1.3,1.5),FLinearColor(.14f,.42f,.15f)); } }
        if(V.Decor==2) { for(int32 Side:{-1,1}) { Part(Cube,P+FVector(Side*280,-430,150),FVector(2.5,1.2,.2),FLinearColor(.62f,.43f,.24f));Part(Cube,P+FVector(Side*280,-380,210),FVector(2.5,.2,1.2),FLinearColor(.62f,.43f,.24f)); } }
    }
    for(int32 Slot=0;Slot<12;++Slot) { BuildHouseDetails(Slot,Residents.IsValidIndex(Slot)?Residents[Slot].Identity:FFiskMarinaIdentity()); }
    const FVector Bar(-4200,-1900,0);Part(Cube,Bar+FVector(0,0,115),FVector(22,24,.4),FLinearColor(.54f,.35f,.17f));Part(Cube,Bar+FVector(0,250,220),FVector(14,2,2.1),FLinearColor(.45f,.25f,.1f));
    Part(Cube,Bar+FVector(0,0,700),FVector(23,15,.4),FLinearColor(.47f,.3f,.13f));
    for(int32 Side:{-1,1}) { for(int32 End:{-1,1}) { Part(Cylinder,Bar+FVector(Side*950,End*550,400),FVector(.22,.22,6),FLinearColor(.46f,.3f,.17f)); } }
    for(int32 I=0;I<5;++I) { Part(Cylinder,Bar+FVector(-600+I*300,-40,230),FVector(.55,.55,2),FLinearColor(.21f,.22f,.14f));Part(Cylinder,Bar+FVector(-600+I*300,-40,335),FVector(.9,.9,.15),FLinearColor(.8f,.54f,.2f));Part(Cylinder,Bar+FVector(-600+I*300,250,365),FVector(.12,.12,.35),FLinearColor(.1f,.65f,.5f)); }
    Text(TEXT("FISKELAGETS STRANDBAR"),Bar+FVector(0,-600,600),65,FColor(255,223,142));
}
