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
    // One F12 milestone per idle period: from home to 15:00, then after another grace period to 17:00.
    // Tests that act between 15:00 and 17:00 (CareerRelease's career-before17) get their window.
    int32 Seconds=-1;
    if(Day->SkipToNextMilestone(Seconds)) { UE_LOG(LogTemp,Warning,TEXT("FISKELAGET_IDLE_SKIP to=%d (test only)"),Seconds); }
    IdleSince=-1;
}
