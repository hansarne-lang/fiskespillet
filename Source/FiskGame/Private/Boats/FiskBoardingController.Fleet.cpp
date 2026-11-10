#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Voyage/FiskVoyagePlan.h"
#include "Expedition/FiskExpeditionState.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskPrototypeRod.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskCrewPanel.h"
#include "UI/FiskMarinaPanel.h"
#include "UI/FiskExpeditionPanel.h"
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskAnglerPanel.h"
#include "UI/FiskCollectionPanel.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "InputCoreTypes.h"
void AFiskBoardingController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{ Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(AFiskBoardingController,SelectedFleetBoat,COND_OwnerOnly); }
AFiskVoyagePlan* AFiskBoardingController::FindVoyage() const
{ const auto* B=FindStarterBoat();return B?B->GetVoyage():nullptr; }
AFiskExpeditionState* AFiskBoardingController::FindExpedition() const
{ const auto* B=FindStarterBoat();return B?B->GetExpedition():nullptr; }
TArray<AFiskStarterBoat*> AFiskBoardingController::FleetBoats() const
{
    TArray<AFiskStarterBoat*> Boats;for(TActorIterator<AFiskStarterBoat> B(GetWorld());B;++B) { Boats.Add(*B); }
    Boats.Sort([](const AFiskStarterBoat& A,const AFiskStarterBoat& B){return A.GetFleetIndex()<B.GetFleetIndex();});return Boats;
}
void AFiskBoardingController::ServerFleetSelect_Implementation(int32 Index,bool CreateOwn)
{
    auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* D=GetWorld()->GetGameState<AFiskDayState>();
    const int32 PlayerId=PlayerState?PlayerState->GetPlayerId():-1;auto* Old=M?M->GetBoatForPlayer(PlayerId):nullptr;
    auto Reject=[&](){ClientActionFeedback(TEXT("Båtbytte avvist. Velg en ledig båt ved brygga før avreise; etter turen venter du til neste dag."));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_SELECT_REJECTED player=%d index=%d"),PlayerId,Index);};
    if(!M || !D || !D->HasStarted() || !IsCareerSynced() || !M->CanJoinFleet(GetServerCareer().Id) || PlayerId<0 || D->GetElapsedMilliseconds()>=1020000
        || (Old && Old->IsAboard(PlayerId) && Old->GetTripProgress()!=AFiskStarterBoat::AtDock)) { Reject();return; }
    for(int32 Slot=0;Slot<3;++Slot) { if(!GetRodAt(Slot)->CanConfigure()) { Reject();return; } }
    AFiskStarterBoat* Target=CreateOwn?M->CreateOwnedBoat(this):M->GetFleet().IsValidIndex(Index)?M->GetFleet()[Index].Get():nullptr;
    if(!Target || Target->GetTripProgress()!=AFiskStarterBoat::AtDock || (!Target->IsAboard(PlayerId) && Target->GetCrewCount()>=Target->GetCapacity())) { Reject();return; }
    if(Old && Old!=Target && Old->IsAboard(PlayerId)) { Old->Leave(PlayerId); }
    SelectedFleetBoat=Target;Target->Board(PlayerId);ResetRodForDay(D->GetDayNumber());ForceNetUpdate();
    ClientActionFeedback(FString::Printf(TEXT("Om bord i båt %d. O: din båts turplan. J: ekspedisjon. R: avreise fra 02:00."),Target->GetFleetIndex()+1));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_SELECTED player=%d boat=%d owner=%d crew=%d"),PlayerId,Target->GetFleetIndex(),Target->GetOwnerId(),Target->GetCrewCount());
}
void AFiskBoardingController::ServerFleetLeave_Implementation()
{
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();auto* B=const_cast<AFiskStarterBoat*>(FindStarterBoat());
    if(!D || !PlayerState || !B || B->GetTripProgress()!=AFiskStarterBoat::AtDock || !B->IsAboard(PlayerState->GetPlayerId()))
    { ClientActionFeedback(TEXT("Gå i land ved startbrygga før avreise. Mannskapet er låst på denne turen til neste døgn."));return; }
    B->Leave(PlayerState->GetPlayerId());ResetRodForDay(D->GetDayNumber());ClientActionFeedback(TEXT("Du er i land. B: strandbar. H: naust. Tab: velg en annen båt."));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_FLEET_LEFT player=%d boat=%d"),PlayerState->GetPlayerId(),B->GetFleetIndex());
}
bool AFiskBoardingController::HandleFleetKey(const FInputKeyEventArgs& A)
{
    const bool Press=A.Event==IE_Pressed || A.Event==IE_Repeat;
    if(!bFleetVisible)
    {
        if(!Press || A.Key!=EKeys::Tab || FrontEnd->IsVisible() || SeaChart->IsVisible() || RigPanel->IsVisible() || CrewPanel->IsVisible()
            || MarinaPanel->IsVisible() || ExpeditionPanel->IsVisible() || SkipperPanel->IsVisible() || AnglerPanel->IsVisible() || CollectionPanel->IsVisible()
            || JournalPanel->IsVisible() || DeckPanel->IsVisible() || bCareerVisible || bHelpVisible || bResultsVisible || bHistoryVisible) { return false; }
        bFleetVisible=true;FleetRow=0;ReleaseRodInputsForMenu();bShowMouseCursor=true;SetInputMode(FInputModeGameAndUI());return true;
    }
    if(!Press) { return true; }
    if(A.Key==EKeys::Escape || A.Key==EKeys::Tab) { bFleetVisible=false;bShowMouseCursor=false;SetInputMode(FInputModeGameOnly());return true; }
    const auto Boats=FleetBoats();if(A.Key==EKeys::Up) { FleetRow=FMath::Max(0,FleetRow-1); }if(A.Key==EKeys::Down) { FleetRow=FMath::Min(FMath::Max(0,Boats.Num()-1),FleetRow+1); }
    if(A.Key==EKeys::Home) { FleetRow=0; }if(A.Key==EKeys::End) { FleetRow=FMath::Max(0,Boats.Num()-1); }
    if(A.Key==EKeys::C && A.Event==IE_Pressed) { ServerFleetSelect(-1,true); }
    if(A.Key==EKeys::L && A.Event==IE_Pressed) { ServerFleetLeave(); }
    if(A.Key==EKeys::Enter && A.Event==IE_Pressed && Boats.IsValidIndex(FleetRow)) { ServerFleetSelect(Boats[FleetRow]->GetFleetIndex(),false); }
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;int32 W=0,H=0;GetViewportSize(W,H);const float S=FMath::Min(W/1280.f,H/720.f);
        if(S>0 && GetWorld()->GetGameViewport() && GetWorld()->GetGameViewport()->GetMousePosition(Mouse))
        { for(int32 I=0;I<Boats.Num();++I) { if(FBox2D(FVector2D(40*S,(176+34*I)*S),FVector2D(1235*S,(207+34*I)*S)).IsInside(Mouse)) { FleetRow=I; } } }
    }return true;
}
void AFiskBoardingController::DrawFleet(AHUD& H,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Col=FLinearColor::White){H.DrawText(V,Col,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("FISKEFLÅTEN"),48,55,2);
    const auto* D=GetWorld()->GetGameState<AFiskDayState>();const int32 Now=D?D->GetElapsedMilliseconds():0;const auto Boats=FleetBoats();
    T(FString::Printf(TEXT("%d/12 spillere | %d/12 båter | %02d:%02d | Felles oppgjør 17:00"),D?D->PlayerArray.Num():0,Boats.Num(),Now/60000,Now/1000%60),48,105,1.05f,Teal);
    T(TEXT("Piler / klikk: velg | Enter: gå om bord | C: hent din egen båt | L: gå i land | Tab / Escape: lukk"),48,144,.92f);
    for(int32 I=0;I<Boats.Num();++I)
    {
        const auto* B=Boats[I];FString OwnerName=TEXT("Frakoblet");if(D) { for(const auto& P:D->PlayerArray) { if(P && P->GetPlayerId()==B->GetOwnerId()) { OwnerName=P->GetPlayerName();break; } } }
        const int32 P=B->GetTripProgress();const TCHAR* Status=P==-1?TEXT("Ved brygga"):P<100?TEXT("På vei ut"):P==100?TEXT("Fisker"):P<200?TEXT("På vei hjem"):TEXT("Hjemme");
        const bool Here=PlayerState && B->IsAboard(PlayerState->GetPlayerId());const bool Own=PlayerState && B->GetOwnerId()==PlayerState->GetPlayerId();
        const float Y=176+34*I;H.DrawRect(I==FleetRow?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),40*S,Y*S,1195*S,31*S);
        T(FString::Printf(TEXT("%02d  %s%s | %s | %d/%d om bord | %s%s"),B->GetFleetIndex()+1,*OwnerName,Own?TEXT(" [DIN]"):TEXT(""),B->GetKind()==1?TEXT("Offshore"):TEXT("Startbåt"),B->GetCrewCount(),B->GetCapacity(),Status,Here?TEXT(" [OM BORD]"):TEXT("")),52,Y+7,.96f,Here?Gold:Teal);
    }
    T(GetActionFeedback(),48,603,.9f,Gold);T(TEXT("Bytt mannskap ved startbrygga. Hver båt har egen rute, kaptein, last og returfrist."),48,637,.92f,Teal);
    T(TEXT("Vekt og verdi er skjult til 17:00. Den enkelte fangstens båt avgjør om den kommer frem i tide."),48,659,.85f,Teal);
}
// Expedition commands stay on the controller until #113 step 7 (UFiskExpeditionCommands).
void AFiskBoardingController::ServerExpeditionChoose_Implementation(int32 Revision,int32 Goal)
{
    auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if(!IsCareerReady() || !PlayerState || !M || !FindExpedition() || !FindExpedition()->Choose(PlayerState->GetPlayerId(),Revision,Goal))
    { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_CHOOSE_REJECTED"));ClientActionFeedback(TEXT("Bare kapteinen kan velge dagens mål ved brygga. Åpne J på nytt.")); }
}
void AFiskBoardingController::ServerExpeditionReady_Implementation(int32 Revision,bool Ready)
{
    auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    if(!IsCareerReady() || !PlayerState || !M || !FindExpedition() || !FindExpedition()->SetReady(PlayerState->GetPlayerId(),Revision,Ready))
    { ClientActionFeedback(TEXT("Gå om bord ved brygga for å markere deg klar.")); }
}
void AFiskBoardingController::ServerExpeditionSay_Implementation(int32 Code)
{
    auto* M=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const double Now=GetWorld()->GetRealTimeSeconds();
    if(!IsCareerReady() || !PlayerState || !M || !FindExpedition() || Now-LastExpeditionMessage<2) { return; }
    if(FindExpedition()->Say(PlayerState->GetPlayerId(),Code)) { LastExpeditionMessage=Now; }
}
// Sea point and travel vote stay on the controller until #113 step 8 (UFiskBoatCommands).
void AFiskBoardingController::ServerSelectSeaPoint_Implementation(int32 Revision,FVector_NetQuantize Point)
{
    AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();FString Message;
    if(Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && PlayerState) { Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->SelectPoint(PlayerState->GetPlayerId(),Revision,Point,Message); }
    ClientGroundReply(Message);
}
void AFiskBoardingController::ServerTravelVote_Implementation(int32 Leg,bool Vote)
{
    auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const bool Accepted=IsCareerReady() && PlayerState && Mode && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE) && Mode->GetBoatForPlayer(PlayerState?PlayerState->GetPlayerId():INDEX_NONE)->VoteTravel(PlayerState->GetPlayerId(),Leg,Vote);
    if(!Accepted) { ClientActionFeedback(TEXT("Reisevisning kan velges av dem om bord mens båten er underveis.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TRAVEL_VOTE_REPLY accepted=%d"),Accepted);
}
