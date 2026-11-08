#include "UI/FiskSeaChart.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskRigPanel.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"
#include "TimerManager.h"
void UFiskSeaChart::Key(FKey Value)
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    FInputKeyEventArgs Args;Args.InputDevice=FInputDeviceId::CreateFromInternalId(0);Args.Key=Value;Args.Event=IE_Pressed;PC->InputKey(Args);Args.Event=IE_Released;PC->InputKey(Args);
}
void UFiskSeaChart::Capture(const FString& Name)
{
    if(CaptureDir.IsEmpty()) { return; }
    IFileManager::Get().MakeDirectory(*CaptureDir,true);FScreenshotRequest::RequestScreenshot(FPaths::Combine(CaptureDir,Name+TEXT(".png")),false,false);
}
void UFiskSeaChart::Smoke()
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    if(!Boat || !Day || !Day->HasStarted() || !PC->PlayerState) { return; }
    const int32 Id=PC->PlayerState->GetPlayerId(),Progress=Boat->GetTripProgress();
    const bool bCaptain=Boat->GetCaptainId()==Id;
    const UFiskPrototypeRod& Rod=*PC->GetRod();const int32 Depth=GetNetMode()==NM_Client?90:80;
    const double Now=Day->GetElapsedRealSeconds();
    // Observe return immediately, even while the other player's landing animation finishes.
    if(Progress>100 && Progress<200 && ReturnAt<0) { ReturnAt=bCaptain?TravelAt:Now; }
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CHART_TEST_FAILED player=%d stage=%d %s"),Id,Stage,Why);};
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(Settlement && !Settlement->GetView().Results.IsEmpty() && Day->GetElapsedSeconds()<1020) { Fail(TEXT("early_results"));return; }
    if(Stage==0)
    {
        if(!Boat->IsAboard(Id)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=2) { return; }
        if(bCaptain) { PC->ServerSelectGround(Boat->GetRouteRevision(),999);PC->ServerSelectGround(Boat->GetRouteRevision()+99,1); }
        else { PC->ServerSelectGround(Boat->GetRouteRevision(),2); }
        PC->RequestRig(0,90,0); // Invalid at the initial reef; no settings must change.
        Key(EKeys::M);Capture(TEXT("chart-initial"));Key(EKeys::F);Key(EKeys::R);MarkAt=Now;Stage=1;
    }
    else if(Stage==1)
    {
        if(Now-MarkAt<2) { return; }
        if(!IsVisible() || Progress!=AFiskStarterBoat::AtDock || Rod.GetState()!=EFiskRodState::Ready || Rod.GetRig().Depth!=30)
        { Fail(TEXT("chart_input_or_depth_validation"));return; }
        Key(EKeys::G);if(PC->GetRigPanel()->IsVisible()) { Fail(TEXT("overlapping_panels"));return; }
        if(bCaptain) { Key(EKeys::Home);Key(EKeys::Down);Key(EKeys::Enter); }
        else { Row=2;Key(EKeys::Enter);if(!Message.Contains(TEXT("kapteinen"))) { Fail(TEXT("passenger_message"));return; } }
        Stage=2;
    }
    else if(Stage==2)
    {
        if(Boat->GetGroundId()!=1 || Boat->GetRouteRevision()!=1) { return; }
        Capture(TEXT("chart-selected"));
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CHART_SELECTION_PASSED player=%d ground=1 revision=1"),Id);
        Stage=3;
    }
    else if(Stage==3) { Key(EKeys::Escape);Key(EKeys::G);Stage=4; }
    else if(Stage==4)
    {
        if(!PC->GetRigPanel()->IsVisible()) { Fail(TEXT("rig_not_open"));return; }
        if(Rod.GetRig().Depth!=Depth) { Key(EKeys::Home);Key(EKeys::Down);Key(EKeys::Right);return; }
        Capture(TEXT("rig-dropoff"));Stage=41;
    }
    else if(Stage==41) { Key(EKeys::G);Stage=5; }
    else if(Stage==5)
    {
        if(Progress==AFiskStarterBoat::AtDock && bCaptain && Day->GetElapsedSeconds()>=120) { TravelAt=Now;Key(EKeys::R); }
        if(Boat->GetTripProgress()<0 || Boat->GetTripProgress()>=100) { return; }
        if(!bCaptain) { TravelAt=Now; }
        PC->ServerSelectGround(Boat->GetRouteRevision(),0);Key(EKeys::M);MarkAt=Now;Stage=6;
    }
    else if(Stage==6)
    {
        if(Boat->GetGroundId()!=1) { Fail(TEXT("mid_trip_route_changed"));return; }
        if(Now-MarkAt>=5 && Now-MarkAt<6) { Capture(TEXT("chart-underway")); }
        if(Progress!=100) { return; }
        const double Duration=Now-TravelAt;
        if(Duration<39 || Duration>43) { Fail(TEXT("travel_time"));return; }
        if(FVector::Dist(Boat->GetActorLocation(),FiskGrounds::Find(1)->Position)>5) { return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CHART_TRAVEL_PASSED player=%d seconds=%.3f ground=%d"),Id,Duration,Boat->GetGroundId());
        Key(EKeys::M);PC->ServerSelectGround(Boat->GetRouteRevision(),2);Key(EKeys::F);Stage=7;
    }
    else if(Stage==7)
    {
        if(Rod.GetState()!=EFiskRodState::Fishing) { return; }
        if(Boat->GetGroundId()!=1 || Rod.GetRig().Depth!=Depth) { Fail(TEXT("fishing_profile"));return; }
        Stage=8;
    }
    else if(Stage==8)
    {
        if(Rod.GetState()!=EFiskRodState::Fighting) { return; }
        if(Rod.GetLineOut()<Depth) { Fail(TEXT("line_shorter_than_fishing_depth"));return; }
        Capture(TEXT("chart-fight"));MarkAt=Now;Stage=9;
    }
    else if(Stage==9)
    {
        if(Rod.GetState()==EFiskRodState::Lost) { Fail(TEXT("fight_lost"));return; }
        if(Rod.GetState()==EFiskRodState::ReadyToLand) { Key(EKeys::SpaceBar);Stage=10;return; }
        auto Input=[&](FKey Value,EInputEvent Event){FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=Value;A.Event=Event;PC->InputKey(A);};
        const bool Lift=!Rod.IsSurging() && static_cast<int32>((Now-MarkAt)/2)%2==0;
        Input(EKeys::LeftMouseButton,Lift?IE_Pressed:IE_Released);
        Input(EKeys::RightMouseButton,!Rod.IsSurging() && !Lift?IE_Pressed:IE_Released);
    }
    else if(Stage==10)
    {
        if(Rod.GetState()!=EFiskRodState::Landed) { return; }
        Capture(TEXT("chart-landed"));MarkAt=Now;Stage=11;
    }
    else if(Stage==11)
    {
        if(Now-MarkAt<4) { return; }
        if(bCaptain && Progress==100)
        {
            int32 Landed=0;
            for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            { const AFiskBoardingController* Other=Cast<AFiskBoardingController>(It->Get());if(Other) { Landed+=Other->GetRod()->GetTestLandingCount(); } }
            if(Landed<2) { return; }
            TravelAt=Now;Key(EKeys::R);
        }
        if(Boat->GetTripProgress()<=100) { return; }
        if(ReturnAt<0) { ReturnAt=bCaptain?TravelAt:Now; }
        PC->ServerSelectGround(Boat->GetRouteRevision(),2);Key(EKeys::M);Capture(TEXT("chart-return"));Stage=12;
    }
    else if(Stage==12)
    {
        if(Boat->GetGroundId()!=1 || Rod.GetRig().Depth!=Depth) { Fail(TEXT("return_route_or_rig_changed"));return; }
        if(Progress!=200 || Boat->GetActorLocation().Size()>5) { return; }
        const double Duration=Now-ReturnAt;
        if(Duration<39 || Duration>43) { Fail(TEXT("return_time"));return; }
        if(Boat->GetMarinaArrivalMs(Day->GetElapsedMilliseconds())<0) { Fail(TEXT("missing_marina_crossing"));return; }
        Capture(TEXT("chart-home"));
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CHART_TEST_PASSED player=%d ground=%d depth=%d return_seconds=%.3f"),Id,Boat->GetGroundId(),Rod.GetRig().Depth,Duration);
        Stage=13;GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);
    }
}
#endif
