#include "Boats/Controller/FiskExpeditionCommands.h"
#include "Boats/FiskBoardingController.h"
#include "Expedition/FiskExpeditionState.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerState.h"

UFiskExpeditionCommands::UFiskExpeditionCommands() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskExpeditionCommands::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

// The server's game mode must exist, as before; the expedition itself is found through the player's boat.
void UFiskExpeditionCommands::ServerExpeditionChoose_Implementation(int32 Revision,int32 Goal)
{
    AFiskBoardingController& PC=Controller();AFiskExpeditionState* E=PC.FindExpedition();
    if(!PC.IsCareerReady() || !PC.PlayerState || !GetWorld()->GetAuthGameMode() || !E || !E->Choose(PC.PlayerState->GetPlayerId(),Revision,Goal))
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_CHOOSE_REJECTED"));PC.ClientActionFeedback(TEXT("Bare kapteinen kan velge dagens mål ved brygga. Åpne J på nytt.")); }
}
void UFiskExpeditionCommands::ServerExpeditionReady_Implementation(int32 Revision,bool Ready)
{
    AFiskBoardingController& PC=Controller();AFiskExpeditionState* E=PC.FindExpedition();
    if(!PC.IsCareerReady() || !PC.PlayerState || !GetWorld()->GetAuthGameMode() || !E || !E->SetReady(PC.PlayerState->GetPlayerId(),Revision,Ready))
    { PC.ClientActionFeedback(TEXT("Gå om bord ved brygga for å markere deg klar.")); }
}
void UFiskExpeditionCommands::ServerExpeditionSay_Implementation(int32 Code)
{
    AFiskBoardingController& PC=Controller();AFiskExpeditionState* E=PC.FindExpedition();const double Now=GetWorld()->GetRealTimeSeconds();
    if(!PC.IsCareerReady() || !PC.PlayerState || !GetWorld()->GetAuthGameMode() || !E || Now-LastExpeditionMessage<2) { return; }
    if(E->Say(PC.PlayerState->GetPlayerId(),Code)) { LastExpeditionMessage=Now; }
}
