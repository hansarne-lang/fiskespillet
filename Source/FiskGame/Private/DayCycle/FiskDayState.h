#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FiskDayState.generated.h"

UCLASS()
class AFiskDayState final : public AGameStateBase
{
    GENERATED_BODY()

public:
    static constexpr int32 PreparationEnd = 2 * 60;
    static constexpr int32 SettlementStart = 17 * 60;
    static constexpr int32 DayEnd = 20 * 60;

    void StartDay();
    bool HasStarted() const { return StartedAt >= 0.0; }
    int32 GetElapsedSeconds() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_StartedAt)
    double StartedAt = -1.0;

    UFUNCTION()
    void OnRep_StartedAt();
};
