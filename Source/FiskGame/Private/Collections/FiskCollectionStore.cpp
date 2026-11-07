#include "Collections/FiskCollectionModel.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace
{
const TCHAR* CollectionDiskSection=TEXT("Fiskelaget.Collections");
bool CollectionDiskFuture(const FString& P)
{ if(IFileManager::Get().FileSize(*P)>8192) { return true; }FConfigFile C;C.Read(P);int32 V=0;return C.GetInt(CollectionDiskSection,TEXT("Version"),V) && V>1; }
FString CollectionDiskLock(const FString& P) { return FString::Printf(TEXT("FiskelagetCollections-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(P).ToLower())); }
bool CollectionDiskRead(const FString& P,const FGuid& Id,FFiskCollectionPins& Out)
{
    const int64 Size=IFileManager::Get().FileSize(*P);if(Size<0 || Size>8192) { return false; }FConfigFile C;C.Read(P);int32 V=0;FString Profile;FFiskCollectionPins D;
    if(C.Num()!=1 || !C.GetInt(CollectionDiskSection,TEXT("Version"),V) || V!=1 || !C.GetString(CollectionDiskSection,TEXT("Profile"),Profile) || Profile!=Id.ToString(EGuidFormats::Digits)) { return false; }
    for(int32 I=0;I<3;++I) { if(!C.GetInt(CollectionDiskSection,*FString::Printf(TEXT("Pin%d"),I),D.Values[I])) { return false; } }if(!D.Valid()) { return false; }Out=MoveTemp(D);return true;
}
}
bool FFiskCollectionPins::Load(const FString& File,const FGuid& Id)
{
    *this=FFiskCollectionPins();FSystemWideCriticalSection Lock(CollectionDiskLock(File),FTimespan::FromSeconds(1));
    if(!Id.IsValid() || !Lock.IsValid() || CollectionDiskFuture(File) || CollectionDiskFuture(File+TEXT(".bak"))) { bBlocked=true;return false; }
    FFiskCollectionPins D;if(CollectionDiskRead(File,Id,D)) { *this=MoveTemp(D);return true; }if(CollectionDiskRead(File+TEXT(".bak"),Id,D)) { *this=MoveTemp(D);bRecovered=true;return true; }
    bBlocked=IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")));return !bBlocked;
}
bool FFiskCollectionPins::Save(const FString& File,const FGuid& Id) const
{
    FSystemWideCriticalSection Lock(CollectionDiskLock(File),FTimespan::FromSeconds(1));if(bBlocked || !Id.IsValid() || !Lock.IsValid() || !Valid() || CollectionDiskFuture(File) || CollectionDiskFuture(File+TEXT(".bak"))) { return false; }
    FFiskCollectionPins Old;const bool Main=CollectionDiskRead(File,Id,Old),Back=CollectionDiskRead(File+TEXT(".bak"),Id,Old);if(!Main && !Back && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak"))))) { return false; }
    FConfigFile C;C.SetString(CollectionDiskSection,TEXT("Version"),TEXT("1"));C.SetString(CollectionDiskSection,TEXT("Profile"),*Id.ToString(EGuidFormats::Digits));for(int32 I=0;I<3;++I) { C.SetString(CollectionDiskSection,*FString::Printf(TEXT("Pin%d"),I),*FString::FromInt(Values[I])); }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");C.Dirty=true;if(!C.Write(Temp)) { return false; }FFiskCollectionPins Check;if(!CollectionDiskRead(Temp,Id,Check)) { return false; }const FString Prior=Main?File:Back?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Prior,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
