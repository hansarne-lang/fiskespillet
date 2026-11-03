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
    void RequestLand();
    bool CanUseRod() const;

    UFUNCTION(Server, Reliable)
    void ServerActOnRod(EFiskRodAction Action);

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;
};
