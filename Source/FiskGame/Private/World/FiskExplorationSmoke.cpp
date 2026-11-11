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
    const bool Host=Smoke.CareerSmokeMode==TEXT("host") || Smoke.CareerSmokeMode==TEXT("solo"),Solo=Smoke.CareerSmokeMode==TEXT("solo");
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Shot=[&](const TCHAR* Name){if(Smoke.ExplorationDir.IsEmpty()) { return; }const FString File=FPaths::Combine(Smoke.ExplorationDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorldTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_EXPLORATION_FAILED mode=%s stage=%d %s"),*Smoke.ExplorationMode,Smoke.ExplorationStage,Why);GetWorldTimerManager().ClearTimer(Smoke.ExplorationTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPLORATION_PASSED mode=%s journal=%d medals=%d cash=%lld"),*Smoke.ExplorationMode,GI->GetJournal().GetEntries().Num(),GI->Expeditions.Num(),static_cast<long long>(GI->Career.Cash));GetWorldTimerManager().ClearTimer(Smoke.ExplorationTimer);};
    if(Smoke.ExplorationStage==0 && Smoke.CareerSmokeStage==13 && Now>=70 && Now<120)
    {
        if(Host) { TActorIterator<AFiskExpeditionState> E(GetWorld());if(E) { ExpeditionCommands->ServerExpeditionChoose(E->GetPlan().Revision,2); } }
        if(!GI->SaveRigPreset(0,GetRod()->GetRig()) || !GI->RenameRigPreset(0,TEXT("REVFISKE 9"))) { Fail(TEXT("preset_storage"));return; }
        Key(EKeys::M);Key(EKeys::A);if(!SeaChart->IsAtlas()) { Fail(TEXT("atlas_not_open"));return; }if(FParse::Param(FCommandLine::Get(),TEXT("FiskDeep"))) { Key(EKeys::End); }Shot(TEXT("ocean-atlas"));Smoke.ExplorationAt=Real;Smoke.ExplorationStage=1;
    }
    else if(Smoke.ExplorationStage==1 && Real-Smoke.ExplorationAt>=1)
    { Key(EKeys::M);Key(EKeys::L);if(!JournalPanel->IsPassport()) { Fail(TEXT("passport_not_open"));return; }Shot(TEXT("passport-before17"));Key(EKeys::F10);Smoke.ExplorationAt=Real;Smoke.ExplorationStage=2; }
    else if(Smoke.ExplorationStage==2 && Real-Smoke.ExplorationAt>=1)
    { Key(EKeys::L);Smoke.ExplorationStage=3; }
    else if(Smoke.ExplorationStage==3 && B->IsTravelling())
    {
        Smoke.ExplorationLeg=B->GetLegSerial();Smoke.ExplorationAt=Real;
        if(Smoke.CareerSmokeMode!=TEXT("guest3")) { Key(EKeys::V); }Smoke.ExplorationStage=4;
    }
    else if(Smoke.ExplorationStage==4)
    {
        if(Smoke.CareerSmokeMode==TEXT("guest3") && Real-Smoke.ExplorationAt>=5 && !Smoke.bExplorationLateVote) { Key(EKeys::V);Smoke.bExplorationLateVote=true; }
        if(!Solo && Real-Smoke.ExplorationAt>=1 && Real-Smoke.ExplorationAt<4 && B->UsesTravelCard()) { Fail(TEXT("non_unanimous_view_skip"));return; }
        if(B->UsesTravelCard()) { Shot(TEXT("unanimous-travel"));Smoke.ExplorationStage=5; }
        else if(Progress==100) { Fail(TEXT("travel_vote_never_unanimous"));return; }
    }
    else if(Smoke.ExplorationStage==5 && Progress==100)
    {
        if(B->UsesTravelCard()) { Fail(TEXT("travel_card_survived_arrival"));return; }
        if(!Smoke.bExplorationRejected && GetRodAt(0)->GetState()==EFiskRodState::Fishing)
        {
            Smoke.ExplorationRevision=B->GetRouteRevision();BoatCommands->ServerSelectSeaPoint(Smoke.ExplorationRevision,FiskAtlas::Find(5)->Point);BoatCommands->ServerTravelVote(Smoke.ExplorationLeg-1,true);
            if(!Smoke.SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();BoatCommands->ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
            Smoke.bExplorationRejected=true;Smoke.ExplorationAt=Real;
        }
        if(Smoke.bExplorationRejected && Real-Smoke.ExplorationAt>=2)
        { if(B->GetRouteRevision()!=Smoke.ExplorationRevision) { Fail(TEXT("active_rod_or_foreign_move_accepted"));return; }Smoke.ExplorationStage=6; }
    }
    else if(Smoke.ExplorationStage==6)
    {
        if(Host && Progress==100)
        {
            bool All=true;for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            { const auto* P=Cast<AFiskBoardingController>(It->Get());if(P && P->PlayerState && B->IsAboard(P->PlayerState->GetPlayerId()) && P->GetRodAt(0)->GetTestLandingCount()!=1) { All=false; } }
            if(All)
            {
                if(!Smoke.SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();BoatCommands->ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
                else { BoatCommands->ServerSelectSeaPoint(B->GetRouteRevision(),FiskAtlas::Find(5)->Point); }
            }
        }
        if(B->GetLegSerial()>Smoke.ExplorationLeg && B->IsTravelling())
        { if(B->GetTravelVoteCount()!=0 || B->UsesTravelCard()) { Fail(TEXT("votes_survived_new_leg"));return; }Smoke.ExplorationLeg=B->GetLegSerial();Shot(TEXT("moving-between-grounds"));Smoke.ExplorationStage=7; }
    }
    else if(Smoke.ExplorationStage==7 && Progress==100)
    {
        if(FVector::Dist2D(B->GetActorLocation(),FiskAtlas::Find(5)->Point)>5) { Fail(TEXT("transfer_arrived_wrong_point"));return; }
        Smoke.ExplorationAt=Real;Smoke.ExplorationStage=71;
    }
    else if(Smoke.ExplorationStage==71 && Real-Smoke.ExplorationAt>=3)
    {
        // Leave enough time for remote half-second probes to observe arrival before the next leg.
        if(Host)
        {
            if(!Smoke.SkipperMode.IsEmpty()) { TActorIterator<AFiskVoyagePlan> V(GetWorld());if(V) { const auto R=V->GetRoute();BoatCommands->ServerVoyageNext(R.Day,R.Revision,B->GetRouteRevision()); } }
            else { BoatCommands->ServerSelectSeaPoint(B->GetRouteRevision(),FiskAtlas::Find(7)->Point); }
            if(B->GetLegSerial()<=Smoke.ExplorationLeg || B->GetDestination()!=FiskAtlas::Find(7)->Point) { Fail(TEXT("second_transfer_not_started"));return; }
        }
        Smoke.ExplorationAt=Real;Smoke.ExplorationStage=70;
    }
    else if(Smoke.ExplorationStage==70 && Real-Smoke.ExplorationAt>=2 && B->GetLegSerial()>Smoke.ExplorationLeg)
    {
        if(B->GetDestination()!=FiskAtlas::Find(7)->Point) { Fail(TEXT("second_transfer_destination_wrong"));return; }
        if(Host)
        { if(!B->IsTravelling() || Progress>=100) { Fail(TEXT("second_transfer_not_outbound"));return; }Key(EKeys::R); }
        Smoke.ExplorationStage=8;
    }
    if(Smoke.ExplorationStage==8 && Progress==200)
    { Shot(TEXT("home-after-interrupted-leg"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTISTOP_PASSED player=%d arrival=%d"),Self,B->GetMarinaArrivalMs(Day->GetElapsedMilliseconds()));Smoke.ExplorationStage=80; }
    if(Smoke.ExplorationStage>=3 && Day->GetDayNumber()==1 && Now<(HasAuthority()?1020.0:1018.0))
    {
        const auto Rows=FiskSeaPassport::Build(GI->GetJournal(),GI->Expeditions);
        for(const auto& R:Rows) { if(R.Current!=0) { Fail(TEXT("passport_leaked_today"));return; } }
        if(Now>=960 && Smoke.ExplorationMode==TEXT("offline") && Smoke.ExplorationStage==80)
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPLORATION_DISCONNECT_READY at=%.3f"),Now);Smoke.ExplorationStage=81; }
    }
    if(Smoke.ExplorationStage==80 && Day->GetDayNumber()==1 && Now>=1050)
    {
        const auto Rows=FiskSeaPassport::Build(GI->GetJournal(),GI->Expeditions);
        if(GI->GetJournal().GetEntries().Num()!=1 || GI->Expeditions.Num()!=1 || !Rows[0].Complete() || !Rows[9].Complete()) { Fail(TEXT("confirmed_passport_missing"));return; }
        if(IsCareerVisible() || IsResultsVisible() || IsHistoryVisible() || IsHelpVisible()) { Key(EKeys::Escape); }
        Key(EKeys::L);Key(EKeys::F10);Shot(TEXT("passport-after17"));Smoke.ExplorationAt=Real;Smoke.ExplorationStage=82;
    }
    else if(Smoke.ExplorationStage==82 && Real-Smoke.ExplorationAt>=1) { Key(EKeys::L);Smoke.ExplorationStage=83; }
    if(Solo && Day->GetDayNumber()==1 && Now>=1200 && Smoke.ExplorationStage==83) { Key(EKeys::N);return; }
    if(Day->GetDayNumber()==2 && Now>=35 && Smoke.ExplorationStage==83)
    { if(B->GetTravelVoteCount()!=0 || B->UsesTravelCard() || GI->GetJournal().GetEntries().Num()!=1 || GI->Expeditions.Num()!=1) { Fail(TEXT("day2_or_duplicate"));return; }Done(); }
}
#endif
