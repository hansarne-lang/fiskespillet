#include "World/FiskPierWalk.h"
#include "FiskPierGuide.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#if WITH_DEV_AUTOMATION_TESTS
void UFiskPierWalk::RunGuideSmoke()
{
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());auto* Panel=P->GetMarinaPanel();auto* H=Panel->FindHub();auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!P->IsCareerReady() || !P->PlayerState || !H || !Day || !Day->HasStarted()) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();const bool Lan=FParse::Param(FCommandLine::Get(),TEXT("FiskGuideLan"));int32 Role=0;FParse::Value(FCommandLine::Get(),TEXT("FiskPierPeer="),Role);
    const auto* Own=H->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==GI->Career.Id;});if(!Own) { return; }
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;P->InputKey(A);A.Event=IE_Released;P->InputKey(A);};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(SmokeDir,FString(Name)+TEXT(".png"));if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }};
    auto Fail=[&](const TCHAR* Why){ClearKeys();StopGuide(TEXT("Test stopped"));UE_LOG(LogTemp,Error,TEXT("FISKELAGET_GUIDE_FAILED stage=%d role=%d %s"),SmokeStage,Role,Why);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    auto Select=[&](int32 Choice){Key(EKeys::F10);for(int32 N=0;N<14 && GuideChoice!=Choice;++N) { Key(EKeys::Down); }Key(EKeys::Enter);SmokeAt=Now;};
    if(SmokeStage>1 && !FiskPier::Inside(Position)) { Fail(TEXT("unsafe_server_position"));return; }
    if(SmokeStage==0) { SmokeCash=GI->Career.Cash;Key(EKeys::T);SmokeStage=1;SmokeAt=Now;return; }
    if(SmokeStage==1)
    {
        if(!bEnabled || !Own->bWalking) { if(Now-SmokeAt>15) { Fail(TEXT("walking_not_started")); }return; }
        Key(EKeys::F10);Key(EKeys::Up);if(!bGuidePanel || GuideChoice!=13) { Fail(TEXT("destination_panel_input"));return; }Shot(TEXT("guide-destinations"));SmokeStage=15;SmokeAt=Now;return;
    }
    if(SmokeStage==15 && Now-SmokeAt>.75) { Key(EKeys::Enter);SmokeStage=2;SmokeAt=Now;return; }
    if(SmokeStage==2 && Now-SmokeAt>1)
    {
        if(!bGuiding || FVector::Dist2D(Position,FiskPier::Spawn())<100) { Fail(TEXT("guide_did_not_send_ordinary_movement"));return; }Key(EKeys::W);if(bGuiding) { Fail(TEXT("manual_did_not_cancel"));return; }Select(13);Key(EKeys::H);SmokeStage=3;SmokeAt=Now;return;
    }
    if(SmokeStage==3 && Now-SmokeAt>1)
    {
        if(bGuiding || !HasMenu()) { Fail(TEXT("menu_did_not_cancel"));return; }SmokeOrigin=Position;SmokeStage=4;SmokeAt=Now;return;
    }
    if(SmokeStage==4 && Now-SmokeAt>1)
    {
        if(FVector::Dist2D(Position,SmokeOrigin)>5) { Fail(TEXT("menu_kept_moving"));return; }Key(EKeys::H);Key(EKeys::C);if(CameraMode!=1) { Fail(TEXT("near_camera"));return; }Shot(TEXT("guide-camera-near"));SmokeStage=5;SmokeAt=Now;return;
    }
    if(SmokeStage==5 && Now-SmokeAt>.6) { Key(EKeys::C);if(CameraMode!=2) { Fail(TEXT("overview_camera"));return; }Shot(TEXT("guide-camera-overview"));SmokeStage=6;SmokeAt=Now;return; }
    if(SmokeStage==6 && Now-SmokeAt>.6) { Key(EKeys::Home);if(CameraMode!=0) { Fail(TEXT("camera_reset"));return; }Key(EKeys::One);SmokeWaypoint=Lan?(Role?13:7):0;Select(SmokeWaypoint);Shot(TEXT("guide-route"));SmokeStage=7;return; }
    if(SmokeStage==7)
    {
        if(bGuiding) { if(Now-SmokeAt>140) { Fail(TEXT("destination_timeout")); }return; }
        if(FVector::Dist2D(Position,FiskPier::Destination(SmokeWaypoint))>30 || !bEnabled) { Fail(TEXT("guide_stopped_before_destination"));return; }
        const auto* Boat=P->FindStarterBoat();if(Boat && Boat->IsAboard(P->PlayerState->GetPlayerId())) { Fail(TEXT("auto_boarded_without_E"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_DESTINATION_PASSED choice=%d role=%d"),SmokeWaypoint,Role);
        if(SmokeWaypoint==1) { Shot(TEXT("guide-at-bar")); }
        if(Lan || ++SmokeWaypoint>=14) { SmokeStage=8;SmokeAt=Now;return; }Select(SmokeWaypoint);return;
    }
    if(SmokeStage==8 && Now-SmokeAt>1)
    {
        if(Lan) { int32 Count=0;for(const auto& R:H->GetResidents()) { if(R.PlayerId>=0 && R.bWalking && FiskPier::Inside(R.WalkPosition)) { ++Count; } }if(Count<2) { if(Now-SmokeAt>35) { Fail(TEXT("remote_guided_walker_missing")); }return; }Shot(TEXT("guide-lan")); }
        if(GI->Career.Cash!=SmokeCash || !GI->GetJournal().GetEntries().IsEmpty() || FVector::Dist2D(Own->WalkPosition,Position)>80 || H->GetMessages().IsEmpty()) { Fail(TEXT("public_replication_or_private_state_changed"));return; }
        if(!Lan) { Shot(TEXT("guide-farthest-house"));SmokeStage=14;SmokeAt=Now;return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_PASSED lan=1 role=%d"),Role);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);return;
    }
    if(SmokeStage==9)
    {
        if(bGuiding) { if(Now-SmokeAt>140) { Fail(TEXT("return_to_boat_timeout")); }return; }
        if(!FiskPier::NearBoat(Position)) { Fail(TEXT("boarding_destination"));return; }Key(EKeys::E);SmokeStage=10;SmokeAt=Now;return;
    }
    if(SmokeStage==14 && Now-SmokeAt>.75) { SmokeCoast=0;SmokeStage=11;SmokeAt=Now;return; }
    if(SmokeStage==11 && Now-SmokeAt>3) { Shot(*FString::Printf(TEXT("coast-island-%d"),SmokeCoast));SmokeStage=12;SmokeAt=Now;return; }
    if(SmokeStage==12 && Now-SmokeAt>.75) { if(++SmokeCoast<3) { SmokeStage=11;SmokeAt=Now; }else { SmokeCoast=-1;Select(0);SmokeStage=9; }return; }
    if(SmokeStage==10 && Now-SmokeAt>2)
    {
        const auto* Boat=P->FindStarterBoat();if(bEnabled || bGuiding || !Boat || !Boat->IsAboard(P->PlayerState->GetPlayerId())) { Fail(TEXT("normal_E_boarding_failed"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_PASSED lan=0 destinations=14 cancellation=manual_and_menu cameras=3 ordinary_boarding=1"));GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);
    }
}
#endif
