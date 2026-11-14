#include "Expedition/FiskExpeditionModel.h"
#include "FiskRouteGeometry.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskCatchCatalog.h"

bool FFiskExpeditionRecord::IsValid() const
{ return DayId.IsValid() && DayNumber>=1 && DayNumber<=9999 && Goal>=0 && Goal<5 && Weather>=0 && Weather<3; }
bool FFiskExpeditionRecord::operator==(const FFiskExpeditionRecord& B) const
{ return DayId==B.DayId && DayNumber==B.DayNumber && Goal==B.Goal && Weather==B.Weather && bCompleted==B.bCompleted; }
const TCHAR* FiskExpeditions::WeatherName(int32 Id)
{ const TCHAR* Names[]={TEXT("Rolig sjø"),TEXT("Passatvind"),TEXT("Tropisk regn")};return Id>=0 && Id<3?Names[Id]:TEXT("Venter på vær"); }
const TCHAR* FiskExpeditions::GoalName(int32 Id)
{ const TCHAR* Names[]={TEXT("Habitatfangst"),TEXT("To ulike arter"),TEXT("Mannskapsfangst"),TEXT("Personlig rekord"),TEXT("Fangst ved valgt sjøpunkt")};return Id>=0 && Id<5?Names[Id]:TEXT("Ukjent mål"); }
const TCHAR* FiskExpeditions::Phrase(int32 Id)
{ const TCHAR* Names[]={TEXT("Klar for avreise!"),TEXT("Vent litt ved brygga."),TEXT("På tide å vende hjem!")};return Id>=0 && Id<3?Names[Id]:TEXT(""); }
FString FiskExpeditions::RecordKey(const FGuid& Profile,int32 Species)
{ return Profile.ToString(EGuidFormats::Digits)+TEXT("-")+FString::FromInt(Species); }
bool FiskExpeditions::Evaluate(const FFiskExpeditionPlan& P,const FFiskSettlementView& V,int32 NowMs,
    const TMap<int32,FGuid>& Profiles,const TArray<FGuid>& Participants,const TMap<FString,int32>& Records)
{
    if(NowMs<0 || !V.bRevealed || !P.bLocked || !P.DayId.IsValid() || P.DayId!=V.DayId || P.DayNumber!=V.DayNumber
        || P.Goal<0 || P.Goal>=5 || P.RequiredCrew<1 || P.RequiredCrew>2 || Participants.IsEmpty() || Participants.Num()>4
        || !FiskGrounds::Find(P.Ground) || !FiskCatches::Find(P.Species) || !FiskRouteGeometry::Sea(P.Point)) { return false; }
    TSet<int32> Species;TSet<FGuid> Fishers;bool Habitat=false,Record=false,Point=false;
    for(const FFiskDayResult& Row:V.Results)
    {
        const FGuid* Profile=Profiles.Find(Row.PlayerId);if(!Profile || !Participants.Contains(*Profile)) { continue; }
        for(const FFiskCatchResult& C:Row.Catches)
        {
            if(!C.bDelivered || !C.CatchId.IsValid() || !FiskCatches::IsValid(C.Fish) || !C.Context.IsValid(true)) { continue; }
            Species.Add(C.Fish.Species);Fishers.Add(*Profile);
            Habitat|=C.Context.Ground==P.Ground && C.Fish.Species==P.Species;
            Record|=C.Fish.WeightGrams>Records.FindRef(RecordKey(*Profile,C.Fish.Species));
            Point|=C.Context.bHasSeaPoint && FVector::Dist2D(C.Context.SeaPoint,P.Point)<=500;
        }
    }
    switch(P.Goal) { case 0:return Habitat;case 1:return Species.Num()>=2;case 2:return Fishers.Num()>=P.RequiredCrew;case 3:return Record;case 4:return Point;default:return false; }
}
