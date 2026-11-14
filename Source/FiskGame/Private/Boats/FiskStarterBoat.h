#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Fishing/FiskFishVisualState.h"
#include "FiskBoatLayout.h"
#include "FiskHelm.h"
#include "Boats/FiskCargoModel.h"
#include "FiskStarterBoat.generated.h"

USTRUCT()
struct FFiskRouteView
{
    GENERATED_BODY()
    UPROPERTY() int32 GroundId = 0;
    UPROPERTY() int32 Revision = 0;
    UPROPERTY() int32 CaptainId = INDEX_NONE;
    UPROPERTY() FVector Destination=FVector(20000,0,0);
    UPROPERTY() int32 BaseTravelMs=12000;
    bool Authorizes(int32 Requester,int32 ExpectedRevision,const TArray<int32>& Crew) const;
    bool Transfer(int32 Requester,int32 ExpectedRevision,int32 Target,const TArray<int32>& Crew);
    bool RefreshCaptain(const TArray<int32>& Crew,int32 OwnerId,bool bPreferOwner);
};

class AFiskVoyagePlan;
class AFiskExpeditionState;
UCLASS()
class AFiskStarterBoat final : public AInfo
{
    GENERATED_BODY()

public:
    static constexpr int32 Capacity = 2;
    static constexpr int32 AtDock = -1;
    static constexpr int32 AtFishingSpot = 100;
    static constexpr int32 AtMarina = 200;
    static constexpr int32 UnderHelm = FiskHelm::TripUnderHelm;   // Moving under the captain's hand (#176)

    AFiskStarterBoat();
    void InitializeFleet(int32 Index,AFiskExpeditionState* Expedition,AFiskVoyagePlan* Voyage);
    int32 GetFleetIndex() const { return FleetIndex; }
    AFiskVoyagePlan* GetVoyage() const { return Voyage; }
    AFiskExpeditionState* GetExpedition() const { return Expedition; }
    FVector DockPosition() const { return FVector(0,FleetIndex*1250,0); }
    const FFiskCargoModel& GetCargo() const { return Cargo; }
    const FFiskCargoView& GetCargoView() const { return CargoView; }
    void AcceptCargo(const FFiskCargoModel& Candidate);
    void RevealCargo();
    int32 GetReturnTravelMs() const { return FFiskCargoModel::ReturnDuration(GetTravelMs(),CargoView.Mode==static_cast<uint8>(EFiskCargoMode::Towing)); }
    int32 GetKind() const { return Kind; }
    int32 GetCapacity() const { return FiskBoatLayout::Seats(Kind); }
    int32 GetHolderCount() const { return FiskBoatLayout::Holders(Kind); }
    int32 GetRodsFor(int32 Player) const { return FiskBoatLayout::Rods(Kind,Crew.Num(),GetSeatIndex(Player)); }
    int32 GetHolderFor(int32 Player,int32 Slot) const { const int32 Seat=GetSeatIndex(Player);return Seat<0 || Slot<0 || Slot>=GetRodsFor(Player)?-1:FiskBoatLayout::FirstHolder(Kind,Crew.Num(),Seat)+Slot; }
    int32 GetOwnerId() const { return OwnerId; }
    int32 GetTravelMs() const { return Kind==1?FMath::Max(1,Route.BaseTravelMs*4/5):Route.BaseTravelMs; }
    int32 GetLegTravelMs() const { return LegTravelMs; }
    int32 GetLegSerial() const { return LegSerial; }
    int32 GetLegStartedMs() const { return LegStartedMs; }   // Server day time when the current leg began (replicated)
    bool IsTravelling() const { return TripProgress>=0 && TripProgress!=AtFishingSpot && TripProgress!=AtMarina; }
    bool UsesTravelCard() const;
    bool HasTravelVote(int32 Player) const { return TravelVotes.Contains(Player); }
    int32 GetTravelVoteCount() const { return TravelVotes.Num(); }
    bool VoteTravel(int32 Player,int32 ExpectedLeg,bool Vote);
    bool Relocate(int32 Player,int32 Revision,FVector Point,FString& Message);
    // Hand steering (#176 step 2). The captain's throttle and rudder (-1..1) move the boat on the server.
    // Serial must increase (unreliable input). Stopping with the throttle released at sea makes the spot a
    // fishing place; steering into the marina ends the trip. The chart's autopilot and R still work.
    bool SteerHelm(int32 Player,int32 Serial,float Throttle,float Rudder,FString& Message);
    bool IsUnderHelm() const { return bHelm; }
    float GetHelmSpeed() const { return HelmSpeed; }   // cm/s, negative when reversing (replicated)
    TArray<FVector> GetActivePath() const;
    FVector PositionAt(int32 NowMs) const;
    FVector GetDestination() const { return Route.Destination; }
    bool ChooseKind(int32 Requester,int32 Revision,int32 NewKind);
    bool SelectPoint(int32 Requester,int32 Revision,FVector Point,FString& Message);
    int32 ReturnMarginMs(int32 NowMs) const;
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
    uint8 GetRodActivity(int32 Seat) const { return RodActivities.IsValidIndex(Seat)?RodActivities[Seat]:0; }

    void SetRodActivities(uint8 Port, uint8 Starboard);
    const FFiskFishVisualState& GetFishVisual(int32 Seat) const { static const FFiskFishVisualState Empty;return FishVisuals.IsValidIndex(Seat)?FishVisuals[Seat]:Empty; }
    void SetFishVisuals(const FFiskFishVisualState& Port, const FFiskFishVisualState& Starboard);
    void SetHolderVisuals(const TArray<uint8>& Activities,const TArray<FFiskFishVisualState>& Fish);


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated) int32 FleetIndex=0;
    UPROPERTY(Replicated) TObjectPtr<AFiskVoyagePlan> Voyage;
    UPROPERTY(Replicated) TObjectPtr<AFiskExpeditionState> Expedition;
    FFiskCargoModel Cargo;
    UPROPERTY(ReplicatedUsing=OnRep_CargoView) FFiskCargoView CargoView;
    UFUNCTION() void OnRep_CargoView();
    bool bCargoReturnPending=false;
    bool ReturnHome(int32 NowMs);
    void ForceCargoReturn(int32 NowMs);
    UPROPERTY(Replicated) int32 Kind=0;
    UPROPERTY(Replicated) TArray<uint8> RodActivities;
    UPROPERTY(Replicated) TArray<FFiskFishVisualState> FishVisuals;
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


    UPROPERTY(Replicated) int32 LegStartedMs = -1;
    UPROPERTY(Replicated) int32 LegTravelMs=12000;
    UPROPERTY(Replicated) int32 LegSerial=0;
    UPROPERTY(Replicated) FVector LegOrigin=FVector::ZeroVector;
    UPROPERTY(Replicated) FVector LegDestination=FVector(20000,0,0);
    UPROPERTY(Replicated) TArray<int32> TravelVotes;
    void BeginLeg(FVector Origin,FVector Destination,int32 Duration,int32 NowMs,bool Returning);
    UPROPERTY(Replicated) bool bHelm=false;
    UPROPERTY(Replicated) float HelmSpeed=0;
    FiskHelm::FState Helm;
    double HelmThrottle=0,HelmRudder=0,HelmInputAt=-1,HelmBlockedUntil=0;
    int32 HelmSerial=0,HelmLastMs=-1;
    bool bHelmLeftMarina=false;
    void TakeHelm(FVector Position,double Speed,int32 NowMs);
    void TickHelm(int32 NowMs);
    bool CollectRods(TArray<class UFiskPrototypeRod*>& Waiting) const;   // false when a rod has a fish on
    int32 ReturnStartedMs = -1;
    UPROPERTY(Replicated)
    int32 MarinaArrivalMs = -1;

    void RefreshCaptain();
    bool bManualCaptain = false;
    bool bReportedDeparture = false;
    bool bReportedReturn = false;
};
