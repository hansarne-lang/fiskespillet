#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskExpeditionCommands.generated.h"
class AFiskBoardingController;

// The day's expedition at the pier: the captain's goal, ready marks and short crew messages
// (#113 step 7). Moved from AFiskBoardingController with the same network calls; the component is
// replicated and owned by the player's controller.
UCLASS()
class UFiskExpeditionCommands final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskExpeditionCommands();
    UFUNCTION(Server,Reliable) void ServerExpeditionChoose(int32 Revision,int32 Goal);
    UFUNCTION(Server,Reliable) void ServerExpeditionReady(int32 Revision,bool Ready);
    UFUNCTION(Server,Reliable) void ServerExpeditionSay(int32 Code);
private:
    AFiskBoardingController& Controller() const;
    double LastExpeditionMessage=-10;   // Server: 2 s between crew messages
};
