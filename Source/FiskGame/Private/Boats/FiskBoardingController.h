#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FiskBoardingController.generated.h"

UCLASS()
class AFiskBoardingController final : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void SetupInputComponent() override;

private:
    void RequestBoarding();

    UFUNCTION(Server, Reliable)
    void ServerBoardStarterBoat();
};
