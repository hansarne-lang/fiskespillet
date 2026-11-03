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
    int32 GetTripProgress() const { return TripProgress; }
    bool IsAboard(int32 PlayerId) const { return Crew.Contains(PlayerId); }
    int32 GetCrewCount() const { return Crew.Num(); }
    int32 GetCaptainId() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)
    int32 OwnerId = INDEX_NONE;

    UPROPERTY(ReplicatedUsing = OnRep_Crew)
    TArray<int32> Crew;

    UFUNCTION()
    void OnRep_Crew();

    UPROPERTY(ReplicatedUsing = OnRep_TripProgress)
    int32 TripProgress = AtDock;

    UFUNCTION()
    void OnRep_TripProgress();

    float TravelSeconds = 0.0f;
    bool bReportedDeparture = false;
    bool bReportedReturn = false;
};
