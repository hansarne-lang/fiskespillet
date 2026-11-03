#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Bootstrap/FiskBootstrapHUD.h"
#include "DayCycle/FiskDayState.h"

AFiskBootstrapGameMode::AFiskBootstrapGameMode()
{
    HUDClass = AFiskBootstrapHUD::StaticClass();
    DefaultPawnClass = nullptr;
    GameStateClass = AFiskDayState::StaticClass();
}

void AFiskBootstrapGameMode::StartPlay()
{
    Super::StartPlay();
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_BOOTSTRAP_READY"));
}

void AFiskBootstrapGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (GetNetMode() != NM_Standalone && GetNumPlayers() >= 2)
    {
        if (AFiskDayState* Day = GetGameState<AFiskDayState>())
        {
            Day->StartDay();
        }
    }
}
