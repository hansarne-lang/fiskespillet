#include "Boats/FiskMarinaZone.h"
#include "Boats/FiskStarterBoat.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

AFiskMarinaZone::AFiskMarinaZone()
{
    bReplicates = true;
    bAlwaysRelevant = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void AFiskMarinaZone::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    DrawDebugCircle(GetWorld(), FVector(0, 0, 2), AFiskStarterBoat::MarinaRadiusCm, 64,
        FColor::Green, false, 0.2f, 0, 8.0f, FVector::ForwardVector, FVector::RightVector, false);
}
