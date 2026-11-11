#include "Boats/Controller/FiskRodCommands.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "FiskFishingGround.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

UFiskRodCommands::UFiskRodCommands() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskRodCommands::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

void UFiskRodCommands::ServerSelectRod_Implementation(int32 ExpectedRevision,int32 ExpectedDay,int32 Slot)
{
    AFiskBoardingController& PC=Controller();
    const bool Allowed=PC.ApplyRodSelection(ExpectedRevision,ExpectedDay,Slot);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_ROD_SELECTION_%s player=%d slot=%d revision=%d"),Allowed?TEXT("ACCEPTED"):TEXT("REJECTED"),PC.PlayerState?PC.PlayerState->GetPlayerId():-1,Slot,PC.GetRodSelection().Revision);
    ClientRodSelection(Allowed,PC.GetRodSelection());
}
void UFiskRodCommands::ClientRodSelection_Implementation(bool bAccepted,FFiskRodSelection Snapshot) { Controller().ReceiveRodSelection(bAccepted,Snapshot); }
void UFiskRodCommands::ServerConfigureSelectedRod_Implementation(FFiskRodSelection Expected,FGuid ExpectedCast,int32 ExpectedRevision,int32 Bait,int32 Depth,int32 Program)
{
    AFiskBoardingController& PC=Controller();
    const bool bAccepted = PC.GetRodSelection().Matches(Expected) && PC.CanOpenRig() && PC.GetActiveRodForCommand()->Configure(ExpectedCast, ExpectedRevision, Bait, Depth, Program);
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_RIG_%s player=%d revision=%d bait=%d depth=%d program=%d"),
        bAccepted ? TEXT("ACCEPTED") : TEXT("REJECTED"), PC.PlayerState ? PC.PlayerState->GetPlayerId() : INDEX_NONE,
        PC.GetActiveRodForCommand()->GetRig().Revision, PC.GetActiveRodForCommand()->GetRig().Bait, PC.GetActiveRodForCommand()->GetRig().Depth, PC.GetActiveRodForCommand()->GetRig().Program);
    ClientRigReply(bAccepted);
}
void UFiskRodCommands::ServerCastSelectedRod_Implementation(FFiskRodSelection Expected,FGuid ExpectedId)
{
    AFiskBoardingController& PC=Controller();
    if(!PC.GetRodSelection().Matches(Expected) || !PC.CanUseRod())
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTI_ROD_REJECTED command=cast player=%d"),PC.PlayerState?PC.PlayerState->GetPlayerId():-1);return; }
    if (PC.GetRodSelection().Matches(Expected) && PC.CanUseRod())
    {
        if (!PC.GetActiveRodForCommand()->Cast(ExpectedId))
        {
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CAST_REJECTED player=%d expected=%s current=%s"),
                PC.PlayerState->GetPlayerId(), *ExpectedId.ToString(EGuidFormats::Digits), *PC.GetActiveRodForCommand()->GetCastId().ToString(EGuidFormats::Digits));
        }
    }
}
void UFiskRodCommands::ServerActOnSelectedRod_Implementation(EFiskRodAction Action,FFiskRodSelection Expected,FGuid ExpectedId)
{
    AFiskBoardingController& PC=Controller();
    if (!PC.GetRodSelection().Matches(Expected) || !PC.CanUseRod())
    {
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MULTI_ROD_REJECTED command=action player=%d"),PC.PlayerState?PC.PlayerState->GetPlayerId():-1);
        return;
    }

    if (!PC.GetActiveRodForCommand()->IsCurrentCommand(ExpectedId))
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_COMMAND_REJECTED player=%d action=%d expected=%s current=%s"),
            PC.PlayerState->GetPlayerId(), static_cast<int32>(Action), *ExpectedId.ToString(EGuidFormats::Digits),
            *PC.GetActiveRodForCommand()->GetCastId().ToString(EGuidFormats::Digits));
        return;
    }
    switch (Action)
    {
    case EFiskRodAction::Lift:
        PC.GetActiveRodForCommand()->Lift();
        break;
    case EFiskRodAction::Lower:
        PC.GetActiveRodForCommand()->Lower();
        break;
    case EFiskRodAction::Reel:
        PC.GetActiveRodForCommand()->Reel();
        break;
    case EFiskRodAction::StopReel: PC.GetActiveRodForCommand()->StopReel(); break;
    case EFiskRodAction::DragUp: PC.GetActiveRodForCommand()->AdjustDrag(1); break;
    case EFiskRodAction::DragDown: PC.GetActiveRodForCommand()->AdjustDrag(-1); break;
    case EFiskRodAction::Withdraw: PC.GetActiveRodForCommand()->Withdraw(); break;
    case EFiskRodAction::Land:
        PC.GetActiveRodForCommand()->Land();
        break;
    }
}
void UFiskRodCommands::ServerApplyRigBundle_Implementation(FFiskRigBundle Request)
{
    AFiskBoardingController& PC=Controller();
    const auto* B=PC.FindStarterBoat();const auto* D=GetWorld()->GetGameState<AFiskDayState>();const int32 Count=PC.GetRodCapacity();
    TArray<FGuid> Casts;TArray<FFiskRigSettings> Current,Prepared;TArray<bool> Safe;
    for(int32 I=0;I<Count;++I) { const auto* R=PC.GetRodAt(I);Casts.Add(R->GetCastId());Current.Add(R->GetRig());Safe.Add(R->CanConfigure()); }
    const auto* G=B?FiskGrounds::Find(B->GetGroundId()):nullptr;
    const bool OK=PC.IsCareerReady() && PC.PlayerState && D && D->HasStarted() && D->GetElapsedMilliseconds()<1020000 && B && G && B->IsAboard(PC.PlayerState->GetPlayerId())
        && (B->GetTripProgress()==AFiskStarterBoat::AtDock || B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) && FiskAngler::Prepare(Request,D->GetDayNumber(),Count,G->BottomDepth,Casts,Current,Safe,Prepared);
    if(OK)
    { for(int32 I=0;I<Count;++I) { PC.SetRodRig(I,Prepared[I]); }PC.ForceNetUpdate(); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RIG_BUNDLE accepted=%d player=%d count=%d"),OK,PC.PlayerState?PC.PlayerState->GetPlayerId():-1,Count);ClientRigBundleReply(OK);
}
void UFiskRodCommands::ClientRigBundleReply_Implementation(bool OK)
{ Controller().ClientActionFeedback_Implementation(OK?TEXT("Stangsettet er anvendt samlet på dine egne stenger."):TEXT("Ingen stenger ble endret. Bruk et ferskt sett for riktig antall egne stenger, trekk inn alle, og prøv ved brygga eller fiskeplassen.")); }
void UFiskRodCommands::ClientRigReply_Implementation(bool bAccepted) { Controller().ShowRigReply(bAccepted); }
