#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Fishing/FiskRodSelection.h"
#include "Fishing/FiskRigSettings.h"
#include "FiskCareerProfile.h"
#include "FiskMarinaTypes.h"
#include "FiskRodStick.h"
#include "Angler/FiskAnglerModel.h"
#include "Boats/Controller/FiskMarinaSync.h"
#include "Boats/Controller/FiskRecoverySync.h"
#include "Boats/Controller/FiskCareerSync.h"
#include "Boats/Controller/FiskFleetSync.h"
#include "Boats/Controller/FiskExpeditionCommands.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingSmokeState.h"
#endif
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;
class UFiskFrontEnd;
class UFiskRigPanel;
class UFiskSeaChart;
class UFiskCrewPanel;
class UFiskJournalPanel;
class UFiskDeckPanel;
class AFiskStarterBoat;
class AFiskVoyagePlan;
class AFiskExpeditionState;
class UFiskMarinaPanel;
class UFiskExpeditionPanel;
class UFiskSkipperPanel;
class UFiskAnglerPanel;
class UFiskCollectionPanel;
class UFiskPierWalk;
class UFiskFishingHelp;

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
    UFiskFishingHelp* GetFishingHelp() const { return FishingHelp; }
    // Mouse-steered rod (#132): the rod follows mouse movement once it moves during a fight.
    const FFiskRodStick& GetRodStick() const { return RodStick; }
    bool IsRodSteering() const { return bRodSteering; }
    UFUNCTION(Server,Reliable) void ServerReturnToMarina(int32 Revision,int32 Day,int32 Leg);
    UFiskPierWalk* GetPierWalk() const { return PierWalk; }
    UFiskSkipperPanel* GetSkipperPanel() const { return SkipperPanel; }
    UFiskCollectionPanel* GetCollectionPanel() const { return CollectionPanel; }
    UFiskAnglerPanel* GetAnglerPanel() const { return AnglerPanel; }
    FFiskRigBundle MakeRigBundle(const TArray<FFiskRigSettings>& Rigs) const;
    UFUNCTION(Server,Reliable) void ServerApplyRigBundle(FFiskRigBundle Bundle);
    UFUNCTION(Client,Reliable) void ClientRigBundleReply(bool Accepted);
    UFUNCTION(Server,Reliable) void ServerVoyageReplace(int32 Day,int32 Revision,const TArray<FVector>& Points);
    UFUNCTION(Server,Reliable) void ServerVoyageNext(int32 Day,int32 Revision,int32 BoatRevision);
    UFiskMarinaPanel* GetMarinaPanel() const { return MarinaPanel; }
    UFiskMarinaSync* GetMarinaSync() const { return MarinaSync; }   // Boathouse, bar and harbour calls (#113 step 3)
    UFiskRecoverySync* GetRecoverySync() const { return RecoverySync; }   // Journal and medal recovery (#113 step 4)
    UFiskExpeditionPanel* GetExpeditionPanel() const { return ExpeditionPanel; }
    UFiskExpeditionCommands* GetExpeditionCommands() const { return ExpeditionCommands; }   // Goal, ready and crew messages (#113 step 7)
    // The career itself lives in UFiskCareerSync (#113 step 5); these forward to it.
    UFiskCareerSync* GetCareerSync() const { return CareerSync; }
    bool IsCareerReady() const { return CareerSync->IsReady(); }
    bool IsCareerSynced() const { return CareerSync->IsSynced(); }
    const FFiskCareerProfile& GetServerCareer() const { return CareerSync->GetServerCareer(); }
    bool HasEquipment(int32 Item) const { return IsCareerReady() && GetServerCareer().Has(Item); }
    bool HandleCareerKey(const FInputKeyEventArgs& Args);
    void DrawCareer(class AHUD& HUD,class UCanvas& Canvas);
    bool IsCareerVisible() const { return bCareerVisible; }
    void TickCareer();
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
    FFiskBoardingSmokeState Smoke; // Test-only state for the Run*Smoke scenarios
    // Test-only fields with Fishing types; kept here so FiskBoardingSmokeState.h needs no Fishing include.
    TArray<FFiskRigSettings> AnglerOriginalRigs,AnglerChangedRigs,AnglerFishingRigs;
    FFiskRigSettings ExpeditionSmokeRig;
    FFiskRodSelection FishingReleaseOldSelection;
    void RunFleetSmoke();
    void RunCargoSmoke();
    void RunCollectionSmoke();
    void RunAnglerSmoke();
    void RunSkipperSmoke();
    int32 GetRejectedCaptainRequests() const { return Smoke.RejectedCaptainRequests; }
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
    AFiskStarterBoat* GetSelectedFleetBoat() const { return SelectedFleetBoat; }
    AFiskVoyagePlan* FindVoyage() const;
    AFiskExpeditionState* FindExpedition() const;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    UFiskFleetSync* GetFleetSync() const { return FleetSync; }   // Boat choice at the start pier (#113 step 6)
    void SetSelectedFleetBoat(AFiskStarterBoat* Boat);
    bool AreRodsIdle() const;   // No rod is cast or fighting, so the player may change boat
    bool HandleFleetKey(const FInputKeyEventArgs& Args);
    bool IsFleetVisible() const { return bFleetVisible; }
    void DrawFleet(class AHUD& HUD,class UCanvas& Canvas);
    TArray<AFiskStarterBoat*> FleetBoats() const;
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
    friend class UFiskFishingHelp;
    UPROPERTY() TObjectPtr<UFiskFishingHelp> FishingHelp;
    UPROPERTY(Replicated) TObjectPtr<AFiskStarterBoat> SelectedFleetBoat;
    bool bFleetVisible=false;
    int32 FleetRow=0;
    UPROPERTY() TObjectPtr<UFiskCollectionPanel> CollectionPanel;
    UPROPERTY() TObjectPtr<UFiskAnglerPanel> AnglerPanel;
    UPROPERTY() TObjectPtr<UFiskSkipperPanel> SkipperPanel;
    UPROPERTY() TObjectPtr<UFiskExpeditionPanel> ExpeditionPanel;
    UPROPERTY() TObjectPtr<UFiskMarinaPanel> MarinaPanel;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskPierWalk> PierWalk;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskMarinaSync> MarinaSync;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskRecoverySync> RecoverySync;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskCareerSync> CareerSync;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskFleetSync> FleetSync;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UFiskExpeditionCommands> ExpeditionCommands;
    int32 ShopRow=0;
    bool bCareerVisible=false;
    FTimerHandle CareerTimer;
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

    FFiskRodStick RodStick;
    FGuid StickCast;
    FFiskStickEngage StickEngage;
    bool bRodSteering=false,bStickRaised=false,bStickReeling=false,bLiftHeld=false,bReelHeld=false;
    double StickSentAt=-10;
    void TickRodStick(float DeltaTime);
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
    void RunExplorationSmoke();
    void RunExpeditionSmoke();
    void RunCareerReleaseSmoke();
    void RunFishingReleaseSmoke();
    void RunNavigationSmoke();
    void RunJournalSmoke();
    void RunSessionSmoke();
    void RunContentSmoke();
    void RunLanSmoke();
    void RunRepeatedFishingSmoke(const class AFiskStarterBoat& Boat, const class AFiskDayState& Day);
#endif
};
