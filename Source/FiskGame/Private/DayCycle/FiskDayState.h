#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "FiskDayState.generated.h"

enum class EFiskDayPhase : uint8 { Preparation, FishingAndReturn, Settlement, Finished };

USTRUCT()
struct FFiskDayClock
{
    GENERATED_BODY()
    UPROPERTY() int32 Number = 0;
    UPROPERTY() int32 ElapsedMs = -1;
};

UCLASS()
class AFiskDayState final : public AGameStateBase
{
    GENERATED_BODY()
public:
    static constexpr int32 PreparationEnd = 120;
    static constexpr int32 SettlementStart = 1020;
    static constexpr int32 DayEnd = 1200;
    // Test-only acceleration (-FiskClockScale=N). Always 1 in Shipping and without the flag.
    static constexpr int32 MaxTestClockScale = 20;
    static int32 ParseTestClockScale(const TCHAR* CommandLine);
    static int32 ScaledElapsedMilliseconds(double WallSeconds, int32 Scale);

    AFiskDayState();
    void StartDay();
    bool AdvanceDay();
    int32 GetDayNumber() const { return Clock.Number; }
    bool HasStarted() const { return Clock.ElapsedMs >= 0; }
    int32 GetElapsedMilliseconds() const;
    int32 GetElapsedSeconds() const { return GetElapsedMilliseconds() / 1000; }
    double GetElapsedRealSeconds() const { return GetElapsedMilliseconds() / 1000.0; }
    static int32 CalculateElapsedSeconds(double ServerTime, double StartTime);
    static EFiskDayPhase GetPhase(int32 ElapsedSeconds);
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(ReplicatedUsing = OnRep_Clock)
    FFiskDayClock Clock;
    UFUNCTION()
    void OnRep_Clock();
    double StartedMonotonic = 0.0;
    int32 ClockScale = 1;
    int32 ReportedDay = 0;
    int32 LastMinute = -1;
};
