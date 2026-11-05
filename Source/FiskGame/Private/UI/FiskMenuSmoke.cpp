#include "UI/FiskFrontEnd.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameModeBase.h"
#include "TimerManager.h"

void UFiskFrontEnd::SmokeKey(FKey Key)
{
    APlayerController* PC=CastChecked<APlayerController>(GetOwner());
    FInputKeyEventArgs Args;
    Args.InputDevice=FInputDeviceId::CreateFromInternalId(0); Args.Key=Key; Args.Event=IE_Pressed;
    PC->InputKey(Args); Args.Event=IE_Released; PC->InputKey(Args);
}
void UFiskFrontEnd::SmokeSelect(int32 Row)
{
    SmokeKey(EKeys::Home); for (int32 I=0;I<Row;++I) { SmokeKey(EKeys::Down); } SmokeKey(EKeys::Enter);
}
void UFiskFrontEnd::SmokeAddress(const FString& Address)
{
    Model.Selected=0;
    FInputKeyEventArgs Args; Args.InputDevice=FInputDeviceId::CreateFromInternalId(0); Args.Key=EKeys::LeftControl; Args.Event=IE_Pressed;
    CastChecked<APlayerController>(GetOwner())->InputKey(Args); SmokeKey(EKeys::A);
    Args.Event=IE_Released; CastChecked<APlayerController>(GetOwner())->InputKey(Args);
    const FKey Digits[]={EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
    for (TCHAR C:Address)
    {
        if(C==TEXT('.')) { SmokeKey(EKeys::Period); }
        else if(C==TEXT(':'))
        {
            Args.Key=EKeys::LeftShift; Args.Event=IE_Pressed; CastChecked<APlayerController>(GetOwner())->InputKey(Args);
            SmokeKey(EKeys::Semicolon); Args.Event=IE_Released; CastChecked<APlayerController>(GetOwner())->InputKey(Args);
        }
        else { SmokeKey(Digits[C-TEXT('0')]); }
    }
}
void UFiskFrontEnd::Capture(const FString& Name)
{
    if (ScreenshotDirectory.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
    IFileManager::Get().MakeDirectory(*ScreenshotDirectory,true);
    FScreenshotRequest::RequestScreenshot(FPaths::Combine(ScreenshotDirectory,Name+TEXT(".png")),false,false);
}
void UFiskFrontEnd::RunMenuSmoke()
{
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    int32& Step=GI->MenuSmokeStep;
    const double Now=FPlatformTime::Seconds();
    auto Fail=[&](const TCHAR* Why) { UE_LOG(LogTemp,Error,TEXT("FISKELAGET_MENU_TEST_FAILED %s step=%d"),Why,Step); };
    if(SmokeCase==TEXT("solo"))
    {
        const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
        if(GetNetMode()==NM_Standalone)
        {
            if(Step==0)
            {
                if(!Day || Day->HasStarted() || Model.Page!=EFiskMenuPage::Home) { Fail(TEXT("solo_menu_started_day"));return; }
                Capture(TEXT("solo-menu"));Step=1;SmokeSelect(0);
            }
            return;
        }
        TActorIterator<AFiskStarterBoat> Boats(GetWorld());
        const AGameModeBase* Mode=GetWorld()->GetAuthGameMode();
        if(!Day || !Boats || !PC->PlayerState || !Mode) { return; }
        if(Mode->GetNumPlayers()!=1) { Fail(TEXT("solo_requires_exactly_one_test_player"));return; }
        AFiskStarterBoat* Boat=*Boats;
        if(!Day->HasStarted()) { Fail(TEXT("solo_day_did_not_start"));return; }
        if(Step==1)
        {
            SmokeKey(EKeys::E);
            if(Boat->GetCrewCount()!=1 || Boat->GetCaptainId()!=PC->PlayerState->GetPlayerId()) { Fail(TEXT("solo_boarding"));return; }
            SmokeKey(EKeys::R);
            if(Day->GetElapsedSeconds()>=120 || Boat->GetTripProgress()!=AFiskStarterBoat::AtDock) { Fail(TEXT("solo_early_departure"));return; }
            GI->SmokeClockAt=Day->GetElapsedMilliseconds();GI->SmokeAt=Now;
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SOLO_STARTED players=1"));Step=2;SmokeKey(EKeys::Escape);
        }
        else if(Step==2 && Now-GI->SmokeAt>=2.0)
        {
            if(Day->GetElapsedMilliseconds()<GI->SmokeClockAt+1000) { Fail(TEXT("solo_menu_paused_day"));return; }
            SmokeKey(EKeys::Escape);Capture(TEXT("solo-ready"));Step=3;
        }
        else if(Step==3 && Day->GetElapsedSeconds()>=120) { SmokeKey(EKeys::R);Step=4; }
        else if(Step==4 && Boat->GetTripProgress()==AFiskStarterBoat::AtFishingSpot)
        { SmokeKey(EKeys::F);Step=5; }
        else if(Step==5 && PC->GetRod()->GetState()==EFiskRodState::Fishing)
        { Capture(TEXT("solo-fishing"));GI->SmokeAt=Now;Step=6; }
        else if(Step==6 && Now-GI->SmokeAt>=2.0) { SmokeKey(EKeys::R);Step=7; }
        else if(Step==7 && Boat->GetTripProgress()==AFiskStarterBoat::AtMarina)
        {
            Capture(TEXT("solo-home"));Step=8;
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_SOLO_TEST_PASSED players=1 crew=1"));
            GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);
        }
        return;
    }
    if(GetNetMode()==NM_Standalone || Model.Page==EFiskMenuPage::Connecting || SmokeCase==TEXT("failure"))
    {
        if(SmokeCase==TEXT("host"))
        {
            if(Step==0) { Capture(TEXT("home")); SmokeKey(EKeys::E); Step=1; }
            else if(Step==1)
            {
                TActorIterator<AFiskStarterBoat> Boat(GetWorld());
                if(!Boat || Boat->GetCrewCount()!=0) { Fail(TEXT("menu_key_reached_gameplay")); return; }
                UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_INPUT_ISOLATED"));
                SmokeSelect(1); SmokeAddress(TEXT("999.0.0.1")); SmokeKey(EKeys::Enter); Step=2;
            }
            else if(Step==2)
            {
                if(Model.Page!=EFiskMenuPage::Join || Model.Message.IsEmpty()) { Fail(TEXT("invalid_address")); return; }
                Capture(TEXT("invalid-address")); UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_VALIDATION_PASSED"));
                Step=21;
            }
            else if(Step==21) { SmokeKey(EKeys::Escape); SmokeSelect(2); Step=3; }
            else if(Step==3) { Capture(TEXT("settings-before")); Step=31; }
            else if(Step==31) { SmokeSelect(0); SmokeSelect(1); Step=4; }
            else if(Step==4)
            {
                if(GI->bSound || GI->FrameRate!=30) { Fail(TEXT("settings_not_applied")); return; }
                Capture(TEXT("settings-after")); Step=41;
            }
            else if(Step==41) { SmokeSelect(2); Step=5; }
            else if(Step==5) { Step=10; SmokeSelect(0); }
            else if(Step>=100 && Step<101)
            {
                Capture(TEXT("returned-home")); UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_RETURNED_HOME")); Step=101;
            }
        }
        else if(SmokeCase==TEXT("join"))
        {
            if(Step==0)
            {
                if(GI->bSound || GI->FrameRate!=30) { Fail(TEXT("settings_not_persistent")); return; }
                Capture(TEXT("home")); UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_SETTINGS_PERSISTED"));
                if(Buttons.Num()!=4) { Fail(TEXT("missing_hit_regions")); return; }
                Step=1;
            }
            else if(Step==1) { Click(Buttons[1].GetCenter()); SmokeAddress(TEXT("127.0.0.1")); Capture(TEXT("join")); Step=2; }
            else if(Step==2) { Step=10; SmokeSelect(1); }
            else if(Step>=50 && Step<51)
            {
                if(Model.Message.IsEmpty()) { Fail(TEXT("missing_disconnect_message")); return; }
                Capture(TEXT("disconnect")); UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_DISCONNECT_RECOVERED")); Step=51;
            }
        }
        else if(SmokeCase==TEXT("failure"))
        {
            if(Step==0) { SmokeSelect(1); SmokeAddress(TEXT("127.0.0.1:17784")); Step=1; SmokeSelect(1); Capture(TEXT("connecting")); }
            else if(Step==1)
            {
                if (Model.Page == EFiskMenuPage::Home) { SmokeSelect(1); SmokeAddress(TEXT("127.0.0.1:17784")); SmokeSelect(1); }
                SmokeKey(EKeys::Escape); Step=2;
            }
            else if(Step==2)
            {
                if(Model.Page!=EFiskMenuPage::Home || Model.Message.IsEmpty()) { Fail(TEXT("cancel_did_not_recover")); return; }
                if (const FWorldContext* Context = GEngine->GetWorldContextFromWorld(GetWorld()))
                {
                    if (!Context->TravelURL.IsEmpty() || Context->PendingNetGame) { Fail(TEXT("cancel_left_pending_travel")); return; }
                }
                Capture(TEXT("cancelled")); Step=21;
            }
            else if(Step==21) { SmokeSelect(1); SmokeAddress(TEXT("127.0.0.1:17784")); Step=3; SmokeSelect(1); }
            else if(Step==3 && Model.Page==EFiskMenuPage::Home && !Model.Message.IsEmpty())
            {
                Capture(TEXT("connection-failed")); UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_FAILURE_PASSED")); Step=4;
            }
            else if(Step==4) { SmokeSelect(3); }
        }
        return;
    }
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    TActorIterator<AFiskStarterBoat> Boats(GetWorld());
    if(!Day || !Boats || !PC->PlayerState) { return; }
    AFiskStarterBoat* Boat=*Boats;
    if(SmokeCase==TEXT("host") && Step==10) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_HOST_READY")); Step=11; }
    if(!Boat->IsAboard(PC->PlayerState->GetPlayerId())) { SmokeKey(EKeys::E); }
    if(!Day->HasStarted()) { return; }
    if(SmokeCase==TEXT("host") && Step==11 && Day->GetElapsedMilliseconds()>=30000)
    {
        GI->SmokeAt=Now; GI->SmokeClockAt=Day->GetElapsedMilliseconds(); SmokeKey(EKeys::Escape); Capture(TEXT("session")); Step=12;
    }
    else if(SmokeCase==TEXT("host") && Step==12 && Now-GI->SmokeAt>=2.0)
    {
        if(Day->GetElapsedMilliseconds()<GI->SmokeClockAt+1000) { Fail(TEXT("menu_paused_day")); return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_CLOCK_CONTINUED")); SmokeKey(EKeys::Escape); Step=13;
    }
    if(IsVisible()) { return; }
    if(Boat->GetCaptainId()==PC->PlayerState->GetPlayerId() && Boat->GetCrewCount()==2
        && Boat->GetTripProgress()==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()>=120) { SmokeKey(EKeys::R); }
    if(Boat->GetTripProgress()==AFiskStarterBoat::AtFishingSpot && PC->GetRod()->GetState()==EFiskRodState::Ready) { SmokeKey(EKeys::F); }
    if(SmokeCase==TEXT("join") && PC->GetRod()->GetState()==EFiskRodState::Fishing && Step<50)
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_CLIENT_GAMEPLAY_PASSED")); Step=50; }
    if(SmokeCase==TEXT("host") && Step==13)
    {
        int32 Rods=0;
        for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
        {
            const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());
            if(Other && Other->GetRod()->GetState()==EFiskRodState::Fishing) { ++Rods; }
        }
        if(Rods==2)
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_GAMEPLAY_PASSED crew=2 rods=2")); GI->SmokeAt=Now; Step=14; }
    }
    else if(SmokeCase==TEXT("host") && Step==14 && Now-GI->SmokeAt>=2.0)
    {
        Step=100; SmokeKey(EKeys::Escape); SmokeSelect(2);
    }
}
#endif
