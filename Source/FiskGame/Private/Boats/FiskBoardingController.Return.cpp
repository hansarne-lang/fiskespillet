#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskTestPilot.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

void AFiskBoardingController::ServerReturnToMarina_Implementation(int32 Revision,int32 Day,int32 Leg)
{
    auto* Boat=const_cast<AFiskStarterBoat*>(FindStarterBoat());
    const auto* Clock=GetWorld()->GetGameState<AFiskDayState>();
    const int32 Self=PlayerState?PlayerState->GetPlayerId():INDEX_NONE;
    const bool Accepted=Boat && Clock && FiskQuickControls::CanReturn(
        IsCareerReady() && Clock->HasStarted(),Boat->IsAboard(Self),Boat->GetCaptainId()==Self,
        Day==Clock->GetDayNumber(),Boat->CanCaptainCommand(Self,Revision),Leg==Boat->GetLegSerial(),Boat->GetTripProgress());
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RETURN_BUTTON accepted=%d player=%d"),Accepted,Self);
    if(!Accepted) { ClientActionFeedback(TEXT("Retur avvist: kapteinen må være om bord med oppdatert båtstatus. Båten må være på sjøen."));return; }
    Boat->StartTrip(Self,Revision);
    ClientActionFeedback(TEXT("Hjemreise startet. Båten reiser tilbake til marinaen med vanlig reisetid."));
}
