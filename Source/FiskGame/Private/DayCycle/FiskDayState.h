#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FiskDayState.generated.h"

enum class EFiskDayPhase : uint8 { Preparation, FishingAndReturn, Settlement, Finished };

UCLASS()
class AFiskDayState final : public AGameStateBase
{
    GENERATED_BODY()
public:
    static constexpr int32 PreparationEnd = 120;
    static constexpr int32 SettlementStart = 1020;
    static constexpr int32 DayEnd = 1200;

    AFiskDayState();
    void StartDay();
    bool HasStarted() const { return ElapsedMilliseconds >= 0; }
    int32 GetElapsedMilliseconds() const;
    int32 GetElapsedSeconds() const { return GetElapsedMilliseconds() / 1000; }
    double GetElapsedRealSeconds() const { return GetElapsedMilliseconds() / 1000.0; }
    static int32 CalculateElapsedSeconds(double ServerTime, double StartTime);
    static EFiskDayPhase GetPhase(int32 ElapsedSeconds);
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_Clock)
    int32 ElapsedMilliseconds = -1;
    UFUNCTION()
    void OnRep_Clock();
    double StartedMonotonic = 0.0;
    bool bReportedStart = false;
    int32 LastMinute = -1;
};
