#include "Voyage/FiskVoyagePlan.h"
#include "FiskRouteGeometry.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
bool FFiskVoyageRoute::Valid(const TArray<FVector>& Points)
{
    if(Points.Num()>5) { return false; }
    for(int32 I=0;I<Points.Num();++I)
    {
        if(!FiskRouteGeometry::Sea(Points[I])) { return false; }
        for(int32 J=0;J<I;++J) { if(FVector::Dist2D(Points[I],Points[J])<500) { return false; } }
    }return true;
}
bool FFiskVoyageRoute::Matches(int32 D,int32 R) const { return Day==D && Revision==R && Revision<MAX_int32; }
AFiskVoyagePlan::AFiskVoyagePlan()
{ bReplicates=true;bAlwaysRelevant=true;PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.2f; }
void AFiskVoyagePlan::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{ Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(AFiskVoyagePlan,Route); }
void AFiskVoyagePlan::OnRep_Route()
{ UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_PLAN day=%d revision=%d stops=%d next=%d active=%d"),Route.Day,Route.Revision,Route.Points.Num(),Route.Next,Route.Active); }
void AFiskVoyagePlan::Tick(float Delta) { Super::Tick(Delta);Refresh(); }
void AFiskVoyagePlan::Refresh()
{
    if(!HasAuthority()) { return; }const auto* Day=GetWorld()->GetGameState<AFiskDayState>();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const auto* B=Mode?Mode->GetStarterBoat():nullptr;if(!Day || !B || Route.Revision==MAX_int32) { return; }bool Changed=false;
    if(Route.Day!=Day->GetDayNumber()) { Route.Day=Day->GetDayNumber();Route.Next=0;Route.Active=-1;Changed=true; }
    if(Route.Points.IsValidIndex(Route.Active))
    {
        if(B->GetTripProgress()==100 && FVector::Dist2D(B->GetActorLocation(),Route.Points[Route.Active])<500)
        { Route.Next=Route.Active+1;Route.Active=-1;Changed=true; }
        else if(B->GetTripProgress()>100) { Route.Active=-1;Changed=true; }
        else if(B->GetDestination()!=Route.Points[Route.Active]) { Route.Active=-1;Changed=true; }
    }
    if(Changed) { ++Route.Revision;OnRep_Route();ForceNetUpdate(); }
}
bool AFiskVoyagePlan::Replace(int32 Player,int32 D,int32 R,const TArray<FVector>& Points,FString& Message)
{
    Refresh();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* B=Mode?Mode->GetStarterBoat():nullptr;
    const auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!HasAuthority() || !B || !Day || !Route.Matches(D,R) || B->GetCaptainId()!=Player || !B->IsAboard(Player)
        || B->GetTripProgress()!=-1 || Day->GetElapsedMilliseconds()>=1020000 || !FFiskVoyageRoute::Valid(Points))
    { Message=TEXT("Plan avvist. Kapteinen redigerer ved brygga, med oppdatert dags- og rutestatus.");return false; }
    Route.Points=Points;Route.Next=0;Route.Active=-1;++Route.Revision;OnRep_Route();ForceNetUpdate();
    Message=TEXT("Felles turplan oppdatert. Enter velger neste stopp; R starter første avreise.");return true;
}
bool AFiskVoyagePlan::NextStop(int32 Player,int32 D,int32 R,int32 BoatRevision,FString& Message)
{
    Refresh();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();auto* B=Mode?Mode->GetStarterBoat():nullptr;
    if(!HasAuthority() || !B || !Route.Matches(D,R) || Route.Active>=0 || !Route.Points.IsValidIndex(Route.Next)
        || !B->CanCaptainCommand(Player,BoatRevision))
    { Message=TEXT("Neste stopp avvist. Kontroller kaptein, plan og dagsstatus.");return false; }
    const FVector Point=Route.Points[Route.Next];
    if(!B->SelectPoint(Player,BoatRevision,Point,Message)) { return false; }
    Route.Active=Route.Next;++Route.Revision;OnRep_Route();ForceNetUpdate();return true;
}
