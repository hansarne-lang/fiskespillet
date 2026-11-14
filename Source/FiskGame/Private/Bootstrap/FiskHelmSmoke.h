#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskHelmSmoke.generated.h"

// Test-only (-FiskHelmSmoke, Tools/Testing/Run-HelmRelease.ps1, #176 step 3). Added to the host's own
// controller by the game mode. One captain leaves the dock under the helm, turns, stops to make a fishing
// spot, moves on again and lets R's autopilot take the boat home. Uses the same keys as a player.
UCLASS()
class UFiskHelmSmoke final : public UActorComponent
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
private:
    FString SmokeDir;FTimerHandle SmokeTimer;int32 SmokeStage=0;double SmokeAt=0;FVector SmokeStop=FVector::ZeroVector;float SmokeYaw=0;
    void RunSmoke();
};
