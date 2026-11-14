#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskBoatCommands.generated.h"
class AFiskBoardingController;

// Boarding, departure and return, the route (fishing ground, sea point, voyage plan), travel view
// votes and the captain role (#113 step 8). Moved from AFiskBoardingController with the same network
// calls; the component is replicated and owned by the player's controller. Replies to the client are
// shown by the controller's panels (ShowCaptainReply, ShowGroundReply).
UCLASS()
class UFiskBoatCommands final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskBoatCommands();
    UFUNCTION(Server,Reliable) void ServerBoardStarterBoat();
    UFUNCTION(Server,Reliable) void ServerStartBoatTrip(int32 ExpectedRevision);
    UFUNCTION(Server,Reliable) void ServerReturnToMarina(int32 Revision,int32 Day,int32 Leg);
    UFUNCTION(Server,Reliable) void ServerTravelVote(int32 Leg,bool Vote);
    UFUNCTION(Server,Reliable) void ServerSelectSeaPoint(int32 Revision,FVector_NetQuantize Point);
    UFUNCTION(Server,Reliable) void ServerSelectGround(int32 ExpectedRevision,int32 GroundId);
    UFUNCTION(Client,Reliable) void ClientGroundReply(const FString& Message);
    UFUNCTION(Server,Reliable) void ServerVoyageReplace(int32 Day,int32 Revision,const TArray<FVector>& Points);
    UFUNCTION(Server,Reliable) void ServerVoyageNext(int32 Day,int32 Revision,int32 BoatRevision);
    UFUNCTION(Server,Reliable) void ServerTransferCaptain(int32 ExpectedRevision,int32 Target);
    UFUNCTION(Client,Reliable) void ClientCaptainReply(bool bAccepted);
    // Hand steering (#176 step 2): the captain's throttle and rudder, -1..1, about ten times a second.
    // Unreliable; Serial must increase. The server treats half a second without input as released.
    UFUNCTION(Server,Unreliable) void ServerHelm(int32 Serial,float Throttle,float Rudder);
#if WITH_DEV_AUTOMATION_TESTS
    int32 GetRejectedCaptainRequests() const { return RejectedCaptainRequests; }
#endif
private:
    AFiskBoardingController& Controller() const;
    FString LastHelmMessage;double LastHelmMessageAt=-100;   // Server: one reply per message, not ten a second
#if WITH_DEV_AUTOMATION_TESTS
    int32 RejectedCaptainRequests=0;   // Server: read by the captain smoke test
#endif
};
