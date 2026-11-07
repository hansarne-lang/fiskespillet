#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Fishing/FiskRodSelection.h"
#include "Fishing/FiskRigSettings.h"
#include "FiskCareerProfile.h"
#include "FiskMarinaTypes.h"
#include "UI/FiskJournalModel.h"
#include "Expedition/FiskExpeditionModel.h"
#include "Angler/FiskAnglerModel.h"
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;
class UFiskFrontEnd;
class UFiskRigPanel;
class UFiskSeaChart;
class UFiskCrewPanel;
class UFiskJournalPanel;
class UFiskDeckPanel;
class AFiskStarterBoat;
class UFiskMarinaPanel;
class UFiskExpeditionPanel;
class UFiskSkipperPanel;
class UFiskAnglerPanel;
class UFiskCollectionPanel;

UENUM()
enum class EFiskRodAction : uint8
{
    Lift,
    Lower,
    Reel,
    StopReel,
    DragUp,
    DragDown,
    Withdraw,
    Land
};

UCLASS()
class AFiskBoardingController final : public APlayerController
{
    GENERATED_BODY()

public:
    AFiskBoardingController();
    UFiskSkipperPanel* GetSkipperPanel() const { return SkipperPanel; }
    UFiskCollectionPanel* GetCollectionPanel() const { return CollectionPanel; }
    UFiskAnglerPanel* GetAnglerPanel() const { return AnglerPanel; }
    FFiskRigBundle MakeRigBundle(const TArray<FFiskRigSettings>& Rigs) const;
    UFUNCTION(Server,Reliable) void ServerApplyRigBundle(FFiskRigBundle Bundle);
    UFUNCTION(Client,Reliable) void ClientRigBundleReply(bool Accepted);
    UFUNCTION(Server,Reliable) void ServerVoyageReplace(int32 Day,int32 Revision,const TArray<FVector>& Points);
    UFUNCTION(Server,Reliable) void ServerVoyageNext(int32 Day,int32 Revision,int32 BoatRevision);
    UFiskMarinaPanel* GetMarinaPanel() const { return MarinaPanel; }
    UFiskExpeditionPanel* GetExpeditionPanel() const { return ExpeditionPanel; }
    UFUNCTION(Server,Reliable) void ServerMarinaIdentity(FGuid Profile,FFiskMarinaIdentity Identity);
    UFUNCTION(Client,Reliable) void ClientMarinaIdentity(bool Accepted,FFiskMarinaIdentity Identity,const FString& Reason);
    UFUNCTION(Server,Reliable) void ServerMarinaRecords(FGuid Profile,const TArray<FFiskMarinaTrophy>& Records);
    UFUNCTION(Server,Reliable) void ServerMarinaSay(int32 Code);
    UFUNCTION(Client,Reliable) void ClientJournalBatch(FGuid Profile,int32 Serial,const TArray<FFiskJournalEntry>& Entries);
    UFUNCTION(Server,Reliable) void ServerJournalAck(FGuid Profile,int32 Serial);
    void TickJournalRecovery();
    UFUNCTION(Client,Reliable) void ClientExpeditionBatch(FGuid Profile,int32 Serial,const TArray<FFiskExpeditionRecord>& Entries);
    UFUNCTION(Server,Reliable) void ServerExpeditionAck(FGuid Profile,int32 Serial);
    UFUNCTION(Server,Reliable) void ServerExpeditionChoose(int32 Revision,int32 Goal);
    UFUNCTION(Server,Reliable) void ServerExpeditionReady(int32 Revision,bool Ready);
    UFUNCTION(Server,Reliable) void ServerExpeditionSay(int32 Code);
    bool IsCareerReady() const { return bCareerReady; }
    bool IsCareerSynced() const { return bCareerReady && CareerAck>=ServerCareer.Revision; }
    const FFiskCareerProfile& GetServerCareer() const { return ServerCareer; }
    bool HasEquipment(int32 Item) const { return bCareerReady && ServerCareer.Has(Item); }
    bool HandleCareerKey(const FInputKeyEventArgs& Args);
    void DrawCareer(class AHUD& HUD,class UCanvas& Canvas);
    bool IsCareerVisible() const { return bCareerVisible; }
    void TickCareer();
    UFUNCTION(Server,Reliable) void ServerCareerHello(FFiskCareerProfile Profile);
    UFUNCTION(Client,Reliable) void ClientCareerSnapshot(FFiskCareerProfile Profile,const FString& Message);
    UFUNCTION(Server,Reliable) void ServerCareerAck(FGuid Profile,int32 Revision);
    UFUNCTION(Server,Reliable) void ServerBuyCareer(FGuid Command,int32 Revision,int32 Item);
    UFUNCTION(Server,Reliable) void ServerChooseBoat(int32 Revision,int32 Kind);
    UFUNCTION(Server,Reliable) void ServerSelectSeaPoint(int32 Revision,FVector_NetQuantize Point);
    UFUNCTION(Server,Reliable) void ServerTravelVote(int32 Leg,bool Vote);
    virtual bool InputKey(const struct FInputKeyEventArgs& Params) override;
    void ReleaseRodInputsForMenu();
    UFiskFrontEnd* GetFrontEnd() const { return FrontEnd; }
    const UFiskPrototypeRod* GetRod() const { return GetRodAt(Selection.Slot); }
    const UFiskPrototypeRod* GetRodAt(int32 Slot) const { return Slot==0?Rod.Get():Slot==1?ExtraRod.Get():Slot==2?ThirdRod.Get():nullptr; }
    int32 GetRodCapacity() const;
    int32 GetSelectedRod() const { return Selection.Slot; }
    bool IsRodSelectionPending() const { return PendingRod>=0; }
    int32 GetCameraView() const { return CameraView; }
#if WITH_DEV_AUTOMATION_TESTS
    void RunCargoSmoke();
    FString CargoMode,CargoDir;FTimerHandle CargoTimer;
    int32 CargoStage=0,CargoLandings=0;double CargoAt=0;int64 CargoPaidCash=0;
    void RunCollectionSmoke();
    FString CollectionMode,CollectionDir;
    FTimerHandle CollectionTimer;
    int32 CollectionStage=0;double CollectionAt=0;
    void RunAnglerSmoke();
    FString AnglerMode,AnglerDir;
    FTimerHandle AnglerTimer;
    int32 AnglerStage=0,AnglerInitialJournal=0,AnglerInitialMedals=0;
    double AnglerAt=0;
    bool bAnglerActiveProbe=false,bAnglerActiveChecked=false;
    FFiskCareerProfile AnglerInitialCareer;
    TArray<FFiskRigSettings> AnglerOriginalRigs,AnglerChangedRigs,AnglerFishingRigs;
    void RunSkipperSmoke();
    FString SkipperMode,SkipperDir;
    FTimerHandle SkipperTimer;
    int32 SkipperStage=0;
    double SkipperAt=0;
    int32 GetRejectedCaptainRequests() const { return RejectedCaptainRequests; }
#endif
    void RequestRodSelection(int32 Slot);
    UFUNCTION(Server,Reliable) void ServerSelectRod(int32 ExpectedRevision,int32 ExpectedDay,int32 Slot);
    UFUNCTION(Client,Reliable) void ClientRodSelection(bool bAccepted,FFiskRodSelection Snapshot);
    UFiskRigPanel* GetRigPanel() const { return RigPanel; }
    UFiskSeaChart* GetSeaChart() const { return SeaChart; }
    UFiskCrewPanel* GetCrewPanel() const { return CrewPanel; }
    UFiskJournalPanel* GetJournalPanel() const { return JournalPanel; }
    UFiskDeckPanel* GetDeckPanel() const { return DeckPanel; }
    bool IsHelpVisible() const { return bHelpVisible; }
    bool IsResultsVisible() const { return bResultsVisible; }
    bool IsHistoryVisible() const { return bHistoryVisible; }
    void ResetRodForDay(int32 NewDay=0);
    UFUNCTION(Server,Reliable) void ServerNextDay(int32 ExpectedDay);
    int32 GetResultPage() const { return ResultPage; }
    FString GetActionFeedback() const;
    UFUNCTION(Client,Reliable) void ClientActionFeedback(const FString& Message);
    UFUNCTION(Client,Reliable) void ClientRigReply(bool bAccepted);
    UFUNCTION(Server,Reliable) void ServerTransferCaptain(int32 ExpectedRevision,int32 Target);
    UFUNCTION(Client,Reliable) void ClientCaptainReply(bool bAccepted);
    UFUNCTION(Server,Reliable) void ServerStartBoatTrip(int32 ExpectedRevision);
    const AFiskStarterBoat* FindStarterBoat() const;
    UFUNCTION(Server,Reliable) void ServerSelectGround(int32 ExpectedRevision,int32 GroundId);
    UFUNCTION(Client,Reliable) void ClientGroundReply(const FString& Message);
    bool CanOpenRig() const;
    void RequestRig(int32 Bait, int32 Depth, int32 Program);
    void ServerConfigureRig(FGuid ExpectedCast, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program);
    UFUNCTION(Server,Reliable) void ServerConfigureSelectedRod(FFiskRodSelection Expected,FGuid ExpectedCast,int32 ExpectedRevision,int32 Bait,int32 Depth,int32 Program);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

private:
    UPROPERTY() TObjectPtr<UFiskCollectionPanel> CollectionPanel;
    UPROPERTY() TObjectPtr<UFiskAnglerPanel> AnglerPanel;
    UPROPERTY() TObjectPtr<UFiskSkipperPanel> SkipperPanel;
    UPROPERTY() TObjectPtr<UFiskExpeditionPanel> ExpeditionPanel;
    UPROPERTY() TObjectPtr<UFiskMarinaPanel> MarinaPanel;
    double LastMarinaMessage=-10;
    FFiskJournalModel RecoverySnapshot;
    TArray<FFiskExpeditionRecord> ExpeditionSnapshot;
    int32 ExpeditionOffset=0,ExpeditionSerial=0,ExpeditionSentCount=0,ClientExpeditionSerial=0;
    bool bExpeditionAwaiting=false;
    double LastExpeditionMessage=-10;
    void TickExpeditionRecovery();
    int32 RecoveryRevision=-1,RecoveryOffset=0,RecoverySerial=0,RecoverySentCount=0,ClientRecoverySerial=0;
    bool bRecoveryAwaiting=false,bRecoveryReported=false;
    FFiskCareerProfile ServerCareer,PendingCareer;
    int32 CareerAck=0,ShopRow=0;
    bool bCareerReady=false,bCareerVisible=false,bCareerHelloSent=false;
    double CareerRetryAt=0;
    FTimerHandle CareerTimer;
    FString CareerAuthorityFile() const;
    bool SaveServerCareer(const FFiskCareerProfile& Candidate,const FString& Message);
    FFiskRodSelection Selection;
    int32 PendingRod=-1,CameraView=0;
    void ReconcileRodCapacity();
    UFiskPrototypeRod* MutableRod() const { return const_cast<UFiskPrototypeRod*>(GetRodAt(Selection.Slot)); }
    bool bHelpVisible=false,bResultsVisible=false,bHistoryVisible=false,bAutoResultsSeen=false;
    int32 ObservedDay=0, ReturnWarningLevel=0;
    int32 ResultPage=0;
    FString ActionFeedback;
    double ActionFeedbackUntil=0;
    void RequestBoarding();

    UFUNCTION(Server, Reliable)
    void ServerBoardStarterBoat();

    void RequestDeparture();


    void RequestCast();

    void ServerCastRod(FGuid ExpectedId);
    UFUNCTION(Server,Reliable) void ServerCastSelectedRod(FFiskRodSelection Expected,FGuid ExpectedId);

    void RequestLift();
    void RequestLower();
    void RequestReel();
    void RequestStopReel();
    void RequestDragUp();
    void RequestDragDown();
    void RequestWithdraw();
    void RequestLand();
    bool CanUseRod() const;

    void ServerActOnRod(EFiskRodAction Action, FGuid ExpectedId);
    UFUNCTION(Server,Reliable) void ServerActOnSelectedRod(EFiskRodAction Action,FFiskRodSelection Expected,FGuid ExpectedId);

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;
    UPROPERTY() TObjectPtr<UFiskPrototypeRod> ExtraRod;
    UPROPERTY() TObjectPtr<UFiskPrototypeRod> ThirdRod;
    UPROPERTY() TObjectPtr<UFiskFrontEnd> FrontEnd;
    UPROPERTY() TObjectPtr<UFiskRigPanel> RigPanel;
    UPROPERTY() TObjectPtr<UFiskSeaChart> SeaChart;
    UPROPERTY() TObjectPtr<UFiskCrewPanel> CrewPanel;
    UPROPERTY() TObjectPtr<UFiskJournalPanel> JournalPanel;
    UPROPERTY() TObjectPtr<UFiskDeckPanel> DeckPanel;

#if WITH_DEV_AUTOMATION_TESTS
    int32 RejectedCaptainRequests=0;
    FString ExplorationMode,ExplorationDir;
    FTimerHandle ExplorationTimer;
    int32 ExplorationStage=0,ExplorationLeg=0,ExplorationRevision=0;
    double ExplorationAt=0;
    bool bExplorationRejected=false,bExplorationLateVote=false;
    void RunExplorationSmoke();
    FString ExpeditionSmokeMode,ExpeditionSmokeDir;
    int32 ExpeditionSmokeStage=0;
    double ExpeditionSmokeAt=0;
    FTimerHandle ExpeditionSmokeTimer;
    FFiskRigSettings ExpeditionSmokeRig;
    bool bTestJournalAckDropped=false;
    void RunExpeditionSmoke();
    void RunCareerReleaseSmoke();
    FString CareerSmokeMode,CareerSmokeDir;
    FTimerHandle CareerSmokeTimer;
    int32 CareerSmokeStage=0,CareerSmokeRevision=0;
    int64 CareerSmokeCash=0;
    double CareerSmokeAt=0,CareerSmokeFightAt=0;
    bool bCareerSmokePrivate=false;
    void RunFishingReleaseSmoke();
    FString FishingReleaseMode,FishingReleaseCapture;
    FTimerHandle FishingReleaseTimer;
    int32 FishingReleaseStage=0;
    double FishingReleaseAt=0,FishingReleaseFightAt=0;
    FGuid FishingReleaseOldCast;
    FFiskRodSelection FishingReleaseOldSelection;
    void RunNavigationSmoke();
    FString NavigationMode,NavigationCaptureDir;
    FTimerHandle NavigationTimer;
    int32 NavigationStage=0;
    double NavigationAt=0;
    void RunJournalSmoke();
    FString JournalMode,JournalCaptureDir;
    FTimerHandle JournalTimer;
    int32 JournalStage=0,JournalExpectedDepth=0;
    double JournalAt=0;
    void RunSessionSmoke();
    FString SessionMode, SessionCaptureDir;
    FTimerHandle SessionTimer;
    int32 SessionStage=0;
    double SessionAt=0;
    FGuid SessionOldCast;
    int32 SessionOldRoute=0;
    void RunContentSmoke();
    FString ContentMode,ContentCaptureDir;
    FTimerHandle ContentTimer;
    int32 ContentStage=0;
    double ContentAt=0,ContentFightAt=0;
    bool bContentPrivateChecked=false;
    void RunLanSmoke();
    void RunRepeatedFishingSmoke(const class AFiskStarterBoat& Boat, const class AFiskDayState& Day);
    int32 RepeatStage = 0, RepeatCatches = 0;
    FGuid RepeatFirstLandedId, RepeatCountedId, RepeatFightingId, RepeatWithdrawnId;
    double RepeatFightAt = -1.0, RepeatParallelAt = -1.0;
    bool bRepeatParallel = false, bRepeatStaleSent = false, bRepeatReadyStaleSent = false;
    bool bRepeatAttemptedEarlyCast = false;
    FTimerHandle LanSmokeTimer;
    bool bSmokeTriedEarlyLanding = false;
    double SmokeParallelAt = -1.0;
    bool bSmokeAdjustedDrag = false;
    bool bSmokeTriedDuplicateLanding = false;

    double SmokeReturnAt = 0.0;
    bool bSmokeCheckedPrivacy = false;
    bool bSmokeSawSettlement = false;
    bool bSmokeSawEnd = false;
    int32 SmokeClockMinute = -1;

#endif
};
