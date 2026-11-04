#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskPrototypeRod.generated.h"

UENUM()
enum class EFiskRodState : uint8
{
    Ready,
    Fishing,
    InitialRun,
    Fighting,
    ReadyToLand,
    Landed
};

UCLASS()
class UFiskPrototypeRod final : public UActorComponent
{
    GENERATED_BODY()

public:
    UFiskPrototypeRod();
    void Cast();
    void Lift();
    void Lower();
    void Reel();
    void Land();
    int32 GetLineOut() const { return LineOut; }
    int32 GetLandedAtMs() const { return LandedAtMs; }
    EFiskRodState GetState() const { return State; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_State)
    EFiskRodState State = EFiskRodState::Ready;

    UFUNCTION()
    void OnRep_State();

    void SetState(EFiskRodState NewState);
    UPROPERTY(Replicated)
    int32 LineOut = 0;

    double NextTransitionAt = 0.0;
    FGuid CatchId;
    int32 LandedAtMs = -1;
    bool bRodRaised = false;
    bool bCanReel = false;
};
