#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Expedition/FiskExpeditionState.h"
#include "Voyage/FiskVoyagePlan.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
AFiskStarterBoat* AFiskBootstrapGameMode::SpawnFleetBoat(int32 Player)
{
    if(Fleet.Num()>=12) { return nullptr; }
    auto* B=GetWorld()->SpawnActor<AFiskStarterBoat>();
    auto* E=GetWorld()->SpawnActor<AFiskExpeditionState>();E->SetOwner(B);
    auto* V=GetWorld()->SpawnActor<AFiskVoyagePlan>();V->SetOwner(B);
    B->InitializeFleet(Fleet.Num(),E,V);B->SetOwnerPlayer(Player);Fleet.Add(B);return B;
}
AFiskStarterBoat* AFiskBootstrapGameMode::GetBoatForPlayer(int32 Player) const
{
    for(auto B:Fleet) { if(B->IsAboard(Player)) { return B.Get(); } }
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    { const auto* PC=Cast<AFiskBoardingController>(It->Get());if(PC && PC->PlayerState && PC->PlayerState->GetPlayerId()==Player && PC->GetSelectedFleetBoat()) { return PC->GetSelectedFleetBoat(); } }
    return StarterBoat;
}
void AFiskBootstrapGameMode::RegisterFleetCareer(int32 Player,const FGuid& Profile)
{
    if(const int32* Slot=OwnedFleet.Find(Profile);Slot && Fleet.IsValidIndex(*Slot)) { Fleet[*Slot]->SetOwnerPlayer(Player);return; }
    for(auto B:Fleet) { if(B->GetOwnerId()==Player) { OwnedFleet.Add(Profile,B->GetFleetIndex());return; } }
}
AFiskStarterBoat* AFiskBootstrapGameMode::CreateOwnedBoat(AFiskBoardingController* PC)
{
    if(!PC || !PC->PlayerState || !PC->IsCareerReady()) { return nullptr; }
    const auto Id=PC->GetServerCareer().Id;
    if(const int32* Slot=OwnedFleet.Find(Id);Slot && Fleet.IsValidIndex(*Slot)) { return Fleet[*Slot]; }
    if(auto* B=SpawnFleetBoat(PC->PlayerState->GetPlayerId())) { OwnedFleet.Add(Id,B->GetFleetIndex());return B; }return nullptr;
}
