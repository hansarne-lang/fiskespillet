#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "UI/FiskJournalModel.h"
#include "FiskJournalPanel.generated.h"
class AHUD;
class UCanvas;
UCLASS()
class UFiskJournalPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    bool IsVisible() const { return bVisible; }
    bool IsRecords() const { return bRecords; }
    int32 GetStatusFilter() const { return StatusFilter; }
    EFiskJournalSort GetSort() const { return Sort; }
    void Toggle();
    void TogglePassport();
    bool IsPassport() const { return bPassport; }
    bool HandleKey(const FInputKeyEventArgs& Args);
    void Draw(AHUD& HUD,UCanvas& Canvas);
private:
    bool bVisible=false,bRecords=false,bPassport=false;
    FString ExportMessage;
    void DrawPassport(AHUD& HUD,UCanvas& Canvas);
    EFiskJournalSort Sort=EFiskJournalSort::Date;
    int32 StatusFilter=0,SpeciesFilter=-1,Page=0,CachedRevision=-1;
    TArray<int32> Selected;
};
