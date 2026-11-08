#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunSessionSmoke()
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const AFiskStarterBoat* Boat=FindStarterBoat();
    if(!Day || !Day->HasStarted() || !Boat || !PlayerState) { return; }
    const int32 Self=PlayerState->GetPlayerId();const bool Host=GetNetMode()==NM_ListenServer;
    const double RealNow=FPlatformTime::Seconds();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Capture=[&](const TCHAR* Name){
        if(Smoke.SessionCaptureDir.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*Smoke.SessionCaptureDir,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Smoke.SessionCaptureDir,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_SESSION_FAILED player=%d stage=%d %s"),Self,Smoke.SessionStage,Why);GetWorldTimerManager().ClearTimer(Smoke.SessionTimer);};
    if(Smoke.SessionMode.StartsWith(TEXT("settings-")))
    {
        UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
        if(Smoke.SessionMode==TEXT("settings-save")) { GI->bSound=false;GI->FrameRate=30;GI->ApplySettings(true); }
        if(Smoke.SessionMode==TEXT("settings-default"))
        {
            if(!GI->bSound || GI->FrameRate!=60 || GI->RigPresets.Get(0)->bExists) { Fail(TEXT("profile_isolation"));return; }
        }
        else
        {
            FFiskUserSettings Saved;
            if(!Saved.Load(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("FiskelagetSettings.ini")))
                || Saved.bSound || Saved.FrameRate!=30 || GI->bSound || GI->FrameRate!=30)
            { Fail(TEXT("settings_not_persistent"));return; }
        }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SESSION_SETTINGS_PASSED mode=%s"),*Smoke.SessionMode);
        GetWorldTimerManager().ClearTimer(Smoke.SessionTimer);return;
    }
    if(Smoke.SessionMode!=TEXT("day")) { Fail(TEXT("unknown_mode"));return; }
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());if(!Settlement) { return; }
    const FFiskSettlementView& View=Settlement->GetView();
    const TArray<FFiskHistoryDay>& History=Settlement->GetHistory();
    // GameState clock and settlement use different actor channels. A valid server
    // reveal can arrive before the client's next clock snapshot. The wrapper also
    // checks the reveal against the host's actual 1020-second wall-clock start.
    if(Day->GetDayNumber()==1 && View.DayNumber==1 && !View.bRevealed && !History.IsEmpty())
    { Fail(TEXT("history_without_confirmed_reveal"));return; }
    if(Day->GetDayNumber()==2 && View.DayNumber==2 && (View.bRevealed || !View.Results.IsEmpty())) { Fail(TEXT("new_day_result_leak"));return; }
    if(Smoke.SessionStage==0)
    {
        ServerNextDay(1);Smoke.SessionStage=1; // Both host-early and remote requests must fail.
    }
    else if(Smoke.SessionStage==1 && Smoke.ContentStage>=17)
    {
        if(Smoke.SessionAt==0) { Smoke.SessionAt=RealNow;return; }
        if(RealNow-Smoke.SessionAt<1) { return; }
        if(History.Num()!=1 || History[0].Number!=1 || History[0].Rows.Num()!=2) { Fail(TEXT("missing_day_history"));return; }
        for(const FFiskHistoryRow& Row:History[0].Rows)
        { if(Row.Delivered!=1 || Row.Lost!=0 || Row.Income<=0 || Row.BestGrams<=0) { Fail(TEXT("wrong_history_summary"));return; } }
        Smoke.SessionOldCast=Rod->GetCastId();Smoke.SessionOldRoute=Boat->GetRouteRevision();
        if(IsResultsVisible()) { Key(EKeys::F2); }Key(EKeys::F3);Capture(TEXT("history-day1"));
        Smoke.SessionAt=RealNow;Smoke.SessionStage=2;
    }
    else if(Smoke.SessionStage==2 && RealNow-Smoke.SessionAt>=3)
    {
        Key(EKeys::F3);
        Key(EKeys::N); // Host advances; guest is denied even after 20:00.
        if(Host) { ServerNextDay(1); } // Delayed duplicate cannot reset twice.
        Smoke.SessionStage=3;
    }
    else if(Smoke.SessionStage==3 && Day->GetDayNumber()==2 && View.DayNumber==2 && Boat->GetTripProgress()==AFiskStarterBoat::AtDock)
    {
        if(Boat->GetCrewCount()!=2 || !Boat->IsAboard(Self) || Boat->GetGroundId()!=1 || Boat->GetRouteRevision()<=Smoke.SessionOldRoute
            || Rod->GetState()!=EFiskRodState::Ready || !Rod->GetCastId().IsValid() || Rod->GetCastId()==Smoke.SessionOldCast
            || Rod->GetRig().Depth!=(Host?80:90) || History.Num()!=1 || History[0].Number!=1 || Day->GetElapsedMilliseconds()>10000)
        { Fail(TEXT("bad_day_reset"));return; }
        if(Host) { ServerStartBoatTrip(Smoke.SessionOldRoute); }
        ServerConfigureRig(Smoke.SessionOldCast,Rod->GetRig().Revision,2,10,2);
        Capture(TEXT("day2-start"));Smoke.SessionAt=RealNow;Smoke.SessionStage=4;
    }
    else if(Smoke.SessionStage==4 && RealNow-Smoke.SessionAt>=1)
    { Key(EKeys::F2);Capture(TEXT("day2-results-locked"));Smoke.SessionAt=RealNow;Smoke.SessionStage=5; }
    else if(Smoke.SessionStage==5 && RealNow-Smoke.SessionAt>=1)
    { Key(EKeys::F2);Key(EKeys::F3);Capture(TEXT("day2-old-history"));Smoke.SessionAt=RealNow;Smoke.SessionStage=6; }
    else if(Smoke.SessionStage==6 && RealNow-Smoke.SessionAt>=1)
    {
        Key(EKeys::F3);Key(EKeys::R);
        if(Rod->GetRig().Depth!=(Host?80:90)) { Fail(TEXT("stale_rig_changed_new_day"));return; }
        Smoke.SessionStage=7;
    }
    else if(Smoke.SessionStage==7)
    {
        if(Host && Boat->GetTripProgress()==-1 && Day->GetElapsedSeconds()>=120) { Key(EKeys::R); }
        if(Boat->GetTripProgress()==100) { ServerCastRod(Smoke.SessionOldCast);Smoke.SessionAt=RealNow;Smoke.SessionStage=8; }
    }
    else if(Smoke.SessionStage==8 && RealNow-Smoke.SessionAt>=1)
    {
        if(Rod->GetState()!=EFiskRodState::Ready) { Fail(TEXT("old_cast_started_new_day"));return; }
        Key(EKeys::F);Smoke.SessionStage=9;
    }
    else if(Smoke.SessionStage==9 && Rod->GetState()==EFiskRodState::Fishing)
    {
        Capture(TEXT("day2-fishing"));Smoke.SessionAt=RealNow;Smoke.SessionStage=10;
    }
    else if(Smoke.SessionStage==10)
    {
        if(Host && RealNow-Smoke.SessionAt>=3)
        {
            int32 Fishing=0;
            for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            {const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other && Other->GetRod()->GetState()==EFiskRodState::Fishing){++Fishing;}}
            if(Fishing==2) { Key(EKeys::R); }
        }
        if(Boat->GetTripProgress()==200)
        {
            if(History.Num()!=1 || View.bRevealed || !View.Results.IsEmpty() || Day->GetDayNumber()!=2) { Fail(TEXT("new_day_final_state"));return; }
            Capture(TEXT("day2-home"));Smoke.SessionStage=11;
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SESSION_PASSED player=%d day=2 ms=%d history=1 crew=2"),Self,Day->GetElapsedMilliseconds());
            GetWorldTimerManager().ClearTimer(Smoke.SessionTimer);
        }
    }
}
#endif
