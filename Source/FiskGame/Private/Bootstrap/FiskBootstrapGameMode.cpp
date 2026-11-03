#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Bootstrap/FiskBootstrapHUD.h"
#include "DayCycle/FiskDayState.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

AFiskBootstrapGameMode::AFiskBootstrapGameMode()
{
    HUDClass = AFiskBootstrapHUD::StaticClass();
    DefaultPawnClass = nullptr;
    GameStateClass = AFiskDayState::StaticClass();
    PlayerControllerClass = AFiskBoardingController::StaticClass();
}

void AFiskBootstrapGameMode::StartPlay()
{
    Super::StartPlay();
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_BOOTSTRAP_READY"));
}

void AFiskBootstrapGameMode::InitGameState()
{
    Super::InitGameState();
    StarterBoat = GetWorld()->SpawnActor<AFiskStarterBoat>();
}

void AFiskBootstrapGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (StarterBoat && NewPlayer->IsLocalController() && NewPlayer->PlayerState)
    {
        StarterBoat->SetOwnerPlayer(NewPlayer->PlayerState->GetPlayerId());
    }

    if (GetNetMode() != NM_Standalone && GetNumPlayers() >= 2)
    {
        if (AFiskDayState* Day = GetGameState<AFiskDayState>())
        {
            Day->StartDay();
        }
    }
}

void AFiskBootstrapGameMode::Logout(AController* Exiting)
{
    if (StarterBoat && Exiting->PlayerState)
    {
        StarterBoat->Leave(Exiting->PlayerState->GetPlayerId());
    }

    Super::Logout(Exiting);
}
