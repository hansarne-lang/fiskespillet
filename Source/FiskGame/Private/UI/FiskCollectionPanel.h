#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "Collections/FiskCollectionModel.h"
#include "FiskCollectionPanel.generated.h"
class AHUD;class UCanvas;
UCLASS()
class UFiskCollectionPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    bool IsVisible() const { return bVisible; }
    bool HandleKey(const FInputKeyEventArgs& Args);
    void Open();
    void Draw(AHUD& HUD,UCanvas& Canvas);
    const FFiskCollectionPins& GetPins() const { return Pins; }
    int32 GetPage() const { return Page; }
    int32 GetSelected() const { return Row; }
private:
    bool bVisible=false;
    int32 Page=0,Row=0,ResultPage=0;
    FFiskCatchQuery Query;
    FFiskCollectionPins Pins;
    FGuid Profile;
    FString Message;
    FString File() const;
    void SavePins();
};
