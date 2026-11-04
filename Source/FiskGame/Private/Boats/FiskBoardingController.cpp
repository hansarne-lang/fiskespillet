#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskCombatFeedback.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "EngineUtils.h"
#include "World/FiskOceanScene.h"

AFiskBoardingController::AFiskBoardingController()
{
    Rod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("PrototypeRod"));
    FrontEnd = CreateDefaultSubobject<UFiskFrontEnd>(TEXT("FrontEnd"));
    RigPanel = CreateDefaultSubobject<UFiskRigPanel>(TEXT("RigPanel"));
    CreateDefaultSubobject<UFiskCombatFeedback>(TEXT("CombatFeedback"));
}

bool AFiskBoardingController::InputKey(const FInputKeyEventArgs& Params)
{
    if (RigPanel && RigPanel->HandleKey(Params)) { return true; }
    if (FrontEnd && FrontEnd->HandleKey(Params)) { return true; }
    return Super::InputKey(Params);
}
void AFiskBoardingController::ReleaseRodInputsForMenu() { RequestLower(); RequestStopReel(); }

void AFiskBoardingController::BeginPlay()
{
    Super::BeginPlay();

#if WITH_DEV_AUTOMATION_TESTS
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
    ServerStartBoatTrip();
}

void AFiskBoardingController::ServerStartBoatTrip_Implementation()
{
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (Mode && Mode->GetStarterBoat() && PlayerState)
    {
        Mode->GetStarterBoat()->StartTrip(PlayerState->GetPlayerId());
    }
}

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
