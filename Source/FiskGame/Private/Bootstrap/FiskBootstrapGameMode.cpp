#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Bootstrap/FiskBootstrapHUD.h"

AFiskBootstrapGameMode::AFiskBootstrapGameMode()
{
    HUDClass = AFiskBootstrapHUD::StaticClass();
    DefaultPawnClass = nullptr;
}

void AFiskBootstrapGameMode::StartPlay()
{
    Super::StartPlay();
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_BOOTSTRAP_READY"));
}
