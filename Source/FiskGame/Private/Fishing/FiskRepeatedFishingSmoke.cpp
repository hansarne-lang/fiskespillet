#include "Boats/FiskBoardingController.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Fishing/FiskPrototypeRod.h"
#include "GameFramework/PlayerState.h"

void AFiskBoardingController::RunRepeatedFishingSmoke(const AFiskStarterBoat& Boat, const AFiskDayState& Day)
{
    const EFiskRodState State = Rod->GetState();
    const int32 Id = PlayerState->GetPlayerId();
    const bool bCaptain = Boat.GetCaptainId() == Id;
    const double Now = Day.GetElapsedRealSeconds();
    if (State == EFiskRodState::Lost)
    {
        UE_LOG(LogTemp, Error, TEXT("FISKELAGET_REPEAT_FAILED unexpected_loss player=%d"), Id);
        return;
    }
    // Both players first withdraw a waiting rod and start over.
    if (RepeatStage == 0)
    {
        if (State == EFiskRodState::Ready) { RequestCast(); }
        else if (State == EFiskRodState::Fishing) { RepeatStage = 1; }
        return;
    }
    if (RepeatStage == 1)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_CANCEL_WAITING player=%d"), Id);
        RepeatWithdrawnId = Rod->GetCastId();
        RequestWithdraw(); RepeatStage = 2;
        return;
    }
    if (RepeatStage == 2 || RepeatStage == 4)
    {
        if (State == EFiskRodState::Cancelled) { RequestCast(); }
        else if (State == EFiskRodState::Fishing && Rod->GetCastId() != RepeatWithdrawnId) { RepeatStage = RepeatStage == 2 ? 3 : 5; }
        return;
    }
    if (State == EFiskRodState::Fishing && RepeatCatches == 1 && !bRepeatStaleSent)
    {
        bRepeatStaleSent = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_STALE_ACTIONS player=%d"), Id);
        for (EFiskRodAction Action : {EFiskRodAction::Lift, EFiskRodAction::Reel, EFiskRodAction::Withdraw, EFiskRodAction::Land})
        {
            ServerActOnRod(Action, RepeatFirstLandedId);
        }
        ServerCastRod(RepeatFirstLandedId);
    }
    if (State == EFiskRodState::Fighting)
    {
        if (RepeatFightingId != Rod->GetCastId())
        {
            RepeatFightingId = Rod->GetCastId();
            RepeatFightAt = Now;
            RequestLand(); // A current token does not permit early landing.
        }
        if (!bRepeatParallel)
        {
            RequestLift(); RequestStopReel();
            if (Boat.GetFishVisual(0).Phase != EFiskFishVisualPhase::Fighting
                || Boat.GetFishVisual(1).Phase != EFiskFishVisualPhase::Fighting) { return; }
            bRepeatParallel = true;
            RepeatParallelAt = Now;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_PARALLEL player=%d"), Id);
        }
        if (!bCaptain && RepeatStage == 3)
        {
            if (Now < RepeatParallelAt + 1.0) { return; }
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_CANCEL_FIGHT player=%d"), Id);
            RepeatWithdrawnId = Rod->GetCastId();
            RequestWithdraw(); RepeatStage = 4;
            return;
        }
        const double Age = Now - RepeatFightAt;
        if (Rod->IsSurging()) { RequestLower(); RequestStopReel(); }
        else if ((static_cast<int32>(Age / 2.0) % 2) == 0) { RequestLift(); RequestStopReel(); }
        else { RequestLower(); RequestReel(); }
    }
    else if (State == EFiskRodState::ReadyToLand)
    {
        if (RepeatCatches == 1 && !bRepeatReadyStaleSent)
        {
            bRepeatReadyStaleSent = true;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_STALE_READY player=%d"), Id);
            ServerActOnRod(EFiskRodAction::Land, RepeatFirstLandedId);
            return; // Let the remote rejection arrive before issuing the valid command.
        }
        if (RepeatCatches == 1)
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_STALE_READY_SURVIVED player=%d"), Id);
        }
        RequestLand();
    }
    else if (State == EFiskRodState::Landed)
    {
        if (RepeatCountedId != Rod->GetCastId())
        {
            RepeatCountedId = Rod->GetCastId();
            ++RepeatCatches;
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_LANDED player=%d ordinal=%d cast=%s"),
                Id, RepeatCatches, *RepeatCountedId.ToString(EGuidFormats::Digits));
            RequestLand(); RequestLand();
            if (RepeatCatches == 1)
            {
                RepeatFirstLandedId = Rod->GetCastId();
                bRepeatAttemptedEarlyCast = true;
                UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_EARLY_CAST player=%d"), Id);
                RequestCast();
            }
        }
        if (RepeatCatches == 1 && Day.GetElapsedMilliseconds() >= Rod->GetAvailableAtMs())
        {
            RequestCast(); RequestCast(); RequestCast();
        }
    }
    if (bCaptain && HasAuthority())
    {
        int32 FinishedPlayers = 0;
        for (auto It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            const AFiskBoardingController* Other = Cast<AFiskBoardingController>(It->Get());
            if (Other && Other->PlayerState && Boat.IsAboard(Other->PlayerState->GetPlayerId())
                && Other->GetRod()->GetState() == EFiskRodState::Landed && Other->GetRod()->GetTestLandingCount() == 2)
            {
                ++FinishedPlayers;
            }
        }
        if (FinishedPlayers == 2)
        {
            if (SmokeReturnAt == 0.0) { SmokeReturnAt = Now + 5.0; }
            else if (Now >= SmokeReturnAt)
            {
                UE_LOG(LogTemp, Display, TEXT("FISKELAGET_REPEAT_CAPTAIN_RETURN player=%d"), Id);
                RequestDeparture();
            }
        }
    }
}
#endif
