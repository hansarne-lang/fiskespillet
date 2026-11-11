#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Fishing/FiskRodSelection.h"
#include "Fishing/FiskRigSettings.h"
#include "Angler/FiskAnglerModel.h"
#include "FiskRodCommands.generated.h"
class AFiskBoardingController;

UENUM()
enum class EFiskRodAction : uint8
{
    Lift,
    Lower,
    Reel,
    StopReel,
    DragUp,
    DragDown,
    Withdraw,
    Land
};

// The player's own rods: choosing the active rod, rig changes, casting, the fight actions and rig
// sets from the tackle shop (#113 step 9). Moved from AFiskBoardingController with the same network
// calls; the component is replicated and owned by the player's controller. The rods and the chosen
// rod (FFiskRodSelection) stay on the controller and are changed through its public functions.
UCLASS()
class UFiskRodCommands final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskRodCommands();
    UFUNCTION(Server,Reliable) void ServerSelectRod(int32 ExpectedRevision,int32 ExpectedDay,int32 Slot);
    UFUNCTION(Client,Reliable) void ClientRodSelection(bool bAccepted,FFiskRodSelection Snapshot);
    UFUNCTION(Server,Reliable) void ServerConfigureSelectedRod(FFiskRodSelection Expected,FGuid ExpectedCast,int32 ExpectedRevision,int32 Bait,int32 Depth,int32 Program);
    UFUNCTION(Client,Reliable) void ClientRigReply(bool bAccepted);
    UFUNCTION(Server,Reliable) void ServerCastSelectedRod(FFiskRodSelection Expected,FGuid ExpectedId);
    UFUNCTION(Server,Reliable) void ServerActOnSelectedRod(EFiskRodAction Action,FFiskRodSelection Expected,FGuid ExpectedId);
    UFUNCTION(Server,Reliable) void ServerApplyRigBundle(FFiskRigBundle Bundle);
    UFUNCTION(Client,Reliable) void ClientRigBundleReply(bool Accepted);
private:
    AFiskBoardingController& Controller() const;
};
