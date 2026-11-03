#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"

AFiskBoardingController::AFiskBoardingController()
{
    Rod = CreateDefaultSubobject<UFiskPrototypeRod>(TEXT("PrototypeRod"));
}

void AFiskBoardingController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AFiskBoardingController::RequestBoarding);
    InputComponent->BindKey(EKeys::R, IE_Pressed, this, &AFiskBoardingController::RequestDeparture);
    InputComponent->BindKey(EKeys::F, IE_Pressed, this, &AFiskBoardingController::RequestCast);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AFiskBoardingController::RequestLift);
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &AFiskBoardingController::RequestLower);
    InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AFiskBoardingController::RequestReel);
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

void AFiskBoardingController::RequestCast()
{
    ServerCastRod();
}

void AFiskBoardingController::ServerCastRod_Implementation()
{
    if (CanUseRod())
    {
        Rod->Cast();
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
    ServerActOnRod(EFiskRodAction::Lift);
}

void AFiskBoardingController::RequestLower()
{
    ServerActOnRod(EFiskRodAction::Lower);
}

void AFiskBoardingController::RequestReel()
{
    ServerActOnRod(EFiskRodAction::Reel);
}

void AFiskBoardingController::RequestLand()
{
    ServerActOnRod(EFiskRodAction::Land);
}

void AFiskBoardingController::ServerActOnRod_Implementation(EFiskRodAction Action)
{
    if (!CanUseRod())
    {
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
    case EFiskRodAction::Land:
        Rod->Land();
        break;
    }
}
