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
    auto Capture=[&](const TCHAR* Name){
        if(ContentCaptureDir.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*ContentCaptureDir,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(ContentCaptureDir,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CONTENT_FAILED player=%d stage=%d %s"),Self,ContentStage,Why);GetWorldTimerManager().ClearTimer(ContentTimer);};
    auto Matches=[&](int32 B,int32 D,int32 P){const FFiskRigSettings& R=CurrentRod.GetRig();return R.Bait==B && R.Depth==D && R.Program==P;};
    if(ContentMode==TEXT("preset-save") || ContentMode==TEXT("preset-load"))
    {
        UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
        if(ContentStage==0)
        {
            if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
            if(ContentMode==TEXT("preset-save")) { RequestRig(1,40,1);ContentStage=1; }
            else
            {
                const FFiskSavedRig* Saved=GI->RigPresets.Get(0);
                if(!Saved || !Saved->bExists || Saved->Rig.Bait!=1 || Saved->Rig.Depth!=40 || Saved->Rig.Program!=1)
                { Fail(TEXT("preset_not_persistent"));return; }
                Key(EKeys::G);Key(EKeys::One);Key(EKeys::F9);ContentStage=5;
            }
        }
        else if(ContentStage==1 && Matches(1,40,1))
        {
            Key(EKeys::G);Key(EKeys::One);Key(EKeys::F5);Capture(TEXT("preset-saved"));
            const FFiskSavedRig* Saved=GI->RigPresets.Get(0);
            if(!Saved || !Saved->bExists || Saved->Rig.Depth!=40) { Fail(TEXT("preset_save_failed"));return; }
            RequestRig(2,20,2);ContentStage=2;
        }
        else if(ContentStage==2 && Matches(2,20,2)) { Key(EKeys::F9);ContentStage=3; }
        else if(ContentStage==3 && Matches(1,40,1))
        {
            Capture(TEXT("preset-applied"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_PRESET_SAVED"));GetWorldTimerManager().ClearTimer(ContentTimer);
        }
        else if(ContentStage==5 && Matches(1,40,1))
        {
            Capture(TEXT("preset-reloaded"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_PRESET_LOADED"));GetWorldTimerManager().ClearTimer(ContentTimer);
        }
        return;
    }
    if(ContentMode!=TEXT("day")) { Fail(TEXT("unknown_test_mode"));return; }
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(!Settlement) { return; }
    const FFiskSettlementView& View=Settlement->GetView();
    if((Now<1019 && View.bRevealed) || (!View.bRevealed && !View.Results.IsEmpty())) { Fail(TEXT("early_result_exposure"));return; }
    const int32 Depth=Host?80:90;
    if(ContentStage==0)
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=2) { return; }
        if(Host) { Key(EKeys::M);Key(EKeys::Home);Key(EKeys::Down);Key(EKeys::Enter); }
        ContentStage=1;
    }
    else if(ContentStage==1)
    {
        if(Boat->GetGroundId()!=1 || Boat->GetRouteRevision()<1) { return; }
        if(GetSeaChart()->IsVisible()) { Key(EKeys::M); }
        RequestRig(0,Depth,0);ContentStage=2;
    }
    else if(ContentStage==2 && Matches(0,Depth,0))
    { Key(EKeys::F1);Capture(TEXT("help"));ContentAt=Now;ContentStage=3; }
    else if(ContentStage==3 && Now-ContentAt>=1)
    { Key(EKeys::F1);Key(EKeys::F2);Capture(TEXT("results-locked"));ContentAt=Now;ContentStage=4; }
    else if(ContentStage==4 && Now-ContentAt>=1)
    { Key(EKeys::F2);Key(EKeys::R);ContentStage=5; }
    else if(ContentStage==5 && !GetActionFeedback().IsEmpty())
    {
        Capture(TEXT("departure-feedback"));ContentStage=6;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_READY player=%d"),Self);
    }
    else if(ContentStage==6)
    {
        if(Host && Progress==AFiskStarterBoat::AtDock && Now>=120) { Key(EKeys::R); }
        if(Progress>=0 && Progress<100) { ContentStage=7; }
    }
    else if(ContentStage==7 && Progress==100) { Key(EKeys::F);ContentStage=8; }
    else if(ContentStage==8 && CurrentRod.GetState()==EFiskRodState::Fighting)
    { Capture(TEXT("fish-fight"));ContentFightAt=Now;ContentStage=9; }
    else if(ContentStage==9)
    {
        if(CurrentRod.GetState()==EFiskRodState::Lost) { Fail(TEXT("fight_lost"));return; }
        if(CurrentRod.GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);ContentStage=10;return; }
        auto Input=[&](FKey K,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=Event;InputKey(A);};
        const bool Lift=!CurrentRod.IsSurging() && static_cast<int32>((Now-ContentFightAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);
        Input(EKeys::RightMouseButton,!CurrentRod.IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(ContentStage==10 && CurrentRod.GetState()==EFiskRodState::Landed)
    { Capture(TEXT("fish-landed"));ContentAt=Now;ContentStage=11; }
    else if(ContentStage==11 && Now-ContentAt>=4)
    {
        if(Host && Progress==100)
        {
            int32 Landed=0;
            for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            {const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other){Landed+=Other->GetRod()->GetTestLandingCount();}}
            if(Landed>=2) { Key(EKeys::R); }
        }
        if(Progress>100) { ContentStage=12; }
    }
    else if(ContentStage==12 && Progress==200)
    {
        if(Boat->GetMarinaArrivalMs(Day->GetElapsedMilliseconds())<0) { Fail(TEXT("missing_arrival"));return; }
        Capture(TEXT("home"));ContentStage=13;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_RETURNED player=%d"),Self);
    }
    else if(ContentStage==13)
    { Key(EKeys::F2);Capture(TEXT("results-before17"));ContentAt=Now;ContentStage=14; }
    else if(ContentStage==14 && Now-ContentAt>=1)
    { Key(EKeys::F2);ContentStage=15; }
    else if(ContentStage==15 && Now>=1020 && View.bRevealed && IsResultsVisible())
    {
        if(View.Results.Num()!=2) { Fail(TEXT("wrong_result_rows"));return; }
        TSet<FGuid> Ids;
        for(const FFiskDayResult& Row:View.Results)
        {
            if(Row.DeliveredFish!=1 || Row.LostFish!=0 || Row.Catches.Num()!=1) { Fail(TEXT("wrong_catch_count"));return; }
            const FFiskCatchResult& Catch=Row.Catches[0];
            if(!Catch.bDelivered || Catch.Fish.Species!=1 || !FiskCatches::IsValid(Catch.Fish)
                || Row.SessionBalance!=Catch.Fish.Value || Row.DeliveredWeightGrams!=Catch.Fish.WeightGrams)
            { Fail(TEXT("wrong_catch_details_or_payment"));return; }
            Ids.Add(Catch.CatchId);
        }
        if(Ids.Num()!=2) { Fail(TEXT("duplicate_catch_id"));return; }
        Capture(TEXT("results-revealed"));ContentStage=16;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_REVEAL_PASSED player=%d"),Self);
    }
    else if(ContentStage==16 && Day->GetElapsedMilliseconds()==1200000)
    {
        Capture(TEXT("day-finished"));ContentStage=17;
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CONTENT_DAY_PASSED player=%d ms=1200000"),Self);
        GetWorldTimerManager().ClearTimer(ContentTimer);
    }
}
#endif
