#include "Career/FiskCareerStore.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace FiskCareerDisk
{
const TCHAR* Section=TEXT("Fiskelaget.Career");
bool Number(const FConfigFile& C,const TCHAR* S,const TCHAR* K,int64& V)
{ FString T;return C.GetString(S,K,T) && T.Len()<=20 && LexTryParseString(V,*T) && FString::Printf(TEXT("%lld"),static_cast<long long>(V))==T; }
FString Lock(const FString& File)
{ FString P=FPaths::ConvertRelativePathToFull(File);FPaths::NormalizeFilename(P);return FString::Printf(TEXT("FiskelagetCareer-%08X"),GetTypeHash(P.ToLower())); }
void Put(FConfigFile& C,const TCHAR* S,const TCHAR* K,int64 V) { C.SetString(S,K,*FString::Printf(TEXT("%lld"),static_cast<long long>(V))); }
bool Future(const FString& File)
{ FConfigFile C;const int64 Size=IFileManager::Get().FileSize(*File);if(Size<1 || Size>262144) { return false; }C.Read(File);int64 V=0;return Number(C,Section,TEXT("Version"),V) && V>2; }
}
bool FFiskCareerStore::Read(const FString& File,FFiskCareerProfile& Profile)
{
    using namespace FiskCareerDisk;
    const int64 Bytes=IFileManager::Get().FileSize(*File);if(Bytes<1 || Bytes>262144) { return false; }
    FConfigFile C;C.Read(File);FFiskCareerProfile P;FString Id;int64 Version=0,Revision=0,Owned=0;
    if(!Number(C,Section,TEXT("Version"),Version) || (Version!=1 && Version!=2) || !Number(C,Section,TEXT("Revision"),Revision)
        || Revision<1 || Revision>FFiskCareerProfile::MaxReceipts || C.Num()!=Revision+1
        || !C.GetString(Section,TEXT("Id"),Id) || !FGuid::ParseExact(Id,EGuidFormats::Digits,P.Id)
        || !C.GetString(Section,TEXT("Name"),P.Name) || !Number(C,Section,TEXT("Cash"),P.Cash)
        || !Number(C,Section,TEXT("Owned"),Owned) || Owned<0 || Owned>(Version==1?15:FFiskCareerProfile::OwnedMask)) { return false; }
    P.Revision=static_cast<int32>(Revision);P.Owned=static_cast<int32>(Owned);
    for(int32 I=0;I<P.Revision;++I)
    {
        const FString S=FString::Printf(TEXT("Receipt%d"),I);FFiskCareerReceipt R;int64 Kind=0,Item=0;
        if(!C.GetString(*S,TEXT("Id"),Id) || !FGuid::ParseExact(Id,EGuidFormats::Digits,R.Id)
            || !Number(C,*S,TEXT("Kind"),Kind) || Kind<0 || Kind>2 || !Number(C,*S,TEXT("Item"),Item)
            || Item< -1 || Item>(Version==1?3:FFiskCareerProfile::ItemCount-1) || !Number(C,*S,TEXT("Amount"),R.Amount)) { return false; }
        R.Kind=static_cast<int32>(Kind);R.Item=static_cast<int32>(Item);P.Receipts.Add(R);
    }
    if(!P.IsValid()) { return false; }Profile=MoveTemp(P);return true;
}
bool FFiskCareerStore::Write(const FString& File,const FFiskCareerProfile& P)
{
    using namespace FiskCareerDisk;
    FConfigFile C;Put(C,Section,TEXT("Version"),(P.Owned&~15)?2:1);Put(C,Section,TEXT("Revision"),P.Revision);Put(C,Section,TEXT("Cash"),P.Cash);Put(C,Section,TEXT("Owned"),P.Owned);
    C.SetString(Section,TEXT("Id"),*P.Id.ToString(EGuidFormats::Digits));C.SetString(Section,TEXT("Name"),*P.Name);
    for(int32 I=0;I<P.Receipts.Num();++I)
    {
        const FString S=FString::Printf(TEXT("Receipt%d"),I);const FFiskCareerReceipt& R=P.Receipts[I];
        C.SetString(*S,TEXT("Id"),*R.Id.ToString(EGuidFormats::Digits));Put(C,*S,TEXT("Kind"),R.Kind);Put(C,*S,TEXT("Item"),R.Item);Put(C,*S,TEXT("Amount"),R.Amount);
    }
    C.Dirty=true;return C.Write(File) && IFileManager::Get().FileSize(*File)<=262144;
}
EFiskCareerLoad FFiskCareerStore::Load(const FString& File,FFiskCareerProfile& P)
{
    FSystemWideCriticalSection Lock(FiskCareerDisk::Lock(File),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return EFiskCareerLoad::Blocked; }
    if(FiskCareerDisk::Future(File) || FiskCareerDisk::Future(File+TEXT(".bak"))) { return EFiskCareerLoad::Blocked; }
    FFiskCareerProfile Main,Back;const bool M=Read(File,Main),B=Read(File+TEXT(".bak"),Back);
    if(M && (!B || Main.Extends(Back))) { P=Main;return EFiskCareerLoad::Loaded; }
    if(B && (!M || Back.Extends(Main))) { P=Back;return EFiskCareerLoad::Recovered; }
    return IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")))?EFiskCareerLoad::Blocked:EFiskCareerLoad::Empty;
}
bool FFiskCareerStore::Save(const FString& File,const FFiskCareerProfile& P)
{
    if(!P.IsValid()) { return false; }
    FSystemWideCriticalSection Lock(FiskCareerDisk::Lock(File),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return false; }
    if(FiskCareerDisk::Future(File) || FiskCareerDisk::Future(File+TEXT(".bak"))) { return false; }
    FFiskCareerProfile Main,Back;const bool M=Read(File,Main),B=Read(File+TEXT(".bak"),Back);
    if((M && !P.Extends(Main)) || (B && !P.Extends(Back))
        || (!M && !B && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")))))) { return false; }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    const FString Temp=File+TEXT(".tmp"),Backup=File+TEXT(".bak.tmp");
    if(!Write(Temp,P)) { return false; }
    FFiskCareerProfile Verify;if(!Read(Temp,Verify) || !Verify.Extends(P) || Verify.Revision!=P.Revision) { return false; }
    // Once premium ownership commits, both copies require a v2-aware reader.
    const bool Promote=(P.Owned&~15)!=0 && (!M || (Main.Owned&~15)==0) && (!B || (Back.Owned&~15)==0);
    const FString Prior=Promote?Temp:M && (!B || Main.Extends(Back))?File:B?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Backup,*Prior,true,true)==COPY_OK
        && IFileManager::Get().Move(*(File+TEXT(".bak")),*Backup,true,true)
        && IFileManager::Get().Move(*File,*Temp,true,true);
}
