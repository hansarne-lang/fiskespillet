#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskRigPanel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"

void AFiskBoardingController::RunFishingReleaseSmoke()
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const AFiskStarterBoat* Boat=FindStarterBoat();
    if(!Day || !Day->HasStarted() || !Boat || !PlayerState) { return; }
    const double Now=Day->GetElapsedRealSeconds();const int32 Self=PlayerState->GetPlayerId(),Progress=Boat->GetTripProgress();
    const UFiskPrototypeRod* First=GetRodAt(0);const UFiskPrototypeRod* Second=GetRodAt(1);
    auto Key=[&](FKey K){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;InputKey(A);A.Event=IE_Released;InputKey(A);};
    auto Capture=[&](const TCHAR* Name){
        if(Smoke.FishingReleaseCapture.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*Smoke.FishingReleaseCapture,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(Smoke.FishingReleaseCapture,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_FISHING_RELEASE_FAILED mode=%s player=%d stage=%d %s"),*Smoke.FishingReleaseMode,Self,Smoke.FishingReleaseStage,Why);GetWorldTimerManager().ClearTimer(Smoke.FishingReleaseTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FISHING_RELEASE_PASSED mode=%s player=%d"),*Smoke.FishingReleaseMode,Self);GetWorldTimerManager().ClearTimer(Smoke.FishingReleaseTimer);};
    if(Smoke.FishingReleaseMode==TEXT("day"))
    {
        if(Smoke.FishingReleaseStage==0 && Day->GetDayNumber()==1 && Now>=40 && Smoke.ContentStage>=6)
        {
            if(GetRodCapacity()!=1) { Fail(TEXT("shared_capacity_wrong"));return; }
            Key(EKeys::Two);ServerSelectRod(Selection.Revision,Selection.Day,1);
            FFiskRodSelection Forged=Selection;Forged.Slot=1;ServerCastSelectedRod(Forged,Second->GetCastId());
            Smoke.FishingReleaseOldSelection=Selection;Smoke.FishingReleaseOldCast=First->GetCastId();
            Capture(TEXT("shared-rod-capacity"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=1;
        }
        else if(Smoke.FishingReleaseStage==1 && Now-Smoke.FishingReleaseAt>=2)
        {
            if(GetSelectedRod()!=0 || Second->GetState()!=EFiskRodState::Ready) { Fail(TEXT("guest_extra_rod_accepted"));return; }
            Key(EKeys::C);if(GetCameraView()!=1) { Fail(TEXT("camera_one"));return; }
            Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=12;
        }
        else if(Smoke.FishingReleaseStage==12 && Now-Smoke.FishingReleaseAt>=1)
        { Capture(TEXT("shared-camera-starboard"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=2; }
        else if(Smoke.FishingReleaseStage==2 && Now-Smoke.FishingReleaseAt>=1)
        { Key(EKeys::C);if(GetCameraView()!=2) { Fail(TEXT("camera_two"));return; }Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=22; }
        else if(Smoke.FishingReleaseStage==22 && Now-Smoke.FishingReleaseAt>=1)
        { Capture(TEXT("shared-camera-overview"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=3; }
        else if(Smoke.FishingReleaseStage==3 && Now-Smoke.FishingReleaseAt>=1) { Key(EKeys::C);Smoke.FishingReleaseStage=4; }
        else if(Smoke.FishingReleaseStage==4 && Day->GetDayNumber()==2 && Now>=40)
        {
            if(Selection.Day!=2 || Selection.Slot!=0 || Second->GetState()!=EFiskRodState::Ready || Second->GetRig().Depth!=30)
            { Fail(TEXT("extra_rod_day_reset"));return; }
            ServerActOnSelectedRod(EFiskRodAction::Withdraw,Smoke.FishingReleaseOldSelection,Smoke.FishingReleaseOldCast);
            Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=5;
        }
        else if(Smoke.FishingReleaseStage==5 && Now-Smoke.FishingReleaseAt>=1)
        { if(First->GetState()!=EFiskRodState::Ready || Second->GetState()!=EFiskRodState::Ready) { Fail(TEXT("old_day_selection_acted"));return; }Done(); }
        return;
    }
    if(Smoke.FishingReleaseMode!=TEXT("solo")) { Fail(TEXT("unknown_mode"));return; }
    if(Smoke.FishingReleaseStage==0)
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=1 || GetRodCapacity()!=2) { Fail(TEXT("solo_allocation"));return; }
        RequestRig(0,30,0);Smoke.FishingReleaseStage=1;
    }
    else if(Smoke.FishingReleaseStage==1 && First->GetRig().Depth==30)
    { Smoke.FishingReleaseOldSelection=Selection;Key(EKeys::Two);Smoke.FishingReleaseStage=2; }
    else if(Smoke.FishingReleaseStage==2 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { RequestRig(1,40,1);Smoke.FishingReleaseStage=3; }
    else if(Smoke.FishingReleaseStage==3 && Second->GetRig().Depth==40 && Second->GetRig().Bait==1)
    {
        if(First->GetRig().Depth!=30 || First->GetRig().Bait!=0) { Fail(TEXT("rigs_not_independent"));return; }
        Key(EKeys::G);Capture(TEXT("second-rod-rig"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=4;
    }
    else if(Smoke.FishingReleaseStage==4 && Now-Smoke.FishingReleaseAt>=1)
    { Key(EKeys::G);Key(EKeys::C);Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=45; }
    else if(Smoke.FishingReleaseStage==45 && Now-Smoke.FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-starboard"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=5; }
    else if(Smoke.FishingReleaseStage==5 && Now-Smoke.FishingReleaseAt>=1)
    { Key(EKeys::C);Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=55; }
    else if(Smoke.FishingReleaseStage==55 && Now-Smoke.FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-overview"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=6; }
    else if(Smoke.FishingReleaseStage==6 && Now-Smoke.FishingReleaseAt>=1)
    { Key(EKeys::C);Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=65; }
    else if(Smoke.FishingReleaseStage==65 && Now-Smoke.FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-port"));Smoke.FishingReleaseStage=7; }
    else if(Smoke.FishingReleaseStage==7)
    {
        if(Progress==-1 && Now>=120) { Key(EKeys::R); }
        if(Progress==100) { Key(EKeys::One);Smoke.FishingReleaseStage=8; }
    }
    else if(Smoke.FishingReleaseStage==8 && GetSelectedRod()==0 && !IsRodSelectionPending())
    { Key(EKeys::F);Smoke.FishingReleaseStage=9; }
    else if(Smoke.FishingReleaseStage==9 && First->GetState()==EFiskRodState::Fishing)
    { Key(EKeys::Two);Smoke.FishingReleaseStage=10; }
    else if(Smoke.FishingReleaseStage==10 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { Key(EKeys::F);Smoke.FishingReleaseStage=11; }
    else if(Smoke.FishingReleaseStage==11 && Second->GetState()==EFiskRodState::Fishing)
    {
        if(First->GetState()!=EFiskRodState::Fishing || First->GetCastId()==Second->GetCastId()) { Fail(TEXT("two_passive_rods_failed"));return; }
        Smoke.FishingReleaseOldCast=Second->GetCastId();
        ServerCastSelectedRod(Smoke.FishingReleaseOldSelection,First->GetCastId());
        ServerConfigureSelectedRod(Smoke.FishingReleaseOldSelection,First->GetCastId(),First->GetRig().Revision,2,10,2);
        Capture(TEXT("two-passive-rods"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=12;
    }
    else if(Smoke.FishingReleaseStage==12 && Now-Smoke.FishingReleaseAt>=2)
    {
        if(First->GetState()!=EFiskRodState::Fishing || First->GetRig().Depth!=30 || Second->GetRig().Depth!=40) { Fail(TEXT("stale_selection_changed_other_rod"));return; }
        Key(EKeys::X);Smoke.FishingReleaseStage=13;
    }
    else if(Smoke.FishingReleaseStage==13 && Second->GetState()==EFiskRodState::Cancelled)
    {
        if(First->GetState()!=EFiskRodState::Fishing) { Fail(TEXT("withdraw_changed_first"));return; }
        Key(EKeys::F);Smoke.FishingReleaseStage=14;
    }
    else if(Smoke.FishingReleaseStage==14 && Second->GetState()==EFiskRodState::Fishing)
    {
        if(Second->GetCastId()==Smoke.FishingReleaseOldCast) { Fail(TEXT("extra_cast_id_not_renewed"));return; }
        Capture(TEXT("second-rod-recast"));Key(EKeys::X);Key(EKeys::One);Smoke.FishingReleaseStage=15;
    }
    else if(Smoke.FishingReleaseStage==15 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(Second->GetState()!=EFiskRodState::Cancelled) { return; }
        if(First->GetState()==EFiskRodState::Fighting)
        { Smoke.FishingReleaseFightAt=Now;Smoke.FishingReleaseOldSelection=Selection;Smoke.FishingReleaseOldCast=First->GetCastId();RequestLift();Smoke.FishingReleaseStage=16; }
    }
    else if(Smoke.FishingReleaseStage==16 && First->IsRaised())
    { Key(EKeys::Two);Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=17; }
    else if(Smoke.FishingReleaseStage==17 && GetSelectedRod()==1 && Now-Smoke.FishingReleaseAt>=1)
    {
        if(First->IsRaised() || First->IsReeling()) { Fail(TEXT("old_input_held_after_switch"));return; }
        ServerActOnSelectedRod(EFiskRodAction::Lift,Smoke.FishingReleaseOldSelection,Smoke.FishingReleaseOldCast);
        Capture(TEXT("fight-input-released"));Smoke.FishingReleaseAt=Now;Smoke.FishingReleaseStage=18;
    }
    else if(Smoke.FishingReleaseStage==18 && Now-Smoke.FishingReleaseAt>=1)
    {
        if(First->IsRaised()) { Fail(TEXT("stale_action_reactivated_old_rod"));return; }
        Key(EKeys::One);Smoke.FishingReleaseStage=19;
    }
    else if(Smoke.FishingReleaseStage==19 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(First->GetState()==EFiskRodState::Lost) { Fail(TEXT("solo_fight_lost"));return; }
        if(First->GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);Smoke.FishingReleaseStage=20;return; }
        auto Input=[&](FKey K,EInputEvent E){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;InputKey(A);};
        const bool Lift=!First->IsSurging() && static_cast<int32>((Now-Smoke.FishingReleaseFightAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!First->IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(Smoke.FishingReleaseStage==20 && First->GetState()==EFiskRodState::Landed)
    {
        if(First->GetTestLandingCount()!=1 || Second->GetTestLandingCount()!=0 || Second->GetState()!=EFiskRodState::Cancelled) { Fail(TEXT("landing_not_isolated"));return; }
        Capture(TEXT("solo-rod-landed"));Key(EKeys::R);Smoke.FishingReleaseStage=21;
    }
    else if(Smoke.FishingReleaseStage==21 && Progress==200)
    {
        if(!CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->GetJournal().GetEntries().IsEmpty() || Now>=1020) { Fail(TEXT("solo_privacy_or_time"));return; }
        if(FScreenshotRequest::IsScreenshotRequested()) { return; }
        Capture(TEXT("solo-two-rod-home"));Done();
    }
}
#endif
