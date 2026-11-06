#include "Angler/FiskAnglerModel.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace
{
const TCHAR* AnglerDiskMeta=TEXT("Fiskelaget.Angler");
bool AnglerDiskFuture(const FString& P)
{ if(IFileManager::Get().FileSize(*P)>32768) { return true; }FConfigFile C;C.Read(P);int32 V=0;return C.GetInt(AnglerDiskMeta,TEXT("Version"),V) && V>2; }
FString AnglerDiskLock(const FString& P) { return FString::Printf(TEXT("FiskelagetAngler-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(P).ToLower())); }
bool AnglerDiskRead(const FString& P,const FGuid& Id,FFiskAnglerData& Out)
{
    const int64 N=IFileManager::Get().FileSize(*P);if(N<0 || N>32768) { return false; }FConfigFile C;C.Read(P);int32 V=0;FString Profile;
    if(!C.GetInt(AnglerDiskMeta,TEXT("Version"),V) || (V!=1 && V!=2) || C.Num()!=(V==1?18:4+FiskAngler::ScoreCount) || !C.GetString(AnglerDiskMeta,TEXT("Profile"),Profile) || Profile!=Id.ToString(EGuidFormats::Digits)) { return false; }
    FFiskAnglerData D;
    for(int32 I=0;I<3;++I)
    {
        const FString S=FString::Printf(TEXT("Loadout%d"),I);auto& L=D.Loadouts[I];int32 Count=0;
        if(!C.GetString(*S,TEXT("Name"),L.Name) || !C.GetInt(*S,TEXT("Count"),Count) || Count<0 || Count>3) { return false; }
        for(int32 J=0;J<Count;++J) { FFiskRigSettings R;const FString K=FString::Printf(TEXT("Rod%d"),J);if(!C.GetInt(*S,*(K+TEXT("Bait")),R.Bait) || !C.GetInt(*S,*(K+TEXT("Depth")),R.Depth) || !C.GetInt(*S,*(K+TEXT("Program")),R.Program)) { return false; }L.Rigs.Add(R); }
    }
    for(int32 I=0;I<(V==1?14:FiskAngler::ScoreCount);++I)
    {
        const FString S=FString::Printf(TEXT("Practice%d"),I);auto& A=D.Scores[I];FString Last,Active;
        if(!C.GetInt(*S,TEXT("Attempts"),A.Attempts) || !C.GetInt(*S,TEXT("Completed"),A.Completed) || !C.GetInt(*S,TEXT("BestMs"),A.BestMs) || !C.GetString(*S,TEXT("Last"),Last) || !C.GetString(*S,TEXT("Active"),Active)) { return false; }
        if(!Last.IsEmpty() && !FGuid::ParseExact(Last,EGuidFormats::Digits,A.LastCompleted)) { return false; }
        if(!Active.IsEmpty() && !FGuid::ParseExact(Active,EGuidFormats::Digits,A.ActiveAttempt)) { return false; }
    }
    if(!D.Valid()) { return false; }Out=MoveTemp(D);return true;
}
}
bool FFiskAnglerData::Load(const FString& File,const FGuid& Id)
{
    *this=FFiskAnglerData();FSystemWideCriticalSection Lock(AnglerDiskLock(File),FTimespan::FromSeconds(1));
    if(!Id.IsValid() || !Lock.IsValid() || AnglerDiskFuture(File) || AnglerDiskFuture(File+TEXT(".bak"))) { bBlocked=true;return false; }
    FFiskAnglerData D;if(AnglerDiskRead(File,Id,D)) { *this=MoveTemp(D);return true; }
    if(AnglerDiskRead(File+TEXT(".bak"),Id,D)) { *this=MoveTemp(D);bRecovered=true;return true; }
    bBlocked=IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")));return !bBlocked;
}
bool FFiskAnglerData::Save(const FString& File,const FGuid& Id) const
{
    FSystemWideCriticalSection Lock(AnglerDiskLock(File),FTimespan::FromSeconds(1));
    if(bBlocked || !Id.IsValid() || !Lock.IsValid() || !Valid() || AnglerDiskFuture(File) || AnglerDiskFuture(File+TEXT(".bak"))) { return false; }
    FFiskAnglerData Old;const bool Main=AnglerDiskRead(File,Id,Old),Back=AnglerDiskRead(File+TEXT(".bak"),Id,Old);
    if(!Main && !Back && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak"))))) { return false; }
    FConfigFile C;C.SetString(AnglerDiskMeta,TEXT("Version"),TEXT("2"));C.SetString(AnglerDiskMeta,TEXT("Profile"),*Id.ToString(EGuidFormats::Digits));
    for(int32 I=0;I<3;++I) { const FString S=FString::Printf(TEXT("Loadout%d"),I);const auto& L=Loadouts[I];C.SetString(*S,TEXT("Name"),*L.Name);C.SetString(*S,TEXT("Count"),*FString::FromInt(L.Rigs.Num()));
        for(int32 J=0;J<L.Rigs.Num();++J) { const FString K=FString::Printf(TEXT("Rod%d"),J);const auto& R=L.Rigs[J];C.SetString(*S,*(K+TEXT("Bait")),*FString::FromInt(R.Bait));C.SetString(*S,*(K+TEXT("Depth")),*FString::FromInt(R.Depth));C.SetString(*S,*(K+TEXT("Program")),*FString::FromInt(R.Program)); } }
    for(int32 I=0;I<FiskAngler::ScoreCount;++I) { const FString S=FString::Printf(TEXT("Practice%d"),I);const auto& A=Scores[I];C.SetString(*S,TEXT("Attempts"),*FString::FromInt(A.Attempts));C.SetString(*S,TEXT("Completed"),*FString::FromInt(A.Completed));C.SetString(*S,TEXT("BestMs"),*FString::FromInt(A.BestMs));C.SetString(*S,TEXT("Last"),A.LastCompleted.IsValid()?*A.LastCompleted.ToString(EGuidFormats::Digits):TEXT(""));C.SetString(*S,TEXT("Active"),A.ActiveAttempt.IsValid()?*A.ActiveAttempt.ToString(EGuidFormats::Digits):TEXT("")); }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");C.Dirty=true;
    if(!C.Write(Temp)) { return false; }FFiskAnglerData Check;if(!AnglerDiskRead(Temp,Id,Check)) { return false; }const FString Previous=Main?File:Back?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Previous,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
