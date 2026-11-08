#pragma once

#include "CoreMinimal.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "TimerManager.h"
#include "FiskCareerProfile.h"

// Test-only state for AFiskBoardingController's Run*Smoke scenarios (issue #113, step 1).
// Kept out of the controller header so gameplay state and test bookkeeping are separate.
// Fields and defaults are moved unchanged from FiskBoardingController.h. Fields whose types
// belong to the Fishing area stay in the controller, so this Boats header includes no other area.
struct FFiskBoardingSmokeState
{
    int32 FleetRole=-1,FleetStage=0;
    FString FleetDir;
    FTimerHandle FleetTimer;
    double FleetAt=0;
    int64 FleetBaseCash=0;
    bool bFleetPrivacyLogged=false,bFleetAttackSent=false,bFleetResume=false,bFleetHouseVisited=false;
    FString CargoMode,CargoDir;FTimerHandle CargoTimer;
    int32 CargoStage=0,CargoLandings=0;double CargoAt=0;int64 CargoPaidCash=0;
    FString CollectionMode,CollectionDir;
    FTimerHandle CollectionTimer;
    int32 CollectionStage=0;double CollectionAt=0;
    FString AnglerMode,AnglerDir;
    FTimerHandle AnglerTimer;
    int32 AnglerStage=0,AnglerInitialJournal=0,AnglerInitialMedals=0;
    double AnglerAt=0;
    bool bAnglerActiveProbe=false,bAnglerActiveChecked=false;
    FFiskCareerProfile AnglerInitialCareer;
    FString SkipperMode,SkipperDir;
    FTimerHandle SkipperTimer;
    int32 SkipperStage=0;
    double SkipperAt=0;
    int32 RejectedCaptainRequests=0;
    FString ExplorationMode,ExplorationDir;
    FTimerHandle ExplorationTimer;
    int32 ExplorationStage=0,ExplorationLeg=0,ExplorationRevision=0;
    double ExplorationAt=0;
    bool bExplorationRejected=false,bExplorationLateVote=false;
    FString ExpeditionSmokeMode,ExpeditionSmokeDir;
    int32 ExpeditionSmokeStage=0;
    double ExpeditionSmokeAt=0;
    FTimerHandle ExpeditionSmokeTimer;
    bool bTestJournalAckDropped=false;
    FString CareerSmokeMode,CareerSmokeDir;
    FTimerHandle CareerSmokeTimer;
    int32 CareerSmokeStage=0,CareerSmokeRevision=0;
    int64 CareerSmokeCash=0;
    double CareerSmokeAt=0,CareerSmokeFightAt=0;
    bool bCareerSmokePrivate=false;
    FString FishingReleaseMode,FishingReleaseCapture;
    FTimerHandle FishingReleaseTimer;
    int32 FishingReleaseStage=0;
    double FishingReleaseAt=0,FishingReleaseFightAt=0;
    FGuid FishingReleaseOldCast;
    FString NavigationMode,NavigationCaptureDir;
    FTimerHandle NavigationTimer;
    int32 NavigationStage=0;
    double NavigationAt=0;
    FString JournalMode,JournalCaptureDir;
    FTimerHandle JournalTimer;
    int32 JournalStage=0,JournalExpectedDepth=0;
    double JournalAt=0;
    FString SessionMode, SessionCaptureDir;
    FTimerHandle SessionTimer;
    int32 SessionStage=0;
    double SessionAt=0;
    FGuid SessionOldCast;
    int32 SessionOldRoute=0;
    FString ContentMode,ContentCaptureDir;
    FTimerHandle ContentTimer;
    int32 ContentStage=0;
    double ContentAt=0,ContentFightAt=0;
    bool bContentPrivateChecked=false;
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
};
#endif
