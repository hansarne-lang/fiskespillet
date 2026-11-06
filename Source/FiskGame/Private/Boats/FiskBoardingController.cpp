#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskCombatFeedback.h"
#include "Economy/FiskDaySettlement.h"
#include "HAL/PlatformTime.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "World/FiskOceanScene.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskSessionRules.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskDeckPanel.h"
#include "FiskBoatLayout.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "UI/FiskExpeditionPanel.h"

AFiskBoardingController::AFiskBoardingController()
{
    Rod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("PrototypeRod"));
    ExtraRod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("ExtraRod"));
    ExtraRod->SetSlot(1);
    ThirdRod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("ThirdRod"));ThirdRod->SetSlot(2);
    FrontEnd = CreateDefaultSubobject<UFiskFrontEnd>(TEXT("FrontEnd"));
    RigPanel = CreateDefaultSubobject<UFiskRigPanel>(TEXT("RigPanel"));
    SeaChart = CreateDefaultSubobject<UFiskSeaChart>(TEXT("SeaChart"));
    CrewPanel = CreateDefaultSubobject<UFiskCrewPanel>(TEXT("CrewPanel"));
    JournalPanel = CreateDefaultSubobject<UFiskJournalPanel>(TEXT("JournalPanel"));
    DeckPanel = CreateDefaultSubobject<UFiskDeckPanel>(TEXT("DeckPanel"));
    CreateDefaultSubobject<UFiskCombatFeedback>(TEXT("CombatFeedback"));
    MarinaPanel=CreateDefaultSubobject<UFiskMarinaPanel>(TEXT("MarinaPanel"));
    ExpeditionPanel=CreateDefaultSubobject<UFiskExpeditionPanel>(TEXT("ExpeditionPanel"));
}

bool AFiskBoardingController::InputKey(const FInputKeyEventArgs& Params)
{
    const bool Press=Params.Event==IE_Pressed;
    if(ExpeditionPanel->HandleKey(Params)) { return true; }
    if(MarinaPanel->HandleKey(Params)) { return true; }
    if(HandleCareerKey(Params)) { return true; }
    if(DeckPanel->IsVisible()) { return DeckPanel->HandleKey(Params); }
    if(Press && (Params.Key==EKeys::F7 || Params.Key==EKeys::F8))
    {
        if(JournalPanel->IsVisible() || FrontEnd->IsVisible() || CrewPanel->IsVisible() || SeaChart->IsVisible() || RigPanel->IsVisible()
            || bHelpVisible || bResultsVisible || bHistoryVisible)
        { ClientActionFeedback_Implementation(TEXT("Lukk det åpne panelet med Escape først."));return true; }
        DeckPanel->Open(Params.Key==EKeys::F7);return true;
    }
    if(JournalPanel->IsVisible()) { return JournalPanel->HandleKey(Params); }
    if(Press && Params.Key==EKeys::F4)
    {
        if(CrewPanel->IsVisible() || SeaChart->IsVisible() || RigPanel->IsVisible())
        { ClientActionFeedback_Implementation(TEXT("Lukk panelet med Escape før du åpner F4-journalen."));return true; }
        bHelpVisible=bResultsVisible=bHistoryVisible=false;JournalPanel->Toggle();return true;
    }
    if(SeaChart->IsVisible() && Params.Key==EKeys::F2) { return SeaChart->HandleKey(Params); }
    const bool OtherPanel=(FrontEnd && FrontEnd->IsVisible()) || (CrewPanel && CrewPanel->IsVisible())
        || (SeaChart && SeaChart->IsVisible()) || (RigPanel && RigPanel->IsVisible());
    if(Press && Params.Key==EKeys::N && !OtherPanel && !bHelpVisible)
    {
        if(const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>()) { ServerNextDay(Day->GetDayNumber()); }
        return true;
    }
    if(Press && (Params.Key==EKeys::F1 || Params.Key==EKeys::F2 || Params.Key==EKeys::F3))
    {
        if(OtherPanel) { ClientActionFeedback_Implementation(TEXT("Lukk panelet først med Escape. F1 viser hjelp, F2 viser oppgjøret."));return true; }
        if(Params.Key==EKeys::F1) { bHelpVisible=!bHelpVisible;bResultsVisible=bHistoryVisible=false; }
        else if(Params.Key==EKeys::F2) { bResultsVisible=!bResultsVisible;bHelpVisible=bHistoryVisible=false;ResultPage=0; }
        else { bHistoryVisible=!bHistoryVisible;bHelpVisible=bResultsVisible=false;ResultPage=0; }
        ReleaseRodInputsForMenu();return true;
    }
    if(bHelpVisible || bResultsVisible || bHistoryVisible)
    {
        if(Press && Params.Key==EKeys::Escape) { bHelpVisible=bResultsVisible=bHistoryVisible=false; }
        if((bResultsVisible || bHistoryVisible) && (Press || Params.Event==IE_Repeat))
        {
            if(Params.Key==EKeys::PageDown || Params.Key==EKeys::Down) { ResultPage=FMath::Min(ResultPage+1,9999); }
            if(Params.Key==EKeys::PageUp || Params.Key==EKeys::Up) { ResultPage=FMath::Max(ResultPage-1,0); }
        }
        return true;
    }
    if(Press && Params.Key==EKeys::R && OtherPanel)
    { ClientActionFeedback_Implementation(TEXT("Lukk kart og menyer med Escape før du trykker R.")); }
    if (CrewPanel && CrewPanel->HandleKey(Params)) { return true; }
    if (SeaChart && SeaChart->HandleKey(Params)) { return true; }
    if (RigPanel && RigPanel->HandleKey(Params)) { return true; }
    if (FrontEnd && FrontEnd->HandleKey(Params)) { return true; }
    // Handle rod commands before a later key can change the selected holder.
    // Deferred input bindings otherwise make X followed by 1 act on rod 1.
    const bool Release=Params.Event==IE_Released;
    const bool RodKey=Params.Key==EKeys::F || Params.Key==EKeys::X || Params.Key==EKeys::LeftMouseButton
        || Params.Key==EKeys::RightMouseButton || Params.Key==EKeys::MouseScrollUp || Params.Key==EKeys::MouseScrollDown || Params.Key==EKeys::SpaceBar;
    if(RodKey)
    {
        if(PendingRod>=0) { return true; }
        if(Press && Params.Key==EKeys::F) { RequestCast(); }
        else if(Press && Params.Key==EKeys::X) { RequestWithdraw(); }
        else if(Press && Params.Key==EKeys::LeftMouseButton) { RequestLift(); }
        else if(Release && Params.Key==EKeys::LeftMouseButton) { RequestLower(); }
        else if(Press && Params.Key==EKeys::RightMouseButton) { RequestReel(); }
        else if(Release && Params.Key==EKeys::RightMouseButton) { RequestStopReel(); }
        else if(Press && Params.Key==EKeys::MouseScrollUp) { RequestDragUp(); }
        else if(Press && Params.Key==EKeys::MouseScrollDown) { RequestDragDown(); }
        else if(Press && Params.Key==EKeys::SpaceBar) { RequestLand(); }
        return true;
    }
    if(Press && (Params.Key==EKeys::One || Params.Key==EKeys::Two || Params.Key==EKeys::Three))
    { RequestRodSelection(Params.Key==EKeys::One?0:Params.Key==EKeys::Two?1:2);return true; }
    if(Press && Params.Key==EKeys::C)
    {
        if(PlayerState && FindStarterBoat() && FindStarterBoat()->IsAboard(PlayerState->GetPlayerId()))
        {
            CameraView=(CameraView+1)%3;
            if(PlayerCameraManager) { PlayerCameraManager->SetGameCameraCutThisFrame(); }
            ClientActionFeedback_Implementation(FString::Printf(TEXT("Kamera: %s"),CameraView==0?TEXT("Babord"):CameraView==1?TEXT("Styrbord"):TEXT("Oversikt")));
        }
        return true;
    }
    return Super::InputKey(Params);
}
void AFiskBoardingController::ReleaseRodInputsForMenu() { RequestLower(); RequestStopReel(); }

void AFiskBoardingController::BeginPlay()
{
    Super::BeginPlay();
    // Remote server controllers have no PlayerInput and never receive PlayerTick.
    // Settlement and durable snapshot retries must run for every connection.
    GetWorldTimerManager().SetTimer(CareerTimer,this,&AFiskBoardingController::TickCareer,.5f,true);

#if WITH_DEV_AUTOMATION_TESTS
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskExpeditionSmoke="),ExpeditionSmokeMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),ExpeditionSmokeDir);
        GetWorldTimerManager().SetTimer(ExpeditionSmokeTimer,this,&AFiskBoardingController::RunExpeditionSmoke,.5f,true,2.f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskCareerReleaseSmoke="),CareerSmokeMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),CareerSmokeDir);
        GetWorldTimerManager().SetTimer(CareerSmokeTimer,this,&AFiskBoardingController::RunCareerReleaseSmoke,.5f,true,2.f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskFishingReleaseSmoke="),FishingReleaseMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),FishingReleaseCapture);
        GetWorldTimerManager().SetTimer(FishingReleaseTimer,this,&AFiskBoardingController::RunFishingReleaseSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskNavigationSmoke="),NavigationMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),NavigationCaptureDir);
        GetWorldTimerManager().SetTimer(NavigationTimer,this,&AFiskBoardingController::RunNavigationSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskJournalSmoke="),JournalMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),JournalCaptureDir);
        FParse::Value(FCommandLine::Get(),TEXT("FiskJournalExpectedDepth="),JournalExpectedDepth);
        GetWorldTimerManager().SetTimer(JournalTimer,this,&AFiskBoardingController::RunJournalSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskSessionSmoke="),SessionMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),SessionCaptureDir);
        GetWorldTimerManager().SetTimer(SessionTimer,this,&AFiskBoardingController::RunSessionSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskContentSmoke="),ContentMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),ContentCaptureDir);
        GetWorldTimerManager().SetTimer(ContentTimer,this,&AFiskBoardingController::RunContentSmoke,0.5f,true,2.0f);
    }
    if (IsLocalController() && GetNetMode() != NM_Standalone
        && FParse::Param(FCommandLine::Get(), TEXT("FiskLanSmoke")))
    {
        GetWorldTimerManager().SetTimer(LanSmokeTimer, this, &AFiskBoardingController::RunLanSmoke, 0.5f, true);
    }
#endif
}

void AFiskBoardingController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AFiskBoardingController::RequestBoarding);
    InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AFiskBoardingController::RequestDeparture);
}

void AFiskBoardingController::RequestBoarding()
{
    ServerBoardStarterBoat();
}

void AFiskBoardingController::ServerBoardStarterBoat_Implementation()
{
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (Mode && Mode->GetStarterBoat() && PlayerState)
    {
        Mode->GetStarterBoat()->Board(PlayerState->GetPlayerId());
    }
}

void AFiskBoardingController::RequestDeparture()
{
    const AFiskStarterBoat* Boat=FindStarterBoat();
    if(Boat) { ServerStartBoatTrip(Boat->GetRouteRevision()); }
}

void AFiskBoardingController::ServerStartBoatTrip_Implementation(int32 ExpectedRevision)
{
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (Mode && Mode->GetStarterBoat() && PlayerState)
    {
        AFiskStarterBoat* Boat=Mode->GetStarterBoat();
        const bool bAuthorized=Boat->CanCaptainCommand(PlayerState->GetPlayerId(),ExpectedRevision);
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_TRIP_%s player=%d expected=%d current=%d"),
            bAuthorized?TEXT("AUTHORIZED"):TEXT("REJECTED"),PlayerState->GetPlayerId(),ExpectedRevision,Boat->GetRouteRevision());
        if(!Boat->IsAboard(PlayerState->GetPlayerId())) { ClientActionFeedback(TEXT("Trykk E for å gå om bord først."));return; }
        if(!bAuthorized) { ClientActionFeedback(Boat->GetCaptainId()==PlayerState->GetPlayerId()
            ?TEXT("Båtstatusen ble oppdatert. Prøv R igjen."):TEXT("Bare kapteinen kan starte avreise og retur. K viser mannskapet."));return; }
        const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
        if(!Day || !Day->HasStarted()) { ClientActionFeedback(TEXT("Fiskedagen starter. Prøv igjen om et øyeblikk."));return; }
        const int32 Progress=Boat->GetTripProgress();
        if(Progress==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()<120)
        { ClientActionFeedback(FString::Printf(TEXT("Avreise fra 02:00. Vent %d sekunder."),120-Day->GetElapsedSeconds()));return; }
        if(Progress==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()>=1020)
        { ClientActionFeedback(TEXT("Sjøfasen er ferdig. Start en ny dag fra hovedmenyen."));return; }
        if(Progress==AFiskStarterBoat::AtMarina)
        { ClientActionFeedback(TEXT("Turen er avsluttet. Velg Til hovedmeny og Spill for en ny dag."));return; }
        if(Progress!=AFiskStarterBoat::AtDock && Progress!=AFiskStarterBoat::AtFishingSpot)
        { ClientActionFeedback(TEXT("Båten er allerede på vei."));return; }
        Boat->StartTrip(PlayerState->GetPlayerId(),ExpectedRevision);
        ClientActionFeedback(Progress==AFiskStarterBoat::AtFishingSpot?TEXT("Retur til marinaen er startet."):TEXT("Avreise er startet. F setter ut stanga ved ankomst."));
    }
}
void AFiskBoardingController::ClientActionFeedback_Implementation(const FString& Message)
{
    ActionFeedback=Message;ActionFeedbackUntil=FPlatformTime::Seconds()+5.0;
}
FString AFiskBoardingController::GetActionFeedback() const
{
    return FPlatformTime::Seconds()<ActionFeedbackUntil?ActionFeedback:FString();
}
void AFiskBoardingController::ClientRigReply_Implementation(bool bAccepted) { RigPanel->RigReply(bAccepted); }

void AFiskBoardingController::ServerTransferCaptain_Implementation(int32 ExpectedRevision,int32 Target)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool bAccepted=Mode && Mode->GetStarterBoat() && PlayerState
        && Mode->GetStarterBoat()->TransferCaptain(PlayerState->GetPlayerId(),ExpectedRevision,Target);
#if WITH_DEV_AUTOMATION_TESTS
    if(!bAccepted) { ++RejectedCaptainRequests; }
#endif
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_TRANSFER_%s player=%d target=%d"),bAccepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():INDEX_NONE,Target);
    ClientCaptainReply(bAccepted);
}
void AFiskBoardingController::ClientCaptainReply_Implementation(bool bAccepted)
{
    CrewPanel->SetMessage(bAccepted?TEXT("Kapteinsrollen er overført."):TEXT("Overføring avvist. Bare gjeldende kaptein kan velge en annen om bord. Prøv igjen med oppdatert status."));
}
const AFiskStarterBoat* AFiskBoardingController::FindStarterBoat() const
{
    TActorIterator<AFiskStarterBoat> Boat(GetWorld());return Boat?*Boat:nullptr;
}
void AFiskBoardingController::ServerSelectGround_Implementation(int32 ExpectedRevision,int32 GroundId)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    FString Message=TEXT("Båten er ikke klar.");
    const bool bAccepted=Mode && Mode->GetStarterBoat() && PlayerState
        && Mode->GetStarterBoat()->SelectGround(PlayerState->GetPlayerId(),ExpectedRevision,GroundId,Message);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CHART_%s player=%d requested=%d"),bAccepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():INDEX_NONE,GroundId);
    ClientGroundReply(Message);
}
void AFiskBoardingController::ClientGroundReply_Implementation(const FString& Message) { SeaChart->SetMessage(Message); }
bool AFiskBoardingController::CanOpenRig() const
{
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!PlayerState || !Day || !Day->HasStarted() || Day->GetElapsedSeconds() >= AFiskDayState::SettlementStart || Selection.Slot>=GetRodCapacity() || PendingRod>=0 || !MutableRod()->CanConfigure()) { return false; }
    for (TActorIterator<AFiskStarterBoat> Boat(GetWorld()); Boat; ++Boat)
    {
        if (Boat->IsAboard(PlayerState->GetPlayerId()) && Boat->GetTripProgress() <= AFiskStarterBoat::AtFishingSpot) { return true; }
    }
    return false;
}
void AFiskBoardingController::RequestRig(int32 Bait, int32 Depth, int32 Program)
{
    ServerConfigureRig(MutableRod()->GetCastId(), MutableRod()->GetRig().Revision, Bait, Depth, Program);
}
void AFiskBoardingController::ServerConfigureRig(FGuid ExpectedCast,int32 ExpectedRevision,int32 Bait,int32 Depth,int32 Program)
{ ServerConfigureSelectedRod(Selection,ExpectedCast,ExpectedRevision,Bait,Depth,Program); }
void AFiskBoardingController::ServerConfigureSelectedRod_Implementation(FFiskRodSelection Expected,FGuid ExpectedCast,int32 ExpectedRevision,int32 Bait,int32 Depth,int32 Program)
{
    const bool bAccepted = Selection.Matches(Expected) && CanOpenRig() && MutableRod()->Configure(ExpectedCast, ExpectedRevision, Bait, Depth, Program);
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RIG_%s player=%d revision=%d bait=%d depth=%d program=%d"),
        bAccepted ? TEXT("ACCEPTED") : TEXT("REJECTED"), PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE,
        MutableRod()->GetRig().Revision, MutableRod()->GetRig().Bait, MutableRod()->GetRig().Depth, MutableRod()->GetRig().Program);
    ClientRigReply(bAccepted);
}

void AFiskBoardingController::RequestCast()
{
    ServerCastRod(MutableRod()->GetCastId());
}

void AFiskBoardingController::ServerCastRod(FGuid ExpectedId) { ServerCastSelectedRod(Selection,ExpectedId); }
void AFiskBoardingController::ServerCastSelectedRod_Implementation(FFiskRodSelection Expected,FGuid ExpectedId)
{
    if(!Selection.Matches(Expected) || !CanUseRod())
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTI_ROD_REJECTED command=cast player=%d"),PlayerState?PlayerState->GetPlayerId():-1);return; }
    if (Selection.Matches(Expected) && CanUseRod())
    {
        if (!MutableRod()->Cast(ExpectedId))
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CAST_REJECTED player=%d expected=%s current=%s"),
                PlayerState->GetPlayerId(), *ExpectedId.ToString(EGuidFormats::Digits), *MutableRod()->GetCastId().ToString(EGuidFormats::Digits));
        }
    }
}

bool AFiskBoardingController::CanUseRod() const
{
    if(!bCareerReady) { return false; }
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat = Mode ? Mode->GetStarterBoat() : nullptr;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    return Selection.Slot<GetRodCapacity() && Boat && PlayerState && Boat->IsAboard(PlayerState->GetPlayerId())
        && Boat->GetTripProgress() == AFiskStarterBoat::AtFishingSpot
        && Day && AFiskDayState::GetPhase(Day->GetElapsedSeconds()) == EFiskDayPhase::FishingAndReturn;
}

void AFiskBoardingController::RequestLift()
{
    ServerActOnRod(EFiskRodAction::Lift, MutableRod()->GetCastId());
}

void AFiskBoardingController::RequestLower()
{
    ServerActOnRod(EFiskRodAction::Lower, MutableRod()->GetCastId());
}

void AFiskBoardingController::RequestReel()
{
    ServerActOnRod(EFiskRodAction::Reel, MutableRod()->GetCastId());
}

void AFiskBoardingController::RequestStopReel() { ServerActOnRod(EFiskRodAction::StopReel, MutableRod()->GetCastId()); }
void AFiskBoardingController::RequestDragUp() { ServerActOnRod(EFiskRodAction::DragUp, MutableRod()->GetCastId()); }
void AFiskBoardingController::RequestDragDown() { ServerActOnRod(EFiskRodAction::DragDown, MutableRod()->GetCastId()); }

void AFiskBoardingController::RequestWithdraw() { ServerActOnRod(EFiskRodAction::Withdraw, MutableRod()->GetCastId()); }

void AFiskBoardingController::RequestLand()
{
    ServerActOnRod(EFiskRodAction::Land, MutableRod()->GetCastId());
}

void AFiskBoardingController::ServerActOnRod(EFiskRodAction Action,FGuid ExpectedId)
{ ServerActOnSelectedRod(Action,Selection,ExpectedId); }
void AFiskBoardingController::ServerActOnSelectedRod_Implementation(EFiskRodAction Action,FFiskRodSelection Expected,FGuid ExpectedId)
{
    if (!Selection.Matches(Expected) || !CanUseRod())
    {
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTI_ROD_REJECTED command=action player=%d"),PlayerState?PlayerState->GetPlayerId():-1);
        return;
    }

    if (!MutableRod()->IsCurrentCommand(ExpectedId))
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_COMMAND_REJECTED player=%d action=%d expected=%s current=%s"),
            PlayerState->GetPlayerId(), static_cast<int32>(Action), *ExpectedId.ToString(EGuidFormats::Digits),
            *MutableRod()->GetCastId().ToString(EGuidFormats::Digits));
        return;
    }
    switch (Action)
    {
    case EFiskRodAction::Lift:
        MutableRod()->Lift();
        break;
    case EFiskRodAction::Lower:
        MutableRod()->Lower();
        break;
    case EFiskRodAction::Reel:
        MutableRod()->Reel();
        break;
    case EFiskRodAction::StopReel: MutableRod()->StopReel(); break;
    case EFiskRodAction::DragUp: MutableRod()->AdjustDrag(1); break;
    case EFiskRodAction::DragDown: MutableRod()->AdjustDrag(-1); break;
    case EFiskRodAction::Withdraw: MutableRod()->Withdraw(); break;
    case EFiskRodAction::Land:
        MutableRod()->Land();
        break;
    }
}


void AFiskBoardingController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    if (!IsLocalController() || !PlayerState) { return; }
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(Day && Day->HasStarted())
    {
        for(TActorIterator<AFiskDaySettlement> Settlement(GetWorld());Settlement;++Settlement)
        {
            if(Settlement->GetView().DayNumber==Day->GetDayNumber())
            { CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->ObserveJournal(Settlement->GetView(),PlayerState->GetPlayerId(),Day->GetElapsedMilliseconds()); }
        }
    }
    if(Day && Day->GetDayNumber()!=ObservedDay)
    {
        ObservedDay=Day->GetDayNumber(); bAutoResultsSeen=bHelpVisible=bResultsVisible=bHistoryVisible=false;
        ResultPage=0; ReturnWarningLevel=0; RigPanel->ResetForDay();
        FInputKeyEventArgs Escape; Escape.Key=EKeys::Escape;Escape.Event=IE_Pressed;
        if(SeaChart->IsVisible()) { SeaChart->HandleKey(Escape); }
        if(CrewPanel->IsVisible()) { CrewPanel->HandleKey(Escape); }
    }
    if(Day && Day->HasStarted())
    {
        const AFiskStarterBoat* Boat=FindStarterBoat();
        if(Boat && Boat->IsAboard(PlayerState->GetPlayerId()) && Boat->GetTripProgress()==AFiskStarterBoat::AtFishingSpot
            && Day->GetElapsedSeconds()<AFiskDayState::SettlementStart)
        {
            const int32 Level=FiskSession::ReturnWarning(Boat->ReturnMarginMs(Day->GetElapsedMilliseconds()));
            if(Level>ReturnWarningLevel)
            {
                ReturnWarningLevel=Level;
                ClientActionFeedback_Implementation(Level==2?TEXT("Returvarsel: beregnet ankomst er etter fristen. Kapteinen kan fortsatt starte retur med R.")
                    :TEXT("Returvarsel: under ett minutts margin. Kapteinen bør vurdere retur med R."));
            }
        }
    }
    if(!bAutoResultsSeen && !ExpeditionPanel->IsVisible() && !MarinaPanel->IsVisible() && !bHelpVisible && !bHistoryVisible && !DeckPanel->IsVisible() && !JournalPanel->IsVisible() && !FrontEnd->IsVisible() && !CrewPanel->IsVisible()
        && !SeaChart->IsVisible() && !RigPanel->IsVisible())
    {
        for(TActorIterator<AFiskDaySettlement> Settlement(GetWorld());Settlement;++Settlement)
        {
            if(Day && Settlement->GetView().DayNumber==Day->GetDayNumber() && Settlement->GetView().bRevealed)
            { bAutoResultsSeen=true;bResultsVisible=true;ResultPage=0;ReleaseRodInputsForMenu();break; }
        }
    }
    if(MarinaPanel->WantsCamera()) { if(AFiskMarinaHub* Hub=MarinaPanel->FindHub()) { if(GetViewTarget()!=Hub) { SetViewTarget(Hub); }return; } }
    for (TActorIterator<AFiskOceanScene> Scene(GetWorld()); Scene; ++Scene)
    {
        TActorIterator<AFiskStarterBoat> Boat(GetWorld());
        if (Boat && !Boat->IsAboard(PlayerState->GetPlayerId()) && GetViewTarget() != *Scene)
        {
            SetViewTarget(*Scene);
        }
    }

    for (TActorIterator<AFiskStarterBoat> Boat(GetWorld()); Boat; ++Boat)
    {
        if (Boat->IsAboard(PlayerState->GetPlayerId()) && GetViewTarget() != *Boat)
        {
            SetViewTarget(*Boat);
            return;
        }
    }
}

void AFiskBoardingController::ResetRodForDay(int32 NewDay)
{
    if(!HasAuthority()) { return; }
    Rod->ResetForDay();ExtraRod->ResetForDay();ThirdRod->ResetForDay();
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    Selection.Reset(NewDay>0?NewDay:Day?Day->GetDayNumber():1);
    ClientRodSelection(true,Selection);
}
void AFiskBoardingController::ServerNextDay_Implementation(int32 ExpectedDay)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool Accepted=Mode && Mode->StartNextDay(this,ExpectedDay);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_NEXT_DAY_%s player=%d expected=%d"),Accepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():INDEX_NONE,ExpectedDay);
    ClientActionFeedback(Accepted?TEXT("Ny fiskedag! Mannskap, fiskeplass og oppsett er beholdt. Avreise fra 02:00.")
        :TEXT("Bare verten kan starte neste dag, etter 20:00 og ferdig oppgjør. Oppdaterte dagsdata kreves."));
}

int32 AFiskBoardingController::GetRodCapacity() const
{
    const AFiskStarterBoat* Boat=FindStarterBoat();
    return Boat && PlayerState?Boat->GetRodsFor(PlayerState->GetPlayerId()):0;
}
void AFiskBoardingController::RequestRodSelection(int32 Slot)
{
    if(PendingRod>=0) { ClientActionFeedback_Implementation(TEXT("Vent på vertens stangbekreftelse."));return; }
    if(Slot<0 || Slot>=GetRodCapacity())
    { ClientActionFeedback_Implementation(TEXT("Denne stangen er ikke tilgjengelig. Se dine egne stangplasser i oversikten."));return; }
    ReleaseRodInputsForMenu();PendingRod=Slot;ServerSelectRod(Selection.Revision,Selection.Day,Slot);
}
void AFiskBoardingController::ServerSelectRod_Implementation(int32 ExpectedRevision,int32 ExpectedDay,int32 Slot)
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const bool Allowed=Day && ExpectedDay==Day->GetDayNumber() && Selection.Select(ExpectedRevision,ExpectedDay,Slot,GetRodCapacity());
    if(Allowed)
    {
        Rod->Lower();Rod->StopReel();ExtraRod->Lower();ExtraRod->StopReel();ThirdRod->Lower();ThirdRod->StopReel();
    }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_ROD_SELECTION_%s player=%d slot=%d revision=%d"),Allowed?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():-1,Slot,Selection.Revision);
    ClientRodSelection(Allowed,Selection);
}
void AFiskBoardingController::ClientRodSelection_Implementation(bool bAccepted,FFiskRodSelection Snapshot)
{
    if(Snapshot.Revision<Selection.Revision) { return; }
    Selection=Snapshot;PendingRod=-1;
    if(!bAccepted) { ClientActionFeedback_Implementation(TEXT("Stangvalget ble avvist. Les oppdatert båtstatus og prøv igjen.")); }
}
void AFiskBoardingController::ReconcileRodCapacity()
{
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(Day && Selection.Day!=Day->GetDayNumber())
    {
        Rod->Lower();Rod->StopReel();ExtraRod->Lower();ExtraRod->StopReel();ThirdRod->Lower();ThirdRod->StopReel();
        Selection.Reset(Day->GetDayNumber());ClientRodSelection(true,Selection);
    }
    if(Selection.Slot>=GetRodCapacity() && Selection.Slot!=0)
    {
        for(int32 I=GetRodCapacity();I<3;++I) { UFiskPrototypeRod* R=const_cast<UFiskPrototypeRod*>(GetRodAt(I));R->Lower();R->StopReel();R->Withdraw(); }
        Selection.Reset(Selection.Day);ClientRodSelection(true,Selection);
    }
}
