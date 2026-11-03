#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;

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

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;
};
