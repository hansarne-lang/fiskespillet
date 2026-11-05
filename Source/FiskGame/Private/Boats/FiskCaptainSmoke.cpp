#include "UI/FiskCrewPanel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskRigPanel.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UnrealClient.h"
#include "TimerManager.h"
void UFiskCrewPanel::Key(FKey Value)
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());FInputKeyEventArgs A;
    A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=Value;A.Event=IE_Pressed;PC->InputKey(A);A.Event=IE_Released;PC->InputKey(A);
}
void UFiskCrewPanel::Capture(const FString& Name)
{
    if(CaptureDir.IsEmpty()) { return; }
    IFileManager::Get().MakeDirectory(*CaptureDir,true);FScreenshotRequest::RequestScreenshot(FPaths::Combine(CaptureDir,Name+TEXT(".png")),false,false);
}
void UFiskCrewPanel::ChooseOther()
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    if(!IsVisible()) { Key(EKeys::K); }
    for(int32 I=0;Boat && I<Boat->GetCrewCount();++I) { if(Boat->GetCrewMemberId(I)!=PC->PlayerState->GetPlayerId()) { Row=I;Key(EKeys::Enter);return; } }
}
void UFiskCrewPanel::Smoke()
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!Boat || !Day || !Day->HasStarted() || !PC->PlayerState) { return; }
    const int32 Self=PC->PlayerState->GetPlayerId();int32 Other=INDEX_NONE;
    for(int32 I=0;I<Boat->GetCrewCount();++I) { if(Boat->GetCrewMemberId(I)!=Self) { Other=Boat->GetCrewMemberId(I); } }
    const bool Host=GetNetMode()!=NM_Client;const int32 Owner=Host?Self:Other,Guest=Host?Other:Self;
    const int32 Progress=Boat->GetTripProgress(),Rev=Boat->GetRouteRevision();const double Now=Day->GetElapsedRealSeconds();
    const UFiskPrototypeRod& Rod=*PC->GetRod();
    auto Fail=[&](const TCHAR* Why){UE_LOG(LogTemp,Error,TEXT("FISKELAGET_CAPTAIN_TEST_FAILED player=%d stage=%d %s"),Self,Stage,Why);};
    if(Progress>100 && Progress<200 && ReturnAt<0) { ReturnAt=Now; }
    if(Stage==0)
    {
        if(!Boat->IsAboard(Self)) { Key(EKeys::E);return; }
        if(Boat->GetCrewCount()!=2) { return; }
        InitialRevision=Rev;
        if(Host) { PC->ServerTransferCaptain(Rev,999);PC->ServerTransferCaptain(Rev+1,Other);PC->ServerTransferCaptain(Rev,Self); }
        else { PC->ServerTransferCaptain(Rev,Other); }
        Key(EKeys::K);Capture(TEXT("crew-initial"));Key(EKeys::F);Key(EKeys::R);Key(EKeys::M);Key(EKeys::G);MarkAt=Now;Stage=1;
    }
    else if(Stage==1)
    {
        if(Now-MarkAt<2) { return; }
        if(!IsVisible() || Progress!=-1 || Rod.GetState()!=EFiskRodState::Ready || PC->GetSeaChart()->IsVisible() || PC->GetRigPanel()->IsVisible()) { Fail(TEXT("menu_input_isolation"));return; }
        if(Host)
        {
            // Wait for the passenger's explicit server-observed rejection. A local
            // timer alone allowed the handoff to race the passenger's menu check.
            bool PassengerChecked=false;
            for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
            {
                const AFiskBoardingController* OtherPC=Cast<AFiskBoardingController>(It->Get());
                if(OtherPC && OtherPC!=PC && OtherPC->GetRejectedCaptainRequests()>=2) { PassengerChecked=true; }
            }
            if(!PassengerChecked) { return; }ChooseOther();
        }
        else
        {
            ChooseOther();if(!Message.Contains(TEXT("kapteinen"))) { Fail(TEXT("passenger_message"));return; }
            PC->ServerTransferCaptain(InitialRevision,999);
        }
        Stage=2;
    }
    else if(Stage==2)
    {
        if(Boat->GetCaptainId()!=Guest || Rev!=1) { return; }
        Capture(TEXT("crew-guest-captain"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_FIRST_PASSED player=%d"),Self);Stage=3;
    }
    else if(Stage==3)
    {
        Key(EKeys::K);
        if(!Host) { Key(EKeys::M);Key(EKeys::Home);Key(EKeys::Enter); }
        Stage=31;
    }
    else if(Stage==31)
    {
        if(Rev!=2 || Boat->GetGroundId()!=0) { return; }
        if(Host) { Key(EKeys::M); }
        Capture(TEXT("captain-route"));Stage=32;
    }
    else if(Stage==32) { Key(EKeys::M);Stage=4; }
    else if(Stage==4)
    {
        if(Host && Now>=120) { Key(EKeys::R);MarkAt=Now;Stage=5; }
        else if(!Host && Now>=125) { TripAt=Now;Key(EKeys::R);Stage=6; }
    }
    else if(Stage==5)
    {
        if(Now-MarkAt<2) { return; }
        if(Progress!=-1) { Fail(TEXT("former_captain_departed"));return; }
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_FORMER_DENIED player=%d"),Self);Stage=6;
    }
    else if(Stage==6)
    {
        if(Progress<0 || Progress>=100) { return; }
        if(Host) { TripAt=Now; }
        else { ChooseOther(); }
        Stage=7;
    }
    else if(Stage==7)
    {
        if(Rev<3) { return; }
        if(Host)
        {
            if(Boat->GetCaptainId()!=Owner) { Fail(TEXT("owner_handoff"));return; }
            PC->ServerStartBoatTrip(InitialRevision);PC->ServerSelectGround(InitialRevision,2);
            Key(EKeys::K);Capture(TEXT("crew-owner-midtrip"));MarkAt=Now;Stage=71;
        }
        else { Stage=8; }
    }
    else if(Stage==71) { if(Now-MarkAt>=1) { ChooseOther();Stage=8; } }
    else if(Stage==8)
    {
        if(Boat->GetCaptainId()!=Guest || Rev!=4) { return; }
        if(Progress>=100) { Fail(TEXT("outbound_handoff_too_late"));return; }
        Capture(TEXT("crew-moving"));
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_MOVING_PASSED player=%d revision=%d"),Self,Rev);Stage=81;
    }
    else if(Stage==81) { Key(EKeys::K);Stage=9; }
    else if(Stage==9)
    {
        if(Progress!=100 || FVector::Dist(Boat->GetActorLocation(),FiskGrounds::Find(0)->Position)>5) { return; }
        if(Now-TripAt<11 || Now-TripAt>15) { Fail(TEXT("handoff_changed_travel_time"));return; }
        Key(EKeys::F);MarkAt=Now;Stage=10;
    }
    else if(Stage==10)
    {
        if(Rod.GetState()!=EFiskRodState::Fishing) { return; }
        if(Host) { Key(EKeys::R);PC->ServerTransferCaptain(Rev,Self); }
        else { PC->ServerTransferCaptain(Rev,999); }
        Key(EKeys::K);Capture(TEXT("crew-fishing"));Key(EKeys::R);Key(EKeys::F);Key(EKeys::M);Key(EKeys::G);MarkAt=Now;Stage=11;
    }
    else if(Stage==11)
    {
        if(Now-MarkAt<2) { return; }
        if(Progress!=100 || Rod.GetState()!=EFiskRodState::Fishing || !IsVisible()) { Fail(TEXT("fishing_menu_isolation"));return; }
        Stage=111;
    }
    else if(Stage==111)
    {
        if(!Host && Progress==100 && Now-MarkAt>=4) { Key(EKeys::K);Key(EKeys::R);Stage=112;return; }
        if(Progress<=100) { return; }
        if(Host)
        {
            if(!IsVisible()) { Fail(TEXT("passenger_panel_not_held"));return; }
            UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_OTHER_MENU_DID_NOT_BLOCK player=%d"),Self);Key(EKeys::K);
        }
        Stage=12;
    }
    else if(Stage==112) { if(Progress>100) { Stage=12; } }
    else if(Stage==12)
    {
        if(Rod.GetState()==EFiskRodState::Fishing) { return; }
        if(!Host) { ChooseOther(); }
        else { Key(EKeys::K); }
        Stage=13;
    }
    else if(Stage==13)
    {
        if(Rev<5) { return; }
        if(Host) { Capture(TEXT("crew-return-owner"));MarkAt=Now;Stage=131; }
        else { Stage=14; }
    }
    else if(Stage==131) { if(Now-MarkAt>=1) { ChooseOther();Stage=14; } }
    else if(Stage==14)
    {
        if(Boat->GetCaptainId()!=Guest || Rev!=6) { return; }
        if(Progress>=200) { Fail(TEXT("return_handoff_too_late"));return; }
        Capture(TEXT("crew-return-guest"));Stage=15;
    }
    else if(Stage==15)
    {
        if(Progress!=200 || Boat->GetActorLocation().Size()>5) { return; }
        if(Now-ReturnAt<11 || Now-ReturnAt>15) { Fail(TEXT("handoff_changed_return_time"));return; }
        if(Rod.GetState()==EFiskRodState::Fishing || Boat->GetCaptainId()!=Guest) { Fail(TEXT("return_status"));return; }
        Capture(TEXT("crew-home"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_TEST_PASSED player=%d revision=%d return_seconds=%.3f"),Self,Rev,Now-ReturnAt);
        Stage=16;if(!Host) { GetWorld()->GetTimerManager().ClearTimer(SmokeTimer); }
    }
    else if(Stage==16 && Host && Boat->GetCrewCount()==1)
    {
        if(Boat->GetCaptainId()!=Self || Rev<=6) { Fail(TEXT("disconnect_fallback"));return; }
        Capture(TEXT("crew-fallback"));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_FALLBACK_PASSED player=%d revision=%d"),Self,Rev);
        Stage=17;GetWorld()->GetTimerManager().ClearTimer(SmokeTimer);
    }
}
#endif
