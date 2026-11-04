#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;
class UFiskFrontEnd;
class UFiskRigPanel;

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

UCLASS()
class AFiskBoardingController final : public APlayerController
{
    GENERATED_BODY()

public:
    AFiskBoardingController();
    virtual bool InputKey(const struct FInputKeyEventArgs& Params) override;
    void ReleaseRodInputsForMenu();
    UFiskFrontEnd* GetFrontEnd() const { return FrontEnd; }
    const UFiskPrototypeRod* GetRod() const { return Rod; }
    UFiskRigPanel* GetRigPanel() const { return RigPanel; }
    bool CanOpenRig() const;
    void RequestRig(int32 Bait, int32 Depth, int32 Program);
    UFUNCTION(Server, Reliable)
    void ServerConfigureRig(FGuid ExpectedCast, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

private:
    void RequestBoarding();

    UFUNCTION(Server, Reliable)
    void ServerBoardStarterBoat();

    void RequestDeparture();

    UFUNCTION(Server, Reliable)
    void ServerStartBoatTrip();

    void RequestCast();

    UFUNCTION(Server, Reliable)
    void ServerCastRod(FGuid ExpectedId);

    void RequestLift();
    void RequestLower();
    void RequestReel();
    void RequestStopReel();
    void RequestDragUp();
    void RequestDragDown();
    void RequestWithdraw();
    void RequestLand();
    bool CanUseRod() const;

    UFUNCTION(Server, Reliable)
    void ServerActOnRod(EFiskRodAction Action, FGuid ExpectedId);

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;
    UPROPERTY() TObjectPtr<UFiskFrontEnd> FrontEnd;
    UPROPERTY() TObjectPtr<UFiskRigPanel> RigPanel;

#if WITH_DEV_AUTOMATION_TESTS
    void RunLanSmoke();
    void RunRepeatedFishingSmoke(const class AFiskStarterBoat& Boat, const class AFiskDayState& Day);
    int32 RepeatStage = 0, RepeatCatches = 0;
    FGuid RepeatFirstLandedId, RepeatCountedId, RepeatFightingId, RepeatWithdrawnId;
    double RepeatFightAt = -1.0, RepeatParallelAt = -1.0;
    bool bRepeatParallel = false, bRepeatStaleSent = false, bRepeatReadyStaleSent = false;
    bool bRepeatAttemptedEarlyCast = false;
    FTimerHandle LanSmokeTimer;
    bool bSmokeTriedEarlyLanding = false;
    double SmokeParallelAt = -1.0;
    bool bSmokeAdjustedDrag = false;
    bool bSmokeTriedDuplicateLanding = false;

    double SmokeReturnAt = 0.0;
    bool bSmokeCheckedPrivacy = false;
    bool bSmokeSawSettlement = false;
    bool bSmokeSawEnd = false;
    int32 SmokeClockMinute = -1;

#endif
};
