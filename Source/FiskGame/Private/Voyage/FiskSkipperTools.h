#pragma once
#include "Voyage/FiskTripBook.h"
struct FFiskRouteBudget
{
    bool Valid=false;
    double DistanceCm=0;
    int64 SailingMs=0,FishingMs=0,HomeAtMs=0,MarginMs=0;
};
namespace FiskSkipperTools
{
    constexpr int32 TemplateCount=6;
    const TCHAR* TemplateName(int32 Id);
    TArray<FVector> Template(int32 Id);
    FFiskRouteBudget Budget(const TArray<FVector>& Points,int32 Kind,int32 StartMs,int32 StopMs);
    TArray<FVector> Optimize(const TArray<FVector>& Points);
    FString CompareHtml(const FFiskJournalModel& Journal,int32 First,int32 Second);
    bool ExportComparison(const FString& Directory,const FFiskJournalModel& Journal,int32 First,int32 Second,FString& File);
}
struct FFiskPhotoPresets
{
    TArray<FFiskPhotoView> Views={{0,20,1800,60},{90,30,2200,50},{180,50,3500,70}};
    bool bBlocked=false,bRecovered=false;
    bool Valid() const;
    bool Load(const FString& File,const FGuid& Profile);
    bool Save(const FString& File,const FGuid& Profile) const;
};
