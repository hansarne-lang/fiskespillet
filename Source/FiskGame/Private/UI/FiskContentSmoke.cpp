#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunContentSmoke()
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const AFiskStarterBoat* Boat=FindStarterBoat();
    if(!Day || !Day->HasStarted() || !Boat || !PlayerState) { return; }
    const int32 Self=PlayerState->GetPlayerId();const bool Host=GetNetMode()!=NM_Client;
    const double Now=Day->GetElapsedRealSeconds();
    const int32 Progress=Boat->GetTripProgress();
    const UFiskPrototypeRod& CurrentRod=*GetRod();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    // Returns false while an earlier screenshot is still pending; callers that must have the
    // image stay on their stage and try again on the next tick (#124).
    auto Capture=[&](const TCHAR* Name)->bool{
        if(Smoke.ContentCaptureDir.IsEmpty()) { return true; }
        if(FScreenshotRequest::IsScreenshotRequested()) { return false; }
        IFileManager::Get().MakeDirectory(*Smoke.ContentCaptureDir,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Smoke.ContentCaptureDir,FString(Name)+TEXT(".png")),false,false);
        return true;
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CONTENT_FAILED player=%d stage=%d %s"),Self,Smoke.ContentStage,Why);GetWorldTimerManager().ClearTimer(Smoke.ContentTimer);};
    auto Matches=[&](int32 B,int32 D,int32 P){const FFiskRigSettings& R=CurrentRod.GetRig();return R.Bait==B && R.Depth==D && R.Program==P;};
    if(Smoke.ContentMode==TEXT("preset-save") || Smoke.ContentMode==TEXT("preset-load"))
    {
        UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
        if(Smoke.ContentStage==0)
        {
            if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
            if(Smoke.ContentMode==TEXT("preset-save")) { RequestRig(1,40,1);Smoke.ContentStage=1; }
            else
            {
                const FFiskSavedRig* Saved=GI->RigPresets.Get(0);
                if(!Saved || !Saved->bExists || Saved->Rig.Bait!=1 || Saved->Rig.Depth!=40 || Saved->Rig.Program!=1)
                { Fail(TEXT("preset_not_persistent"));return; }
                Key(EKeys::G);Key(EKeys::One);Key(EKeys::F9);Smoke.ContentStage=5;
            }
        }
        else if(Smoke.ContentStage==1 && Matches(1,40,1))
        {
            Key(EKeys::G);Key(EKeys::One);Key(EKeys::F5);Capture(TEXT("preset-saved"));
            const FFiskSavedRig* Saved=GI->RigPresets.Get(0);
            if(!Saved || !Saved->bExists || Saved->Rig.Depth!=40) { Fail(TEXT("preset_save_failed"));return; }
            RequestRig(2,20,2);Smoke.ContentStage=2;
        }
        else if(Smoke.ContentStage==2 && Matches(2,20,2)) { Key(EKeys::F9);Smoke.ContentStage=3; }
        else if(Smoke.ContentStage==3 && Matches(1,40,1))
        {
            Capture(TEXT("preset-applied"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_PRESET_SAVED"));GetWorldTimerManager().ClearTimer(Smoke.ContentTimer);
        }
        else if(Smoke.ContentStage==5 && Matches(1,40,1))
        {
            Capture(TEXT("preset-reloaded"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_PRESET_LOADED"));GetWorldTimerManager().ClearTimer(Smoke.ContentTimer);
        }
        return;
    }
    if(Smoke.ContentMode!=TEXT("day")) { Fail(TEXT("unknown_test_mode"));return; }
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(!Settlement) { return; }
    const FFiskSettlementView& View=Settlement->GetView();
    if((Now<1019 && View.bRevealed) || (!View.bRevealed && !View.Results.IsEmpty())) { Fail(TEXT("early_result_exposure"));return; }
    const int32 Depth=Host?80:90;
    if(Smoke.ContentStage==0)
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=2) { return; }
        if(Host) { Key(EKeys::M);Key(EKeys::Home);Key(EKeys::Down);Key(EKeys::Enter); }
        Smoke.ContentStage=1;
    }
    else if(Smoke.ContentStage==1)
    {
        if(Boat->GetGroundId()!=1 || Boat->GetRouteRevision()<1) { return; }
        if(GetSeaChart()->IsVisible()) { Key(EKeys::M); }
        RequestRig(0,Depth,0);Smoke.ContentStage=2;
    }
    else if(Smoke.ContentStage==2 && Matches(0,Depth,0))
    { if(FScreenshotRequest::IsScreenshotRequested()) { return; }Key(EKeys::F1);Capture(TEXT("help"));Smoke.ContentAt=Now;Smoke.ContentStage=3; }
    else if(Smoke.ContentStage==3 && Now-Smoke.ContentAt>=1)
    { if(FScreenshotRequest::IsScreenshotRequested()) { return; }Key(EKeys::F1);Key(EKeys::F2);Capture(TEXT("results-locked"));Smoke.ContentAt=Now;Smoke.ContentStage=4; }
    else if(Smoke.ContentStage==4 && Now-Smoke.ContentAt>=1)
    { Key(EKeys::F2);Key(EKeys::R);Smoke.ContentStage=5; }
    else if(Smoke.ContentStage==5 && !GetActionFeedback().IsEmpty())
    {
        if(!Capture(TEXT("departure-feedback"))) { return; }
        Smoke.ContentStage=6;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_READY player=%d"),Self);
    }
    else if(Smoke.ContentStage==6)
    {
        if(Host && Progress==AFiskStarterBoat::AtDock && Now>=120) { Key(EKeys::R); }
        if(Progress>=0 && Progress<100) { Smoke.ContentStage=7; }
    }
    else if(Smoke.ContentStage==7 && Progress==100) { Key(EKeys::F);Smoke.ContentStage=8; }
    else if(Smoke.ContentStage==8 && CurrentRod.GetState()==EFiskRodState::Fighting)
    { if(!Capture(TEXT("fish-fight"))) { return; }Smoke.ContentFightAt=Now;Smoke.ContentStage=9; }
    else if(Smoke.ContentStage==9)
    {
        if(CurrentRod.GetState()==EFiskRodState::Lost) { Fail(TEXT("fight_lost"));return; }
        if(CurrentRod.GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);Smoke.ContentStage=10;return; }
        auto Input=[&](FKey K,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Event;InputKey(A);};
        const bool Lift=!CurrentRod.IsSurging() && static_cast<int32>((Now-Smoke.ContentFightAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);
        Input(EKeys::RightMouseButton,!CurrentRod.IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(Smoke.ContentStage==10 && CurrentRod.GetState()==EFiskRodState::Landed)
    { if(!Capture(TEXT("fish-landed"))) { return; }Smoke.ContentAt=Now;Smoke.ContentStage=11; }
    else if(Smoke.ContentStage==11 && Now-Smoke.ContentAt>=4)
    {
        if(Host && Progress==100)
        {
            int32 Landed=0;
            for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            {const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other){Landed+=Other->GetRod()->GetTestLandingCount();}}
            if(Landed>=2) { Key(EKeys::R); }
        }
        if(Progress>100) { Smoke.ContentStage=12; }
    }
    else if(Smoke.ContentStage==12 && Progress==200)
    {
        if(Boat->GetMarinaArrivalMs(Day->GetElapsedMilliseconds())<0) { Fail(TEXT("missing_arrival"));return; }
        if(!Capture(TEXT("home"))) { return; }
        Smoke.ContentStage=13;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_RETURNED player=%d"),Self);
    }
    else if(Smoke.ContentStage==13)
    { if(FScreenshotRequest::IsScreenshotRequested()) { return; }Key(EKeys::F2);Capture(TEXT("results-before17"));Smoke.ContentAt=Now;Smoke.ContentStage=14; }
    else if(Smoke.ContentStage==14 && Now-Smoke.ContentAt>=1)
    { Key(EKeys::F2);Smoke.ContentStage=15; }
    else if(Smoke.ContentStage==15 && Now>=1020 && View.bRevealed && IsResultsVisible())
    {
        if(View.Results.Num()!=2) { Fail(TEXT("wrong_result_rows"));return; }
        TSet<FGuid> Ids;
        for(const FFiskDayResult& Row:View.Results)
        {
            if(Row.DeliveredFish!=1 || Row.LostFish!=0 || Row.Catches.Num()!=1) { Fail(TEXT("wrong_catch_count"));return; }
            const FFiskCatchResult& Catch=Row.Catches[0];
            if(!Catch.bDelivered || !FiskCatches::IsValid(Catch.Fish)
                || Row.SessionBalance!=Catch.Fish.Value || Row.DeliveredWeightGrams!=Catch.Fish.WeightGrams)
            { Fail(TEXT("wrong_catch_details_or_payment"));return; }
            Ids.Add(Catch.CatchId);
        }
        if(Ids.Num()!=2) { Fail(TEXT("duplicate_catch_id"));return; }
        if(!Capture(TEXT("results-revealed"))) { return; }
        Smoke.ContentStage=16;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_REVEAL_PASSED player=%d"),Self);
    }
    else if(Smoke.ContentStage==16 && Day->GetElapsedMilliseconds()==1200000)
    {
        if(!Capture(TEXT("day-finished"))) { return; }
        Smoke.ContentStage=17;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_DAY_PASSED player=%d ms=1200000"),Self);
        GetWorldTimerManager().ClearTimer(Smoke.ContentTimer);
    }
}
#endif
