#include "Boats/Controller/FiskFleetSync.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

UFiskFleetSync::UFiskFleetSync() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskFleetSync::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

void UFiskFleetSync::ServerFleetSelect_Implementation(int32 Index,bool CreateOwn)
{
    AFiskBoardingController& PC=Controller();
    auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* D=GetWorld()->GetGameState<AFiskDayState>();
    const int32 PlayerId=PC.PlayerState?PC.PlayerState->GetPlayerId():-1;auto* Old=M?M->GetBoatForPlayer(PlayerId):nullptr;
    auto Reject=[&](){PC.ClientActionFeedback(TEXT("Båtbytte avvist. Velg en ledig båt ved brygga før avreise; etter turen venter du til neste dag."));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_SELECT_REJECTED player=%d index=%d"),PlayerId,Index);};
    if(!M || !D || !D->HasStarted() || !PC.IsCareerSynced() || !M->CanJoinFleet(PC.GetServerCareer().Id) || PlayerId<0 || D->GetElapsedMilliseconds()>=1020000
        || (Old && Old->IsAboard(PlayerId) && Old->GetTripProgress()!=AFiskStarterBoat::AtDock)) { Reject();return; }
    if(!PC.AreRodsIdle()) { Reject();return; }
    AFiskStarterBoat* Target=CreateOwn?M->CreateOwnedBoat(&PC):M->GetFleet().IsValidIndex(Index)?M->GetFleet()[Index].Get():nullptr;
    if(!Target || Target->GetTripProgress()!=AFiskStarterBoat::AtDock || (!Target->IsAboard(PlayerId) && Target->GetCrewCount()>=Target->GetCapacity())) { Reject();return; }
    if(Old && Old!=Target && Old->IsAboard(PlayerId)) { Old->Leave(PlayerId); }
    PC.SetSelectedFleetBoat(Target);Target->Board(PlayerId);PC.ResetRodForDay(D->GetDayNumber());PC.ForceNetUpdate();
    PC.ClientActionFeedback(FString::Printf(TEXT("Om bord i båt %d. O: din båts turplan. J: ekspedisjon. R: avreise fra 02:00."),Target->GetFleetIndex()+1));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_SELECTED player=%d boat=%d owner=%d crew=%d"),PlayerId,Target->GetFleetIndex(),Target->GetOwnerId(),Target->GetCrewCount());
}
void UFiskFleetSync::ServerFleetLeave_Implementation()
{
    AFiskBoardingController& PC=Controller();
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();auto* B=const_cast<AFiskStarterBoat*>(PC.FindStarterBoat());
    if(!D || !PC.PlayerState || !B || B->GetTripProgress()!=AFiskStarterBoat::AtDock || !B->IsAboard(PC.PlayerState->GetPlayerId()))
    { PC.ClientActionFeedback(TEXT("Gå i land ved startbrygga før avreise. Mannskapet er låst på denne turen til neste døgn."));return; }
    B->Leave(PC.PlayerState->GetPlayerId());PC.ResetRodForDay(D->GetDayNumber());PC.ClientActionFeedback(TEXT("Du er i land. B: strandbar. H: naust. Tab: velg en annen båt."));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_LEFT player=%d boat=%d"),PC.PlayerState->GetPlayerId(),B->GetFleetIndex());
}
