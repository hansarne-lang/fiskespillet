#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineBaseTypes.h"
#include "UI/FiskRigPresets.h"
#include "UI/FiskJournalStore.h"
#include "UI/FiskNavigationNotebook.h"
#include "FiskGameInstance.generated.h"
class UNetDriver;
UCLASS()
class UFiskGameInstance final : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    void ApplySettings(bool bSave);
    bool bSound = true;
    int32 FrameRate = 60;
    FString ConnectionMessage;
    FFiskRigPresets RigPresets;
    bool SaveRigPreset(int32 Slot,const FFiskRigSettings& Rig);
    FFiskNavigationNotebook Places;
    FString PlacesMessage;
    bool SavePlace(int32 Slot,int32 Ground,const FString& Name);
    bool ClearPlace(int32 Slot);
    const FFiskJournalModel& GetJournal() const { return Journal; }
    FString GetJournalMessage() const { return JournalMessage; }
    int32 GetJournalRevision() const { return JournalRevision; }
    bool IsJournalPending() const { return bJournalPending; }
    bool WasJournalRecovered() const { return bJournalRecovered; }
    bool RetryJournalSave();
    void ObserveJournal(const struct FFiskSettlementView& View,int32 OwnPlayerId,int32 NowMs);
#if WITH_DEV_AUTOMATION_TESTS
    int32 MenuSmokeStep = 0;
    double SmokeAt = 0.0;
    int32 SmokeClockAt = 0;
#endif
private:
    FString RigPresetFile() const;
    FString JournalFile() const;
    FString PlacesFile() const;
    FFiskJournalModel Journal;
    FFiskJournalStore JournalStore;
    FString JournalMessage;
    TSet<FGuid> ObservedJournalDays;
    int32 JournalRevision=0;
    bool bJournalPending=false,bJournalRecovered=false,bJournalReceiptRejected=false;
    void NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error);
    void TravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
};
