#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Fishing/FiskFishVisualState.h"
#include "FiskStarterBoat.generated.h"

USTRUCT()
struct FFiskRouteView
{
    GENERATED_BODY()
    UPROPERTY() int32 GroundId = 0;
    UPROPERTY() int32 Revision = 0;
    UPROPERTY() int32 CaptainId = INDEX_NONE;
    bool Authorizes(int32 Requester,int32 ExpectedRevision,const TArray<int32>& Crew) const;
    bool Transfer(int32 Requester,int32 ExpectedRevision,int32 Target,const TArray<int32>& Crew);
    bool RefreshCaptain(const TArray<int32>& Crew,int32 OwnerId,bool bPreferOwner);
};

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
    virtual void CalcCamera(float DeltaTime,struct FMinimalViewInfo& OutResult) override;
    void SetOwnerPlayer(int32 PlayerId);
    void ResetForDay();
    void Board(int32 PlayerId);
    void Leave(int32 PlayerId);
    void StartTrip(int32 PlayerId,int32 ExpectedRevision);
    virtual void Tick(float DeltaSeconds) override;

    static constexpr int32 TripDurationMs = 12000;
    static constexpr double MarinaRadiusCm = 500.0;
    static constexpr double FishingSpotDistanceCm = 20000.0;
    int32 GetTripProgress() const { return TripProgress; }
    int32 GetGroundId() const { return Route.GroundId; }
    int32 GetRouteRevision() const { return Route.Revision; }
    bool SelectGround(int32 PlayerId, int32 ExpectedRevision, int32 GroundId, FString& Message);
    int32 GetMarinaArrivalMs(int32 NowMs) const;
    static int32 CalculateCrossingMs(int32 ReturnStartMs, int32 GroundId = 0);

    bool IsAboard(int32 PlayerId) const { return Crew.Contains(PlayerId); }
    int32 GetCrewCount() const { return Crew.Num(); }

    int32 GetCaptainId() const;
    int32 GetCrewMemberId(int32 Seat) const { return Crew.IsValidIndex(Seat)?Crew[Seat]:INDEX_NONE; }
    bool CanCaptainCommand(int32 Requester,int32 ExpectedRevision) const { return Route.Authorizes(Requester,ExpectedRevision,Crew); }
    bool TransferCaptain(int32 Requester,int32 ExpectedRevision,int32 Target);
    int32 GetSeatIndex(int32 PlayerId) const { return Crew.IndexOfByKey(PlayerId); }
    uint8 GetRodActivity(int32 Seat) const { return Seat == 0 ? PortRodActivity : StarboardRodActivity; }

    void SetRodActivities(uint8 Port, uint8 Starboard);
    const FFiskFishVisualState& GetFishVisual(int32 Seat) const { return Seat == 0 ? PortFish : StarboardFish; }
    void SetFishVisuals(const FFiskFishVisualState& Port, const FFiskFishVisualState& Starboard);


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)

    int32 OwnerId = INDEX_NONE;
    UPROPERTY(ReplicatedUsing=OnRep_Route) FFiskRouteView Route;
    UFUNCTION() void OnRep_Route();
    UPROPERTY(Replicated) uint8 PortRodActivity = 0;

    UPROPERTY(Replicated) uint8 StarboardRodActivity = 0;
    UPROPERTY(Replicated) FFiskFishVisualState PortFish;
    UPROPERTY(Replicated) FFiskFishVisualState StarboardFish;



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

    void RefreshCaptain();
    bool bManualCaptain = false;
    bool bReportedDeparture = false;
    bool bReportedReturn = false;
};
