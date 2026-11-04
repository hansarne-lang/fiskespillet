#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;

UENUM()
enum class EFiskRodAction : uint8
{
    Lift,
    Lower,
    Reel,
    StopReel,
    DragUp,
    DragDown,
    Land
};

UCLASS()
class AFiskBoardingController final : public APlayerController
{
    GENERATED_BODY()

public:
    AFiskBoardingController();
    const UFiskPrototypeRod* GetRod() const { return Rod; }

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
    void ServerCastRod();

    void RequestLift();
    void RequestLower();
    void RequestReel();
    void RequestStopReel();
    void RequestDragUp();
    void RequestDragDown();
    void RequestLand();
    bool CanUseRod() const;

    UFUNCTION(Server, Reliable)
    void ServerActOnRod(EFiskRodAction Action);

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;

#if WITH_DEV_AUTOMATION_TESTS
    void RunLanSmoke();
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
