#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskMarinaTypes.h"
#include "FiskMarinaPanel.generated.h"
class AFiskMarinaHub;
class AHUD;
class UCanvas;
struct FInputKeyEventArgs;
UCLASS()
class UFiskMarinaPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskMarinaPanel();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* TickFunction) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    void Draw(AHUD& HUD,UCanvas& Canvas);
    bool IsVisible() const { return Page!=0; }
    bool WantsCamera() const { return Page==1 || Page==2 || Page==4 || Page==5; }
    int32 GetPage() const { return Page; }
    int32 GetHouseIndex() const { return HouseIndex; }
    AFiskMarinaHub* FindHub() const;
    bool IsAtDock() const;
    void ReceiveIdentity(const FFiskMarinaIdentity& Value);
    void RejectIdentity(const FString& Reason) { bSending=false;bSent=false;Status=Reason; }
    const FFiskMarinaIdentity& GetIdentity() const { return Identity; }
private:
    FGuid LoadedProfile;
    FFiskMarinaIdentity Identity,Draft,Pending;
    bool bBlocked=false,bSent=false,bPendingSave=false,bSending=false,bEditing=false;
    FString EditText,Status;
    int32 Page=0,Row=0,Species=0,HouseIndex=0,SentJournalRevision=-1;
    int32 PresetSlot=0;
    int32 ShopCategory=0,ShopLevel=0,ShopBuyItem=-1;
    FGuid ShopBuyCommand;
    bool bShopPreview=false;
    void ChooseHomeItem();
    void DrawHomeShop(AHUD& HUD,UCanvas& Canvas);
    double NextUpdate=0;
    FString File() const;
    void Show(int32 NewPage);
    void Choose();
#if WITH_DEV_AUTOMATION_TESTS
    FString SmokeMode,SmokeDirectory;
    FTimerHandle SmokeTimer;
    int32 SmokeStage=0;
    double SmokeAt=0;
    int64 SmokeCash=0;
    void RunSmoke();
    FString HomeMode,HomeDirectory;
    FTimerHandle HomeTimer;
    int32 HomeStage=0,HomeItem=4;
    double HomeAt=0;
    void RunHomeSmoke();
#endif
};
