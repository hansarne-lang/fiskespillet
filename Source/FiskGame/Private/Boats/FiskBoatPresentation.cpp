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
        return FiskVisual::AddPart(Boat, VisualRoot, Mesh, Material, Name, Position, Scale, Color);
    };
    Part(TEXT("Hull"), Sphere, FVector(0, 0, 20), FVector(7.2, 2.6, 1.2), White);
    Part(TEXT("HullStripe"), Sphere, FVector(0, 0, 13), FVector(7.3, 2.65, 0.65), Navy);
    Part(TEXT("Deck"), Cube, FVector(-20, 0, 76), FVector(5.5, 2.15, 0.12), Wood);
    Part(TEXT("PortSide"), Cube, FVector(-35, -115, 100), FVector(5.5, 0.12, 0.6), White);
    Part(TEXT("StarboardSide"), Cube, FVector(-35, 115, 100), FVector(5.5, 0.12, 0.6), White);
    Part(TEXT("Transom"), Cube, FVector(-305, 0, 92), FVector(0.12, 2.3, 0.45), White);
    Part(TEXT("Console"), Cube, FVector(45, 0, 145), FVector(1.0, 0.85, 1.3), White);
    Part(TEXT("Windscreen"), Cube, FVector(85, 0, 214), FVector(0.08, 0.9, 0.55), FLinearColor(0.07f, 0.27f, 0.33f));
    Part(TEXT("Roof"), Cube, FVector(20, 0, 270), FVector(3.0, 2.0, 0.10), White);
    for (int32 Side = 0; Side < 2; ++Side)
    {
        const float Sign = Side == 0 ? -1.0f : 1.0f;
        Part(*FString::Printf(TEXT("RoofPost%d"), Side), Cylinder,
            FVector(0, Sign * 80, 180), FVector(0.04, 0.04, 1.8), Steel);
        Part(*FString::Printf(TEXT("Seat%d"), Side), Cube, FVector(-85, Sign * 55, 110), FVector(0.7, 0.65, 0.16), Navy);
        Part(*FString::Printf(TEXT("SeatBack%d"), Side), Cube, FVector(-115, Sign * 55, 138), FVector(0.12, 0.65, 0.5), White);
        Part(*FString::Printf(TEXT("RodHolder%d"), Side), Cylinder,
            FVector(-230, Sign * 97, 120), FVector(0.07, 0.07, 0.5), Steel);
        Part(*FString::Printf(TEXT("Reel%d"), Side), Sphere,
            FVector(-247, Sign * 97, 152), FVector(0.2, 0.2, 0.2), FLinearColor(0.7f, 0.51f, 0.13f));
        for (int32 Segment = 0; Segment < 6; ++Segment)
        {
            RodSections.Add(Part(*FString::Printf(TEXT("Rod%dSection%d"), Side, Segment), Cylinder,
                FVector::ZeroVector, FVector::OneVector, FLinearColor(0.02f, 0.025f, 0.035f)));
        }
    }
    Part(TEXT("Motor"), Cube, FVector(-357, 0, 55), FVector(0.8, 0.7, 1.0), Navy);
    Part(TEXT("MotorCap"), Sphere, FVector(-357, 0, 105), FVector(0.85, 0.72, 0.5), Navy);
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

    uint8 Activities[2] = {0, 0};
    FFiskFishVisualState FishStates[2];
    for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        const AFiskBoardingController* PC = Cast<AFiskBoardingController>(It->Get());
        if (Boat->HasAuthority() && PC && PC->PlayerState)
        {
            const EFiskRodState State = PC->GetRod()->GetState();
            const uint8 Activity = State == EFiskRodState::Fishing ? 1
                : State == EFiskRodState::InitialRun || State == EFiskRodState::Fighting || State == EFiskRodState::ReadyToLand ? 2 : 0;
            const int32 Seat = Boat->GetSeatIndex(PC->PlayerState->GetPlayerId());

            if (Seat >= 0 && Seat < 2)
            {
                Activities[Seat] = Activity;
                FFiskFishVisualState& Fish = FishStates[Seat];
                Fish.LineMeters = PC->GetRod()->GetLineOut();
                if (State == EFiskRodState::Fighting) { Fish.Phase = EFiskFishVisualPhase::Fighting; }
                else if (State == EFiskRodState::ReadyToLand) { Fish.Phase = EFiskFishVisualPhase::ReadyToLand; }
                else if (State == EFiskRodState::Landed)
                {
                    Fish.Phase = EFiskFishVisualPhase::Landing;
                    Fish.LandingAtMs = PC->GetRod()->GetLandedAtMs();
                }
            }

        }
    }


    if (Boat->HasAuthority())
    {
        Boat->SetRodActivities(Activities[0], Activities[1]);
        Boat->SetFishVisuals(FishStates[0], FishStates[1]);
    }


    const AFiskBoardingController* LocalPC = Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());
    const int32 LocalSeat = LocalPC && LocalPC->IsLocalController() && LocalPC->PlayerState
        ? Boat->GetSeatIndex(LocalPC->PlayerState->GetPlayerId()) : INDEX_NONE;
    for (int32 Seat = 0; Seat < 2; ++Seat)
    {
        const float Sign = Seat == 0 ? -1.0f : 1.0f;
        const uint8 Activity = Boat->GetRodActivity(Seat);
        const FVector Start(-230, Sign * 97, 135);
        const float Motion = FMath::Sin(Time * (Activity == 2 ? 3.5 : 1.4) + Seat) * (Activity == 2 ? 22.0f : 4.0f);
        const FVector End(-540, Sign * 220, Activity == 2 ? 175 + Motion : 270 + Motion);
        const FVector Control(-320, Sign * 145, Activity == 2 ? 310 : 260);
        FVector Previous = Start;
        for (int32 Segment = 0; Segment < 6; ++Segment)
        {
            const float T = (Segment + 1) / 6.0f;
            const FVector Point = FMath::Square(1.0f - T) * Start + 2.0f * (1.0f - T) * T * Control + T * T * End;
            FiskVisual::SetCylinder(RodSections[Seat * 6 + Segment], Previous, Point, 1.6f - Segment * 0.18f);
            Previous = Point;
        }
        if (Seat == LocalSeat)
        {
            DrawDebugString(GetWorld(), VisualRoot->GetComponentTransform().TransformPosition(Start + FVector(0, 0, 45)),
                TEXT("DIN STANG"), nullptr, FColor::Yellow, 0.07f, true, 0.8f);
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