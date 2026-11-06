#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskExpeditionPanel.generated.h"
class AHUD;
class UCanvas;
struct FInputKeyEventArgs;
UCLASS()
class UFiskExpeditionPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    bool IsVisible() const { return bVisible; }
    bool HandleKey(const FInputKeyEventArgs& Args);
    void Draw(AHUD& HUD,UCanvas& Canvas);
private:
    bool bVisible=false;
    int32 Page=0;
};
