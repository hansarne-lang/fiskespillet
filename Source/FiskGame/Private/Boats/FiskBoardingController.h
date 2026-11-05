#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FiskBoardingController.generated.h"

class UFiskPrototypeRod;
class UFiskFrontEnd;
class UFiskRigPanel;
class UFiskSeaChart;
class UFiskCrewPanel;
class UFiskJournalPanel;
class UFiskDeckPanel;
class AFiskStarterBoat;

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
    virtual bool InputKey(const struct FInputKeyEventArgs& Params) override;
    void ReleaseRodInputsForMenu();
    UFiskFrontEnd* GetFrontEnd() const { return FrontEnd; }
    const UFiskPrototypeRod* GetRod() const { return Rod; }
    UFiskRigPanel* GetRigPanel() const { return RigPanel; }
    UFiskSeaChart* GetSeaChart() const { return SeaChart; }
    UFiskCrewPanel* GetCrewPanel() const { return CrewPanel; }
    UFiskJournalPanel* GetJournalPanel() const { return JournalPanel; }
    UFiskDeckPanel* GetDeckPanel() const { return DeckPanel; }
    bool IsHelpVisible() const { return bHelpVisible; }
    bool IsResultsVisible() const { return bResultsVisible; }
    bool IsHistoryVisible() const { return bHistoryVisible; }
    void ResetRodForDay();
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
    UFUNCTION(Server, Reliable)
    void ServerConfigureRig(FGuid ExpectedCast, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

private:
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

    UFUNCTION(Server, Reliable)
    void ServerCastRod(FGuid ExpectedId);

    void RequestLift();
    void RequestLower();
    void RequestReel();
    void RequestStopReel();
    void RequestDragUp();
    void RequestDragDown();
    void RequestWithdraw();
    void RequestLand();
    bool CanUseRod() const;

    UFUNCTION(Server, Reliable)
    void ServerActOnRod(EFiskRodAction Action, FGuid ExpectedId);

    UPROPERTY()
    TObjectPtr<UFiskPrototypeRod> Rod;
    UPROPERTY() TObjectPtr<UFiskFrontEnd> FrontEnd;
    UPROPERTY() TObjectPtr<UFiskRigPanel> RigPanel;
    UPROPERTY() TObjectPtr<UFiskSeaChart> SeaChart;
    UPROPERTY() TObjectPtr<UFiskCrewPanel> CrewPanel;
    UPROPERTY() TObjectPtr<UFiskJournalPanel> JournalPanel;
    UPROPERTY() TObjectPtr<UFiskDeckPanel> DeckPanel;

#if WITH_DEV_AUTOMATION_TESTS
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
