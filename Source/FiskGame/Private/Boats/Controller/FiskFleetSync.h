#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskFleetSync.generated.h"
class AFiskBoardingController;

// Choosing, fetching and leaving a boat in the fleet at the start pier (#113 step 6). Moved from
// AFiskBoardingController with the same network calls; the component is replicated and owned by the
// player's controller. The chosen boat stays on the controller (SelectedFleetBoat, replicated to the owner).
UCLASS()
class UFiskFleetSync final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskFleetSync();
    UFUNCTION(Server,Reliable) void ServerFleetSelect(int32 Index,bool CreateOwn);
    UFUNCTION(Server,Reliable) void ServerFleetLeave();
private:
    AFiskBoardingController& Controller() const;
};
