#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "FiskDeckPanel.generated.h"
class AHUD;
class UCanvas;
class AFiskBoardingController;
UCLASS()
class UFiskDeckPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    void Open(bool bSonar);
    bool HandleKey(const FInputKeyEventArgs& Args);
    bool IsVisible() const { return bVisible; }
    void Draw(AHUD& HUD,UCanvas& Canvas);
    static void DrawOverlay(AHUD& HUD,UCanvas& Canvas,const AFiskBoardingController& PC);
private:
    bool bVisible=false,bSonar=true,bRecords=false;
    int32 Page=0;
};
