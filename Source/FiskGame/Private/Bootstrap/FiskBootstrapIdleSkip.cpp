#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"

// Test-only idle skip (FISK_SKIP_IDLE=1, set by Run-All.ps1 for PR runs). The timer only runs when the
// variable is set, and IsIdleSkipEnabled() is always false in Shipping.
void AFiskBootstrapGameMode::CheckIdleSkip()
{
    auto* Day=GetGameState<AFiskDayState>();if(!Day || !Day->HasStarted()) { IdleSince=-1;return; }
    bool bAway=false,bHome=false;
    for(const AFiskStarterBoat* Boat:Fleet)
    {
        if(!Boat) { continue; }
        const int32 Trip=Boat->GetTripProgress();
        bAway|=Trip!=AFiskStarterBoat::AtDock && Trip!=AFiskStarterBoat::AtMarina;
        bHome|=Trip==AFiskStarterBoat::AtMarina;
    }
    const double Now=GetWorld()->GetRealTimeSeconds();
    if(bAway || !bHome) { IdleSince=-1;return; }
    if(IdleSince<0) { IdleSince=Now; }
    if(!AFiskDayState::ShouldSkipIdle(Day->GetElapsedSeconds(),bAway,bHome,Now-IdleSince)) { return; }
    // The F12 milestones include 15:00, so it can take two jumps to reach 17:00.
    int32 Seconds=-1;
    for(int32 Jump=0;Jump<3 && Day->GetElapsedSeconds()<AFiskDayState::SettlementStart && Day->SkipToNextMilestone(Seconds);++Jump) {}
    UE_LOG(LogTemp,Warning,TEXT("FISKELAGET_IDLE_SKIP to=%d (test only)"),Day->GetElapsedSeconds());
    IdleSince=-1;
}
