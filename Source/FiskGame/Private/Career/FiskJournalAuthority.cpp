#include "Career/FiskJournalAuthority.h"
#include "Career/FiskCareerStore.h"
#include "Economy/FiskDaySettlement.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace
{
bool Preservable(const FString& File)
{
    for(const FString& Path:{File,File+TEXT(".bak")})
    {
        const int64 Size=IFileManager::Get().FileSize(*Path);
        if(Size>FFiskJournalStore::MaxBytes) { return false; }
        if(Size>=0) { FConfigFile Config;Config.Read(Path);int32 Version=0;Config.GetInt(TEXT("Fiskelaget.Journal"),TEXT("Version"),Version);if(Version>1) { return false; } }
    }
    return true;
}
}
FString FiskJournalAuthority::File(const FString& Directory,const FGuid& Profile)
{ return FPaths::Combine(Directory,TEXT("Journal-")+Profile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
bool FiskJournalAuthority::Load(const FString& Directory,const FGuid& Profile,FFiskJournalModel& Journal)
{
    if(!Profile.IsValid()) { return false; }const FString Path=File(Directory,Profile);if(!Preservable(Path)) { return false; }
    FFiskJournalStore Store;return Store.Load(Path,Journal)!=EFiskJournalLoad::Unavailable;
}
bool FiskJournalAuthority::RecoverIncome(const FString& Directory,FFiskCareerProfile& Career)
{
    if(!Career.IsValid()) { return false; }FFiskJournalModel Journal;if(!Load(Directory,Career.Id,Journal)) { return false; }
    struct FDay { FGuid Id;int64 Income=0;FString Utc; };TArray<FDay> Days;
    for(const FFiskJournalEntry& Entry:Journal.GetEntries())
    {
        int32 Index=Days.IndexOfByPredicate([&](const FDay& D){return D.Id==Entry.DayId;});
        if(Index<0) { Index=Days.Add(FDay{Entry.DayId,0,Entry.Context.LandedUtc}); }
        FDay& D=Days[Index];D.Income+=Entry.bSold?static_cast<int64>(Entry.Fish.Value)*100:0;
        if(Entry.Context.LandedUtc<D.Utc) { D.Utc=Entry.Context.LandedUtc; }
    }
    Days.Sort([](const FDay& A,const FDay& B){return A.Utc==B.Utc?A.Id.ToString()<B.Id.ToString():A.Utc<B.Utc;});
    FFiskCareerProfile Candidate=Career;
    for(const FDay& D:Days)
    {
        const FFiskCareerReceipt* Receipt=Candidate.Receipts.FindByPredicate([&](const FFiskCareerReceipt& R){return R.Id==D.Id;});
        if(Receipt) { if(Receipt->Kind!=FFiskCareerReceipt::KindSettlement || Receipt->Amount!=D.Income) { return false; } }
        else if(!Candidate.Credit(D.Id,D.Income)) { return false; }
    }
    Career=MoveTemp(Candidate);return true;
}
bool FiskJournalAuthority::Commit(const FString& Directory,const FFiskSettlementView& View,int32 NowMs,const TMap<int32,FGuid>& Profiles)
{
    if(!View.bRevealed || !View.DayId.IsValid() || View.DayNumber<1 || NowMs<0) { return false; }
    for(const FFiskDayResult& Row:View.Results)
    { const FGuid* Owner=Profiles.Find(Row.PlayerId);if(!Row.Catches.IsEmpty() && (!Owner || !Owner->IsValid())) { return false; } }
    TSet<FGuid> Unique;for(const auto& Pair:Profiles) { if(Pair.Value.IsValid()) { Unique.Add(Pair.Value); } }
    for(const FGuid& Profile:Unique)
    {
        FFiskJournalModel Journal;if(!Load(Directory,Profile,Journal)) { return false; }
        for(const FFiskDayResult& Row:View.Results)
        {
            const FGuid* Owner=Profiles.Find(Row.PlayerId);
            if(Owner && *Owner==Profile && Journal.Apply(View.DayId,View.DayNumber,NowMs,true,Row)==EFiskJournalApply::Rejected) { return false; }
        }
        FFiskJournalStore Store;if(!Store.Save(File(Directory,Profile),Journal)) { return false; }
        const FString CareerFile=FPaths::Combine(Directory,Profile.ToString(EGuidFormats::Digits)+TEXT(".ini"));
        FFiskCareerProfile Career;const EFiskCareerLoad Status=FFiskCareerStore::Load(CareerFile,Career);
        if((Status!=EFiskCareerLoad::Loaded && Status!=EFiskCareerLoad::Recovered) || Career.Id!=Profile || !RecoverIncome(Directory,Career)) { return false; }
        if(!Career.Receipts.ContainsByPredicate([&](const FFiskCareerReceipt& R){return R.Id==View.DayId;}) && !Career.Credit(View.DayId,0)) { return false; }
        if(!FFiskCareerStore::Save(CareerFile,Career)) { return false; }
    }
    return true;
}
