#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskMarinaTypes.h"
#include "FiskMarinaSync.generated.h"
class AFiskBoardingController;

// Network calls for the boathouse, beach bar and harbour presence (#113 step 3). Moved here from
// AFiskBoardingController with the same names and parameters; the component is replicated and owned
// by the player's controller, so the calls use the same connection as before.
UCLASS()
class UFiskMarinaSync final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskMarinaSync();
    UFUNCTION(Server,Reliable) void ServerMarinaIdentity(FGuid Profile,FFiskMarinaIdentity Identity);
    UFUNCTION(Client,Reliable) void ClientMarinaIdentity(bool Accepted,FFiskMarinaIdentity Identity,const FString& Reason);
    UFUNCTION(Server,Reliable) void ServerMarinaRecords(FGuid Profile,const TArray<FFiskMarinaTrophy>& Records);
    UFUNCTION(Server,Reliable) void ServerMarinaSay(int32 Code);
    UFUNCTION(Server,Reliable) void ServerHarbourPlace(FGuid Profile,int32 Place);
private:
    AFiskBoardingController& Controller() const;
    double LastMarinaMessage=-10;   // Server: 2 s between bar messages
};
