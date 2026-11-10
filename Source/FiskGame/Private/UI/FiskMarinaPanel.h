#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskMarinaTypes.h"
#include "FiskMarinaPanel.generated.h"
class AFiskMarinaHub;
class AFiskStarterBoat;
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
    bool WantsCamera() const { return Page==1 || Page==2 || Page==4 || Page==5 || (Page==3 && bGallery); }
    int32 GetPage() const { return Page; }
    int32 GetHouseIndex() const { return HouseIndex; }
    bool IsHomePreview() const { return Page==5 && bShopPreview; }
    AFiskMarinaHub* FindHub() const;
    bool IsAtDock() const;
    bool IsGallery() const { return Page==3 && bGallery; }
    int32 GetGallerySpecies() const { return Species; }
    float GetGalleryYaw() const { return GalleryYaw; }
    float GetGalleryZoom() const { return GalleryZoom; }
    int32 GetHarbourView() const { return HarbourView; }
    const AFiskStarterBoat* FollowBoat() const;
    bool IsHarbourMusic() const { return bHarbourMusic; }
    void ToggleHarbourMusic() { bHarbourMusic=!bHarbourMusic; }
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
    bool bShopPreview=false,bGallery=false,bHarbourMusic=true;
    int32 HarbourView=0,FollowIndex=0;
    float GalleryYaw=0,GalleryZoom=1;
    void DrawGallery(AHUD& HUD,UCanvas& Canvas);
    void ChooseHomeItem();
    void DrawHomeShop(AHUD& HUD,UCanvas& Canvas);
    double NextUpdate=0;
    FString File() const;
    void Show(int32 NewPage);
    void Choose();
    // Beach bar bets (#22 step 2): fishers out at sea, chosen fisher and stake.
    int32 BetFisher=0,BetStake=1;
    TArray<TPair<FGuid,FString>> BetCandidates() const;
    void ChooseBet(int32 Action);
    void DrawBets(AHUD& HUD,float Scale) const;
#if WITH_DEV_AUTOMATION_TESTS
    FString HarbourMode,HarbourDirectory;
    FTimerHandle HarbourTimer;
    int32 HarbourStage=0,HarbourSpecies=0;
    double HarbourAt=0;
    int64 HarbourInitialCash=0;
    void RunHarbourSmoke();
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
