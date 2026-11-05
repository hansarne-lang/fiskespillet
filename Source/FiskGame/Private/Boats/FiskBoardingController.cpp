#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
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

AFiskBoardingController::AFiskBoardingController()
{
    Rod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("PrototypeRod"));
    FrontEnd = CreateDefaultSubobject<UFiskFrontEnd>(TEXT("FrontEnd"));
    RigPanel = CreateDefaultSubobject<UFiskRigPanel>(TEXT("RigPanel"));
    SeaChart = CreateDefaultSubobject<UFiskSeaChart>(TEXT("SeaChart"));
    CrewPanel = CreateDefaultSubobject<UFiskCrewPanel>(TEXT("CrewPanel"));
    JournalPanel = CreateDefaultSubobject<UFiskJournalPanel>(TEXT("JournalPanel"));
    DeckPanel = CreateDefaultSubobject<UFiskDeckPanel>(TEXT("DeckPanel"));
    CreateDefaultSubobject<UFiskCombatFeedback>(TEXT("CombatFeedback"));
}

bool AFiskBoardingController::InputKey(const FInputKeyEventArgs& Params)
{
    const bool Press=Params.Event==IE_Pressed;
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
    return Super::InputKey(Params);
}
void AFiskBoardingController::ReleaseRodInputsForMenu() { RequestLower(); RequestStopReel(); }

void AFiskBoardingController::BeginPlay()
{
    Super::BeginPlay();

#if WITH_DEV_AUTOMATION_TESTS
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
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AFiskBoardingController::RequestCast);
    InputComponent->BindKey(EKeys::X, IE_Pressed, this, &AFiskBoardingController::RequestWithdraw);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFiskBoardingController::RequestLift);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFiskBoardingController::RequestLower);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AFiskBoardingController::RequestReel);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AFiskBoardingController::RequestStopReel);
    InputComponent->BindKey(EKeys::MouseScrollUp, IE_Pressed, this, &AFiskBoardingController::RequestDragUp);
    InputComponent->BindKey(EKeys::MouseScrollDown, IE_Pressed, this, &AFiskBoardingController::RequestDragDown);
    InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AFiskBoardingController::RequestLand);
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
    if (!PlayerState || !Day || !Day->HasStarted() || Day->GetElapsedSeconds() >= AFiskDayState::SettlementStart || !Rod->CanConfigure()) { return false; }
    for (TActorIterator<AFiskStarterBoat> Boat(GetWorld()); Boat; ++Boat)
    {
        if (Boat->IsAboard(PlayerState->GetPlayerId()) && Boat->GetTripProgress() <= AFiskStarterBoat::AtFishingSpot) { return true; }
    }
    return false;
}
void AFiskBoardingController::RequestRig(int32 Bait, int32 Depth, int32 Program)
{
    ServerConfigureRig(Rod->GetCastId(), Rod->GetRig().Revision, Bait, Depth, Program);
}
void AFiskBoardingController::ServerConfigureRig_Implementation(FGuid ExpectedCast, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program)
{
    const bool bAccepted = CanOpenRig() && Rod->Configure(ExpectedCast, ExpectedRevision, Bait, Depth, Program);
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RIG_%s player=%d revision=%d bait=%d depth=%d program=%d"),
        bAccepted ? TEXT("ACCEPTED") : TEXT("REJECTED"), PlayerState ? PlayerState->GetPlayerId() : INDEX_NONE,
        Rod->GetRig().Revision, Rod->GetRig().Bait, Rod->GetRig().Depth, Rod->GetRig().Program);
    ClientRigReply(bAccepted);
}

void AFiskBoardingController::RequestCast()
{
    ServerCastRod(Rod->GetCastId());
}

void AFiskBoardingController::ServerCastRod_Implementation(FGuid ExpectedId)
{
    if (CanUseRod())
    {
        if (!Rod->Cast(ExpectedId))
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CAST_REJECTED player=%d expected=%s current=%s"),
                PlayerState->GetPlayerId(), *ExpectedId.ToString(EGuidFormats::Digits), *Rod->GetCastId().ToString(EGuidFormats::Digits));
        }
    }
}

bool AFiskBoardingController::CanUseRod() const
{
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat = Mode ? Mode->GetStarterBoat() : nullptr;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    return Boat && PlayerState && Boat->IsAboard(PlayerState->GetPlayerId())
        && Boat->GetTripProgress() == AFiskStarterBoat::AtFishingSpot
        && Day && AFiskDayState::GetPhase(Day->GetElapsedSeconds()) == EFiskDayPhase::FishingAndReturn;
}

void AFiskBoardingController::RequestLift()
{
    ServerActOnRod(EFiskRodAction::Lift, Rod->GetCastId());
}

void AFiskBoardingController::RequestLower()
{
    ServerActOnRod(EFiskRodAction::Lower, Rod->GetCastId());
}

void AFiskBoardingController::RequestReel()
{
    ServerActOnRod(EFiskRodAction::Reel, Rod->GetCastId());
}

void AFiskBoardingController::RequestStopReel() { ServerActOnRod(EFiskRodAction::StopReel, Rod->GetCastId()); }
void AFiskBoardingController::RequestDragUp() { ServerActOnRod(EFiskRodAction::DragUp, Rod->GetCastId()); }
void AFiskBoardingController::RequestDragDown() { ServerActOnRod(EFiskRodAction::DragDown, Rod->GetCastId()); }

void AFiskBoardingController::RequestWithdraw() { ServerActOnRod(EFiskRodAction::Withdraw, Rod->GetCastId()); }

void AFiskBoardingController::RequestLand()
{
    ServerActOnRod(EFiskRodAction::Land, Rod->GetCastId());
}

void AFiskBoardingController::ServerActOnRod_Implementation(EFiskRodAction Action, FGuid ExpectedId)
{
    if (!CanUseRod())
    {
        return;
    }

    if (!Rod->IsCurrentCommand(ExpectedId))
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_COMMAND_REJECTED player=%d action=%d expected=%s current=%s"),
            PlayerState->GetPlayerId(), static_cast<int32>(Action), *ExpectedId.ToString(EGuidFormats::Digits),
            *Rod->GetCastId().ToString(EGuidFormats::Digits));
        return;
    }
    switch (Action)
    {
    case EFiskRodAction::Lift:
        Rod->Lift();
        break;
    case EFiskRodAction::Lower:
        Rod->Lower();
        break;
    case EFiskRodAction::Reel:
        Rod->Reel();
        break;
    case EFiskRodAction::StopReel: Rod->StopReel(); break;
    case EFiskRodAction::DragUp: Rod->AdjustDrag(1); break;
    case EFiskRodAction::DragDown: Rod->AdjustDrag(-1); break;
    case EFiskRodAction::Withdraw: Rod->Withdraw(); break;
    case EFiskRodAction::Land:
        Rod->Land();
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
            const int32 Level=FiskSession::ReturnWarning(FiskGrounds::ReturnMarginMs(Day->GetElapsedMilliseconds(),100,Boat->GetGroundId()));
            if(Level>ReturnWarningLevel)
            {
                ReturnWarningLevel=Level;
                ClientActionFeedback_Implementation(Level==2?TEXT("Returvarsel: beregnet ankomst er etter fristen. Kapteinen kan fortsatt starte retur med R.")
                    :TEXT("Returvarsel: under ett minutts margin. Kapteinen bør vurdere retur med R."));
            }
        }
    }
    if(!bAutoResultsSeen && !bHelpVisible && !bHistoryVisible && !DeckPanel->IsVisible() && !JournalPanel->IsVisible() && !FrontEnd->IsVisible() && !CrewPanel->IsVisible()
        && !SeaChart->IsVisible() && !RigPanel->IsVisible())
    {
        for(TActorIterator<AFiskDaySettlement> Settlement(GetWorld());Settlement;++Settlement)
        {
            if(Day && Settlement->GetView().DayNumber==Day->GetDayNumber() && Settlement->GetView().bRevealed)
            { bAutoResultsSeen=true;bResultsVisible=true;ResultPage=0;ReleaseRodInputsForMenu();break; }
        }
    }
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

void AFiskBoardingController::ResetRodForDay() { Rod->ResetForDay(); }
void AFiskBoardingController::ServerNextDay_Implementation(int32 ExpectedDay)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool Accepted=Mode && Mode->StartNextDay(this,ExpectedDay);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_NEXT_DAY_%s player=%d expected=%d"),Accepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PlayerState?PlayerState->GetPlayerId():INDEX_NONE,ExpectedDay);
    ClientActionFeedback(Accepted?TEXT("Ny fiskedag! Mannskap, fiskeplass og oppsett er beholdt. Avreise fra 02:00.")
        :TEXT("Bare verten kan starte neste dag, etter 20:00 og ferdig oppgjør. Oppdaterte dagsdata kreves."));
}
