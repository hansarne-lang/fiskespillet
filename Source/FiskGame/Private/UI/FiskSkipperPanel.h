#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "Voyage/FiskSavedRoutes.h"
#include "Voyage/FiskTripBook.h"
#include "Voyage/FiskSkipperTools.h"
#include "FiskSkipperPanel.generated.h"
class AHUD;class UCanvas;struct FMinimalViewInfo;
UCLASS()
class UFiskSkipperPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    bool HandleKey(const FInputKeyEventArgs& Args);
    bool IsVisible() const { return Page!=0; }
    bool IsPhoto() const { return Page==3; }
    int32 GetPage() const { return Page; }
    void Open(int32 Value);
    void Draw(AHUD& HUD,UCanvas& Canvas);
    void PhotoCamera(const FTransform& Boat,FMinimalViewInfo& Out) const;
    const FFiskPhotoView& GetPhotoView() const { return Photo; }
    int32 GetMark() const { return Mark; }
    const FFiskPhotoPresets& GetPhotoPresets() const { return Cameras; }
    FString LastPhoto;
private:
    int32 Page=0,Mark=0,Slot=0,DayRow=0,Tab=0,DetailPage=0;
    bool bRenaming=false,bPhotoHud=true;
    FString EditName,Message;
    FGuid LoadedProfile;
    FFiskSavedRoutes Presets;
    FFiskPhotoView Photo;
    FFiskPhotoPresets Cameras;
    int32 CameraSlot=0,TemplateRow=0,StopSeconds=180,CompareA=0,CompareB=1;
    bool bPhotoGrid=false;
    FString CameraFile() const;
    void LoadPresets();
    FString PresetFile() const;
    void DrawPlan(AHUD& HUD,UCanvas& Canvas);
    void DrawBook(AHUD& HUD,UCanvas& Canvas);
};
