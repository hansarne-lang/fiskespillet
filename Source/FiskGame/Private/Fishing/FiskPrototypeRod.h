#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskPrototypeRod.generated.h"

UENUM()
enum class EFiskRodState : uint8
{
    Ready,
    Fishing,
    Bite
};

UCLASS()
class UFiskPrototypeRod final : public UActorComponent
{
    GENERATED_BODY()

public:
    UFiskPrototypeRod();
    void Cast();
    EFiskRodState GetState() const { return State; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_State)
    EFiskRodState State = EFiskRodState::Ready;

    UFUNCTION()
    void OnRep_State();

    void SetState(EFiskRodState NewState);
    double BiteAt = 0.0;
};
