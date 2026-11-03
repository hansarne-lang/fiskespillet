#include "Boats/FiskStarterBoat.h"
#include "Components/SceneComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

namespace
{
    constexpr float TripDuration = 12.0f;
    const FVector FishingSpot(20000.0, 0.0, 0.0);
}

AFiskStarterBoat::AFiskStarterBoat()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("BoatRoot")));
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
}

void AFiskStarterBoat::OnRep_Crew()
{
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_STARTER_BOAT_CREW %d"), Crew.Num());
}

void AFiskStarterBoat::StartTrip(int32 PlayerId)
{
    if (!HasAuthority() || TripProgress != AtDock || PlayerId == INDEX_NONE || PlayerId != GetCaptainId())
    {
        return;
    }

    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Day || !Day->HasStarted() || Day->GetPhase(Day->GetElapsedSeconds()) != EFiskDayPhase::FishingAndReturn)
    {
        return;
    }

    TripProgress = 0;
    OnRep_TripProgress();
    SetActorTickEnabled(true);
    ForceNetUpdate();
}

void AFiskStarterBoat::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TravelSeconds = FMath::Min(TravelSeconds + DeltaSeconds, TripDuration);
    const float Fraction = TravelSeconds / TripDuration;
    SetActorLocation(FishingSpot * Fraction);
    TripProgress = FMath::FloorToInt(Fraction * AtFishingSpot);

    if (TripProgress == AtFishingSpot)
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
    else if (TripProgress == AtFishingSpot)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TRIP_ARRIVED crew=%d"), Crew.Num());
    }
}
