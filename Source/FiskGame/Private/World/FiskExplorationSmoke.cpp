#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Expedition/FiskExpeditionState.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskGameInstance.h"
#include "Career/FiskSeaPassport.h"
#include "Fishing/FiskPrototypeRod.h"
#include "World/FiskOceanAtlas.h"
#include "DayCycle/FiskDayState.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UnrealClient.h"
#include "TimerManager.h"
#include "Voyage/FiskVoyagePlan.h"

void AFiskBoardingController::RunExplorationSmoke()
{
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();const auto* B=FindStarterBoat();auto* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(!Day || !B || !PlayerState || !IsCareerReady() || !Day->HasStarted()) { return; }
    const int32 Self=PlayerState->GetPlayerId(),Progress=B->GetTripProgress();const double Now=Day->GetElapsedRealSeconds(),Real=GetWorld()->GetRealTimeSeconds();
    const bool Host=CareerSmokeMode==TEXT("host") || CareerSmokeMode==TEXT("solo"),Solo=CareerSmokeMode==TEXT("solo");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Shot=[&](const TCHAR* Name){if(ExplorationDir.IsEmpty()) { return; }const FString File=FPaths::Combine(ExplorationDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_EXPLORATION_FAILED mode=%s stage=%d %s"),*ExplorationMode,ExplorationStage,Why);GetWorldTimerManager().ClearTimer(ExplorationTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPLORATION_PASSED mode=%s journal=%d medals=%d cash=%lld"),*ExplorationMode,GI->GetJournal().GetEntries().Num(),GI->Expeditions.Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(ExplorationTimer);};
    if(ExplorationStage==0 && CareerSmokeStage==13 && Now>=70 && Now<120)
    {
        if(Host) { TActorIterator<AFiskExpeditionState> E(GetWorld());if(E) { ServerExpeditionChoose(E->GetPlan().Revision,2); } }
        if(!GI->SaveRigPreset(0,GetRod()->GetRig()) || !GI->RenameRigPreset(0,TEXT("REVFISKE 9"))) { Fail(TEXT("preset_storage"));return; }
        Key(EKeys::M);Key(EKeys::A);if(!SeaChart->IsAtlas()) { Fail(TEXT("atlas_not_open"));return; }if(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))) { Key(EKeys::End); }Shot(TEXT("ocean-atlas"));ExplorationAt=Real;ExplorationStage=1;
    }
    else if(ExplorationStage==1 && Real-ExplorationAt>=1)
    { Key(EKeys::M);Key(EKeys::L);if(!JournalPanel->IsPassport()) { Fail(TEXT("passport_not_open"));return; }Shot(TEXT("passport-before17"));Key(EKeys::F10);ExplorationAt=Real;ExplorationStage=2; }
    else if(ExplorationStage==2 && Real-ExplorationAt>=1)
    { Key(EKeys::L);ExplorationStage=3; }
    else if(ExplorationStage==3 && B->IsTravelling())
    {
        ExplorationLeg=B->GetLegSerial();ExplorationAt=Real;
        if(CareerSmokeMode!=TEXT("guest3")) { Key(EKeys::V); }ExplorationStage=4;
    }
    else if(ExplorationStage==4)
    {
        if(CareerSmokeMode==TEXT("guest3") && Real-ExplorationAt>=5 && !bExplorationLateVote) { Key(EKeys::V);bExplorationLateVote=true; }
        if(!Solo && Real-ExplorationAt>=1 && Real-ExplorationAt<4 && B->UsesTravelCard()) { Fail(TEXT("non_unanimous_view_skip"));return; }
        if(B->UsesTravelCard()) { Shot(TEXT("unanimous-travel"));ExplorationStage=5; }
        else if(Progress==100) { Fail(TEXT("travel_vote_never_unanimous"));return; }
    }
    else if(ExplorationStage==5 && Progress==100)
    {
        if(B->UsesTravelCard()) { Fail(TEXT("travel_card_survived_arrival"));return; }
        if(!bExplorationRejected && GetRodAt(0)->GetState()==EFiskRodState::Fishing)
        {
            ExplorationRevision=B->GetRouteRevision();ServerSelectSeaPoint(ExplorationRevision,FiskAtlas::Find(5)->Point);ServerTravelVote(ExplorationLeg-1,true);
            if(!SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
            bExplorationRejected=true;ExplorationAt=Real;
        }
        if(bExplorationRejected && Real-ExplorationAt>=2)
        { if(B->GetRouteRevision()!=ExplorationRevision) { Fail(TEXT("active_rod_or_foreign_move_accepted"));return; }ExplorationStage=6; }
    }
    else if(ExplorationStage==6)
    {
        if(Host && Progress==100)
        {
            bool All=true;for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            { const auto* P=Cast<AFiskBoardingController>(It->Get());if(P && P->PlayerState && B->IsAboard(P->PlayerState->GetPlayerId()) && P->GetRodAt(0)->GetTestLandingCount()!=1) { All=false; } }
            if(All)
            {
                if(!SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
                else { ServerSelectSeaPoint(B->GetRouteRevision(),FiskAtlas::Find(5)->Point); }
            }
        }
        if(B->GetLegSerial()>ExplorationLeg && B->IsTravelling())
        { if(B->GetTravelVoteCount()!=0 || B->UsesTravelCard()) { Fail(TEXT("votes_survived_new_leg"));return; }ExplorationLeg=B->GetLegSerial();Shot(TEXT("moving-between-grounds"));ExplorationStage=7; }
    }
    else if(ExplorationStage==7 && Progress==100)
    {
        if(FVector::Dist2D(B->GetActorLocation(),FiskAtlas::Find(5)->Point)>5) { Fail(TEXT("transfer_arrived_wrong_point"));return; }
        ExplorationAt=Real;ExplorationStage=71;
    }
    else if(ExplorationStage==71 && Real-ExplorationAt>=3)
    {
        // Leave enough time for remote half-second probes to observe arrival before the next leg.
        if(Host)
        {
            if(!SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
            else { ServerSelectSeaPoint(B->GetRouteRevision(),FiskAtlas::Find(7)->Point); }
            if(B->GetLegSerial()<=ExplorationLeg || B->GetDestination()!=FiskAtlas::Find(7)->Point) { Fail(TEXT("second_transfer_not_started"));return; }
        }
        ExplorationAt=Real;ExplorationStage=70;
    }
    else if(ExplorationStage==70 && Real-ExplorationAt>=2 && B->GetLegSerial()>ExplorationLeg)
    {
        if(B->GetDestination()!=FiskAtlas::Find(7)->Point) { Fail(TEXT("second_transfer_destination_wrong"));return; }
        if(Host)
        { if(!B->IsTravelling() || Progress>=100) { Fail(TEXT("second_transfer_not_outbound"));return; }Key(EKeys::R); }
        ExplorationStage=8;
    }
    if(ExplorationStage==8 && Progress==200)
    { Shot(TEXT("home-after-interrupted-leg"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTISTOP_PASSED player=%d arrival=%d"),Self,B->GetMarinaArrivalMs(Day->GetElapsedMilliseconds()));ExplorationStage=80; }
    if(ExplorationStage>=3 && Day->GetDayNumber()==1 && Now<(HasAuthority()?1020.0:1018.0))
    {
        const auto Rows=FiskSeaPassport::Build(GI->GetJournal(),GI->Expeditions);
        for(const auto& R:Rows) { if(R.Current!=0) { Fail(TEXT("passport_leaked_today"));return; } }
        if(Now>=960 && ExplorationMode==TEXT("offline") && ExplorationStage==80)
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPLORATION_DISCONNECT_READY at=%.3f"),Now);ExplorationStage=81; }
    }
    if(ExplorationStage==80 && Day->GetDayNumber()==1 && Now>=1050)
    {
        const auto Rows=FiskSeaPassport::Build(GI->GetJournal(),GI->Expeditions);
        if(GI->GetJournal().GetEntries().Num()!=1 || GI->Expeditions.Num()!=1 || !Rows[0].Complete() || !Rows[9].Complete()) { Fail(TEXT("confirmed_passport_missing"));return; }
        if(IsCareerVisible() || IsResultsVisible() || IsHistoryVisible() || IsHelpVisible()) { Key(EKeys::Escape); }
        Key(EKeys::L);Key(EKeys::F10);Shot(TEXT("passport-after17"));ExplorationAt=Real;ExplorationStage=82;
    }
    else if(ExplorationStage==82 && Real-ExplorationAt>=1) { Key(EKeys::L);ExplorationStage=83; }
    if(Solo && Day->GetDayNumber()==1 && Now>=1200 && ExplorationStage==83) { Key(EKeys::N);return; }
    if(Day->GetDayNumber()==2 && Now>=35 && ExplorationStage==83)
    { if(B->GetTravelVoteCount()!=0 || B->UsesTravelCard() || GI->GetJournal().GetEntries().Num()!=1 || GI->Expeditions.Num()!=1) { Fail(TEXT("day2_or_duplicate"));return; }Done(); }
}
#endif
