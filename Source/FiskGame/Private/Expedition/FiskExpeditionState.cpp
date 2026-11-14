#include "Expedition/FiskExpeditionState.h"
#include "Expedition/FiskExpeditionStore.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Boats/FiskStarterBoat.h"
#include "Career/FiskJournalAuthority.h"
#include "UI/FiskUserSettings.h"
#include "DayCycle/FiskDayState.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AFiskExpeditionState::AFiskExpeditionState()
{ bReplicates=true;bAlwaysRelevant=true;PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=.2f; }
void AFiskExpeditionState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{ Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(AFiskExpeditionState,Plan); }
void AFiskExpeditionState::OnRep_Plan()
{ UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_PLAN day=%d weather=%d goal=%d revision=%d locked=%d evaluated=%d completed=%d ready=%d"),Plan.DayNumber,Plan.Weather,Plan.Goal,Plan.Revision,Plan.bLocked,Plan.bEvaluated,Plan.bCompleted,Plan.Ready.Num()); }
void AFiskExpeditionState::Tick(float Delta) { Super::Tick(Delta);if(HasAuthority()) { Refresh(); } }
void AFiskExpeditionState::Refresh()
{
    const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!HasAuthority() || !Mode || !Mode->GetSettlement() || !Cast<AFiskStarterBoat>(GetOwner()) || !Day) { return; }
    const auto* Boat=Cast<AFiskStarterBoat>(GetOwner());bool Changed=false;
    if(Plan.DayId!=Mode->GetSettlement()->GetView().DayId)
    {
        Plan=FFiskExpeditionPlan();Plan.Revision=Boat->GetFleetIndex()*1000000;Plan.DayId=Mode->GetSettlement()->GetView().DayId;Plan.DayNumber=Day->GetDayNumber();
        Plan.Weather=(Plan.DayNumber-1)%3;
#if WITH_DEV_AUTOMATION_TESTS
        int32 Weather=-1;if(FParse::Value(FCommandLine::Get(),TEXT("FiskWeather="),Weather) && Weather>=0 && Weather<3) { Plan.Weather=Weather; }
#endif
        Participants.Reset();Records.Reset();RouteRevision=-1;Changed=true;
    }
    if(!Plan.bLocked && (RouteRevision!=Boat->GetRouteRevision() || Plan.Point!=Boat->GetDestination()))
    {
        RouteRevision=Boat->GetRouteRevision();Plan.Ground=Boat->GetGroundId();Plan.Point=Boat->GetDestination();Plan.Species=0;
        for(int32 S=1;S<FiskCatches::Count;++S) { if(FiskCatches::HabitatChance(Plan.Ground,S)>FiskCatches::HabitatChance(Plan.Ground,Plan.Species)) { Plan.Species=S; } }
        Plan.Ready.Reset();++Plan.Revision;Changed=true;
    }
    for(int32 I=Plan.Ready.Num()-1;I>=0;--I) { if(!Boat->IsAboard(Plan.Ready[I])) { Plan.Ready.RemoveAt(I);Changed=true; } }
    if(Changed) { OnRep_Plan();ForceNetUpdate(); }
}
bool AFiskExpeditionState::Choose(int32 Player,int32 Revision,int32 Goal)
{
    Refresh();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* B=Cast<AFiskStarterBoat>(GetOwner());
    if(!HasAuthority() || !B || Plan.bLocked || Plan.bEvaluated || Revision!=Plan.Revision || Goal<0 || Goal>=5
        || B->GetTripProgress()!=AFiskStarterBoat::AtDock || B->GetCaptainId()!=Player || !B->IsAboard(Player)) { return false; }
    Plan.Goal=Goal;Plan.Ready.Reset();++Plan.Revision;OnRep_Plan();ForceNetUpdate();return true;
}
bool AFiskExpeditionState::SetReady(int32 Player,int32 Revision,bool Ready)
{
    Refresh();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* B=Cast<AFiskStarterBoat>(GetOwner());
    if(!HasAuthority() || !B || Plan.bLocked || Plan.bEvaluated || Revision!=Plan.Revision || B->GetTripProgress()!=AFiskStarterBoat::AtDock || !B->IsAboard(Player)) { return false; }
    if(Ready) { Plan.Ready.AddUnique(Player); }else { Plan.Ready.Remove(Player); }OnRep_Plan();ForceNetUpdate();return true;
}
bool AFiskExpeditionState::Say(int32 Player,int32 Code)
{
    const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* B=Cast<AFiskStarterBoat>(GetOwner());
    if(!HasAuthority() || !B || !B->IsAboard(Player) || Code<0 || Code>=3) { return false; }
    Plan.MessagePlayer=Player;Plan.MessageCode=Code;++Plan.MessageSerial;ForceNetUpdate();return true;
}
bool AFiskExpeditionState::Depart()
{
    Refresh();const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();const auto* B=Cast<AFiskStarterBoat>(GetOwner());
    if(!HasAuthority() || !B || Plan.bLocked || Plan.bEvaluated || !Plan.DayId.IsValid()) { return false; }
    TArray<FGuid> Crew;TMap<FString,int32> Previous;
    for(int32 Seat=0;Seat<B->GetCrewCount();++Seat)
    {
        const FGuid* Id=Mode->GetDayProfiles().Find(B->GetCrewMemberId(Seat));if(!Id || !Id->IsValid()) { return false; }Crew.AddUnique(*Id);
        FFiskJournalModel Journal;if(!FiskJournalAuthority::Load(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Authority")),*Id,Journal)) { return false; }
        for(int32 S=0;S<FiskCatches::Count;++S) { if(const auto* Best=Journal.Best(S)) { Previous.Add(FiskExpeditions::RecordKey(*Id,S),Best->Fish.WeightGrams); } }
    }
    if(Crew.IsEmpty() || Crew.Num()>4) { return false; }Participants=MoveTemp(Crew);Records=MoveTemp(Previous);
    Plan.RequiredCrew=FMath::Min(2,Participants.Num());Plan.bLocked=true;OnRep_Plan();ForceNetUpdate();return true;
}
bool AFiskExpeditionState::Archive(const FFiskSettlementView& View,int32 NowMs)
{
    Refresh();if(!HasAuthority() || NowMs<0 || !View.bRevealed || View.DayId!=Plan.DayId) { return false; }
    const auto* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();if(!Mode) { return false; }
    if(!Plan.bEvaluated)
    {
        Plan.bCompleted=FiskExpeditions::Evaluate(Plan,View,NowMs,Mode->GetDayProfiles(),Participants,Records);Plan.bEvaluated=true;OnRep_Plan();ForceNetUpdate();
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_EXPEDITION_RESULT day=%d goal=%d completed=%d at=%d"),Plan.DayNumber,Plan.Goal,Plan.bCompleted,NowMs);
    }
    FFiskExpeditionRecord Result;Result.DayId=Plan.DayId;Result.DayNumber=Plan.DayNumber;Result.Goal=Plan.Goal;Result.Weather=Plan.Weather;Result.bCompleted=Plan.bCompleted;
    for(const FGuid& Id:Participants)
    {
        const FString File=FiskExpeditionStore::File(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Authority")),Id);
        TArray<FFiskExpeditionRecord> Entries;if(!FiskExpeditionStore::Load(File,Id,Entries) || !FiskExpeditionStore::Merge(Entries,{Result}) || !FiskExpeditionStore::Save(File,Id,Entries)) { return false; }
    }return true;
}
