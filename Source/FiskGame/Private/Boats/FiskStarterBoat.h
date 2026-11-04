#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "FiskStarterBoat.generated.h"

UCLASS()
class AFiskStarterBoat final : public AInfo
{
    GENERATED_BODY()

public:
    static constexpr int32 Capacity = 2;
    static constexpr int32 AtDock = -1;
    static constexpr int32 AtFishingSpot = 100;
    static constexpr int32 AtMarina = 200;

    AFiskStarterBoat();
    void SetOwnerPlayer(int32 PlayerId);
    void Board(int32 PlayerId);
    void Leave(int32 PlayerId);
    void StartTrip(int32 PlayerId);
    virtual void Tick(float DeltaSeconds) override;

    static constexpr int32 TripDurationMs = 12000;
    static constexpr double MarinaRadiusCm = 500.0;
    static constexpr double FishingSpotDistanceCm = 20000.0;
    int32 GetTripProgress() const { return TripProgress; }
    int32 GetMarinaArrivalMs(int32 NowMs) const;
    static int32 CalculateCrossingMs(int32 ReturnStartMs);

    bool IsAboard(int32 PlayerId) const { return Crew.Contains(PlayerId); }
    int32 GetCrewCount() const { return Crew.Num(); }

    int32 GetCaptainId() const;
    int32 GetSeatIndex(int32 PlayerId) const { return Crew.IndexOfByKey(PlayerId); }
    uint8 GetRodActivity(int32 Seat) const { return Seat == 0 ? PortRodActivity : StarboardRodActivity; }
    void SetRodActivities(uint8 Port, uint8 Starboard);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)

    int32 OwnerId = INDEX_NONE;
    UPROPERTY(Replicated) uint8 PortRodActivity = 0;
    UPROPERTY(Replicated) uint8 StarboardRodActivity = 0;


    UPROPERTY(ReplicatedUsing = OnRep_Crew)
    TArray<int32> Crew;

    UFUNCTION()
    void OnRep_Crew();

    UPROPERTY(ReplicatedUsing = OnRep_TripProgress)
    int32 TripProgress = AtDock;

    UFUNCTION()
    void OnRep_TripProgress();


    int32 LegStartedMs = -1;
    int32 ReturnStartedMs = -1;
    UPROPERTY(Replicated)
    int32 MarinaArrivalMs = -1;

    bool bReportedDeparture = false;
    bool bReportedReturn = false;
};
