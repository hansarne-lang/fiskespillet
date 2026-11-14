#include "Boats/Controller/FiskBoatCommands.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskTestPilot.h"
#include "Voyage/FiskVoyagePlan.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"

UFiskBoatCommands::UFiskBoatCommands() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskBoatCommands::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

void UFiskBoatCommands::ServerBoardStarterBoat_Implementation()
{
    AFiskBoardingController& PC=Controller();
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && PC.PlayerState)
    {
        if(!PC.IsCareerReady() || !Mode->CanJoinFleet(PC.GetServerCareer().Id)) { PC.ClientActionFeedback(TEXT("Karrieren må være klar. Har profilen reist i dag, venter ombordstigning til neste døgn – også etter gjenoppkobling."));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_BOARD_LOCKED player=%d"),PC.PlayerState->GetPlayerId());return; }
        auto* B=Mode->GetBoatForPlayer(PC.PlayerState->GetPlayerId());B->Board(PC.PlayerState->GetPlayerId());
        if(!B->IsAboard(PC.PlayerState->GetPlayerId())) { PC.ClientActionFeedback(TEXT("Denne båten er full eller har reist. Tab åpner flåten; C henter din egen båt ved brygga.")); }
    }
}
void UFiskBoatCommands::ServerStartBoatTrip_Implementation(int32 ExpectedRevision)
{
    AFiskBoardingController& PC=Controller();
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if (Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && PC.PlayerState)
    {
        AFiskStarterBoat* Boat=Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE);
        const bool bAuthorized=Boat->CanCaptainCommand(PC.PlayerState->GetPlayerId(),ExpectedRevision);
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_TRIP_%s player=%d expected=%d current=%d"),
            bAuthorized?TEXT("AUTHORIZED"):TEXT("REJECTED"),PC.PlayerState->GetPlayerId(),ExpectedRevision,Boat->GetRouteRevision());
        if(!Boat->IsAboard(PC.PlayerState->GetPlayerId())) { PC.ClientActionFeedback(TEXT("Trykk E for å gå om bord først."));return; }
        if(!bAuthorized) { PC.ClientActionFeedback(Boat->GetCaptainId()==PC.PlayerState->GetPlayerId()
            ?TEXT("Båtstatusen ble oppdatert. Prøv R igjen."):TEXT("Bare kapteinen kan starte avreise og retur. K viser mannskapet."));return; }
        const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
        if(!Day || !Day->HasStarted()) { PC.ClientActionFeedback(TEXT("Fiskedagen starter. Prøv igjen om et øyeblikk."));return; }
        const int32 Progress=Boat->GetTripProgress();
        if(Progress==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()<120)
        { PC.ClientActionFeedback(FString::Printf(TEXT("Avreise fra 02:00. Vent %d sekunder."),120-Day->GetElapsedSeconds()));return; }
        if(Progress==AFiskStarterBoat::AtDock && Day->GetElapsedSeconds()>=1020)
        { PC.ClientActionFeedback(TEXT("Sjøfasen er ferdig. Start en ny dag fra hovedmenyen."));return; }
        if(Progress==AFiskStarterBoat::AtMarina)
        { PC.ClientActionFeedback(TEXT("Turen er avsluttet. Velg Til hovedmeny og Spill for en ny dag."));return; }
        if(Progress==AFiskStarterBoat::AtDock && Mode->IsWeighInStarted())
        { PC.ClientActionFeedback(TEXT("Veiingen har startet. Ingen flere turer i dag."));return; }
        if(Progress!=AFiskStarterBoat::AtDock && Progress!=AFiskStarterBoat::AtFishingSpot && !(Progress>=0 && Progress<100))
        { PC.ClientActionFeedback(TEXT("Båten er allerede på vei."));return; }
        Boat->StartTrip(PC.PlayerState->GetPlayerId(),ExpectedRevision);
        PC.ClientActionFeedback(Progress!=AFiskStarterBoat::AtDock?TEXT("Retur til marinaen er startet."):TEXT("Avreise er startet. F setter ut stanga ved ankomst."));
    }
}
void UFiskBoatCommands::ServerReturnToMarina_Implementation(int32 Revision,int32 Day,int32 Leg)
{
    AFiskBoardingController& PC=Controller();
    auto* Boat=const_cast<AFiskStarterBoat*>(PC.FindStarterBoat());
    const auto* Clock=GetWorld()->GetGameState<AFiskDayState>();
    const int32 Self=PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE;
    const bool Accepted=Boat && Clock && FiskQuickControls::CanReturn(
        PC.IsCareerReady() && Clock->HasStarted(),Boat->IsAboard(Self),Boat->GetCaptainId()==Self,
        Day==Clock->GetDayNumber(),Boat->CanCaptainCommand(Self,Revision),Leg==Boat->GetLegSerial(),Boat->GetTripProgress());
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RETURN_BUTTON accepted=%d player=%d"),Accepted,Self);
    if(!Accepted) { PC.ClientActionFeedback(TEXT("Retur avvist: kapteinen må være om bord med oppdatert båtstatus. Båten må være på sjøen."));return; }
    Boat->StartTrip(Self,Revision);
    PC.ClientActionFeedback(TEXT("Hjemreise startet. Båten reiser tilbake til marinaen med vanlig reisetid."));
}
void UFiskBoatCommands::ServerSelectSeaPoint_Implementation(int32 Revision,FVector_NetQuantize Point)
{
    AFiskBoardingController& PC=Controller();
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();FString Message;
    if(Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && PC.PlayerState) { Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE)->SelectPoint(PC.PlayerState->GetPlayerId(),Revision,Point,Message); }
    ClientGroundReply(Message);
}
void UFiskBoatCommands::ServerTravelVote_Implementation(int32 Leg,bool Vote)
{
    AFiskBoardingController& PC=Controller();
    auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool Accepted=PC.IsCareerReady() && PC.PlayerState && Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE)->VoteTravel(PC.PlayerState->GetPlayerId(),Leg,Vote);
    if(!Accepted) { PC.ClientActionFeedback(TEXT("Reisevisning kan velges av dem om bord mens båten er underveis.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TRAVEL_VOTE_REPLY accepted=%d"),Accepted);
}
void UFiskBoatCommands::ServerSelectGround_Implementation(int32 ExpectedRevision,int32 GroundId)
{
    AFiskBoardingController& PC=Controller();
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    FString Message=TEXT("Båten er ikke klar.");
    const bool bAccepted=Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && PC.PlayerState
        && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE)->SelectGround(PC.PlayerState->GetPlayerId(),ExpectedRevision,GroundId,Message);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CHART_%s player=%d requested=%d"),bAccepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE,GroundId);
    ClientGroundReply(Message);
}
void UFiskBoatCommands::ClientGroundReply_Implementation(const FString& Message) { Controller().ShowGroundReply(Message); }
void UFiskBoatCommands::ServerVoyageReplace_Implementation(int32 D,int32 R,const TArray<FVector>& Points)
{
    AFiskBoardingController& PC=Controller();
    auto* P=PC.FindVoyage();FString Message=TEXT("Turplanen er ikke klar.");
    const bool OK=PC.IsCareerReady() && PC.PlayerState && P && P->Replace(PC.PlayerState->GetPlayerId(),D,R,Points,Message);
    PC.ClientActionFeedback(Message);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_REPLACE accepted=%d player=%d"),OK,PC.PlayerState?PC.PlayerState->GetPlayerId():-1);
}
void UFiskBoatCommands::ServerVoyageNext_Implementation(int32 D,int32 R,int32 BR)
{
    AFiskBoardingController& PC=Controller();
    auto* P=PC.FindVoyage();FString Message=TEXT("Turplanen er ikke klar.");
    const bool OK=PC.IsCareerReady() && PC.PlayerState && P && P->NextStop(PC.PlayerState->GetPlayerId(),D,R,BR,Message);
    PC.ClientActionFeedback(Message);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_NEXT accepted=%d player=%d"),OK,PC.PlayerState?PC.PlayerState->GetPlayerId():-1);
}
void UFiskBoatCommands::ServerTransferCaptain_Implementation(int32 ExpectedRevision,int32 Target)
{
    AFiskBoardingController& PC=Controller();
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool bAccepted=Mode && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE) && PC.PlayerState
        && Mode->GetBoatForPlayer(PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE)->TransferCaptain(PC.PlayerState->GetPlayerId(),ExpectedRevision,Target);
#if WITH_DEV_AUTOMATION_TESTS
    if(!bAccepted) { ++RejectedCaptainRequests; }
#endif
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_CAPTAIN_TRANSFER_%s player=%d target=%d"),bAccepted?TEXT("ACCEPTED"):TEXT("REJECTED"),PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE,Target);
    ClientCaptainReply(bAccepted);
}
void UFiskBoatCommands::ClientCaptainReply_Implementation(bool bAccepted) { Controller().ShowCaptainReply(bAccepted); }
