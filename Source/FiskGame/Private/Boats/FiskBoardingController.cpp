#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskCombatFeedback.h"
#include "Fishing/FiskFishingHelp.h"
#include "Fishing/FiskBiteAlert.h"
#include "Economy/FiskDaySettlement.h"
#include "HAL/PlatformTime.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "World/FiskOceanScene.h"
#include "World/FiskPierWalk.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskSessionRules.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskDeckPanel.h"
#include "FiskBoatLayout.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "UI/FiskExpeditionPanel.h"
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskAnglerPanel.h"
#include "UI/FiskCollectionPanel.h"

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
    CreateDefaultSubobject<UFiskBiteAlert>(TEXT("BiteAlert"));
    FishingHelp=CreateDefaultSubobject<UFiskFishingHelp>(TEXT("FishingHelp"));
    MarinaPanel=CreateDefaultSubobject<UFiskMarinaPanel>(TEXT("MarinaPanel"));
    PierWalk=CreateDefaultSubobject<UFiskPierWalk>(TEXT("PierWalk"));
    MarinaSync=CreateDefaultSubobject<UFiskMarinaSync>(TEXT("MarinaSync"));
    RecoverySync=CreateDefaultSubobject<UFiskRecoverySync>(TEXT("RecoverySync"));
    CareerSync=CreateDefaultSubobject<UFiskCareerSync>(TEXT("CareerSync"));
    FleetSync=CreateDefaultSubobject<UFiskFleetSync>(TEXT("FleetSync"));
    ExpeditionPanel=CreateDefaultSubobject<UFiskExpeditionPanel>(TEXT("ExpeditionPanel"));
    SkipperPanel=CreateDefaultSubobject<UFiskSkipperPanel>(TEXT("SkipperPanel"));
    CollectionPanel=CreateDefaultSubobject<UFiskCollectionPanel>(TEXT("CollectionPanel"));
    AnglerPanel=CreateDefaultSubobject<UFiskAnglerPanel>(TEXT("AnglerPanel"));
}

bool AFiskBoardingController::InputKey(const FInputKeyEventArgs& Params)
{
    const bool Press=Params.Event==IE_Pressed;
    // Naming a favorite is modal: letters such as L (sea passport) must reach the name field
    // before any global shortcut (#125).
    if(SeaChart && SeaChart->IsRenaming() && SeaChart->HandleKey(Params)) { return true; }
    if(HandleFleetKey(Params)) { return true; }
    if(CollectionPanel->IsVisible()) { return CollectionPanel->HandleKey(Params); }
    if(Press && Params.Key==EKeys::Z && !AnglerPanel->IsVisible() && !SkipperPanel->IsVisible() && !MarinaPanel->IsVisible() && !ExpeditionPanel->IsVisible() && !JournalPanel->IsVisible() && !SeaChart->IsVisible() && !RigPanel->IsVisible() && !CrewPanel->IsVisible() && !DeckPanel->IsVisible() && !IsCareerVisible() && !bHelpVisible && !bResultsVisible && !bHistoryVisible) { CollectionPanel->Open();return true; }
    if(AnglerPanel->IsVisible()) { return AnglerPanel->HandleKey(Params); }
    if(SkipperPanel->IsVisible()) { return SkipperPanel->HandleKey(Params); }
    if(ExpeditionPanel->HandleKey(Params)) { return true; }
    if(MarinaPanel->HandleKey(Params)) { return true; }
    if(PierWalk->HandleKey(Params)) { return true; }
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
    if(Press && (Params.Key==EKeys::F4 || Params.Key==EKeys::L))
    {
        if(CrewPanel->IsVisible() || SeaChart->IsVisible() || RigPanel->IsVisible())
        { ClientActionFeedback_Implementation(TEXT("Lukk panelet med Escape før du åpner F4-journalen."));return true; }
        bHelpVisible=bResultsVisible=bHistoryVisible=false;if(Params.Key==EKeys::L) { JournalPanel->TogglePassport(); }else { JournalPanel->Toggle(); }return true;
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
    if(AnglerPanel->HandleKey(Params)) { return true; }
    if(SkipperPanel->HandleKey(Params)) { return true; }
    if(FishingHelp->HandleKey(Params)) { return true; }
    if(Press && Params.Key==EKeys::V)
    {
        const auto* B=FindStarterBoat();if(B && PlayerState) { ServerTravelVote(B->GetLegSerial(),!B->HasTravelVote(PlayerState->GetPlayerId())); }return true;
    }
    // Handle rod commands before a later key can change the selected holder.
    // Deferred input bindings otherwise make X followed by 1 act on rod 1.
    const bool Release=Params.Event==IE_Released;
    const bool RodKey=Params.Key==EKeys::F || Params.Key==EKeys::X || Params.Key==EKeys::LeftMouseButton
        || Params.Key==EKeys::RightMouseButton || Params.Key==EKeys::MouseScrollUp || Params.Key==EKeys::MouseScrollDown || Params.Key==EKeys::SpaceBar;
    if((Press || Params.Event==IE_Repeat) && (Params.Key==EKeys::Up || Params.Key==EKeys::Down)
        && CastChecked<UFiskGameInstance>(GetGameInstance())->bMouseRod && GetRod() && GetRod()->GetState()==EFiskRodState::Fighting)
    {
        // Arrow keys step the mouse-steered rod, so a fight also works without the mouse.
        RodStick.Move(Params.Key==EKeys::Up?FFiskRodStick::KeyStep:-FFiskRodStick::KeyStep);bRodSteering=true;return true;
    }
    if(RodKey)
    {
        // This handler consumes the buttons before PlayerInput sees them, so track held buttons here.
        if(Params.Key==EKeys::LeftMouseButton && (Press || Release)) { bLiftHeld=Press; }
        if(Params.Key==EKeys::RightMouseButton && (Press || Release)) { bReelHeld=Press; }
        if(Press && (Params.Key==EKeys::LeftMouseButton || Params.Key==EKeys::RightMouseButton)) { bRodSteering=false;StickEngage.Reset(); }
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
void AFiskBoardingController::ReleaseRodInputsForMenu() { bLiftHeld=bReelHeld=false; RequestLower(); RequestStopReel(); }

void AFiskBoardingController::TickRodStick(float DeltaTime)
{
    // Mouse-steered rod (#132). The client turns rod movement into the same Lift/Lower/Reel/StopReel
    // commands the mouse buttons send, so the server's fight rules and limits are unchanged.
    const UFiskGameInstance* GI=Cast<UFiskGameInstance>(GetGameInstance());
    UFiskPrototypeRod* Current=MutableRod();
    const EFiskRodState State=Current?Current->GetState():EFiskRodState::Ready;
    const bool Fight=State==EFiskRodState::InitialRun || State==EFiskRodState::Fighting || State==EFiskRodState::ReadyToLand;
    if(!Fight || !GI || !GI->bMouseRod)
    {
        if(StickCast.IsValid()) { StickCast.Invalidate();RodStick.Reset();StickEngage.Reset();bRodSteering=bStickRaised=bStickReeling=false; }
        return;
    }
    if(Current->GetCastId()!=StickCast) { StickCast=Current->GetCastId();RodStick.Reset();StickEngage.Reset();bRodSteering=bStickRaised=bStickReeling=false; }
    const bool Menu=PendingRod>=0 || bHelpVisible || bResultsVisible || bHistoryVisible || bFleetVisible || FrontEnd->IsVisible() || RigPanel->IsVisible()
        || SeaChart->IsVisible() || CrewPanel->IsVisible() || JournalPanel->IsVisible() || ExpeditionPanel->IsVisible() || MarinaPanel->IsVisible()
        || AnglerPanel->IsVisible() || SkipperPanel->IsVisible() || CollectionPanel->IsVisible();
    float DX=0,DY=0;GetInputMouseDelta(DX,DY);
    // A held mouse button means the player fights with buttons; the rod waits for the next movement.
    if(bLiftHeld || bReelHeld) { bRodSteering=false;DY=0; }
    if(Menu) { DY=0; }
    // Only a deliberate movement takes over the rod (or stops the AI helper), never a single jump.
    if(!bRodSteering)
    {
        if(!StickEngage.Feed(DY,DeltaTime)) { DY=0; }
        else
        {
            StickEngage.Reset();bRodSteering=true;
            if(FishingHelp->IsAssisting()) { FishingHelp->StopAssist(true); }
        }
    }
    if(DY!=0 && bRodSteering && !FishingHelp->IsAssisting()) { RodStick.Move(DY/FFiskRodStick::TravelForSensitivity(GI->RodSensitivity)); }
    RodStick.Advance(DeltaTime,Current->IsSurging() || State==EFiskRodState::InitialRun);
    if(Menu || !bRodSteering || FishingHelp->IsAssisting() || State!=EFiskRodState::Fighting) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();
    const bool Raise=RodStick.WantsRaised(),Reel=RodStick.WantsReeling();
    // Resend when the server has not taken the wanted state, e.g. after a menu released the rod.
    const bool Drift=(Raise!=Current->IsRaised() || Reel!=Current->IsReeling()) && Now-StickSentAt>0.3;
    const bool Changed=(Raise!=bStickRaised || Reel!=bStickReeling) && Now-StickSentAt>=0.05;
    if(!Changed && !Drift) { return; }
    if(Drift || Raise!=bStickRaised) { if(Raise) { RequestLift(); }else { RequestLower(); } }
    if(Drift || Reel!=bStickReeling) { if(Reel) { RequestReel(); }else { RequestStopReel(); } }
    bStickRaised=Raise;bStickReeling=Reel;StickSentAt=Now;
}

void AFiskBoardingController::BeginPlay()
{
    Super::BeginPlay();
#if WITH_DEV_AUTOMATION_TESTS
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskFleetRole="),Smoke.FleetRole) && Smoke.FleetRole>=0 && Smoke.FleetRole<12)
    { Smoke.bFleetResume=FParse::Param(FCommandLine::Get(),TEXT("FiskFleetResume"));FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.FleetDir);GetWorldTimerManager().SetTimer(Smoke.FleetTimer,this,&AFiskBoardingController::RunFleetSmoke,.5f,true,3.f); }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskCargoSmoke="),Smoke.CargoMode))
    { FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.CargoDir);GetWorldTimerManager().SetTimer(Smoke.CargoTimer,this,&AFiskBoardingController::RunCargoSmoke,.5f,true,3.f); }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskCollectionSmoke="),Smoke.CollectionMode))
    { FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.CollectionDir);GetWorldTimerManager().SetTimer(Smoke.CollectionTimer,this,&AFiskBoardingController::RunCollectionSmoke,.5f,true,3.f); }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskAnglerSmoke="),Smoke.AnglerMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.AnglerDir);
        GetWorldTimerManager().SetTimer(Smoke.AnglerTimer,this,&AFiskBoardingController::RunAnglerSmoke,.5f,true,2.f);
    }
#endif
#if WITH_DEV_AUTOMATION_TESTS
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskSkipperSmoke="),Smoke.SkipperMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.SkipperDir);
        GetWorldTimerManager().SetTimer(Smoke.SkipperTimer,this,&AFiskBoardingController::RunSkipperSmoke,.5f,true,2.f);
    }
#endif
    // Remote server controllers have no PlayerInput and never receive PlayerTick.
    // Settlement and durable snapshot retries must run for every connection.
    GetWorldTimerManager().SetTimer(CareerTimer,this,&AFiskBoardingController::TickCareer,.5f,true);

#if WITH_DEV_AUTOMATION_TESTS
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskExplorationSmoke="),Smoke.ExplorationMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.ExplorationDir);
        GetWorldTimerManager().SetTimer(Smoke.ExplorationTimer,this,&AFiskBoardingController::RunExplorationSmoke,.5f,true,2.f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskExpeditionSmoke="),Smoke.ExpeditionSmokeMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.ExpeditionSmokeDir);
        GetWorldTimerManager().SetTimer(Smoke.ExpeditionSmokeTimer,this,&AFiskBoardingController::RunExpeditionSmoke,.5f,true,2.f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskCareerReleaseSmoke="),Smoke.CareerSmokeMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.CareerSmokeDir);
        GetWorldTimerManager().SetTimer(Smoke.CareerSmokeTimer,this,&AFiskBoardingController::RunCareerReleaseSmoke,.5f,true,2.f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskFishingReleaseSmoke="),Smoke.FishingReleaseMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.FishingReleaseCapture);
        GetWorldTimerManager().SetTimer(Smoke.FishingReleaseTimer,this,&AFiskBoardingController::RunFishingReleaseSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskNavigationSmoke="),Smoke.NavigationMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.NavigationCaptureDir);
        GetWorldTimerManager().SetTimer(Smoke.NavigationTimer,this,&AFiskBoardingController::RunNavigationSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskJournalSmoke="),Smoke.JournalMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.JournalCaptureDir);
        FParse::Value(FCommandLine::Get(),TEXT("FiskJournalExpectedDepth="),Smoke.JournalExpectedDepth);
        GetWorldTimerManager().SetTimer(Smoke.JournalTimer,this,&AFiskBoardingController::RunJournalSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskSessionSmoke="),Smoke.SessionMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.SessionCaptureDir);
        GetWorldTimerManager().SetTimer(Smoke.SessionTimer,this,&AFiskBoardingController::RunSessionSmoke,0.5f,true,2.0f);
    }
    if(IsLocalController() && FParse::Value(FCommandLine::Get(),TEXT("FiskContentSmoke="),Smoke.ContentMode))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),Smoke.ContentCaptureDir);
        GetWorldTimerManager().SetTimer(Smoke.ContentTimer,this,&AFiskBoardingController::RunContentSmoke,0.5f,true,2.0f);
    }
    if (IsLocalController() && GetNetMode() != NM_Standalone
        && FParse::Param(FCommandLine::Get(), TEXT("FiskLanSmoke")))
    {
        GetWorldTimerManager().SetTimer(Smoke.LanSmokeTimer, this, &AFiskBoardingController::RunLanSmoke, 0.5f, true);
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
    if (Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState)
    {
        if(!IsCareerReady() || !Mode->CanJoinFleet(GetServerCareer().Id)) { ClientActionFeedback(TEXT("Karrieren må være klar. Har profilen reist i dag, venter ombordstigning til neste døgn – også etter gjenoppkobling."));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_BOARD_LOCKED player=%d"),PlayerState->GetPlayerId());return; }
        auto* B=Mode->GetBoatForPlayer(PlayerState->GetPlayerId());B->Board(PlayerState->GetPlayerId());
        if(!B->IsAboard(PlayerState->GetPlayerId())) { ClientActionFeedback(TEXT("Denne båten er full eller har reist. Tab åpner flåten; C henter din egen båt ved brygga.")); }
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
    if (Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState)
    {
        AFiskStarterBoat* Boat=Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE);
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
        if(Progress!=AFiskStarterBoat::AtDock && Progress!=AFiskStarterBoat::AtFishingSpot && !(Progress>=0 && Progress<100))
        { ClientActionFeedback(TEXT("Båten er allerede på vei."));return; }
        Boat->StartTrip(PlayerState->GetPlayerId(),ExpectedRevision);
        ClientActionFeedback(Progress!=AFiskStarterBoat::AtDock?TEXT("Retur til marinaen er startet."):TEXT("Avreise er startet. F setter ut stanga ved ankomst."));
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
    const bool bAccepted=Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState
        && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->TransferCaptain(PlayerState->GetPlayerId(),ExpectedRevision,Target);
#if WITH_DEV_AUTOMATION_TESTS
    if(!bAccepted) { ++Smoke.RejectedCaptainRequests; }
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
    if(PlayerState) { for(TActorIterator<AFiskStarterBoat> B(GetWorld());B;++B) { if(B->IsAboard(PlayerState->GetPlayerId())) { return *B; } } }
    if(SelectedFleetBoat) { return SelectedFleetBoat; }
    TActorIterator<AFiskStarterBoat> Boat(GetWorld());return Boat?*Boat:nullptr;
}
void AFiskBoardingController::ServerSelectGround_Implementation(int32 ExpectedRevision,int32 GroundId)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    FString Message=TEXT("Båten er ikke klar.");
    const bool bAccepted=Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState
        && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->SelectGround(PlayerState->GetPlayerId(),ExpectedRevision,GroundId,Message);
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
    if(!IsCareerReady()) { return false; }
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat = Mode ? Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) : nullptr;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    return Selection.Slot<GetRodCapacity() && Boat && PlayerState && Boat->IsAboard(PlayerState->GetPlayerId())
        && !Boat->GetCargo().bMustReturn && Boat->GetTripProgress() == AFiskStarterBoat::AtFishingSpot
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
    TickRodStick(DeltaTime);
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
    if(!bAutoResultsSeen && !bFleetVisible && !ExpeditionPanel->IsVisible() && !MarinaPanel->IsVisible() && !bHelpVisible && !bHistoryVisible && !DeckPanel->IsVisible() && !JournalPanel->IsVisible() && !FrontEnd->IsVisible() && !CrewPanel->IsVisible()
        && !SeaChart->IsVisible() && !RigPanel->IsVisible())
    {
        for(TActorIterator<AFiskDaySettlement> Settlement(GetWorld());Settlement;++Settlement)
        {
            if(Day && Settlement->GetView().DayNumber==Day->GetDayNumber() && Settlement->GetView().bRevealed)
            { bAutoResultsSeen=true;bResultsVisible=true;ResultPage=0;ReleaseRodInputsForMenu();break; }
        }
    }
    if(PierWalk->IsEnabled() && !PierWalk->HasMenu()) { if(auto* Hub=MarinaPanel->FindHub()) { if(GetViewTarget()!=Hub) { SetViewTarget(Hub); }return; } }
    if(MarinaPanel->WantsCamera()) { if(AFiskMarinaHub* Hub=MarinaPanel->FindHub()) { if(GetViewTarget()!=Hub) { SetViewTarget(Hub); }return; } }
    for (TActorIterator<AFiskOceanScene> Scene(GetWorld()); Scene; ++Scene)
    {
        const auto* Boat=FindStarterBoat();
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
void AFiskBoardingController::TickCareer()
{
    if(HasAuthority()) { ReconcileRodCapacity(); }
    CareerSync->TickSync();
}
// Career panel (P): shop and boat choice. The career data and its network calls are in UFiskCareerSync.
bool AFiskBoardingController::HandleCareerKey(const FInputKeyEventArgs& A)
{
    const bool Press=A.Event==IE_Pressed;
    if(!bCareerVisible && Press && A.Key==EKeys::P)
    {
        // The chart owns typed letters while naming a favorite.
        if(SeaChart->IsVisible()) { return false; }
        if(FrontEnd->IsVisible() || RigPanel->IsVisible() || SeaChart->IsVisible() || CrewPanel->IsVisible() || JournalPanel->IsVisible() || DeckPanel->IsVisible()
            || bHelpVisible || bResultsVisible || bHistoryVisible) { ClientActionFeedback_Implementation(TEXT("Lukk panelet først med Escape."));return true; }
        bCareerVisible=true;ReleaseRodInputsForMenu();bShowMouseCursor=true;SetInputMode(FInputModeGameAndUI());return true;
    }
    if(!bCareerVisible) { return false; }
    if(!Press) { return true; }
    if(A.Key==EKeys::P || A.Key==EKeys::Escape) { bCareerVisible=false;bShowMouseCursor=false;SetInputMode(FInputModeGameOnly());return true; }
    if(A.Key==EKeys::Up) { ShopRow=(ShopRow+5)%6; }
    if(A.Key==EKeys::Down) { ShopRow=(ShopRow+1)%6; }
    bool Choose=A.Key==EKeys::Enter;
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;int32 W=0,H=0;GetViewportSize(W,H);const float S=FMath::Min(W/1280.f,H/720.f);
        if(S>0 && GetWorld()->GetGameViewport() && GetWorld()->GetGameViewport()->GetMousePosition(Mouse))
        { for(int32 I=0;I<6;++I) { if(FBox2D(FVector2D(52*S,(180+58*I)*S),FVector2D(1210*S,(230+58*I)*S)).IsInside(Mouse)) { ShopRow=I;Choose=true; } } }
    }
    UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());
    if(A.Key==EKeys::F5) { CareerSync->RetryLocalSave(); }
    if(Choose)
    { if(ShopRow<4) { CareerSync->ServerBuyCareer(FGuid::NewGuid(),GI->Career.Revision,ShopRow); }else if(const AFiskStarterBoat* Boat=FindStarterBoat()) { CareerSync->ServerChooseBoat(Boat->GetRouteRevision(),ShopRow-4); } }
    return true;
}
void AFiskBoardingController::DrawCareer(AHUD& HUD,UCanvas& C)
{
    const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetGameInstance());const FFiskCareerProfile& P=GI->Career;
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.45f,.94f,.82f),Gold(1,.79f,.36f);
    auto T=[&](const FString& Text,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(Text,Color,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);
    T(TEXT("KARRIERE OG MARINABUTIKK"),52,55,2);T(P.Money(),990,61,1.8f,Gold);
    T(FString::Printf(TEXT("%s | Profil %s | Kvitteringer %d/512"),*P.Name,*P.Id.ToString(EGuidFormats::Digits).Left(8),P.Revision),52,105,1.f,Teal);
    T(TEXT("Kjøp ved brygga. Dagens nye fangstinntekt bokføres først ved 17:00."),52,143,1.f);
    const TCHAR* Effects[]={TEXT("15% høyere grense for linebrudd"),TEXT("15% raskere innsveiving"),TEXT("10% sterkere pumping"),TEXT("4 plasser | 6 holdere | 25% raskere"),TEXT("Velg startbåt: 2 plasser / 2 holdere"),TEXT("Velg eid offshorebåt: 4 plasser / 6 holdere")};
    for(int32 I=0;I<6;++I)
    {
        const float Y=180+58*I;HUD.DrawRect(I==ShopRow?FLinearColor(.14f,.34f,.38f):FLinearColor(.035f,.12f,.16f),52*S,Y*S,1158*S,50*S);
        const FString Label=I<4?FFiskCareerProfile::ItemName(I):I==4?TEXT("Bruk startbåt"):TEXT("Bruk offshorebåt");
        T(Label,68,Y+7,1.1f);T(Effects[I],440,Y+12,.95f,Teal);
        T(I<4?(P.Has(I)?TEXT("EID"):FString::Printf(TEXT("$%lld"),static_cast<long long>(FFiskCareerProfile::Price(I)/100))):TEXT("VELG"),1050,Y+12,1.f,Gold);
    }
    T(GI->CareerMessage,52,551,.95f,Gold);T(GetActionFeedback(),52,581,.85f);
    T(TEXT("Piler / klikk: velg | Enter: kjøp eller bruk båt | F5: lagre igjen | P / Escape: lukk"),52,622,.95f,Teal);
    T(TEXT("Testøkonomi: ny karriere starter med $30 000 én gang. Egen fangst gir egen inntekt."),52,648,.85f);
}
bool AFiskBoardingController::AreRodsIdle() const
{ for(int32 Slot=0;Slot<3;++Slot) { if(!GetRodAt(Slot)->CanConfigure()) { return false; } }return true; }
void AFiskBoardingController::SetSelectedFleetBoat(AFiskStarterBoat* Boat) { SelectedFleetBoat=Boat; }
