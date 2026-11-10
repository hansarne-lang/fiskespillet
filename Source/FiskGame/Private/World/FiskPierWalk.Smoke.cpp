#include "World/FiskPierWalk.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "World/FiskHarbourLife.h"
#include "World/FiskHarbourSound.h"
#include "FiskPierRules.h"
#include "DayCycle/FiskDayState.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
void UFiskPierWalk::BeginPlay()
{
    Super::BeginPlay();
#if WITH_DEV_AUTOMATION_TESTS
    if(CastChecked<AFiskBoardingController>(GetOwner())->IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskPierSmoke="),SmokeMode))
    { FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),SmokeDir);GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskPierWalk::RunSmoke,.25f,true,3.5f); }
#endif
}
#if WITH_DEV_AUTOMATION_TESTS
void UFiskPierWalk::RunSmoke()
{
    if(SmokeMode==TEXT("guide")) { RunGuideSmoke();return; }
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());auto* Panel=P->GetMarinaPanel();auto* H=Panel->FindHub();auto* D=GetWorld()->GetGameState<AFiskDayState>();
    if(!P->IsCareerReady() || !P->PlayerState || !H || !D || !D->HasStarted()) { return; }
    const auto* Own=H->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==GI->Career.Id;});if(!Own) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();auto* Life=H->FindComponentByClass<UFiskHarbourLife>();auto* Sound=H->FindComponentByClass<UFiskHarbourSound>();
    auto Key=[&](FKey K,EInputEvent E=IE_Pressed){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;P->InputKey(A);if(E==IE_Pressed && K!=EKeys::W && K!=EKeys::A && K!=EKeys::S && K!=EKeys::D && K!=EKeys::LeftShift) { A.Event=IE_Released;P->InputKey(A); }};
    auto Held=[&](FKey K,bool Current,bool Want){if(Current!=Want) { Key(K,Want?IE_Pressed:IE_Released); }};
    auto Stop=[&](){Held(EKeys::W,bUp,false);Held(EKeys::S,bDown,false);Held(EKeys::A,bLeft,false);Held(EKeys::D,bRight,false);Held(EKeys::LeftShift,bSprint,false);};
    auto Go=[&](FVector Target){const FVector V=Target-Position;if(FVector::Dist2D(Target,Position)<65) { Stop();return true; }Held(EKeys::W,bUp,V.Y>25);Held(EKeys::S,bDown,V.Y<-25);Held(EKeys::A,bLeft,V.X>25);Held(EKeys::D,bRight,V.X<-25);Held(EKeys::LeftShift,bSprint,true);return false;};
    auto Shot=[&](const TCHAR* Name){const FString File=FPaths::Combine(SmokeDir,FString(Name)+TEXT(".png"));FTimerHandle T;GetWorld()->GetTimerManager().SetTimer(T,FTimerDelegate::CreateWeakLambda(this,[File](){if(!FScreenshotRequest::IsScreenshotRequested()) { FScreenshotRequest::RequestScreenshot(File,false,false); }}),.3f,false);};
    auto Fail=[&](const TCHAR* Why){Stop();UE_LOG(LogTemp,Error,TEXT("FISKELAGET_PIER_FAILED mode=%s stage=%d %s"),*SmokeMode,SmokeStage,Why);GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    auto Done=[&](){Stop();UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_PASSED mode=%s x=%.0f y=%.0f cash=%lld"),*SmokeMode,Position.X,Position.Y,static_cast<long long>(GI->Career.Cash));GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);};
    if(SmokeMode==TEXT("reload")) { if(bEnabled || !Position.Equals(FiskPier::Spawn(),1) || GI->Career.Cash!=3000000 || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("temporary_walk_state_persisted_or_career_changed"));return; }Done();return; }
    const bool Tour=SmokeMode==TEXT("tour");int32 Role=0;FParse::Value(FCommandLine::Get(),TEXT("FiskPierPeer="),Role);
    if(SmokeStage>1 && !FiskPier::Inside(Position)) { Fail(TEXT("walk_left_safe_deck"));return; }
    if(SmokeStage==0) { SmokeCash=GI->Career.Cash;Key(EKeys::T);SmokeAt=Now;SmokeStage=1;return; }
    if(SmokeStage==1)
    {
        if(!bEnabled || !Own->bWalking || P->GetViewTarget()!=H) { if(Now-SmokeAt>12) { Fail(TEXT("mode_camera_or_public_position_missing")); }return; }
        if(!Tour) { auto Identity=Panel->GetIdentity();Identity.Color=(Role+1)%6;P->GetMarinaSync()->ServerMarinaIdentity(GI->Career.Id,Identity);SmokeStage=30;SmokeWaypoint=0;return; }
        ServerPierInput(FGuid::NewGuid(),Generation,200,{0,1},true);ServerPierInput(GI->Career.Id,Generation-1,201,{0,1},true);ServerPierInput(GI->Career.Id,Generation,0,{0,1},true);ServerPierInput(GI->Career.Id,Generation,202,{4,0},true);
        SmokeOrigin=Position;Key(EKeys::W);Key(EKeys::LeftShift);SmokeAt=Now;SmokeStage=2;return;
    }
    if(!Tour)
    {
        if(SmokeStage==30)
        { const FVector Target=SmokeWaypoint==0?FVector(-1300,-2900,130):FVector(-4650+Role*300,-2900,130);if(Go(Target)) { if(++SmokeWaypoint==2) { SmokeAt=Now;SmokeStage=31; } }return; }
        if(SmokeStage==31)
        {
            int32 N=0;for(const auto& R:H->GetResidents()) { if(R.PlayerId>=0 && R.bWalking && FiskPier::Inside(R.WalkPosition) && R.WalkPosition.Y<-2750) { ++N; } }
            if(N<4 || !Life || Life->GetVisiblePeople()<4) { if(Now-SmokeAt>40) { Fail(TEXT("four_remote_walkers_missing")); }return; }
            if(Role==0) { Shot(TEXT("four-walking-fishers")); }Key(EKeys::E);SmokeAt=Now;SmokeStage=32;return;
        }
        if(SmokeStage==32 && Now-SmokeAt>=2)
        { if(Panel->GetPage()!=2 || Own->Place!=2 || !Sound || !Sound->HasConsumedAudio()) { Fail(TEXT("nearby_bar_or_audio"));return; }Key(EKeys::One);if(Role==0) { Shot(TEXT("walkers-in-bar")); }SmokeAt=Now;SmokeStage=33;return; }
        if(SmokeStage==33 && Now-SmokeAt>=1)
        { if(GI->Career.Cash!=SmokeCash || !GI->GetJournal().GetEntries().IsEmpty() || Panel->GetIdentity().Color!=(Role+1)%6) { Fail(TEXT("public_walk_changed_private_career"));return; }Done(); }return;
    }
    if(SmokeStage==2 && Now-SmokeAt>=2)
    {
        if(Position.Y-SmokeOrigin.Y<500 || FVector::Dist2D(Position,SmokeOrigin)>850) { Fail(TEXT("actual_W_shift_motion_not_bounded"));return; }Stop();NextSend=Now+1.7;ServerPierInput(GI->Career.Id,Generation,++Serial,{0,1},true);SmokeAt=Now;SmokeStage=20;return;
    }
    if(SmokeStage==20 && Now-SmokeAt>=.7) { SmokeOrigin=Position;SmokeAt=Now;SmokeStage=21;return; }
    if(SmokeStage==21 && Now-SmokeAt>=.6)
    { if(FVector::Dist2D(Position,SmokeOrigin)>8) { Fail(TEXT("lost_input_did_not_stop"));return; }UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_TIMEOUT_PASSED"));Key(EKeys::H);Key(EKeys::W);SmokeAt=Now;SmokeStage=3;return; }
    if(SmokeStage==3 && Now-SmokeAt>=1) { SmokeOrigin=Position;SmokeAt=Now;SmokeStage=4;return; }
    if(SmokeStage==4 && Now-SmokeAt>=1)
    {
        if(FVector::Dist2D(Position,SmokeOrigin)>5) { Fail(TEXT("menu_did_not_stop_motion"));return; }Key(EKeys::H);Key(EKeys::Left);Key(EKeys::MouseScrollUp);if(CameraYaw!=-15 || Zoom>=1) { Fail(TEXT("camera_controls"));return; }Key(EKeys::Home);Shot(TEXT("pier-start"));SmokeWaypoint=0;SmokeStage=5;return;
    }
    if(SmokeStage==5)
    { const FVector Target=SmokeWaypoint==0?FVector(-1300,3850,130):FiskPier::HouseDoor(0);if(Go(Target)) { if(++SmokeWaypoint==2) { Key(EKeys::E);SmokeAt=Now;SmokeStage=6; } }return; }
    if(SmokeStage==6 && Now-SmokeAt>=1)
    { if(Panel->GetPage()!=1) { Fail(TEXT("nearby_house_did_not_open"));return; }Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Right);Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Down);Key(EKeys::Enter);SmokeAt=Now;SmokeStage=7;return; }
    if(SmokeStage==7 && Now-SmokeAt>=2)
    { if(Panel->GetIdentity().Color!=1 || !Life || !Life->PersonColor(GI->Career.Id).Equals(FLinearColor(.75f,.3f,.2f))) { Fail(TEXT("live_shirt_did_not_follow_identity"));return; }Shot(TEXT("pier-own-house"));Key(EKeys::H);SmokeWaypoint=0;SmokeStage=8;return; }
    if(SmokeStage==8)
    { const FVector Targets[]={FVector(-1300,3850,130),FVector(-1300,-2900,130),FVector(-4200,-2900,130)};if(Go(Targets[SmokeWaypoint])) { if(++SmokeWaypoint==3) { Shot(TEXT("pier-at-bar"));Key(EKeys::E);SmokeAt=Now;SmokeStage=9; } }return; }
    if(SmokeStage==9 && Now-SmokeAt>=2)
    { if(Panel->GetPage()!=2 || !Sound || !Sound->HasConsumedAudio()) { Fail(TEXT("bar_interaction_or_audio"));return; }Key(EKeys::One);Key(EKeys::M);SmokeAt=Now;SmokeStage=10;return; }
    if(SmokeStage==10 && Now-SmokeAt>=1)
    { if(Panel->IsHarbourMusic()) { Fail(TEXT("music_toggle"));return; }Shot(TEXT("pier-bar-wave"));Key(EKeys::M);Key(EKeys::B);SmokeWaypoint=0;SmokeStage=11;return; }
    if(SmokeStage==11)
    { const FVector Targets[]={FVector(-1300,-2900,130),FVector(-1300,0,130),FVector(-550,0,130)};if(Go(Targets[SmokeWaypoint])) { if(++SmokeWaypoint==3) { Shot(TEXT("pier-boarding"));Key(EKeys::E);SmokeAt=Now;SmokeStage=12; } }return; }
    if(SmokeStage==12 && Now-SmokeAt>=2)
    {
        const auto* B=P->FindStarterBoat();if(bEnabled || !B || !B->IsAboard(P->PlayerState->GetPlayerId()) || GI->Career.Cash!=SmokeCash || !GI->GetJournal().GetEntries().IsEmpty()) { Fail(TEXT("boarding_or_private_state"));return; }
        ServerPierMode(GI->Career.Id,true);SmokeAt=Now;SmokeStage=13;return;
    }
    if(SmokeStage==13 && Now-SmokeAt>=1) { if(bEnabled) { Fail(TEXT("aboard_walk_accepted"));return; }Done(); }
}
#endif
