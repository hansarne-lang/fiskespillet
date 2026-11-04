#include "Fishing/FiskFishPresentation.h"
#include "Fishing/FiskFishVisualState.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskBoatPresentation.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformTime.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/FiskVisualPrimitives.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#endif

UFiskFishPresentation::UFiskFishPresentation()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f / 30.0f;
    ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> ConeAsset(TEXT("/Engine/BasicShapes/Cone.Cone"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    Sphere = SphereAsset.Object;
    Cone = ConeAsset.Object;
    Material = MaterialAsset.Object;
}

void UFiskFishPresentation::BeginPlay()
{
    Super::BeginPlay();
    AActor* Boat = GetOwner();
    if (UFiskBoatPresentation* Presentation = Boat->FindComponentByClass<UFiskBoatPresentation>())
    {
        Presentation->AddTickPrerequisiteComponent(this);
    }
#if WITH_DEV_AUTOMATION_TESTS
    FParse::Value(FCommandLine::Get(), TEXT("FiskScreenshotDir="), ScreenshotDirectory);
#endif
    for (int32 Seat = 0; Seat < 2; ++Seat)
    {
        USceneComponent* Root = NewObject<USceneComponent>(Boat, *FString::Printf(TEXT("Fish%dRoot"), Seat));
        Boat->AddInstanceComponent(Root);
        Root->SetupAttachment(Boat->GetRootComponent());
        Root->RegisterComponent();
        FishRoots.Add(Root);
        auto Part = [&](const TCHAR* Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
        {
            const FName UniqueName(*FString::Printf(TEXT("Fish%d_%s"), Seat, Name));
            return FiskVisual::AddPart(Boat, Root, Mesh, Material, UniqueName, Position, Scale, Color, 0.22f);
        };
        const FLinearColor Silver(0.67f, 0.76f, 0.78f);
        const FLinearColor Back(0.025f, 0.10f, 0.18f);
        Part(TEXT("Body"), Sphere, FVector::ZeroVector, FVector(0.85, 0.22, 0.22), Silver);
        Part(TEXT("Back"), Sphere, FVector(-2, 0, 5), FVector(0.79, 0.205, 0.13), Back);
        Part(TEXT("Belly"), Sphere, FVector(1, 0, -4), FVector(0.68, 0.18, 0.12), FLinearColor(0.86f, 0.88f, 0.82f));
        Part(TEXT("Head"), Sphere, FVector(26, 0, 1), FVector(0.33, 0.215, 0.20), Silver);
        Part(TEXT("Mouth"), Sphere, FVector(43, 0, 1), FVector(0.05, 0.09, 0.055), Back);
        for (int32 Side = 0; Side < 2; ++Side)
        {
            const float Sign = Side == 0 ? -1.0f : 1.0f;
            Part(*FString::Printf(TEXT("Eye%d"), Side), Sphere, FVector(29, Sign * 10, 5), FVector(0.04), FLinearColor(0.005f, 0.008f, 0.01f));
            UStaticMeshComponent* Fin = Part(*FString::Printf(TEXT("Fin%d"), Side), Cone,
                FVector(0, Sign * 15, -2), FVector(0.27, 0.04, 0.20), Back);
            Fin->SetRelativeRotation(FRotator(0, Sign * 30, Sign * 80));
        }
        Part(TEXT("Dorsal"), Cone, FVector(-8, 0, 17), FVector(0.30, 0.045, 0.25), Back);
        USceneComponent* Tail = NewObject<USceneComponent>(Boat, *FString::Printf(TEXT("Fish%dTail"), Seat));
        Boat->AddInstanceComponent(Tail);
        Tail->SetupAttachment(Root);
        Tail->SetRelativeLocation(FVector(-40, 0, 0));
        Tail->RegisterComponent();
        TailRoots.Add(Tail);
        for (int32 Fluke = 0; Fluke < 2; ++Fluke)
        {
            const float Sign = Fluke == 0 ? 1.0f : -1.0f;
            UStaticMeshComponent* TailFin = FiskVisual::AddPart(Boat, Tail, Sphere, Material,
                *FString::Printf(TEXT("Fish%dTailFin%d"), Seat, Fluke), FVector(-12, 0, Sign * 9),
                FVector(0.35, 0.05, 0.12), Back, 0.25f);
            TailFin->SetRelativeRotation(FRotator(-Sign * 35, 0, 0));
        }
        Root->SetVisibility(false, true);
        for (int32 Drop = 0; Drop < 8; ++Drop)
        {
            UStaticMeshComponent* Splash = FiskVisual::AddPart(Boat, Boat->GetRootComponent(), Sphere, Material,
                *FString::Printf(TEXT("Fish%dSplash%d"), Seat, Drop), FVector::ZeroVector, FVector(0.035f),
                FLinearColor(0.74f, 0.93f, 0.98f), 0.2f);
            Splash->SetCastShadow(false);
            Splash->SetVisibility(false);
            Droplets.Add(Splash);
        }
    }
}

bool UFiskFishPresentation::GetLineTarget(int32 Seat, FVector& Target) const
{
    if (Seat < 0 || Seat >= FishRoots.Num() || !Visible[Seat]) { return false; }
    Target = FishRoots[Seat]->GetComponentTransform().TransformPosition(FVector(43, 0, 1));
    return true;
}

void UFiskFishPresentation::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    const AFiskStarterBoat* Boat = CastChecked<AFiskStarterBoat>(GetOwner());
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Day) { return; }
    const double Time = FPlatformTime::Seconds();
    for (int32 Seat = 0; Seat < FishRoots.Num(); ++Seat)
    {
        const FFiskFishVisualState& State = Boat->GetFishVisual(Seat);
        const bool bNear = State.Phase == EFiskFishVisualPhase::Fighting && State.LineMeters <= 10;
        const bool bReady = State.Phase == EFiskFishVisualPhase::ReadyToLand;
        const bool bLanding = State.Phase == EFiskFishVisualPhase::Landing && State.LandingAtMs >= 0;
        const double LandingAge = bLanding ? (Day->GetElapsedMilliseconds() - State.LandingAtMs) / 1000.0 : 0.0;
        Visible[Seat] = bNear || bReady || (bLanding && LandingAge >= 0.0 && LandingAge < 3.0);
        FishRoots[Seat]->SetVisibility(Visible[Seat], true);
        if (State.Phase == EFiskFishVisualPhase::Hidden)
        {
            NearReported[Seat] = false;
            LandingReported[Seat] = false;
            HiddenReported[Seat] = false;
        }
        if (bLanding && LandingAge >= 3.0 && LandingReported[Seat] && !HiddenReported[Seat])
        {
            HiddenReported[Seat] = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_VISUAL_FINISHED seat=%d"), Seat);
        }
        const bool bSplash = bNear || bReady || (bLanding && LandingAge >= 0.0 && LandingAge < 0.8);
        const float SplashSign = Seat == 0 ? -1.0f : 1.0f;
        for (int32 Drop = 0; Drop < 8; ++Drop)
        {
            UStaticMeshComponent* Splash = Droplets[Seat * 8 + Drop];
            Splash->SetVisibility(bSplash);
            const double Cycle = FMath::Fmod(Time * (State.bSurging ? 2.5 : 1.5) + Drop * 0.125, 1.0);
            const double Angle = Drop * PI * 0.25;
            Splash->SetRelativeLocation(FVector(-180 + FMath::Cos(Angle) * Cycle * 55,
                SplashSign * (300 + FMath::Min(State.LineMeters, 10) * 12) + FMath::Sin(Angle) * Cycle * 45,
                5 + FMath::Sin(Cycle * PI) * (State.bSurging ? 80 : 45)));
            Splash->SetRelativeScale3D(FVector(0.025, 0.025, 0.055) * (1.0 - Cycle * 0.5));
        }
        if (bSplash && !SplashReported[Seat])
        {
            SplashReported[Seat] = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_SPLASH seat=%d"), Seat);
        }
        if (!Visible[Seat]) { continue; }

        const float Sign = Seat == 0 ? -1.0f : 1.0f;
        FVector Position(-180, Sign * (300 + FMath::Min(State.LineMeters, 10) * 12), 4 + FMath::Sin(Time * 5 + Seat) * 4);
        FRotator Rotation(FMath::Sin(Time * 4) * 4, FMath::Sin(Time * 2 + Seat) * 8, FMath::Sin(Time * 6) * 8);
        if (bLanding)
        {
            const double Alpha = FMath::Clamp(LandingAge / 1.5, 0.0, 1.0);
            Position = FMath::Lerp(FVector(-180, Sign * 300, 4), FVector(-210, Sign * 50, 94), Alpha);
            Position.Z += FMath::Sin(Alpha * PI) * 120.0;
            Rotation = FRotator(0, 0, Alpha * 90.0);
        }
        FishRoots[Seat]->SetRelativeLocationAndRotation(Position, Rotation);
        TailRoots[Seat]->SetRelativeRotation(FRotator(0, FMath::Sin(Time * 12 + Seat) * 22, 0));
        if (bNear && !NearReported[Seat])
        {
            NearReported[Seat] = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_VISUAL_NEAR seat=%d"), Seat);
        }
        if (bLanding && !LandingReported[Seat])
        {
            LandingReported[Seat] = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_VISUAL_LANDING seat=%d"), Seat);
        }
#if WITH_DEV_AUTOMATION_TESTS
        CaptureTestPhase(Seat, bLanding, LandingAge);
#endif
    }
}

#if WITH_DEV_AUTOMATION_TESTS
void UFiskFishPresentation::CaptureTestPhase(int32 Seat, bool bLanding, double LandingAge)
{
    if (ScreenshotDirectory.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
    const AFiskStarterBoat* Boat = CastChecked<AFiskStarterBoat>(GetOwner());
    const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetWorld()->GetFirstPlayerController());
    if (!PC || !PC->IsLocalController() || !PC->PlayerState || PC->GetViewTarget() != Boat
        || Boat->GetSeatIndex(PC->PlayerState->GetPlayerId()) != Seat) { return; }
    if (!bLanding && !bCapturedNear)
    {
        bCapturedNear = true;
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(ScreenshotDirectory, TEXT("fish-near-boat.png")), false, false);
    }
    else if (bLanding && LandingAge >= 0.65 && !bCapturedLanding)
    {
        bCapturedLanding = true;
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(ScreenshotDirectory, TEXT("fish-landing.png")), false, false);
    }
}
#endif
