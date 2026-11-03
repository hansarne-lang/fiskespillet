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
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat = Mode ? Mode->GetStarterBoat() : nullptr;
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (Boat && PlayerState && Boat->IsAboard(PlayerState->GetPlayerId())
        && Boat->GetTripProgress() == AFiskStarterBoat::AtFishingSpot
        && Day && AFiskDayState::GetPhase(Day->GetElapsedSeconds()) == EFiskDayPhase::FishingAndReturn)
    {
        Rod->Cast();
    }
}
