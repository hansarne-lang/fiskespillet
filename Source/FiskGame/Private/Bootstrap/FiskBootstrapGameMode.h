#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FiskBootstrapGameMode.generated.h"

UCLASS()
class AFiskBootstrapGameMode final : public AGameModeBase
{
    GENERATED_BODY()

public:
    AFiskBootstrapGameMode();
    virtual void StartPlay() override;
};
