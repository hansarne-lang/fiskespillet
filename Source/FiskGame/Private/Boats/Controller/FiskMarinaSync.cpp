#include "Boats/Controller/FiskMarinaSync.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "FiskHarbourRules.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"

UFiskMarinaSync::UFiskMarinaSync() { SetIsReplicatedByDefault(true); }
AFiskBoardingController& UFiskMarinaSync::Controller() const { return *CastChecked<AFiskBoardingController>(GetOwner()); }

void UFiskMarinaSync::ServerMarinaIdentity_Implementation(FGuid Profile,FFiskMarinaIdentity Identity)
{
    AFiskBoardingController& PC=Controller();APlayerState* PlayerState=PC.PlayerState;const bool bCareerReady=PC.IsCareerReady();const FFiskCareerProfile& ServerCareer=PC.GetServerCareer();
    TActorIterator<AFiskMarinaHub> Hub(GetWorld());const bool Allowed=bCareerReady && ServerCareer.Id==Profile && Identity.IsOwnedBy(ServerCareer) && PlayerState && PC.GetMarinaPanel()->IsAtDock() && Hub;
    TArray<FFiskMarinaTrophy> Records;
    if(Allowed) { if(const FFiskMarinaResident* R=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& V){return V.Profile==Profile;})) { Records=R->Trophies; } }
    const bool Accepted=Allowed && Hub->Upsert(Profile,PlayerState->GetPlayerId(),Identity,Records);
    if(Accepted) { PlayerState->SetPlayerName(Identity.DisplayName); }
    ClientMarinaIdentity(Accepted,Identity,Accepted?TEXT("Naust og navn bekreftet."):TEXT("Naustendring avvist. Egen profil, kjøpt pynt og marina kreves."));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_IDENTITY accepted=%d player=%d style=%d color=%d decor=%d"),Accepted,PlayerState?PlayerState->GetPlayerId():-1,Identity.Style,Identity.Color,Identity.Decor);
}
void UFiskMarinaSync::ClientMarinaIdentity_Implementation(bool Accepted,FFiskMarinaIdentity Identity,const FString& Reason)
{ UFiskMarinaPanel* Panel=Controller().GetMarinaPanel();if(Accepted) { Panel->ReceiveIdentity(Identity); }else { Panel->RejectIdentity(Reason); } }
void UFiskMarinaSync::ServerMarinaRecords_Implementation(FGuid Profile,const TArray<FFiskMarinaTrophy>& Records)
{
    AFiskBoardingController& PC=Controller();APlayerState* PlayerState=PC.PlayerState;const bool bCareerReady=PC.IsCareerReady();const FFiskCareerProfile& ServerCareer=PC.GetServerCareer();
    TActorIterator<AFiskMarinaHub> Hub(GetWorld());TActorIterator<AFiskDaySettlement> Settlement(GetWorld());const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!bCareerReady || Profile!=ServerCareer.Id || !PlayerState || !Hub || !Day || Records.Num()>FiskCatches::Count) { return; }
    const FFiskMarinaResident* Existing=Hub->GetResidents().FindByPredicate([&](const FFiskMarinaResident& R){return R.Profile==Profile;});if(!Existing) { return; }
    TSet<int32> Seen;TArray<FFiskMarinaTrophy> Combined=Existing->Trophies;
    for(const FFiskMarinaTrophy& T:Records)
    {
        const FFiskSpecies* Species=FiskCatches::Find(T.Species);
        if(!Species || !T.DayId.IsValid() || T.Grams<Species->MinGrams || T.Grams>Species->MaxGrams || T.LengthMm!=FiskCatches::LengthForWeight(*Species,T.Grams) || Seen.Contains(T.Species)
            || (Settlement && T.DayId==Settlement->GetView().DayId && !Settlement->GetView().bRevealed))
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_RECORDS_REJECTED player=%d"),PlayerState->GetPlayerId());return; }
        Seen.Add(T.Species);FFiskMarinaTrophy* Previous=Combined.FindByPredicate([&](const FFiskMarinaTrophy& R){return R.Species==T.Species;});
        if(!Previous) { Combined.Add(T); }else if(T.Grams>Previous->Grams) { *Previous=T; }
    }
    Combined.Sort([](const FFiskMarinaTrophy& A,const FFiskMarinaTrophy& B){return A.Species<B.Species;});
    Hub->Upsert(Profile,PlayerState->GetPlayerId(),Existing->Identity,Combined);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_RECORDS player=%d records=%d"),PlayerState->GetPlayerId(),Combined.Num());
}
void UFiskMarinaSync::ServerMarinaSay_Implementation(int32 Code)
{
    AFiskBoardingController& PC=Controller();APlayerState* PlayerState=PC.PlayerState;const bool bCareerReady=PC.IsCareerReady();
    TActorIterator<AFiskMarinaHub> Hub(GetWorld());const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();const double Now=GetWorld()->GetRealTimeSeconds();
    const bool Accepted=bCareerReady && PlayerState && Day && Hub && Code>=0 && Code<AFiskMarinaHub::PhraseCount && PC.GetMarinaPanel()->IsAtDock() && Now-LastMarinaMessage>=2 && Hub->Say(PlayerState->GetPlayerId(),Code,Day->GetElapsedMilliseconds());
    if(Accepted) { LastMarinaMessage=Now; }else { PC.ClientActionFeedback(TEXT("Melding avvist. Vent 2 sekunder og bruk strandbaren ved marinaen.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MARINA_MESSAGE accepted=%d player=%d code=%d"),Accepted,PlayerState?PlayerState->GetPlayerId():-1,Code);
}

void UFiskMarinaSync::ServerHarbourPlace_Implementation(FGuid Profile,int32 Place)
{
    AFiskBoardingController& PC=Controller();APlayerState* PlayerState=PC.PlayerState;const bool bCareerReady=PC.IsCareerReady();const FFiskCareerProfile& ServerCareer=PC.GetServerCareer();
    TActorIterator<AFiskMarinaHub> Hub(GetWorld());
    const bool Accepted=Hub && PlayerState && FiskHarbour::CanChoose(bCareerReady,Profile==ServerCareer.Id,PC.GetMarinaPanel()->IsAtDock(),Place) && Hub->SetPlace(PlayerState->GetPlayerId(),Place);
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_HARBOUR_PRESENCE accepted=%d player=%d place=%d"),Accepted,PlayerState?PlayerState->GetPlayerId():-1,Place);
}
