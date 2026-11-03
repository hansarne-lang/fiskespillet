#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FiskBootstrapHUD.generated.h"

UCLASS()
class AFiskBootstrapHUD final : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};
