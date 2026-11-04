#include "World/FiskOceanScene.h"
#include "World/FiskVisualPrimitives.h"
#include "Boats/FiskStarterBoat.h"
#include "Camera/CameraComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AFiskOceanScene::AFiskOceanScene()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f / 20.0f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("OceanRoot")));
    USkyAtmosphereComponent* Sky = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Sky"));
    Sky->SetupAttachment(RootComponent);
    UDirectionalLightComponent* Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
    Sun->SetupAttachment(RootComponent);
    Sun->SetMobility(EComponentMobility::Movable);
    Sun->SetRelativeRotation(FRotator(-35, -35, 0));
    Sun->SetAtmosphereSunLight(true);
    Sun->SetIntensity(8.0f);
    Sun->SetLightColor(FLinearColor(1.0f, 0.94f, 0.82f));
    USkyLightComponent* Ambient = CreateDefaultSubobject<USkyLightComponent>(TEXT("Ambient"));
    Ambient->SetupAttachment(RootComponent);
    Ambient->SetMobility(EComponentMobility::Movable);
    Ambient->SetIntensity(1.0f);
    Ambient->SetRealTimeCaptureEnabled(true);
    UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("DockCamera"));
    Camera->SetupAttachment(RootComponent);
    const FVector Position(-1400, -1500, 850);
    Camera->SetRelativeLocation(Position);
    Camera->SetRelativeRotation((FVector(-100, -350, 80) - Position).Rotation());
    Camera->FieldOfView = 75.0f;

    ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane.Plane"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> SphereAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderAsset(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicAsset(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    Plane = PlaneAsset.Object;
    Cube = CubeAsset.Object;
    Sphere = SphereAsset.Object;
    Cylinder = CylinderAsset.Object;
    Material = BasicAsset.Object;
}

void AFiskOceanScene::BeginPlay()
{
    Super::BeginPlay();
    auto Part = [&](const TCHAR* Name, UStaticMesh* Mesh, FVector Position, FVector Scale, FLinearColor Color)
    {
        return FiskVisual::AddPart(this, RootComponent, Mesh, Material, Name, Position, Scale, Color);
    };
    UStaticMeshComponent* Water = FiskVisual::AddPart(this, RootComponent, Plane, Material, TEXT("Ocean"),
        FVector(10000, 0, -10), FVector(10000, 10000, 1), FLinearColor(0.01f, 0.23f, 0.34f), 0.15f);

    Water->SetCastShadow(false);
    Part(TEXT("Sand"), Sphere, FVector(-8000, 0, -70), FVector(140, 180, 2), FLinearColor(0.7f, 0.56f, 0.32f));
    Part(TEXT("IslandGreen"), Sphere, FVector(-12000, 0, 40), FVector(100, 140, 3), FLinearColor(0.14f, 0.29f, 0.06f));
    Part(TEXT("Pier"), Cube, FVector(-1000, 0, 110), FVector(12, 3.0, 0.35), FLinearColor(0.34f, 0.18f, 0.08f));
    for (int32 Index = 0; Index < 4; ++Index)
    {
        for (int32 Side = 0; Side < 2; ++Side)
        {
            Part(*FString::Printf(TEXT("Pile%d_%d"), Index, Side), Cylinder,
                FVector(-500 - Index * 300, Side == 0 ? -130 : 130, 45), FVector(0.25, 0.25, 2.5), FLinearColor(0.23f, 0.11f, 0.05f));
        }
    }
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const float X = -2500 - Index * 650;
        const float Y = 500 + (Index % 2) * 850;
        Part(*FString::Printf(TEXT("PalmTrunk%d"), Index), Cylinder, FVector(X, Y, 400), FVector(0.35, 0.35, 6), FLinearColor(0.28f, 0.16f, 0.08f));
        for (int32 Leaf = 0; Leaf < 5; ++Leaf)
        {
            const float Angle = Leaf * 2.0f * PI / 5.0f;
            UStaticMeshComponent* Frond = Part(*FString::Printf(TEXT("Palm%dLeaf%d"), Index, Leaf), Sphere,
                FVector(X + FMath::Cos(Angle) * 140, Y + FMath::Sin(Angle) * 140, 710), FVector(4.0, 0.7, 0.1), FLinearColor(0.12f, 0.31f, 0.06f));
            Frond->SetRelativeRotation(FRotator(-12, FMath::RadiansToDegrees(Angle), 0));
        }
    }
    Part(TEXT("FishingBuoy"), Sphere, FVector(20000, 450, 25), FVector(0.6, 0.6, 0.6), FLinearColor(1.0f, 0.38f, 0.04f));
    Part(TEXT("BuoyPole"), Cylinder, FVector(20000, 450, 75), FVector(0.04, 0.04, 1.2), FLinearColor(0.8f, 0.8f, 0.7f));
    for (int32 Index = 0; Index < 64; ++Index)
    {
        UStaticMeshComponent* Ripple = Part(*FString::Printf(TEXT("Ripple%d"), Index), Sphere,
            FVector::ZeroVector, FVector(4.0, 0.12, 0.02), FLinearColor(0.16f, 0.48f, 0.51f));
        Ripple->SetCastShadow(false);
        Ripples.Add(Ripple);
    }
}

void AFiskOceanScene::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TActorIterator<AFiskStarterBoat> Boats(GetWorld());
    const FVector Center = Boats ? Boats->GetActorLocation() : FVector::ZeroVector;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const double Time = Day ? Day->GetElapsedRealSeconds() : GetWorld()->GetTimeSeconds();
    for (int32 Index = 0; Index < Ripples.Num(); ++Index)
    {
        const double X = (Index % 8 - 4) * 900.0 + FMath::Sin(Index * 7.3) * 250.0;
        const double Y = (Index / 8 - 4) * 900.0 + FMath::Sin(Index * 3.1) * 300.0;
        const double Wave = FMath::Sin(Time * 1.1 + Index * 1.8);
        Ripples[Index]->SetRelativeLocation(FVector(Center.X + X, Center.Y + Y, -7 + Wave * 1.5));
        Ripples[Index]->SetRelativeScale3D(FVector(4.0 + Wave, 0.12, 0.02));
    }
}
