#include "Career/FiskMarinaStore.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace MarinaDisk
{
const TCHAR* Section=TEXT("Fiskelaget.Marina");
FString LockName(const FString& File) { return FString::Printf(TEXT("FiskelagetMarina-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(File).ToLower())); }
bool Read(const FString& File,const FGuid& Profile,FFiskMarinaIdentity& Out,int32& Revision,bool& Future)
{
    const int64 Size=IFileManager::Get().FileSize(*File);if(Size<1 || Size>8192) { return false; }
    FConfigFile C;C.Read(File);int32 Version=0;C.GetInt(Section,TEXT("Version"),Version);Future=Version>2;
    FString Id;FFiskMarinaIdentity V;int32 ValidatedRevision=0;
    if((Version!=1 && Version!=2) || C.Num()!=1 || !C.GetString(Section,TEXT("Profile"),Id) || Id!=Profile.ToString(EGuidFormats::Digits)
        || !C.GetInt(Section,TEXT("Revision"),ValidatedRevision) || ValidatedRevision<1 || ValidatedRevision>=MAX_int32
        || !C.GetString(Section,TEXT("DisplayName"),V.DisplayName) || !C.GetString(Section,TEXT("HouseName"),V.HouseName)
        || !C.GetInt(Section,TEXT("Color"),V.Color) || !C.GetInt(Section,TEXT("Style"),V.Style) || !C.GetInt(Section,TEXT("Decor"),V.Decor) || !V.IsValid()) { return false; }
    if(Version==2 && (!C.GetInt(Section,TEXT("Lighting"),V.Lighting) || !C.GetInt(Section,TEXT("Pier"),V.Pier) || !C.GetInt(Section,TEXT("Display"),V.Display) || !V.IsValid())) { return false; }
    Out=V;Revision=ValidatedRevision;return true;
}
bool Write(const FString& File,const FGuid& Profile,const FFiskMarinaIdentity& V,int32 Revision)
{
    FConfigFile C;C.SetString(Section,TEXT("Version"),(V.Lighting || V.Pier || V.Display)?TEXT("2"):TEXT("1"));
    if(V.Lighting || V.Pier || V.Display) { C.SetString(Section,TEXT("Lighting"),*FString::FromInt(V.Lighting));C.SetString(Section,TEXT("Pier"),*FString::FromInt(V.Pier));C.SetString(Section,TEXT("Display"),*FString::FromInt(V.Display)); }C.SetString(Section,TEXT("Revision"),*FString::FromInt(Revision));
    C.SetString(Section,TEXT("Profile"),*Profile.ToString(EGuidFormats::Digits));C.SetString(Section,TEXT("DisplayName"),*V.DisplayName);C.SetString(Section,TEXT("HouseName"),*V.HouseName);
    C.SetString(Section,TEXT("Color"),*FString::FromInt(V.Color));C.SetString(Section,TEXT("Style"),*FString::FromInt(V.Style));C.SetString(Section,TEXT("Decor"),*FString::FromInt(V.Decor));C.Dirty=true;
    return C.Write(File) && IFileManager::Get().FileSize(*File)<=8192;
}
}
EFiskMarinaLoad FFiskMarinaStore::Load(const FString& File,const FGuid& Profile,FFiskMarinaIdentity& Identity)
{
    if(!Profile.IsValid()) { return EFiskMarinaLoad::Blocked; }
    FSystemWideCriticalSection Lock(MarinaDisk::LockName(File),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return EFiskMarinaLoad::Blocked; }
    FFiskMarinaIdentity A,B;int32 AR=0,BR=0;bool AF=false,BF=false;
    const bool AM=MarinaDisk::Read(File,Profile,A,AR,AF),BM=MarinaDisk::Read(File+TEXT(".bak"),Profile,B,BR,BF);
    if(AF || BF || (AM && BM && AR==BR && !(A==B))) { return EFiskMarinaLoad::Blocked; }
    if(AM && (!BM || AR>=BR)) { Identity=A;return EFiskMarinaLoad::Loaded; }
    if(BM) { Identity=B;return EFiskMarinaLoad::Recovered; }
    return IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")))?EFiskMarinaLoad::Blocked:EFiskMarinaLoad::Empty;
}
bool FFiskMarinaStore::Save(const FString& File,const FGuid& Profile,const FFiskMarinaIdentity& Identity)
{
    if(!Profile.IsValid() || !Identity.IsValid()) { return false; }
    FSystemWideCriticalSection Lock(MarinaDisk::LockName(File),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return false; }
    FFiskMarinaIdentity A,B;int32 AR=0,BR=0;bool AF=false,BF=false;
    const bool AM=MarinaDisk::Read(File,Profile,A,AR,AF),BM=MarinaDisk::Read(File+TEXT(".bak"),Profile,B,BR,BF);
    if(AF || BF || (AM && BM && AR==BR && !(A==B)) || (!AM && !BM && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")))))) { return false; }
    if(AM && (!BM || AR>=BR) && A==Identity) { return true; }
    const int32 Highest=FMath::Max(AM?AR:0,BM?BR:0);if(Highest>=MAX_int32-1) { return false; }
    const int32 Revision=Highest+1;
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");
    if(!MarinaDisk::Write(Temp,Profile,Identity,Revision)) { return false; }
    FFiskMarinaIdentity Check;int32 CR=0;bool CF=false;if(!MarinaDisk::Read(Temp,Profile,Check,CR,CF) || !(Check==Identity) || CR!=Revision) { return false; }
    const FString Prior=AM && (!BM || AR>=BR)?File:BM?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Prior,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
