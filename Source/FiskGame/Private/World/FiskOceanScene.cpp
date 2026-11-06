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
#include "FiskRouteGeometry.h"
#include "Expedition/FiskExpeditionState.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "UI/FiskGameInstance.h"
#include "World/FiskOceanAtlas.h"
#include "Components/TextRenderComponent.h"

AFiskOceanScene::AFiskOceanScene()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f / 20.0f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("OceanRoot")));
    Fog=CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("WeatherHaze"));Fog->SetupAttachment(RootComponent);Fog->SetFogDensity(.002f);
    USkyAtmosphereComponent* Sky = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Sky"));
    Sky->SetupAttachment(RootComponent);
    UDirectionalLightComponent* Sun = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
    Daylight=Sun;
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
        FVector(10000, 0, -10), FVector(10000, 10000, 1), FLinearColor(0.005f, 0.07f, 0.16f), 0.15f);

    Water->SetCastShadow(false);
    Part(TEXT("ShallowWater"),Sphere,FVector(18000,0,-13),FVector(700,950,.03),FLinearColor(.015f,.32f,.38f))->SetCastShadow(false);
    for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I)
    {
        const FVector P=FiskRouteGeometry::Island(I);const double R=FiskRouteGeometry::Radius(I);
        Part(*FString::Printf(TEXT("SeaIslandSand%d"),I),Sphere,P+FVector(0,0,-40),FVector(R/50,R/50,5),FLinearColor(.68f,.55f,.32f));
        Part(*FString::Printf(TEXT("SeaIslandGreen%d"),I),Sphere,P+FVector(0,0,150),FVector(R/65,R/65,I==2?3:14),FLinearColor(.08f,.23f,.055f));
        if(I<2) { Part(*FString::Printf(TEXT("SeaIslandHill%d"),I),Sphere,P+FVector(-R/4,0,650),FVector(R/110,R/120,20),FLinearColor(.11f,.22f,.08f)); }
    }
    // Root silhouettes stay inside the navigation exclusion around the mangrove island.
    for(int32 I=0;I<12;++I)
    {
        const double Angle=I*2*PI/12,Rad=2200+(I%3)*500;const FVector P=FiskRouteGeometry::Island(2)+FVector(FMath::Cos(Angle)*Rad,FMath::Sin(Angle)*Rad,0);
        Part(*FString::Printf(TEXT("MangroveTrunk%d"),I),Cylinder,P+FVector(0,0,420),FVector(.65,.65,8),FLinearColor(.24f,.18f,.09f));
        Part(*FString::Printf(TEXT("MangroveCrown%d"),I),Sphere,P+FVector(0,0,900),FVector(11,10,5),FLinearColor(.04f,.22f,.10f));
        for(int32 J=0;J<4;++J)
        {
            const double A=J*PI/2+Angle;const FVector Base=P+FVector(FMath::Cos(A)*380,FMath::Sin(A)*380,25),Top=P+FVector(0,0,380),Delta=Top-Base;
            auto* Root=Part(*FString::Printf(TEXT("MangroveRoot%d_%d"),I,J),Cylinder,(Base+Top)/2,FVector(.25,.25,Delta.Size()/100),FLinearColor(.2f,.14f,.07f));
            Root->SetRelativeRotation(FRotationMatrix::MakeFromZ(Delta).Rotator());
        }
    }
    Part(TEXT("MangroveWater"),Sphere,FiskRouteGeometry::Island(2)+FVector(0,0,-16),FVector(130,130,.025),FLinearColor(.025f,.20f,.13f))->SetCastShadow(false);
    for(int32 I=0;I<4;++I)
    {
        const FVector P(-2800-I*1600,-900,270);
        Part(*FString::Printf(TEXT("MarinaHouse%d"),I),Cube,P,FVector(10,7,4.5),FLinearColor(.58f,.68f,.6f));
        Part(*FString::Printf(TEXT("MarinaRoof%d"),I),Cube,P+FVector(0,0,260),FVector(11,8,.25),FLinearColor(.14f,.25f,.31f));
        Part(*FString::Printf(TEXT("HouseWindow%d"),I),Cube,P+FVector(0,-356,45),FVector(5,.06,1.3),FLinearColor(.02f,.12f,.17f));
        Part(*FString::Printf(TEXT("PierWing%d"),I),Cube,FVector(-1000-I*450,-480,110),FVector(.35,7,.25),FLinearColor(.34f,.18f,.08f));
    }
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
    for(int32 I=0;I<12;++I)
    {
        UStaticMeshComponent* Bird=Part(*FString::Printf(TEXT("Seabird%d"),I),Sphere,FVector::ZeroVector,FVector(.5,1.8,.06),FLinearColor(.84f,.87f,.82f));
        Bird->SetCastShadow(false);Birds.Add(Bird);
    }
    for(int32 I=0;I<8;++I)
    {
        UStaticMeshComponent* Fish=Part(*FString::Printf(TEXT("FlyingFish%d"),I),Sphere,FVector::ZeroVector,FVector(.6,.08,.1),FLinearColor(.34f,.6f,.68f));
        Fish->SetCastShadow(false);Flyers.Add(Fish);
    }
    for(int32 I=0;I<96;++I)
    {
        auto* Drop=Part(*FString::Printf(TEXT("Rain%d"),I),Cylinder,FVector::ZeroVector,FVector(.012,.012,1.7),FLinearColor(.36f,.57f,.67f));
        Drop->SetCastShadow(false);Drop->SetVisibility(false);Rain.Add(Drop);
    }
    for(int32 I=0;I<FiskAtlas::Count;++I)
    {
        const auto& Mark=*FiskAtlas::Find(I);const FVector P=Mark.Point+FVector(0,400,0);
        Part(*FString::Printf(TEXT("AtlasBuoy%d"),I),Sphere,P+FVector(0,0,30),FVector(.8,.8,.8),FLinearColor(1,.52f,.08f));
        Part(*FString::Printf(TEXT("AtlasPole%d"),I),Cylinder,P+FVector(0,0,135),FVector(.07,.07,2),FLinearColor(.85f,.85f,.72f));
        auto* Label=NewObject<UTextRenderComponent>(this);AddInstanceComponent(Label);Label->SetupAttachment(RootComponent);Label->SetRelativeLocation(P+FVector(0,0,265));
        Label->SetText(FText::FromString(FString::Printf(TEXT("%d  %s"),I+1,Mark.Name)));Label->SetWorldSize(35);Label->SetHorizontalAlignment(EHTA_Center);Label->SetTextRenderColor(FColor(255,218,140));Label->RegisterComponent();
    }
    Part(TEXT("LighthouseBase"),Cylinder,FVector(-800,1800,350),FVector(2.8,2.8,6),FLinearColor(.9f,.85f,.66f));
    Part(TEXT("LighthouseRoof"),Sphere,FVector(-800,1800,680),FVector(3.2,3.2,.8),FLinearColor(.8f,.18f,.08f));
    Part(TEXT("LighthouseLantern"),Cylinder,FVector(-800,1800,605),FVector(1.9,1.9,1.0),FLinearColor(1,.8f,.25f));
    for(int32 I=0;I<18;++I) { auto* W=Part(*FString::Printf(TEXT("Wake%d"),I),Sphere,FVector::ZeroVector,FVector(2,.1,.02),FLinearColor(.55f,.76f,.77f));W->SetCastShadow(false);Wake.Add(W); }
    for(int32 I=0;I<3;++I)
    {
        auto* Hull=Part(*FString::Printf(TEXT("DistantBoat%d"),I),Cube,FVector::ZeroVector,FVector(7,2,1),FLinearColor(.46f,.6f,.64f));Hull->SetCastShadow(false);Traffic.Add(Hull);
        auto* Cabin=Part(*FString::Printf(TEXT("DistantCabin%d"),I),Cube,FVector::ZeroVector,FVector(2,1.6,1.3),FLinearColor(.8f,.82f,.75f));Cabin->SetCastShadow(false);Traffic.Add(Cabin);
    }
}

void AFiskOceanScene::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TActorIterator<AFiskStarterBoat> Boats(GetWorld());
    const FVector Center = Boats ? Boats->GetActorLocation() : FVector::ZeroVector;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const double Time = Day ? Day->GetElapsedRealSeconds() : GetWorld()->GetTimeSeconds();
    const bool Moving=Boats && Boats->IsTravelling();
    for(int32 I=0;I<Wake.Num();++I)
    {
        Wake[I]->SetVisibility(Moving);if(!Moving) { continue; }const int32 N=I/2;const double Side=I%2?1:-1;
        const FVector Local(-500-N*170,Side*(130+N*48),-3+FMath::Sin(Time*3+I));Wake[I]->SetRelativeLocation(Boats->GetActorTransform().TransformPosition(Local));
        Wake[I]->SetRelativeRotation(FRotator(0,Boats->GetActorRotation().Yaw+Side*15,0));Wake[I]->SetRelativeScale3D(FVector(2.2+N*.3,.1,.02));
    }
    for(int32 I=0;I<3;++I)
    {
        const FVector P(15000+FMath::Fmod(Time*70+I*27000,120000.0),-95000+I*15000,70);
        Traffic[I*2]->SetRelativeLocation(P);Traffic[I*2+1]->SetRelativeLocation(P+FVector(0,0,95));
    }
    TActorIterator<AFiskExpeditionState> E(GetWorld());const int32 Weather=E?E->GetPlan().Weather:0;
    const auto* GI=Cast<UFiskGameInstance>(GetWorld()->GetGameInstance());const bool Low=GI && GI->FrameRate==30;
    if(ReportedWeather!=Weather)
    {
        ReportedWeather=Weather;Fog->SetFogDensity(Weather==2?.025f:Weather==1?.008f:.002f);
        Fog->SetFogInscatteringColor(Weather==2?FLinearColor(.23f,.31f,.37f):FLinearColor(.55f,.69f,.71f));
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_WEATHER_PRESENTED day=%d weather=%d"),E?E->GetPlan().DayNumber:0,Weather);
    }
    for(int32 I=0;I<Rain.Num();++I)
    {
        const bool Show=Weather==2 && (!Low || I<48);Rain[I]->SetVisibility(Show);if(!Show) { continue; }
        const double Phase=FMath::Fmod(Time*1.4+I*.618,1.0);
        Rain[I]->SetRelativeLocation(Center+FVector(FMath::Sin(I*7.31)*2400+Phase*380,FMath::Cos(I*3.27)*2400,1900-Phase*2100));
        Rain[I]->SetRelativeRotation(FRotator(12,0,0));
    }
    if(Daylight)
    {
        const double Progress=Day && Day->HasStarted()?FMath::Clamp(Time/1200.0,0.0,1.0):.2;
        const float Height=FMath::Sin(Progress*PI);
        Daylight->SetRelativeRotation(FRotator(-15-45*Height,-70+130*Progress,0));
        Daylight->SetIntensity((5.f+3.f*Height)*(Weather==2?.36f:Weather==1?.8f:1.f));
        Daylight->SetLightColor(Weather==2?FLinearColor(.63f,.74f,.85f):FMath::Lerp(FLinearColor(1,.73f,.47f),FLinearColor(1,.96f,.87f),Height));
    }
    for(int32 I=0;I<Birds.Num();++I)
    {
        const double A=Time*.12+I*.52,R=1800+I*140;Birds[I]->SetRelativeLocation(Center+FVector(FMath::Cos(A)*R,FMath::Sin(A)*R,700+I*55));
        Birds[I]->SetRelativeRotation(FRotator(0,FMath::RadiansToDegrees(A)+90,FMath::Sin(Time*3+I)*12));
    }
    for(int32 I=0;I<Flyers.Num();++I)
    {
        const double P=FMath::Fmod(Time*.18+I*.13,1.0);Flyers[I]->SetVisibility(P<.65);
        Flyers[I]->SetRelativeLocation(Center+FVector(1800-P*2400,600+I*220,FMath::Sin(FMath::Min(P/.65,1.0)*PI)*150));
        Flyers[I]->SetRelativeRotation(FRotator(FMath::Cos(P/.65*PI)*18,180,0));
    }
    for (int32 Index = 0; Index < Ripples.Num(); ++Index)
    {
        const double X = (Index % 8 - 4) * 900.0 + FMath::Sin(Index * 7.3) * 250.0;
        const double Y = (Index / 8 - 4) * 900.0 + FMath::Sin(Index * 3.1) * 300.0;
        const double Wave = FMath::Sin(Time * (Weather==0?1.1:Weather==1?1.7:2.2) + Index * 1.8);
        Ripples[Index]->SetRelativeLocation(FVector(Center.X + X, Center.Y + Y, -7 + Wave * (Weather==0?1.5:Weather==1?4.5:7.0)));
        Ripples[Index]->SetRelativeScale3D(FVector(4.0 + Wave*(Weather+1), 0.12+Weather*.08, 0.02));
    }
}
