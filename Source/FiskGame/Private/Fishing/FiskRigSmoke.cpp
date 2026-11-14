#include "UI/FiskRigPanel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskFrontEnd.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"
void UFiskRigPanel::Key(FKey Value)
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    FInputKeyEventArgs Args;Args.InputDevice=FInputDeviceId::CreateFromInternalId(0);Args.Key=Value;Args.Event=IE_Pressed;
    PC->InputKey(Args);Args.Event=IE_Released;PC->InputKey(Args);
}
void UFiskRigPanel::Capture(const FString& Name)
{
    if(CaptureDir.IsEmpty()) { return; }
    IFileManager::Get().MakeDirectory(*CaptureDir,true);
    FScreenshotRequest::RequestScreenshot(FPaths::Combine(CaptureDir,Name+TEXT(".png")),false,false);
}
void UFiskRigPanel::Smoke()
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    TActorIterator<AFiskStarterBoat> Boats(GetWorld());
    if(!Boats || !Day || !Day->HasStarted() || !PC->PlayerState) { return; }
    const AFiskStarterBoat& Boat=**Boats;
    const UFiskPrototypeRod& Rod=*PC->GetRod();
    const FFiskRigSettings& Rig=Rod.GetRig();
    const int32 Id=PC->PlayerState->GetPlayerId();
    const bool bCaptain=Boat.GetCaptainId()==Id;
    const int32 WantedBait=GetNetMode()==NM_Client?2:1, WantedDepth=GetNetMode()==NM_Client?20:40;
    const double Now=Day->GetElapsedRealSeconds();
    auto Matches=[&](){ return Rig.Bait==WantedBait && Rig.Depth==WantedDepth && Rig.Program==WantedBait; };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_RIG_TEST_FAILED player=%d stage=%d %s"),Id,Stage,Why);};
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(Settlement && Day->GetElapsedSeconds()<AFiskDayState::SettlementStart
        && (Settlement->GetView().bRevealed || !Settlement->GetView().Results.IsEmpty())) { Fail(TEXT("result_exposed"));return; }
    if(Stage==0)
    {
        if(!Boat.IsAboard(Id)) { Key(EKeys::E);return; }
        PC->ServerConfigureRig(Rod.GetCastId(),Rig.Revision,99,30,0);
        PC->ServerConfigureRig(Rod.GetCastId(),Rig.Revision+99,0,30,0);
        Key(EKeys::G);Stage=1;
    }
    else if(Stage==1)
    {
        if(!IsVisible()) { Fail(TEXT("panel_not_open"));return; }
        if(Rig.Bait!=WantedBait) { Row=0;Key(EKeys::Right);return; }
        if(Rig.Depth!=WantedDepth) { Row=1;Key(WantedDepth>Rig.Depth?EKeys::Right:EKeys::Left);return; }
        if(Rig.Program!=WantedBait) { Row=2;Key(EKeys::Right);return; }
        Capture(TEXT("rig-ready"));
        PC->ServerConfigureRig(Rod.GetCastId(),Rig.Revision-1,0,10,0);
        Key(EKeys::F);Key(EKeys::R);MarkAt=Now;Stage=2;
    }
    else if(Stage==2)
    {
        if(!Matches() || Rod.GetState()!=EFiskRodState::Ready || Boat.GetTripProgress()!=AFiskStarterBoat::AtDock) { Fail(TEXT("input_or_stale_isolation"));return; }
        if(Now-MarkAt<2) { return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RIG_MENU_PASSED player=%d bait=%d depth=%d program=%d clock_continued=1"),Id,Rig.Bait,Rig.Depth,Rig.Program);
        Key(EKeys::Escape);Key(EKeys::Escape);Key(EKeys::G);
        if(IsVisible() || !PC->GetFrontEnd()->IsVisible()) { Fail(TEXT("menu_conflict"));return; }
        Key(EKeys::Escape);Stage=3;
    }
    else if(Stage==3)
    {
        if(Boat.GetTripProgress()==AFiskStarterBoat::AtDock && bCaptain && Boat.GetCrewCount()==2 && Day->GetElapsedSeconds()>=120) { Key(EKeys::R); }
        if(Boat.GetTripProgress()!=AFiskStarterBoat::AtFishingSpot) { return; }
        Key(EKeys::G);Key(EKeys::F);
        if(!IsVisible() || Rod.GetState()!=EFiskRodState::Ready) { Fail(TEXT("cast_through_panel"));return; }
        Key(EKeys::G);Key(EKeys::F);Stage=4;
    }
    else if(Stage==4)
    {
        if(Rod.GetState()!=EFiskRodState::Fishing) { return; }
        FirstCast=Rod.GetCastId();
        PC->ServerConfigureRig(FirstCast,Rig.Revision,0,10,0);
        MarkAt=Now;Stage=5;
    }
    else if(Stage==5)
    {
        if(Now-MarkAt<2) { return; }
        if(!Matches()) { Fail(TEXT("active_configuration_accepted"));return; }
        Key(EKeys::G);if(IsVisible()) { Fail(TEXT("active_panel_opened"));return; }
        Key(EKeys::X);Stage=6;
    }
    else if(Stage==6)
    {
        if(Rod.GetState()!=EFiskRodState::Cancelled) { return; }
        if(!Matches()) { Fail(TEXT("withdraw_lost_settings"));return; }
        Key(EKeys::G);Capture(TEXT("rig-after-withdraw"));Stage=61;
    }
    else if(Stage==61) { Key(EKeys::G);Key(EKeys::F);Stage=7; }
    else if(Stage==7)
    {
        if(Rod.GetState()!=EFiskRodState::Fishing) { return; }
        if(Rod.GetCastId()==FirstCast || !Matches()) { Fail(TEXT("recast_settings_or_identity"));return; }
        PC->ServerConfigureRig(FirstCast,Rig.Revision,0,10,0);
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RIG_RECAST_PASSED player=%d"),Id);
        Stage=8;
    }
    else if(Stage==8)
    {
        if(!Matches()) { Fail(TEXT("old_cast_configuration_accepted"));return; }
        if(Rod.GetState()!=EFiskRodState::Fighting) { return; }
        Capture(TEXT("rig-fight"));MarkAt=Now;Stage=9;
    }
    else if(Stage==9)
    {
        if(Rod.GetState()==EFiskRodState::Lost) { Fail(TEXT("fight_lost"));return; }
        if(Rod.GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);Stage=10;return; }
        auto Input=[&](FKey Value,EInputEvent Event){FInputKeyEventArgs Args;Args.InputDevice=FInputDeviceId::CreateFromInternalId(0);Args.Key=Value;Args.Event=Event;PC->InputKey(Args);};
        const bool bLift=!Rod.IsSurging() && static_cast<int32>((Now-MarkAt)/2)%2==0;
        const bool bReel=!Rod.IsSurging() && !bLift;
        Input(EKeys::LeftMouseButton,bLift?IE_Pressed:IE_Released);
        Input(EKeys::RightMouseButton,bReel?IE_Pressed:IE_Released);
    }
    else if(Stage==10)
    {
        if(Rod.GetState()!=EFiskRodState::Landed) { return; }
        if(!Matches()) { Fail(TEXT("landing_lost_settings"));return; }
        Capture(TEXT("rig-landed"));MarkAt=Now;Stage=11;
    }
    else if(Stage==11)
    {
        // Wait until the rod is free again (its replicated AvailableAtMs), not a fixed time: under load
        // (three scripts at once, PR #180) four seconds were not always enough on the client.
        if(Now-MarkAt<4 || !PC->CanOpenRig()) { if(Now-MarkAt>30) { Fail(TEXT("rod_not_free_after_landing")); }return; }
        Key(EKeys::G);
        if(!IsVisible() || !Matches()) { Fail(TEXT("post_landing_panel"));return; }
        Capture(TEXT("rig-after-landing"));
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RIG_TEST_PASSED player=%d bait=%d depth=%d program=%d"),Id,Rig.Bait,Rig.Depth,Rig.Program);
        Stage=12;GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);
    }
}
#endif
