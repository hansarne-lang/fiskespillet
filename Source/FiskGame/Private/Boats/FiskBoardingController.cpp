#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "InputCoreTypes.h"

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
