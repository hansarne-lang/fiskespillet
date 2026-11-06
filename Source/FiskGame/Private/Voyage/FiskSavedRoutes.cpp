#include "Voyage/FiskSavedRoutes.h"
#include "Voyage/FiskVoyagePlan.h"
#include "FiskCareerProfile.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace
{
const TCHAR* RouteDiskMeta=TEXT("Fiskelaget.Routes");
bool RouteDiskFuture(const FString& P)
{ const int64 N=IFileManager::Get().FileSize(*P);if(N>16384) { return true; }FConfigFile C;C.Read(P);int32 V=0;return C.GetInt(RouteDiskMeta,TEXT("Version"),V) && V>1; }
bool RouteDiskRead(const FString& P,const FGuid& Id,TArray<FFiskSavedRoute>& Out)
{
    const int64 N=IFileManager::Get().FileSize(*P);if(N<0 || N>16384) { return false; }FConfigFile C;C.Read(P);int32 V=0;FString Profile;
    if(!C.GetInt(RouteDiskMeta,TEXT("Version"),V) || V!=1 || !C.GetString(RouteDiskMeta,TEXT("Profile"),Profile) || Profile!=Id.ToString(EGuidFormats::Digits) || C.Num()!=4) { return false; }
    TArray<FFiskSavedRoute> Candidate;
    for(int32 I=0;I<3;++I)
    {
        FFiskSavedRoute R;const FString S=FString::Printf(TEXT("Route%d"),I);int32 Count=0;
        if(!C.GetString(*S,TEXT("Name"),R.Name) || !C.GetInt(*S,TEXT("Count"),Count) || Count<0 || Count>5) { return false; }
        for(int32 J=0;J<Count;++J) { int32 X=0,Y=0;const FString K=FString::Printf(TEXT("Point%d"),J);
            if(!C.GetInt(*S,*(K+TEXT("X")),X) || !C.GetInt(*S,*(K+TEXT("Y")),Y)) { return false; }R.Points.Add(FVector(X,Y,0)); }
        Candidate.Add(R);
    }
    if(!FFiskSavedRoutes::Valid(Candidate)) { return false; }Out=MoveTemp(Candidate);return true;
}
FString RouteDiskLockName(const FString& P) { return FString::Printf(TEXT("FiskelagetRoutes-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(P).ToLower())); }
}
bool FFiskSavedRoutes::Valid(const TArray<FFiskSavedRoute>& S)
{ if(S.Num()!=3) { return false; }for(const auto& R:S) { if(!FFiskCareerProfile::ValidName(R.Name) || !FFiskVoyageRoute::Valid(R.Points,true)) { return false; } }return true; }
bool FFiskSavedRoutes::Load(const FString& File,const FGuid& Id)
{
    Slots={{TEXT("RUTE 1"),{}},{TEXT("RUTE 2"),{}},{TEXT("RUTE 3"),{}}};bBlocked=false;bRecovered=false;
    FSystemWideCriticalSection Lock(RouteDiskLockName(File),FTimespan::FromSeconds(1));
    if(!Id.IsValid() || !Lock.IsValid() || RouteDiskFuture(File) || RouteDiskFuture(File+TEXT(".bak"))) { bBlocked=true;return false; }
    if(RouteDiskRead(File,Id,Slots)) { return true; }if(RouteDiskRead(File+TEXT(".bak"),Id,Slots)) { bRecovered=true;return true; }
    bBlocked=IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")));return !bBlocked;
}
bool FFiskSavedRoutes::Save(const FString& File,const FGuid& Id) const
{
    FSystemWideCriticalSection Lock(RouteDiskLockName(File),FTimespan::FromSeconds(1));
    if(bBlocked || !Id.IsValid() || !Lock.IsValid() || !Valid(Slots) || RouteDiskFuture(File) || RouteDiskFuture(File+TEXT(".bak"))) { return false; }
    TArray<FFiskSavedRoute> Old;const bool Main=RouteDiskRead(File,Id,Old),Back=RouteDiskRead(File+TEXT(".bak"),Id,Old);
    if(!Main && !Back && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak"))))) { return false; }
    FConfigFile C;C.SetString(RouteDiskMeta,TEXT("Version"),TEXT("1"));C.SetString(RouteDiskMeta,TEXT("Profile"),*Id.ToString(EGuidFormats::Digits));
    for(int32 I=0;I<3;++I) { const auto& R=Slots[I];const FString S=FString::Printf(TEXT("Route%d"),I);C.SetString(*S,TEXT("Name"),*R.Name);C.SetString(*S,TEXT("Count"),*FString::FromInt(R.Points.Num()));
        for(int32 J=0;J<R.Points.Num();++J) { const FString K=FString::Printf(TEXT("Point%d"),J);C.SetString(*S,*(K+TEXT("X")),*FString::FromInt(FMath::RoundToInt(R.Points[J].X)));C.SetString(*S,*(K+TEXT("Y")),*FString::FromInt(FMath::RoundToInt(R.Points[J].Y))); } }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");C.Dirty=true;
    if(!C.Write(Temp)) { return false; }TArray<FFiskSavedRoute> Verify;if(!RouteDiskRead(Temp,Id,Verify)) { return false; }
    const FString Previous=Main?File:Back?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Previous,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
