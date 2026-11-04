#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FiskBootstrapGameMode.generated.h"

class AFiskStarterBoat;
class AFiskDaySettlement;

UCLASS()
class AFiskBootstrapGameMode final : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFiskBootstrapGameMode();
    virtual void StartPlay() override;
    virtual void InitGameState() override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;
    AFiskStarterBoat* GetStarterBoat() const { return StarterBoat; }
    AFiskDaySettlement* GetSettlement() const { return Settlement; }

private:
    UPROPERTY()
    TObjectPtr<AFiskStarterBoat> StarterBoat;
    UPROPERTY()
    TObjectPtr<AFiskDaySettlement> Settlement;
};
