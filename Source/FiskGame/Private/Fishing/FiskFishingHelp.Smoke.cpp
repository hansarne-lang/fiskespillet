#include "Fishing/FiskFishingHelp.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskBiteAlert.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"

void UFiskFishingHelp::BeginPlay()
{
    Super::BeginPlay();
#if WITH_DEV_AUTOMATION_TESTS
    if(CastChecked<AFiskBoardingController>(GetOwner())->IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskHelpSmoke="),SmokeMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),SmokeDir);
        bSmokeMute=FParse::Param(FCommandLine::Get(),TEXT("FiskHelpMute"));
        GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskFishingHelp::RunSmoke,.25f,true,3.f);
    }
#endif
}
#if WITH_DEV_AUTOMATION_TESTS
void UFiskFishingHelp::RunSmoke()
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();const auto* Boat=PC->FindStarterBoat();
    if(!Day || !Day->HasStarted() || !PC->IsCareerReady() || !Boat || !PC->PlayerState) { return; }
    const int32 Self=PC->PlayerState->GetPlayerId(),Progress=Boat->GetTripProgress();
    const bool Solo=SmokeMode==TEXT("solo"),Host=SmokeMode!=TEXT("guest");const double Now=GetWorld()->GetRealTimeSeconds();
    auto* Bell=PC->FindComponentByClass<UFiskBiteAlert>();
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;PC->InputKey(A);A.Event=IE_Released;PC->InputKey(A);};
    const FKey Aa=FInputKeyManager::Get().GetKeyFromCodes(0,0x00c5);
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_HELP_FAILED mode=%s stage=%d %s"),*SmokeMode,SmokeStage,Why);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(SmokeDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorld()->GetTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.25f,false);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELP_PASSED mode=%s muted=%d cash=%lld alerts=%d consumed=%d"),*SmokeMode,bSmokeMute,static_cast<long long>(GI->Career.Cash),Bell?Bell->GetAlertCount():-1,Bell?Bell->GetConsumedCount():-1);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    if(Now>650) { Fail(TEXT("normal_test_timeout"));return; }
    if(SmokeMode==TEXT("reload"))
    { if(bAssisting || GI->Career.Cash!=3000000 || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("test_help_persisted_or_early_reward"));return; }Done();return; }
    if(SmokeStage==0)
    {
        SmokeCash=GI->Career.Cash;if(SmokeCash!=3000000 || !Bell) { Fail(TEXT("fresh_profile_or_bell_missing"));return; }
        if(bSmokeMute) { GI->bSound=false; }
        PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber(),Boat->GetLegSerial());
        Key(Aa);if(bAssisting) { Fail(TEXT("assist_started_without_bite"));return; }
        Key(EKeys::E);SmokeAt=Now;SmokeStage=1;return;
    }
    if(SmokeStage==1)
    {
        if(!Boat->IsAboard(Self)) { if(Now-SmokeAt>10) { Fail(TEXT("boarding_failed")); }return; }
        if(Progress!=AFiskStarterBoat::AtDock) { Fail(TEXT("return_button_departed_from_dock"));return; }
        if(!Solo && Boat->GetCrewCount()!=2) { return; }
        if(Host && Boat->GetCaptainId()!=Self) { return; }
        SmokeSlot=Solo?1:0;if(PC->GetRodCapacity()<SmokeSlot+1) { Fail(TEXT("wrong_own_rod_capacity"));return; }
        PC->RequestRodSelection(SmokeSlot);SmokeStage=2;return;
    }
    if(SmokeStage==2)
    {
        if(PC->IsRodSelectionPending() || PC->GetSelectedRod()!=SmokeSlot) { return; }
        if(Host && Progress==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()>=120) { Key(EKeys::R); }
        if(Progress!=100) { return; }
        if(!Host) { PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber(),Boat->GetLegSerial()); }
        Key(EKeys::F);SmokeAt=Now;SmokeStage=3;return;
    }
    if(SmokeStage==3)
    {
        const auto* R=PC->GetRodAt(SmokeSlot);
        if(R->GetState()!=EFiskRodState::Fishing) { if(Now-SmokeAt>5) { Fail(TEXT("cast_failed")); }return; }
        if(Solo) { Key(EKeys::One); }SmokeStage=4;return;
    }
    if(SmokeStage==4)
    {
        if(PC->IsRodSelectionPending()) { return; }
        const auto* R=PC->GetRodAt(SmokeSlot);
        if(R->GetState()!=EFiskRodState::InitialRun && R->GetState()!=EFiskRodState::Fighting) { return; }
        if(Bell->GetAlertCount()!=1 || (!bSmokeMute && Bell->GetConsumedCount()!=1)) { return; }
        if(Solo && PC->GetSelectedRod()!=0) { Fail(TEXT("passive_bite_was_selected_early"));return; }
        Key(EKeys::M);
        int32 Width=0,Height=0;PC->GetViewportSize(Width,Height);const float Scale=FMath::Min(Width/1280.f,Height/720.f);
        PC->SetMouseLocation(FMath::RoundToInt(1080*Scale),FMath::RoundToInt(195*Scale));
        SmokeAt=Now;SmokeStage=41;return;
    }
    if(SmokeStage==41 && Now-SmokeAt>=.5)
    {
        if(HasGameView()) { Fail(TEXT("chart_did_not_open"));return; }
        Key(EKeys::M);SmokeAt=Now;SmokeStage=42;return;
    }
    if(SmokeStage==42 && Now-SmokeAt>=.5)
    {
        auto* View=GetWorld()->GetGameViewport();FVector2D Mouse,Size;
        if(!HasGameView() || !View || !View->GetMousePosition(Mouse)) { Fail(TEXT("chart_close_cursor_missing"));return; }
        View->GetViewportSize(Size);const float Scale=FMath::Min(Size.X/1280.f,Size.Y/720.f);
        if(FiskQuickControls::ReturnBox(Scale).IsInside(Mouse) || FiskQuickControls::AssistBox(Scale).IsInside(Mouse)) { Fail(TEXT("chart_cursor_left_on_game_button"));return; }
        Key(EKeys::LeftMouseButton);
        if(Boat->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { Fail(TEXT("chart_cursor_returned_instead_of_pumping"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELP_CHART_CURSOR_PASSED mode=%s"),*SmokeMode);
        Key(Aa);SmokeAt=Now;SmokeStage=5;return;
    }
    if(SmokeStage==5 && Now-SmokeAt>=1)
    {
        if(!bAssisting || PC->GetSelectedRod()!=SmokeSlot || PC->IsRodSelectionPending()) { Fail(TEXT("Aa_did_not_select_bitten_own_rod"));return; }
        Key(Aa);if(bAssisting) { Fail(TEXT("Aa_did_not_stop"));return; }
        Key(Aa);Key(EKeys::RightMouseButton);if(bAssisting) { Fail(TEXT("manual_input_did_not_take_over"));return; }
        int32 Width=0,Height=0;PC->GetViewportSize(Width,Height);const float Scale=FMath::Min(Width/1280.f,Height/720.f);
        PC->SetMouseLocation(FMath::RoundToInt(1080*Scale),FMath::RoundToInt(270*Scale));Key(EKeys::LeftMouseButton);
        if(!bAssisting) { Fail(TEXT("help_button_restart_failed"));return; }
        // An earlier cast cannot control the current fight through the ordinary RPC.
        PC->ServerActOnSelectedRod(EFiskRodAction::Land,PC->Selection,FGuid::NewGuid());
        Shot(TEXT("ai-help-active"));SmokeAt=Now;SmokeStage=6;return;
    }
    if(SmokeStage==6)
    {
        const auto* R=PC->GetRodAt(SmokeSlot);
        if(R->GetState()==EFiskRodState::Lost) { Fail(TEXT("automatic_fight_lost"));return; }
        if(R->GetState()!=EFiskRodState::Landed || bAssisting) { return; }
        if(Bell->GetAlertCount()!=1 || (!bSmokeMute && Bell->GetConsumedCount()!=1) || (bSmokeMute && Bell->GetConsumedCount()!=0)) { Fail(TEXT("bell_repeat_or_mute_wrong"));return; }
        if(GI->Career.Cash!=SmokeCash || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("early_money_or_private_reveal"));return; }
        if(bSmokeMute) { GI->bSound=true; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HELP_FISH_PASSED mode=%s slot=%d"),*SmokeMode,SmokeSlot);
        Shot(TEXT("ai-fish-landed"));SmokeAt=Now;SmokeStage=7;return;
    }
    if(SmokeStage==7 && Now-SmokeAt>=2)
    {
        if(bSmokeMute && Bell->GetConsumedCount()!=0) { Fail(TEXT("unmute_replayed_old_bite"));return; }
        if(!Host) { SmokeStage=9;return; }
        if(!Solo)
        {
            int32 Landed=0;for(TActorIterator<AFiskBoardingController> Other(GetWorld());Other;++Other) { if(Other->GetRodAt(0)->GetState()==EFiskRodState::Landed) { ++Landed; } }
            if(Landed!=2) { return; }
        }
        PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision()-1,Day->GetDayNumber(),Boat->GetLegSerial());
        PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber()-1,Boat->GetLegSerial());
        PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber(),Boat->GetLegSerial()-1);
        SmokeAt=Now;SmokeStage=8;return;
    }
    if(SmokeStage==8 && Now-SmokeAt>=1)
    {
        if(Progress!=100) { Fail(TEXT("stale_return_changed_boat"));return; }
        int32 W=0,H=0;PC->GetViewportSize(W,H);const float S=FMath::Min(W/1280.f,H/720.f);
        PC->SetMouseLocation(FMath::RoundToInt(1080*S),FMath::RoundToInt(195*S));Key(EKeys::LeftMouseButton);
        SmokeReturnAt=Now;SmokeAt=Now;SmokeStage=9;return;
    }
    if(SmokeStage==9)
    {
        if(Host && Progress==100 && Now-SmokeAt>4) { Fail(TEXT("return_click_did_not_start_home"));return; }
        if(Progress>100 && Progress<200 && !bSmokeShot) { Shot(TEXT("return-button-underway"));bSmokeShot=true; }
        if(Progress!=AFiskStarterBoat::AtMarina) { return; }
        if(Host && Now-SmokeReturnAt<Boat->GetLegTravelMs()/1000.0-1) { Fail(TEXT("return_skipped_physical_travel"));return; }
        if(bAssisting || GI->Career.Cash!=SmokeCash || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("return_changed_unsold_income_or_left_help_on"));return; }
        PC->GetBoatCommands()->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber(),Boat->GetLegSerial());Key(EKeys::R);
        Shot(TEXT("return-button-home"));SmokeAt=Now;SmokeStage=10;return;
    }
    if(SmokeStage==10 && Now-SmokeAt>=1)
    { if(Progress!=200) { Fail(TEXT("repeated_return_departed_again"));return; }Done(); }
}
#endif
