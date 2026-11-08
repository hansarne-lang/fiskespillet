#pragma once
#include "UI/FiskJournalModel.h"
enum class EFiskCollectionRule : uint8 { Species,Ground,Bait,Program,Weight,Count,Sold,DeliveredWeight,Diversity };
struct FFiskCollectionGoal
{
    FString Name,Detail;
    EFiskCollectionRule Rule=EFiskCollectionRule::Count;
    int32 Value=0;int64 Target=1;
};
struct FFiskCatchQuery
{
    int32 Species=-1,Ground=-1,Status=0,MinGrams=0,Sort=0;
    bool Valid() const;
};
namespace FiskCollections
{
    constexpr int32 Count=36;
    const TArray<FFiskCollectionGoal>& Catalog();
    int64 Progress(const FFiskJournalModel& Journal,int32 Goal);
    TArray<int32> Search(const FFiskJournalModel& Journal,const FFiskCatchQuery& Query);
    FString Html(const FFiskJournalModel& Journal,const FFiskCatchQuery& Query,const TArray<int32>& Pins);
    bool Export(const FString& Directory,const FFiskJournalModel& Journal,const FFiskCatchQuery& Query,const TArray<int32>& Pins,FString& File);
}
struct FFiskCollectionPins
{
    TArray<int32> Values={0,11,26};
    bool bBlocked=false,bRecovered=false;
    bool Valid() const;
    bool Set(int32 Slot,int32 Goal);
    bool Load(const FString& File,const FGuid& Profile);
    bool Save(const FString& File,const FGuid& Profile) const;
};
