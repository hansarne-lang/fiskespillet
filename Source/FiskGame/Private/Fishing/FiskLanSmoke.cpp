#include "Boats/FiskBoardingController.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Fishing/FiskPrototypeRod.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Economy/FiskDaySettlement.h"

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
    const int32 Minute = Day->GetElapsedSeconds() / 60;
    if (SmokeClockMinute != Minute)
    {
        SmokeClockMinute = Minute;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_CLOCK ms=%d"), Day->GetElapsedMilliseconds());
    }
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
        TActorIterator<AFiskDaySettlement> Settlements(GetWorld());
        if (!Settlements) { return; }
        const FFiskSettlementView& View = Settlements->GetView();
        if (FParse::Param(FCommandLine::Get(), TEXT("FiskRepeatSmoke"))
            && Day->GetElapsedSeconds() < AFiskDayState::SettlementStart && (View.bRevealed || !View.Results.IsEmpty()))
        {
            UE_LOG(LogTemp, Error, TEXT("FISKELAGET_REPEAT_FAILED early_result_exposure"));
        }
        if (!bSmokeCheckedPrivacy && Day->GetElapsedSeconds() < AFiskDayState::SettlementStart)
        {
            bSmokeCheckedPrivacy = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_PRIVATE revealed=%d rows=%d"),
                View.bRevealed ? 1 : 0, View.Results.Num());
        }
        if (View.bRevealed && !bSmokeSawSettlement)
        {
            bSmokeSawSettlement = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_REVEAL ms=%d rows=%d"),
                Day->GetElapsedMilliseconds(), View.Results.Num());
        }
        if (Day->GetElapsedSeconds() == AFiskDayState::DayEnd && !bSmokeSawEnd)
        {
            bSmokeSawEnd = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_DAY_END ms=%d"), Day->GetElapsedMilliseconds());
            GetWorldTimerManager().ClearTimer(LanSmokeTimer);
        }
        return;
    }

    if (Progress != AFiskStarterBoat::AtFishingSpot)
    {
        return;
    }


    if (FParse::Param(FCommandLine::Get(), TEXT("FiskRepeatSmoke")))
    {
        RunRepeatedFishingSmoke(Boat, *Day);
        return;
    }
    FString FightCase;
    FParse::Value(FCommandLine::Get(), TEXT("FiskFightCase="), FightCase);
    switch (Rod->GetState())
    {
    case EFiskRodState::Ready: RequestCast(); break;
    case EFiskRodState::Fighting:
    {
        if (!bSmokeTriedEarlyLanding)
        {
            bSmokeTriedEarlyLanding = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_EARLY_LANDING player=%d"), PlayerId);
            RequestLand();
        }
        if (SmokeParallelAt < 0.0)
        {
            // Keep tension while waiting for the other independent random bite.
            RequestLift();
            RequestStopReel();
            if (Boat.GetFishVisual(0).Phase != EFiskFishVisualPhase::Fighting
                || Boat.GetFishVisual(1).Phase != EFiskFishVisualPhase::Fighting) { break; }
            SmokeParallelAt = Day->GetElapsedRealSeconds();
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_PARALLEL_FIGHTS player=%d"), PlayerId);
        }
        const double Age = Day->GetElapsedRealSeconds() - SmokeParallelAt;
        const bool bBreak = !bCaptain && FightCase == TEXT("break");
        const bool bSlack = !bCaptain && FightCase == TEXT("slack");
        if (!bSmokeAdjustedDrag && Age >= 1.0)
        {
            bSmokeAdjustedDrag = true;
            if (bBreak) { for (int32 I = 0; I < 6; ++I) { RequestDragUp(); } }
            else if (bSlack) { for (int32 I = 0; I < 3; ++I) { RequestDragDown(); } }
            else { RequestDragDown(); RequestDragUp(); }
        }
        if (bBreak && bSmokeAdjustedDrag) { RequestLift(); RequestReel(); }
        else if (bSlack && bSmokeAdjustedDrag) { RequestLower(); RequestStopReel(); }
        else if (Rod->IsSurging()) { RequestLower(); RequestStopReel(); }
        else if ((static_cast<int32>(Age / 2.0) % 2) == 0) { RequestLift(); RequestStopReel(); }
        else { RequestLower(); RequestReel(); }
        break;
    }
    case EFiskRodState::ReadyToLand: RequestLand(); break;
    case EFiskRodState::Landed:
    case EFiskRodState::Lost:
        if (!bSmokeTriedDuplicateLanding)
        {
            bSmokeTriedDuplicateLanding = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_%s player=%d"),
                Rod->GetState() == EFiskRodState::Landed ? TEXT("DUPLICATE_LANDING") : TEXT("LOSS_LANDING"), PlayerId);
            RequestLand(); RequestLand();
            if (!bCaptain)
            {
                UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_GUEST_RETURN player=%d"), PlayerId);
                RequestDeparture();
            }
        }
        break;
    default: break;
    }

    if (bCaptain && HasAuthority())
    {
        int32 FinishedPlayers = 0;
        for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            const AFiskBoardingController* Other = Cast<AFiskBoardingController>(It->Get());
            if (!Other || !Other->PlayerState || !Boat.IsAboard(Other->PlayerState->GetPlayerId())) { continue; }
            const EFiskRodState State = Other->GetRod()->GetState();
            if (State == EFiskRodState::Landed || State == EFiskRodState::Lost) { ++FinishedPlayers; }
        }
        if (FinishedPlayers == AFiskStarterBoat::Capacity)
        {
            if (SmokeReturnAt == 0.0) { SmokeReturnAt = Day->GetElapsedRealSeconds() + 5.0; }
            else if (Day->GetElapsedRealSeconds() >= SmokeReturnAt)
            {
                UE_LOG(LogTemp, Display, TEXT("FISKELAGET_TEST_CAPTAIN_RETURN player=%d"), PlayerId);
                RequestDeparture();
            }
        }
    }
}
#endif
