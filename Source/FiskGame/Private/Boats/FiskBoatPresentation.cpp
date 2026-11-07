#include "Boats/FiskBoatPresentation.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/FiskBoardingController.h"
#include "Camera/CameraComponent.h"
#include "HAL/PlatformTime.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "HAL/FileManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#endif
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DayCycle/FiskDayState.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskFishPresentation.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/FiskVisualPrimitives.h"
#include "World/FiskMarineMesh.h"
#include "ProceduralMeshComponent.h"
#include "UI/FiskSkipperPanel.h"

UFiskBoatPresentation::UFiskBoatPresentation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
    ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    Cube = CubeAsset.Object;
    Sphere = SphereAsset.Object;
    Cylinder = CylinderAsset.Object;
    Material = MaterialAsset.Object;
}

void UFiskBoatPresentation::BeginPlay()
{
    Super::BeginPlay();
    AActor* Boat = GetOwner();
#if WITH_DEV_AUTOMATION_TESTS
    FParse::Value(FCommandLine::Get(), TEXT("FiskScreenshotDir="), ScreenshotDirectory);
    if (!ScreenshotDirectory.IsEmpty())
    {
        IFileManager::Get().MakeDirectory(*ScreenshotDirectory, true);
    }
#endif
    VisualRoot = NewObject<USceneComponent>(Boat, TEXT("BoatVisualRoot"));
    Boat->AddInstanceComponent(VisualRoot);
    VisualRoot->SetupAttachment(Boat->GetRootComponent());
    VisualRoot->RegisterComponent();
    SmoothedWorldPosition = Boat->GetActorLocation();
    if (UCameraComponent* Camera = Boat->FindComponentByClass<UCameraComponent>())
    {
        Camera->AttachToComponent(VisualRoot, FAttachmentTransformRules::KeepRelativeTransform);
    }

    const FLinearColor White(0.88f, 0.91f, 0.86f);
    const FLinearColor Navy(0.02f, 0.10f, 0.17f);
    const FLinearColor Wood(0.37f, 0.21f, 0.10f);
    const FLinearColor Steel(0.45f, 0.52f, 0.56f);
    auto Part = [&](const TCHAR* Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        const FString Label(Name);const int32 Seat=FCString::Atoi(*Label.Right(1));
        if(Seat>=2 && (Label.StartsWith(TEXT("Seat")) || Label.StartsWith(TEXT("RoofPost")))) { Position.X+=(Seat/2)*110; }
        return FiskVisual::AddPart(Boat, VisualRoot, Mesh, Material, Name, Position, Scale, Color);
    };
    FiskMarine::Add(Boat,VisualRoot,TEXT("SculptedHull"),FiskMarine::Hull(),Material,White,.24f);
    FiskMarine::Add(Boat,VisualRoot,TEXT("TeakDeck"),FiskMarine::Deck(),Material,FLinearColor(.49f,.32f,.16f),.68f);
    FiskMarine::Add(Boat,VisualRoot,TEXT("NavySheerStripe"),FiskMarine::Gunwale(),Material,Navy,.22f);
    for(int32 I=0;I<23;++I) { const double X=-320+I*25,T=(X+335)/690.,W=128*FMath::Pow(FMath::Max(.001,1-FMath::Pow(T,3.6)),.65)-7;Part(*FString::Printf(TEXT("DeckSeam%d"),I),Cube,FVector(X,0,85.6),FVector(.013,FMath::Max(.02,W/50),.012),FLinearColor(.15f,.10f,.055f)); }
    for(int32 Side:{-1,1})
    {
        FVector Prev(50,Side*128,153);for(int32 I=0;I<8;++I) { const double T=I/7.,X=60+T*280,Y=Side*(128-110*FMath::Pow(T,1.5)),Z=153+T*28;const FVector Next(X,Y,Z);auto* Rail=Part(*FString::Printf(TEXT("BowRail%d_%d"),Side,I),Cylinder,FVector::ZeroVector,FVector::OneVector,Steel);FiskVisual::SetCylinder(Rail,Prev,Next,2.7);if(I%2==0) { auto* Post=Part(*FString::Printf(TEXT("BowStanchion%d_%d"),Side,I),Cylinder,FVector::ZeroVector,FVector::OneVector,Steel);FiskVisual::SetCylinder(Post,FVector(X,Y,115+FMath::Pow(T,5)*30),Next,2.3); }Prev=Next; }
        for(int32 I=0;I<2;++I) { const FVector F(-210+I*160,Side*141,59);Part(*FString::Printf(TEXT("Fender%d_%d"),Side,I),Sphere,F,FVector(.25,.25,.7),FLinearColor(.85f,.87f,.81f));auto* Rope=Part(*FString::Printf(TEXT("FenderRope%d_%d"),Side,I),Cylinder,FVector::ZeroVector,FVector::OneVector,White);FiskVisual::SetCylinder(Rope,F,F+FVector(0,-Side*15,65),1.1); }
    }
    Part(TEXT("FishLocker"),Cube,FVector(-190,0,106),FVector(1.45,1.25,.38),White);
    Part(TEXT("LockerLid"),Cube,FVector(-190,0,127),FVector(1.5,1.3,.045),FLinearColor(.73f,.78f,.75f));
    Part(TEXT("LockerLatch"),Cube,FVector(-269,0,116),FVector(.035,.15,.12),Steel);
    Part(TEXT("Transom"), Cube, FVector(-305, 0, 92), FVector(0.12, 2.3, 0.45), White);
    Part(TEXT("Console"), Cube, FVector(45, 0, 145), FVector(1.0, 0.85, 1.3), White);
    Part(TEXT("Windscreen"), Cube, FVector(85, 0, 214), FVector(0.08, 0.9, 0.55), FLinearColor(0.07f, 0.27f, 0.33f));
    Part(TEXT("InstrumentPanel"),Cube,FVector(-7,0,174),FVector(.05,.64,.33),Navy);
    Part(TEXT("Plotter"),Cube,FVector(-10,-14,178),FVector(.015,.25,.16),FLinearColor(.02,.32,.37));
    for(int32 I=0;I<3;++I) { auto* Gauge=Part(*FString::Printf(TEXT("Gauge%d"),I),Cylinder,FVector(-12,15+I*10,182),FVector(.065,.065,.02),White);Gauge->SetRelativeRotation(FRotator(90,0,0)); }
    Part(TEXT("Roof"), Cube, FVector(20, 0, 270), FVector(3.0, 2.0, 0.10), White);
    for (int32 Side = 0; Side < 6; ++Side)
    {
        const float Sign = Side%2 == 0 ? -1.0f : 1.0f;
        Part(*FString::Printf(TEXT("RoofPost%d"), Side), Cylinder,
            FVector(0, Sign * 80, 180), FVector(0.04, 0.04, 1.8), Steel);
        Part(*FString::Printf(TEXT("Seat%d"), Side), Cube, FVector(-85, Sign * 55, 110), FVector(0.7, 0.65, 0.16), Navy);
        Part(*FString::Printf(TEXT("SeatBack%d"), Side), Cube, FVector(-115, Sign * 55, 138), FVector(0.12, 0.65, 0.5), White);
        Part(*FString::Printf(TEXT("RodHolder%d"), Side), Cylinder,
            FiskBoatLayout::RodBase(Side)-FVector(0,0,15), FVector(0.07, 0.07, 0.5), Steel);
        Part(*FString::Printf(TEXT("Reel%d"), Side), Sphere,
            FiskBoatLayout::RodBase(Side)+FVector(-17,0,17), FVector(0.2, 0.2, 0.2), FLinearColor(0.7f, 0.51f, 0.13f));
        for (int32 Segment = 0; Segment < 6; ++Segment)
        {
            RodSections.Add(Part(*FString::Printf(TEXT("Rod%dSection%d"), Side, Segment), Cylinder,
                FVector::ZeroVector, FVector::OneVector, FLinearColor(0.02f, 0.025f, 0.035f)));
        }
    }
    Part(TEXT("Motor"), Cube, FVector(-357, 0, 55), FVector(0.8, 0.7, 1.0), Navy);
    Part(TEXT("MotorCap"), Sphere, FVector(-357, 0, 105), FVector(0.85, 0.72, 0.5), Navy);
    Part(TEXT("OffshoreMotorPort"),Cube,FVector(-357,-65,55),FVector(.8,.6,1),Navy);
    Part(TEXT("OffshoreMotorStarboard"),Cube,FVector(-357,65,55),FVector(.8,.6,1),Navy);
    Part(TEXT("OffshoreRadar"),Sphere,FVector(20,0,305),FVector(.5,.5,.15),White);
    TowFish=FiskMarine::Add(Boat,Boat->GetRootComponent(),TEXT("SecuredTowFish"),FiskMarine::Fish(),Material,FLinearColor(.29f,.39f,.43f),.22f);TowFish->SetVisibility(false);
    TowRope=FiskVisual::AddPart(Boat,Boat->GetRootComponent(),Cylinder,Material,TEXT("TowBridle"),FVector::ZeroVector,FVector::OneVector,FLinearColor(.68f,.58f,.32f),.8f);TowRope->SetVisibility(false);
    for (int32 Index = 0; Index < 12; ++Index)
    {
        UStaticMeshComponent* Foam = FiskVisual::AddPart(Boat, Boat->GetRootComponent(), Sphere, Material,
            *FString::Printf(TEXT("Wake%d"), Index), FVector::ZeroVector, FVector::OneVector,
            FLinearColor(0.56f, 0.85f, 0.85f), 0.7f);
        Foam->SetCastShadow(false);
        Wake.Add(Foam);
    }
}

void UFiskBoatPresentation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    AFiskStarterBoat* Boat = CastChecked<AFiskStarterBoat>(GetOwner());
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const double Time = FPlatformTime::Seconds();
    const int32 Progress = Boat->GetTripProgress();
    const bool bTraveling = Progress >= 0 && Progress != AFiskStarterBoat::AtFishingSpot && Progress != AFiskStarterBoat::AtMarina;
    SmoothedWorldPosition = FMath::VInterpTo(SmoothedWorldPosition, Boat->GetActorLocation(), DeltaTime, 10.0f);
    const FVector Offset = Boat->GetActorTransform().InverseTransformPosition(SmoothedWorldPosition);
    VisualRoot->SetRelativeLocation(Offset + FVector(0, 0, FMath::Sin(Time * 1.5) * 3.0));
    VisualRoot->SetRelativeRotation(FRotator(FMath::Sin(Time * 1.1) * 0.8, 0, FMath::Sin(Time * 1.4) * 1.2));
    VisualRoot->SetRelativeScale3D(Boat->GetKind()==1?FVector(1.35):FVector::OneVector);
    for(USceneComponent* Part:VisualRoot->GetAttachChildren())
    {
        const FString Name=Part->GetName();
        if(Name.StartsWith(TEXT("Offshore"))) { Part->SetVisibility(Boat->GetKind()==1,true); }
        if(Name==TEXT("Motor") || Name==TEXT("MotorCap")) { Part->SetVisibility(Boat->GetKind()==0,true); }
        for(int32 I=2;I<6;++I)
        { if(Name==FString::Printf(TEXT("RodHolder%d"),I) || Name==FString::Printf(TEXT("Reel%d"),I)
            || Name==FString::Printf(TEXT("RoofPost%d"),I) || Name==FString::Printf(TEXT("Seat%d"),I) || Name==FString::Printf(TEXT("SeatBack%d"),I))
            { const bool Holder=Name.StartsWith(TEXT("RodHolder")) || Name.StartsWith(TEXT("Reel"));Part->SetVisibility(Boat->GetKind()==1 && (Holder || I<4),true); } }
    }

    const bool Towing=Boat->GetCargoView().Mode==static_cast<uint8>(EFiskCargoMode::Towing);
    TowFish->SetVisibility(Towing);TowRope->SetVisibility(Towing);
    if(Towing) { const FVector Fish(-1040,30*FMath::Sin(Time*.7),-5+FMath::Sin(Time*1.8)*4);TowFish->SetRelativeLocation(Fish);TowFish->SetRelativeRotation(FRotator(0,FMath::Sin(Time*.7)*3,3));const FVector Stern=VisualRoot->GetRelativeTransform().TransformPosition(FVector(-345,0,102));FiskVisual::SetCylinder(TowRope,Stern,Fish+FVector(207,0,0),2.6); }
    TArray<uint8> Activities;Activities.Init(0,6);
    TArray<FFiskFishVisualState> FishStates;FishStates.SetNum(6);
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const AFiskBoardingController* PC = Cast<AFiskBoardingController>(It->Get());
        if (Boat->HasAuthority() && PC && PC->PlayerState)
        {
            const int32 CrewSeat=Boat->GetSeatIndex(PC->PlayerState->GetPlayerId());
            for(int32 Slot=0;CrewSeat>=0 && Slot<PC->GetRodCapacity();++Slot)
            {
                const UFiskPrototypeRod* Rod=PC->GetRodAt(Slot);
                const int32 Seat=Boat->GetHolderFor(PC->PlayerState->GetPlayerId(),Slot);
                const EFiskRodState State=Rod->GetState();
                Activities[Seat]=State==EFiskRodState::Fishing?1:State==EFiskRodState::InitialRun || State==EFiskRodState::Fighting || State==EFiskRodState::ReadyToLand?2:0;
                FFiskFishVisualState& Fish=FishStates[Seat];
                Fish.CastId=Rod->GetCastId();Fish.LineMeters=Rod->GetLineOut();Fish.bSurging=Rod->IsSurging();Fish.bRaised=Rod->IsRaised();
                if(State==EFiskRodState::Fighting) { Fish.Phase=EFiskFishVisualPhase::Fighting; }
                else if(State==EFiskRodState::ReadyToLand) { Fish.Phase=EFiskFishVisualPhase::ReadyToLand; }
                else if(State==EFiskRodState::Landed) { Fish.Phase=EFiskFishVisualPhase::Landing;Fish.LandingAtMs=Rod->GetLandedAtMs(); }
            }

        }
    }


    if (Boat->HasAuthority())
    {
        Boat->SetHolderVisuals(Activities,FishStates);
    }


    const AFiskBoardingController* LocalPC = Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());
    const int32 LocalSeat = LocalPC && LocalPC->IsLocalController() && LocalPC->PlayerState
        ? Boat->GetHolderFor(LocalPC->PlayerState->GetPlayerId(),LocalPC->GetSelectedRod()) : INDEX_NONE;
    for (int32 Seat = 0; Seat < 6; ++Seat)
    {
        const float Sign = Seat%2 == 0 ? -1.0f : 1.0f;
        const uint8 Activity = Boat->GetRodActivity(Seat);
        const FVector Start=FiskBoatLayout::RodBase(Seat);
        const float Motion = FMath::Sin(Time * (Activity == 2 ? 3.5 : 1.4) + Seat) * (Activity == 2 ? 22.0f : 4.0f);
        const FFiskFishVisualState& Pose = Boat->GetFishVisual(Seat);
        const float FightingHeight = Pose.bSurging ? 155.0f : Pose.bRaised ? 240.0f : 175.0f;
        const FVector End(Start.X-310, Sign * 220, Activity == 2 ? FightingHeight + Motion : 270 + Motion);
        const FVector Control(Start.X-90, Sign * 145, Activity == 2 ? 310 : 260);
        FVector Previous = Start;
        for (int32 Segment = 0; Segment < 6; ++Segment)
        {
            const float T = (Segment + 1) / 6.0f;
            const FVector Point = FMath::Square(1.0f - T) * Start + 2.0f * (1.0f - T) * T * Control + T * T * End;
            FiskVisual::SetCylinder(RodSections[Seat * 6 + Segment], Previous, Point, 1.6f - Segment * 0.18f);
            RodSections[Seat*6+Segment]->SetVisibility(Seat<Boat->GetHolderCount());
            Previous = Point;
        }
        if (Seat == LocalSeat && !LocalPC->GetSkipperPanel()->IsPhoto())
        {
            DrawDebugString(GetWorld(), VisualRoot->GetComponentTransform().TransformPosition(Start + FVector(0, 0, 45)),
                TEXT("VALGT STANG"), nullptr, FColor::Yellow, 0.07f, true, 0.8f);
        }
        if (Activity > 0)
        {
            const FVector Tip = VisualRoot->GetComponentTransform().TransformPosition(End);

            FVector WaterEnd = Boat->GetActorTransform().TransformPosition(FVector(-1600, Sign * 400, 0));
            const FFiskFishVisualState& FishState = Boat->GetFishVisual(Seat);
            if (FishState.Phase == EFiskFishVisualPhase::Fighting)
            {
                WaterEnd = Boat->GetActorTransform().TransformPosition(
                    FVector(-540 - FishState.LineMeters * 80, Sign * (220 + FishState.LineMeters * 60), -20));
            }
            if (const UFiskFishPresentation* Fish = Boat->FindComponentByClass<UFiskFishPresentation>())
            {
                FVector Mouth;
                if (Fish->GetLineTarget(Seat, Mouth)) { WaterEnd = Mouth; }
            }

            DrawDebugLine(GetWorld(), Tip, WaterEnd, Seat == LocalSeat ? FColor(255, 225, 130) : FColor(220, 235, 225), false, 0.07f, 0, 1.0f);
        }
    }
#if WITH_DEV_AUTOMATION_TESTS
    CaptureTestStage();
#endif
    for (int32 Index = 0; Index < Wake.Num(); ++Index)
    {
        const int32 Step = Index / 2;
        const float Sign = Index % 2 == 0 ? -1.0f : 1.0f;
        Wake[Index]->SetVisibility(bTraveling);
        Wake[Index]->SetRelativeLocation(FVector(-430 - Step * 170, Sign * (50 + Step * 30), -3));
        Wake[Index]->SetRelativeScale3D(FVector(2.0 + Step * 0.4, 0.25 + Step * 0.12, 0.035));
    }
}

#if WITH_DEV_AUTOMATION_TESTS
void UFiskBoatPresentation::CaptureTestStage()
{
    if (ScreenshotDirectory.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
    const AFiskStarterBoat* Boat = CastChecked<AFiskStarterBoat>(GetOwner());
    const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController() || PC->GetViewTarget() != Boat) { return; }
    const int32 Progress = Boat->GetTripProgress();
    int32 Stage = INDEX_NONE;
    if (Progress == AFiskStarterBoat::AtDock && Boat->GetCrewCount() == 2) { Stage = 0; }
    else if (Progress >= 0 && Progress < AFiskStarterBoat::AtFishingSpot) { Stage = 1; }
    else if (Progress == AFiskStarterBoat::AtFishingSpot)
    {
        if (PC->GetRod()->GetState() == EFiskRodState::Fishing) { Stage = 2; }
        else if (PC->GetRod()->GetState() == EFiskRodState::Fighting) { Stage = 3; }
    }
    else if (Progress > AFiskStarterBoat::AtFishingSpot && Progress < AFiskStarterBoat::AtMarina) { Stage = 4; }
    else if (Progress == AFiskStarterBoat::AtMarina) { Stage = 5; }
    if (Stage != INDEX_NONE && !CapturedStages.Contains(Stage))
    {
        CapturedStages.Add(Stage);
        const FString File = FPaths::Combine(ScreenshotDirectory, FString::Printf(TEXT("stage-%d.png"), Stage));
        FScreenshotRequest::RequestScreenshot(File, false, false);
    }
}
#endif
