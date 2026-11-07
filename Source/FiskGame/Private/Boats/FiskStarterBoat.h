#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Fishing/FiskFishVisualState.h"
#include "FiskBoatLayout.h"
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
    bool IsTravelling() const { return TripProgress>=0 && TripProgress!=100 && TripProgress!=200; }
    bool UsesTravelCard() const;
    bool HasTravelVote(int32 Player) const { return TravelVotes.Contains(Player); }
    int32 GetTravelVoteCount() const { return TravelVotes.Num(); }
    bool VoteTravel(int32 Player,int32 ExpectedLeg,bool Vote);
    bool Relocate(int32 Player,int32 Revision,FVector Point,FString& Message);
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
    int32 ReturnStartedMs = -1;
    UPROPERTY(Replicated)
    int32 MarinaArrivalMs = -1;

    void RefreshCaptain();
    bool bManualCaptain = false;
    bool bReportedDeparture = false;
    bool bReportedReturn = false;
};
