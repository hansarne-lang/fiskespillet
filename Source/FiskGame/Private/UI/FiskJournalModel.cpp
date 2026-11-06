#include "UI/FiskJournalModel.h"

bool FFiskJournalEntry::IsValid() const
{
    return CatchId.IsValid() && DayId.IsValid() && DayNumber>0 && DayNumber<=9999
        && FiskCatches::IsValid(Fish) && Context.IsValid(true);
}
bool FFiskJournalEntry::operator==(const FFiskJournalEntry& Other) const
{
    return CatchId==Other.CatchId && DayId==Other.DayId && DayNumber==Other.DayNumber && bSold==Other.bSold
        && Fish.Species==Other.Fish.Species && Fish.WeightGrams==Other.Fish.WeightGrams
        && Fish.LengthMm==Other.Fish.LengthMm && Fish.Value==Other.Fish.Value && Context==Other.Context;
}
bool FFiskJournalModel::Restore(const TArray<FFiskJournalEntry>& Records)
{
    if(Records.Num()>Capacity) { return false; }
    TSet<FGuid> Ids;
    for(const FFiskJournalEntry& Entry:Records)
    { if(!Entry.IsValid() || Ids.Contains(Entry.CatchId)) { return false; }Ids.Add(Entry.CatchId); }
    Entries=Records;return true;
}
bool FFiskJournalModel::Merge(const FFiskJournalModel& Other)
{
    TArray<FFiskJournalEntry> Candidate=Entries;
    for(const FFiskJournalEntry& Incoming:Other.Entries)
    {
        const FFiskJournalEntry* Existing=Candidate.FindByPredicate([&](const FFiskJournalEntry& E){return E.CatchId==Incoming.CatchId;});
        if(Existing) { if(!(*Existing==Incoming)) { return false; } }
        else { Candidate.Add(Incoming); }
    }
    return Restore(Candidate);
}
EFiskJournalApply FFiskJournalModel::Apply(const FGuid& DayId,int32 DayNumber,int32 NowMs,bool bRevealed,const FFiskDayResult& Row)
{
    if(!bRevealed || NowMs<1020000 || !DayId.IsValid() || DayNumber<1 || DayNumber>9999 || Row.PlayerId<0
        || Row.Catches.Num()>Capacity) { return EFiskJournalApply::Rejected; }
    FFiskJournalModel Incoming;int64 Delivered=0,Lost=0,Income=0,Weight=0;
    for(const FFiskCatchResult& Catch:Row.Catches)
    {
        FFiskJournalEntry Entry;Entry.CatchId=Catch.CatchId;Entry.DayId=DayId;Entry.DayNumber=DayNumber;
        Entry.Fish=Catch.Fish;Entry.Context=Catch.Context;Entry.bSold=Catch.bDelivered;
        Incoming.Entries.Add(Entry);
        if(Entry.bSold) { ++Delivered;Income+=Entry.Fish.Value;Weight+=Entry.Fish.WeightGrams; } else { ++Lost; }
    }
    if(Delivered!=Row.DeliveredFish || Lost!=Row.LostFish || Income!=Row.SessionBalance || Weight!=Row.DeliveredWeightGrams
        || !Incoming.Restore(Incoming.Entries)) { return EFiskJournalApply::Rejected; }
    const int32 Previous=Entries.Num();
    if(!Merge(Incoming)) { return EFiskJournalApply::Rejected; }
    return Entries.Num()==Previous?EFiskJournalApply::Unchanged:EFiskJournalApply::Added;
}
FFiskJournalStats FFiskJournalModel::Stats(int32 Species) const
{
    FFiskJournalStats Result;
    for(const FFiskJournalEntry& Entry:Entries)
    {
        if(Species>=0 && Entry.Fish.Species!=Species) { continue; }
        ++Result.Count;Result.WeightGrams+=Entry.Fish.WeightGrams;
        Result.Income+=Entry.bSold?Entry.Fish.Value:0;Result.BestGrams=FMath::Max(Result.BestGrams,Entry.Fish.WeightGrams);
    }
    return Result;
}
const FFiskJournalEntry* FFiskJournalModel::Best(int32 Species) const
{
    const FFiskJournalEntry* Result=nullptr;
    for(const FFiskJournalEntry& Entry:Entries)
    { if(Entry.Fish.Species==Species && (!Result || Entry.Fish.WeightGrams>Result->Fish.WeightGrams)) { Result=&Entry; } }
    return Result;
}
TArray<int32> FFiskJournalModel::Select(EFiskJournalSort Sort,int32 StatusFilter,int32 SpeciesFilter) const
{
    TArray<int32> Selected;
    for(int32 I=0;I<Entries.Num();++I)
    {
        const FFiskJournalEntry& E=Entries[I];
        if((StatusFilter==1 && !E.bSold) || (StatusFilter==2 && E.bSold) || (SpeciesFilter>=0 && E.Fish.Species!=SpeciesFilter)) { continue; }
        Selected.Add(I);
    }
    Selected.Sort([&](int32 A,int32 B){
        const FFiskJournalEntry& X=Entries[A];const FFiskJournalEntry& Y=Entries[B];
        if(Sort==EFiskJournalSort::Weight && X.Fish.WeightGrams!=Y.Fish.WeightGrams) { return X.Fish.WeightGrams>Y.Fish.WeightGrams; }
        if(Sort==EFiskJournalSort::Species && X.Fish.Species!=Y.Fish.Species)
        { return FString(FiskCatches::Find(X.Fish.Species)->Name).Compare(FiskCatches::Find(Y.Fish.Species)->Name)<0; }
        if(X.Context.LandedUtc!=Y.Context.LandedUtc) { return X.Context.LandedUtc>Y.Context.LandedUtc; }
        return X.CatchId.ToString()<Y.CatchId.ToString();
    });return Selected;
}
