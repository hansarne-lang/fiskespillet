#pragma once
#include "CoreMinimal.h"
#include "Economy/FiskDayLedger.h"
#include "FiskJournalModel.generated.h"

USTRUCT()
struct FFiskJournalEntry
{
    GENERATED_BODY()
    UPROPERTY() FGuid CatchId;
    UPROPERTY() FGuid DayId;
    UPROPERTY() int32 DayNumber=0;
    UPROPERTY() FFiskCatchData Fish;
    UPROPERTY() FFiskCatchContext Context;
    UPROPERTY() bool bSold=false;
    bool IsValid() const;
    bool operator==(const FFiskJournalEntry& Other) const;
};
struct FFiskJournalStats
{
    int64 Count=0,WeightGrams=0,Income=0;
    int32 BestGrams=0;
};
enum class EFiskJournalApply : uint8 { Rejected,Unchanged,Added };
enum class EFiskJournalSort : uint8 { Date,Species,Weight };
class FFiskJournalModel
{
public:
    static constexpr int32 Capacity=4096;
    const TArray<FFiskJournalEntry>& GetEntries() const { return Entries; }
    EFiskJournalApply Apply(const FGuid& DayId,int32 DayNumber,int32 NowMs,bool bRevealed,const FFiskDayResult& Row);
    bool Restore(const TArray<FFiskJournalEntry>& Records);
    bool Merge(const FFiskJournalModel& Other);
    FFiskJournalStats Stats(int32 Species=-1) const;
    const FFiskJournalEntry* Best(int32 Species) const;
    TArray<int32> Select(EFiskJournalSort Sort,int32 StatusFilter,int32 SpeciesFilter) const;
private:
    TArray<FFiskJournalEntry> Entries;
};
