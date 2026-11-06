#include "Expedition/FiskExpeditionStore.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"

namespace
{
constexpr int64 MaxBytes=1024*1024;
const TCHAR* Meta=TEXT("Fiskelaget.Expeditions");
bool Preservable(const FString& File)
{
    for(const FString& Path:{File,File+TEXT(".bak")})
    {
        const int64 Size=IFileManager::Get().FileSize(*Path);if(Size>MaxBytes) { return false; }
        if(Size>=0) { FConfigFile C;C.Read(Path);int32 Version=0;if(C.GetInt(Meta,TEXT("Version"),Version) && Version>1) { return false; } }
    }return true;
}
bool Read(const FString& File,const FGuid& Profile,TArray<FFiskExpeditionRecord>& Out,int32& Generation)
{
    if(IFileManager::Get().FileSize(*File)<0 || IFileManager::Get().FileSize(*File)>MaxBytes) { return false; }
    FConfigFile C;C.Read(File);int32 Version=0,Count=0;FString Id;
    if(!C.GetInt(Meta,TEXT("Version"),Version) || Version!=1 || !C.GetString(Meta,TEXT("Profile"),Id) || Id!=Profile.ToString(EGuidFormats::Digits)
        || !C.GetInt(Meta,TEXT("Generation"),Generation) || Generation<1 || !C.GetInt(Meta,TEXT("Count"),Count)
        || Count<0 || Count>FiskExpeditions::Capacity || C.Num()!=Count+1) { return false; }
    TArray<FFiskExpeditionRecord> Entries;
    for(const auto& Pair:C)
    {
        if(Pair.Key==Meta) { continue; }FFiskExpeditionRecord R;int32 Completed=-1;const TCHAR* S=*Pair.Key;
        if(!Pair.Key.StartsWith(TEXT("Day.")) || !FGuid::ParseExact(Pair.Key.Mid(4),EGuidFormats::Digits,R.DayId)
            || !C.GetInt(S,TEXT("DayNumber"),R.DayNumber) || !C.GetInt(S,TEXT("Goal"),R.Goal) || !C.GetInt(S,TEXT("Weather"),R.Weather)
            || !C.GetInt(S,TEXT("Completed"),Completed) || (Completed!=0 && Completed!=1) || !R.IsValid()) { return false; }
        R.bCompleted=Completed==1;Entries.Add(R);
    }
    TArray<FFiskExpeditionRecord> Valid;if(!FiskExpeditionStore::Merge(Valid,Entries) || Valid.Num()!=Count) { return false; }Out=MoveTemp(Valid);return true;
}
bool Write(const FString& File,const FGuid& Profile,const TArray<FFiskExpeditionRecord>& Entries,int32 Generation)
{
    FConfigFile C;C.SetString(Meta,TEXT("Version"),TEXT("1"));C.SetString(Meta,TEXT("Profile"),*Profile.ToString(EGuidFormats::Digits));
    C.SetString(Meta,TEXT("Count"),*FString::FromInt(Entries.Num()));C.SetString(Meta,TEXT("Generation"),*FString::FromInt(Generation));
    for(const auto& R:Entries) { const FString S=TEXT("Day.")+R.DayId.ToString(EGuidFormats::Digits);
        C.SetString(*S,TEXT("DayNumber"),*FString::FromInt(R.DayNumber));C.SetString(*S,TEXT("Goal"),*FString::FromInt(R.Goal));
        C.SetString(*S,TEXT("Weather"),*FString::FromInt(R.Weather));C.SetString(*S,TEXT("Completed"),R.bCompleted?TEXT("1"):TEXT("0")); }
    C.Dirty=true;return C.Write(File) && IFileManager::Get().FileSize(*File)<=MaxBytes;
}
FString LockName(const FString& File)
{ return FString::Printf(TEXT("FiskelagetExpeditions-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(File).ToLower())); }
}
FString FiskExpeditionStore::File(const FString& Directory,const FGuid& Profile)
{ return FPaths::Combine(Directory,TEXT("Expeditions-")+Profile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
bool FiskExpeditionStore::Merge(TArray<FFiskExpeditionRecord>& Into,const TArray<FFiskExpeditionRecord>& Entries)
{
    TArray<FFiskExpeditionRecord> Candidate=Into;
    for(const auto& R:Entries)
    {
        if(!R.IsValid()) { return false; }
        const auto* Existing=Candidate.FindByPredicate([&](const auto& E){return E.DayId==R.DayId;});
        if(Existing) { if(!(*Existing==R)) { return false; } }else { if(Candidate.Num()>=FiskExpeditions::Capacity) { return false; }Candidate.Add(R); }
    }Into=MoveTemp(Candidate);return true;
}
bool FiskExpeditionStore::Load(const FString& Path,const FGuid& Profile,TArray<FFiskExpeditionRecord>& Entries)
{
    if(!Profile.IsValid()) { return false; }FSystemWideCriticalSection Lock(LockName(Path),FTimespan::FromSeconds(1));
    if(!Lock.IsValid() || !Preservable(Path)) { return false; }
    TArray<FFiskExpeditionRecord> A,B;int32 GA=0,GB=0;const bool GoodA=Read(Path,Profile,A,GA),GoodB=Read(Path+TEXT(".bak"),Profile,B,GB);
    if(GoodA && (!GoodB || GA>=GB)) { Entries=MoveTemp(A);return true; }if(GoodB) { Entries=MoveTemp(B);return true; }
    if(IFileManager::Get().FileExists(*Path) || IFileManager::Get().FileExists(*(Path+TEXT(".bak")))) { return false; }Entries.Reset();return true;
}
bool FiskExpeditionStore::Save(const FString& Path,const FGuid& Profile,TArray<FFiskExpeditionRecord>& Entries)
{
    if(!Profile.IsValid()) { return false; }FSystemWideCriticalSection Lock(LockName(Path),FTimespan::FromSeconds(1));
    if(!Lock.IsValid() || !Preservable(Path)) { return false; }
    TArray<FFiskExpeditionRecord> A,B;int32 GA=0,GB=0;const bool GoodA=Read(Path,Profile,A,GA),GoodB=Read(Path+TEXT(".bak"),Profile,B,GB);
    if(!GoodA && !GoodB && (IFileManager::Get().FileExists(*Path) || IFileManager::Get().FileExists(*(Path+TEXT(".bak"))))) { return false; }
    const bool UseA=GoodA && (!GoodB || GA>=GB);TArray<FFiskExpeditionRecord> Combined=UseA?A:GoodB?B:TArray<FFiskExpeditionRecord>();
    const int32 Generation=FMath::Max(GA,GB);if(Generation==MAX_int32 || !Merge(Combined,Entries)) { return false; }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);const FString Temp=Path+TEXT(".tmp"),BackTemp=Path+TEXT(".bak.tmp");
    if(!Write(Temp,Profile,Combined,Generation+1)) { return false; }
    const FString Previous=UseA?Path:GoodB?Path+TEXT(".bak"):Temp;
    if(IFileManager::Get().Copy(*BackTemp,*Previous,true,true)!=COPY_OK || !IFileManager::Get().Move(*(Path+TEXT(".bak")),*BackTemp,true,true)
        || !IFileManager::Get().Move(*Path,*Temp,true,true)) { return false; }
    Entries=MoveTemp(Combined);return true;
}
