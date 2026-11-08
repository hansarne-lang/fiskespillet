#pragma once
#include "UI/FiskJournalModel.h"
struct FFiskTripSummary
{
    FGuid Id;int32 Number=0,Count=0,Sold=0,Lost=0,Best=0;int64 Grams=0,Income=0,FightMs=0;
    FString Utc;TSet<int32> Species,Grounds,Baits;
};
struct FFiskPhotoView
{
    double Yaw=0,Pitch=20,Distance=1800,Fov=60;
    void Clamp();
    FVector Offset(double BoatYaw) const;
};
namespace FiskTripBook
{
    TArray<FFiskTripSummary> Days(const FFiskJournalModel& Journal);
    TArray<int32> Rows(const FFiskJournalModel& Journal,const FGuid& Day);
    FString Html(const FFiskJournalModel& Journal,const FGuid& Day);
    bool Export(const FString& Directory,const FFiskJournalModel& Journal,const FGuid& Day,FString& File);
}
