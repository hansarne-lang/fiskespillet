#include "Boats/FiskStarterBoat.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
    const FVector FishingSpot(AFiskStarterBoat::FishingSpotDistanceCm, 0.0, 0.0);
}

AFiskStarterBoat::AFiskStarterBoat()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot")));

    UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("BoatCamera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeLocation(FVector(0.0, 0.0, 1800.0));
    Camera->SetRelativeRotation(FRotator(-90.0, 0.0, 0.0));
    SetReplicateMovement(true);

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void AFiskStarterBoat::SetOwnerPlayer(int32 PlayerId)
{
    if (HasAuthority() && OwnerId == INDEX_NONE)
    {
        OwnerId = PlayerId;
    }
}

void AFiskStarterBoat::Board(int32 PlayerId)
{
    if (!HasAuthority() || TripProgress != AtDock || PlayerId == INDEX_NONE || IsAboard(PlayerId) || Crew.Num() >= Capacity)
    {
        return;
    }

    Crew.Add(PlayerId);
    OnRep_Crew();
    ForceNetUpdate();
}

void AFiskStarterBoat::Leave(int32 PlayerId)
{
    if (HasAuthority() && Crew.Remove(PlayerId) > 0)
    {
        OnRep_Crew();
        ForceNetUpdate();
    }
}

int32 AFiskStarterBoat::GetCaptainId() const
{
    if (IsAboard(OwnerId))
    {
        return OwnerId;
    }
    return Crew.IsEmpty() ? INDEX_NONE : Crew[0];
}

void AFiskStarterBoat::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskStarterBoat, OwnerId);
    DOREPLIFETIME(AFiskStarterBoat, Crew);
    DOREPLIFETIME(AFiskStarterBoat, TripProgress);
    DOREPLIFETIME(AFiskStarterBoat, MarinaArrivalMs);
}

void AFiskStarterBoat::OnRep_Crew()
{
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_STARTER_BOAT_CREW %d"), Crew.Num());
}

void AFiskStarterBoat::StartTrip(int32 PlayerId)
{
    if (!HasAuthority() || PlayerId == INDEX_NONE || PlayerId != GetCaptainId())
    {
        return;
    }


    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Day || !Day->HasStarted()) { return; }
    if (TripProgress == AtFishingSpot)
    {
        LegStartedMs = Day->GetElapsedMilliseconds();
        ReturnStartedMs = LegStartedMs;

        TripProgress = AtFishingSpot + 1;
        OnRep_TripProgress();
        SetActorTickEnabled(true);
        ForceNetUpdate();
        return;
    }
    if (TripProgress != AtDock)
    {
        return;
    }

    if (Day->GetPhase(Day->GetElapsedSeconds()) != EFiskDayPhase::FishingAndReturn)
    {
        return;
    }

    LegStartedMs = Day->GetElapsedMilliseconds();
    TripProgress = 0;
    OnRep_TripProgress();
    SetActorTickEnabled(true);
    ForceNetUpdate();
}

void AFiskStarterBoat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!HasAuthority() || !Day) { return; }
    const int32 NowMs = Day->GetElapsedMilliseconds();
    const double Fraction = FMath::Clamp((NowMs - LegStartedMs) / static_cast<double>(TripDurationMs), 0.0, 1.0);
    if (MarinaArrivalMs < 0)
    {
        MarinaArrivalMs = GetMarinaArrivalMs(NowMs);
        if (MarinaArrivalMs >= 0)
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MARINA_CROSSED ms=%d"), MarinaArrivalMs);
        }
    }

    const bool bReturning = TripProgress > AtFishingSpot;
    SetActorLocation(FishingSpot * (bReturning ? 1.0f - Fraction : Fraction));
    const int32 Percent = FMath::FloorToInt(Fraction * AtFishingSpot);
    TripProgress = bReturning ? AtFishingSpot + FMath::Max(1, Percent) : Percent;

    if (TripProgress == AtFishingSpot || TripProgress == AtMarina)
    {
        SetActorTickEnabled(false);
        OnRep_TripProgress();
        ForceNetUpdate();
    }
}

void AFiskStarterBoat::OnRep_TripProgress()
{
    if (TripProgress >= 0 && TripProgress < AtFishingSpot && !bReportedDeparture)
    {
        bReportedDeparture = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TRIP_UNDERWAY crew=%d"), Crew.Num());
    }
    else if (TripProgress > AtFishingSpot && TripProgress < AtMarina && !bReportedReturn)
    {
        bReportedReturn = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RETURN_UNDERWAY crew=%d"), Crew.Num());
    }
    else if (TripProgress == AtMarina)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MARINA_REACHED crew=%d"), Crew.Num());
    }
    else if (TripProgress == AtFishingSpot)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TRIP_ARRIVED crew=%d"), Crew.Num());
    }
}


int32 AFiskStarterBoat::CalculateCrossingMs(int32 ReturnStartMs)
{
    // The fixed route is linear, so crossing is independent of frame ordering.
    const double Fraction = 1.0 - MarinaRadiusCm / FishingSpotDistanceCm;
    return ReturnStartMs + FMath::CeilToInt(TripDurationMs * Fraction);
}

int32 AFiskStarterBoat::GetMarinaArrivalMs(int32 NowMs) const
{
    if (MarinaArrivalMs >= 0) { return MarinaArrivalMs; }
    if (ReturnStartedMs < 0) { return -1; }
    const int32 CrossingMs = CalculateCrossingMs(ReturnStartedMs);
    return NowMs >= CrossingMs ? CrossingMs : -1;
}
