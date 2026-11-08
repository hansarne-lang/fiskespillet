#include "Voyage/FiskSkipperTools.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace
{
const TCHAR* PhotoDiskMeta=TEXT("Fiskelaget.Camera");
bool PhotoDiskFuture(const FString& P) { if(IFileManager::Get().FileSize(*P)>8192) { return true; }FConfigFile C;C.Read(P);int32 V=0;return C.GetInt(PhotoDiskMeta,TEXT("Version"),V) && V>1; }
FString PhotoDiskLock(const FString& P) { return FString::Printf(TEXT("FiskelagetCamera-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(P).ToLower())); }
bool PhotoDiskRead(const FString& P,const FGuid& Id,FFiskPhotoPresets& Out)
{
    const int64 N=IFileManager::Get().FileSize(*P);if(N<0 || N>8192) { return false; }FConfigFile C;C.Read(P);int32 V=0;FString Profile;FFiskPhotoPresets D;
    if(C.Num()!=4 || !C.GetInt(PhotoDiskMeta,TEXT("Version"),V) || V!=1 || !C.GetString(PhotoDiskMeta,TEXT("Profile"),Profile) || Profile!=Id.ToString(EGuidFormats::Digits)) { return false; }
    for(int32 I=0;I<3;++I) { const FString S=FString::Printf(TEXT("View%d"),I);auto& F=D.Views[I];if(!C.GetDouble(*S,TEXT("Yaw"),F.Yaw) || !C.GetDouble(*S,TEXT("Pitch"),F.Pitch) || !C.GetDouble(*S,TEXT("Distance"),F.Distance) || !C.GetDouble(*S,TEXT("Fov"),F.Fov)) { return false; } }if(!D.Valid()) { return false; }Out=MoveTemp(D);return true;
}
}
bool FFiskPhotoPresets::Valid() const
{ if(Views.Num()!=3) { return false; }for(const auto& V:Views) { if(!FMath::IsFinite(V.Yaw) || V.Yaw<-180 || V.Yaw>180 || !FMath::IsFinite(V.Pitch) || V.Pitch<5 || V.Pitch>70 || !FMath::IsFinite(V.Distance) || V.Distance<800 || V.Distance>5000 || !FMath::IsFinite(V.Fov) || V.Fov<35 || V.Fov>100) { return false; } }return true; }
bool FFiskPhotoPresets::Load(const FString& File,const FGuid& Id)
{
    *this=FFiskPhotoPresets();FSystemWideCriticalSection Lock(PhotoDiskLock(File),FTimespan::FromSeconds(1));if(!Id.IsValid() || !Lock.IsValid() || PhotoDiskFuture(File) || PhotoDiskFuture(File+TEXT(".bak"))) { bBlocked=true;return false; }FFiskPhotoPresets D;
    if(PhotoDiskRead(File,Id,D)) { *this=MoveTemp(D);return true; }if(PhotoDiskRead(File+TEXT(".bak"),Id,D)) { *this=MoveTemp(D);bRecovered=true;return true; }bBlocked=IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")));return !bBlocked;
}
bool FFiskPhotoPresets::Save(const FString& File,const FGuid& Id) const
{
    FSystemWideCriticalSection Lock(PhotoDiskLock(File),FTimespan::FromSeconds(1));if(bBlocked || !Id.IsValid() || !Lock.IsValid() || !Valid() || PhotoDiskFuture(File) || PhotoDiskFuture(File+TEXT(".bak"))) { return false; }FFiskPhotoPresets Old;const bool Main=PhotoDiskRead(File,Id,Old),Back=PhotoDiskRead(File+TEXT(".bak"),Id,Old);if(!Main && !Back && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak"))))) { return false; }
    FConfigFile C;C.SetString(PhotoDiskMeta,TEXT("Version"),TEXT("1"));C.SetString(PhotoDiskMeta,TEXT("Profile"),*Id.ToString(EGuidFormats::Digits));for(int32 I=0;I<3;++I) { const FString S=FString::Printf(TEXT("View%d"),I);const auto& V=Views[I];C.SetString(*S,TEXT("Yaw"),*FString::SanitizeFloat(V.Yaw));C.SetString(*S,TEXT("Pitch"),*FString::SanitizeFloat(V.Pitch));C.SetString(*S,TEXT("Distance"),*FString::SanitizeFloat(V.Distance));C.SetString(*S,TEXT("Fov"),*FString::SanitizeFloat(V.Fov)); }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");C.Dirty=true;if(!C.Write(Temp)) { return false; }FFiskPhotoPresets Check;if(!PhotoDiskRead(Temp,Id,Check)) { return false; }const FString Prior=Main?File:Back?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Prior,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
