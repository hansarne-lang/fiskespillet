#include "Boats/FiskBoardingController.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Fishing/FiskPrototypeRod.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

void AFiskBoardingController::RunLanSmoke()
{
    TActorIterator<AFiskStarterBoat> Boats(GetWorld());
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Boats || !Day || !Day->HasStarted() || !PlayerState)
    {
        return;
    }

    const AFiskStarterBoat& Boat = **Boats;
    const int32 PlayerId = PlayerState->GetPlayerId();
    const bool bCaptain = Boat.GetCaptainId() == PlayerId;
    const int32 Progress = Boat.GetTripProgress();
    if (Progress == AFiskStarterBoat::AtDock)
    {
        if (!Boat.IsAboard(PlayerId))
        {
            RequestBoarding();
        }
        else if (bCaptain && Boat.GetCrewCount() == AFiskStarterBoat::Capacity
            && AFiskDayState::GetPhase(Day->GetElapsedSeconds()) == EFiskDayPhase::FishingAndReturn)
        {
            RequestDeparture();
        }
        return;
    }

    if (Progress == AFiskStarterBoat::AtMarina)
    {
        GetWorldTimerManager().ClearTimer(LanSmokeTimer);
        return;
    }
    if (Progress != AFiskStarterBoat::AtFishingSpot)
    {
        return;
    }

    switch (Rod->GetState())
    {
    case EFiskRodState::Ready:
        RequestCast();
        break;
    case EFiskRodState::Fighting:
        if (!bSmokeTriedEarlyLanding)
        {
            bSmokeTriedEarlyLanding = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_EARLY_LANDING player=%d"), PlayerId);
            RequestLand();
            break;
        }
        RequestLift();
        RequestLower();
        RequestReel();
        break;
    case EFiskRodState::ReadyToLand:
        RequestLand();
        break;
    case EFiskRodState::Landed:
        if (!bSmokeTriedDuplicateLanding)
        {
            bSmokeTriedDuplicateLanding = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_DUPLICATE_LANDING player=%d"), PlayerId);
            RequestLand();
            RequestLand();
            if (!bCaptain)
            {
                UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_GUEST_RETURN player=%d"), PlayerId);
                RequestDeparture();
            }
        }

        if (bCaptain && HasAuthority())
        {
            int32 LandedPlayers = 0;
            for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
            {
                const AFiskBoardingController* Other = Cast<AFiskBoardingController>(It->Get());
                if (Other && Other->PlayerState && Boat.IsAboard(Other->PlayerState->GetPlayerId())
                    && Other->GetRod()->GetState() == EFiskRodState::Landed)
                {
                    ++LandedPlayers;
                }
            }
            if (LandedPlayers == AFiskStarterBoat::Capacity)
            {
                if (SmokeReturnAt == 0.0)
                {
                    SmokeReturnAt = GetWorld()->GetTimeSeconds() + 5.0;
                }
                else if (GetWorld()->GetTimeSeconds() >= SmokeReturnAt)
                {
                    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_CAPTAIN_RETURN player=%d"), PlayerId);
                    RequestDeparture();
                }
            }
        }
        break;
    case EFiskRodState::Fishing:
    case EFiskRodState::InitialRun:
        break;
    }
}
#endif
