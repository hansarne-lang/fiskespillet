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
        if(FishingReleaseCapture.IsEmpty() || FScreenshotRequest::IsScreenshotRequested()) { return; }
        IFileManager::Get().MakeDirectory(*FishingReleaseCapture,true);
        FScreenshotRequest::RequestScreenshot(FPaths::Combine(FishingReleaseCapture,FString(Name)+TEXT(".png")),false,false);
    };
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_FISHING_RELEASE_FAILED mode=%s player=%d stage=%d %s"),*FishingReleaseMode,Self,FishingReleaseStage,Why);GetWorldTimerManager().ClearTimer(FishingReleaseTimer);};
    auto Done=[&](){UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FISHING_RELEASE_PASSED mode=%s player=%d"),*FishingReleaseMode,Self);GetWorldTimerManager().ClearTimer(FishingReleaseTimer);};
    if(FishingReleaseMode==TEXT("day"))
    {
        if(FishingReleaseStage==0 && Day->GetDayNumber()==1 && Now>=40 && ContentStage>=6)
        {
            if(GetRodCapacity()!=1) { Fail(TEXT("shared_capacity_wrong"));return; }
            Key(EKeys::Two);ServerSelectRod(Selection.Revision,Selection.Day,1);
            FFiskRodSelection Forged=Selection;Forged.Slot=1;ServerCastSelectedRod(Forged,Second->GetCastId());
            FishingReleaseOldSelection=Selection;FishingReleaseOldCast=First->GetCastId();
            Capture(TEXT("shared-rod-capacity"));FishingReleaseAt=Now;FishingReleaseStage=1;
        }
        else if(FishingReleaseStage==1 && Now-FishingReleaseAt>=2)
        {
            if(GetSelectedRod()!=0 || Second->GetState()!=EFiskRodState::Ready) { Fail(TEXT("guest_extra_rod_accepted"));return; }
            Key(EKeys::C);if(GetCameraView()!=1) { Fail(TEXT("camera_one"));return; }
            FishingReleaseAt=Now;FishingReleaseStage=12;
        }
        else if(FishingReleaseStage==12 && Now-FishingReleaseAt>=1)
        { Capture(TEXT("shared-camera-starboard"));FishingReleaseAt=Now;FishingReleaseStage=2; }
        else if(FishingReleaseStage==2 && Now-FishingReleaseAt>=1)
        { Key(EKeys::C);if(GetCameraView()!=2) { Fail(TEXT("camera_two"));return; }FishingReleaseAt=Now;FishingReleaseStage=22; }
        else if(FishingReleaseStage==22 && Now-FishingReleaseAt>=1)
        { Capture(TEXT("shared-camera-overview"));FishingReleaseAt=Now;FishingReleaseStage=3; }
        else if(FishingReleaseStage==3 && Now-FishingReleaseAt>=1) { Key(EKeys::C);FishingReleaseStage=4; }
        else if(FishingReleaseStage==4 && Day->GetDayNumber()==2 && Now>=40)
        {
            if(Selection.Day!=2 || Selection.Slot!=0 || Second->GetState()!=EFiskRodState::Ready || Second->GetRig().Depth!=30)
            { Fail(TEXT("extra_rod_day_reset"));return; }
            ServerActOnSelectedRod(EFiskRodAction::Withdraw,FishingReleaseOldSelection,FishingReleaseOldCast);
            FishingReleaseAt=Now;FishingReleaseStage=5;
        }
        else if(FishingReleaseStage==5 && Now-FishingReleaseAt>=1)
        { if(First->GetState()!=EFiskRodState::Ready || Second->GetState()!=EFiskRodState::Ready) { Fail(TEXT("old_day_selection_acted"));return; }Done(); }
        return;
    }
    if(FishingReleaseMode!=TEXT("solo")) { Fail(TEXT("unknown_mode"));return; }
    if(FishingReleaseStage==0)
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=1 || GetRodCapacity()!=2) { Fail(TEXT("solo_allocation"));return; }
        RequestRig(0,30,0);FishingReleaseStage=1;
    }
    else if(FishingReleaseStage==1 && First->GetRig().Depth==30)
    { FishingReleaseOldSelection=Selection;Key(EKeys::Two);FishingReleaseStage=2; }
    else if(FishingReleaseStage==2 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { RequestRig(1,40,1);FishingReleaseStage=3; }
    else if(FishingReleaseStage==3 && Second->GetRig().Depth==40 && Second->GetRig().Bait==1)
    {
        if(First->GetRig().Depth!=30 || First->GetRig().Bait!=0) { Fail(TEXT("rigs_not_independent"));return; }
        Key(EKeys::G);Capture(TEXT("second-rod-rig"));FishingReleaseAt=Now;FishingReleaseStage=4;
    }
    else if(FishingReleaseStage==4 && Now-FishingReleaseAt>=1)
    { Key(EKeys::G);Key(EKeys::C);FishingReleaseAt=Now;FishingReleaseStage=45; }
    else if(FishingReleaseStage==45 && Now-FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-starboard"));FishingReleaseAt=Now;FishingReleaseStage=5; }
    else if(FishingReleaseStage==5 && Now-FishingReleaseAt>=1)
    { Key(EKeys::C);FishingReleaseAt=Now;FishingReleaseStage=55; }
    else if(FishingReleaseStage==55 && Now-FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-overview"));FishingReleaseAt=Now;FishingReleaseStage=6; }
    else if(FishingReleaseStage==6 && Now-FishingReleaseAt>=1)
    { Key(EKeys::C);FishingReleaseAt=Now;FishingReleaseStage=65; }
    else if(FishingReleaseStage==65 && Now-FishingReleaseAt>=1)
    { Capture(TEXT("solo-camera-port"));FishingReleaseStage=7; }
    else if(FishingReleaseStage==7)
    {
        if(Progress==-1 && Now>=120) { Key(EKeys::R); }
        if(Progress==100) { Key(EKeys::One);FishingReleaseStage=8; }
    }
    else if(FishingReleaseStage==8 && GetSelectedRod()==0 && !IsRodSelectionPending())
    { Key(EKeys::F);FishingReleaseStage=9; }
    else if(FishingReleaseStage==9 && First->GetState()==EFiskRodState::Fishing)
    { Key(EKeys::Two);FishingReleaseStage=10; }
    else if(FishingReleaseStage==10 && GetSelectedRod()==1 && !IsRodSelectionPending())
    { Key(EKeys::F);FishingReleaseStage=11; }
    else if(FishingReleaseStage==11 && Second->GetState()==EFiskRodState::Fishing)
    {
        if(First->GetState()!=EFiskRodState::Fishing || First->GetCastId()==Second->GetCastId()) { Fail(TEXT("two_passive_rods_failed"));return; }
        FishingReleaseOldCast=Second->GetCastId();
        ServerCastSelectedRod(FishingReleaseOldSelection,First->GetCastId());
        ServerConfigureSelectedRod(FishingReleaseOldSelection,First->GetCastId(),First->GetRig().Revision,2,10,2);
        Capture(TEXT("two-passive-rods"));FishingReleaseAt=Now;FishingReleaseStage=12;
    }
    else if(FishingReleaseStage==12 && Now-FishingReleaseAt>=2)
    {
        if(First->GetState()!=EFiskRodState::Fishing || First->GetRig().Depth!=30 || Second->GetRig().Depth!=40) { Fail(TEXT("stale_selection_changed_other_rod"));return; }
        Key(EKeys::X);FishingReleaseStage=13;
    }
    else if(FishingReleaseStage==13 && Second->GetState()==EFiskRodState::Cancelled)
    {
        if(First->GetState()!=EFiskRodState::Fishing) { Fail(TEXT("withdraw_changed_first"));return; }
        Key(EKeys::F);FishingReleaseStage=14;
    }
    else if(FishingReleaseStage==14 && Second->GetState()==EFiskRodState::Fishing)
    {
        if(Second->GetCastId()==FishingReleaseOldCast) { Fail(TEXT("extra_cast_id_not_renewed"));return; }
        Capture(TEXT("second-rod-recast"));Key(EKeys::X);Key(EKeys::One);FishingReleaseStage=15;
    }
    else if(FishingReleaseStage==15 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(Second->GetState()!=EFiskRodState::Cancelled) { return; }
        if(First->GetState()==EFiskRodState::Fighting)
        { FishingReleaseFightAt=Now;FishingReleaseOldSelection=Selection;FishingReleaseOldCast=First->GetCastId();RequestLift();FishingReleaseStage=16; }
    }
    else if(FishingReleaseStage==16 && First->IsRaised())
    { Key(EKeys::Two);FishingReleaseAt=Now;FishingReleaseStage=17; }
    else if(FishingReleaseStage==17 && GetSelectedRod()==1 && Now-FishingReleaseAt>=1)
    {
        if(First->IsRaised() || First->IsReeling()) { Fail(TEXT("old_input_held_after_switch"));return; }
        ServerActOnSelectedRod(EFiskRodAction::Lift,FishingReleaseOldSelection,FishingReleaseOldCast);
        Capture(TEXT("fight-input-released"));FishingReleaseAt=Now;FishingReleaseStage=18;
    }
    else if(FishingReleaseStage==18 && Now-FishingReleaseAt>=1)
    {
        if(First->IsRaised()) { Fail(TEXT("stale_action_reactivated_old_rod"));return; }
        Key(EKeys::One);FishingReleaseStage=19;
    }
    else if(FishingReleaseStage==19 && GetSelectedRod()==0 && !IsRodSelectionPending())
    {
        if(First->GetState()==EFiskRodState::Lost) { Fail(TEXT("solo_fight_lost"));return; }
        if(First->GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);FishingReleaseStage=20;return; }
        auto Input=[&](FKey K,EInputEvent E){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=E;InputKey(A);};
        const bool Lift=!First->IsSurging() && static_cast<int32>((Now-FishingReleaseFightAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);Input(EKeys::RightMouseButton,!First->IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(FishingReleaseStage==20 && First->GetState()==EFiskRodState::Landed)
    {
        if(First->GetTestLandingCount()!=1 || Second->GetTestLandingCount()!=0 || Second->GetState()!=EFiskRodState::Cancelled) { Fail(TEXT("landing_not_isolated"));return; }
        Capture(TEXT("solo-rod-landed"));Key(EKeys::R);FishingReleaseStage=21;
    }
    else if(FishingReleaseStage==21 && Progress==200)
    {
        if(!CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->GetJournal().GetEntries().IsEmpty() || Now>=1020) { Fail(TEXT("solo_privacy_or_time"));return; }
        if(FScreenshotRequest::IsScreenshotRequested()) { return; }
        Capture(TEXT("solo-two-rod-home"));Done();
    }
}
#endif
