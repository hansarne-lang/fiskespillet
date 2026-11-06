#include "UI/FiskJournalStore.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/CriticalSection.h"
#include "HAL/FileManager.h"

namespace
{
const TCHAR* Meta=TEXT("Fiskelaget.Journal");
bool Integer(const FConfigFile& Config,const TCHAR* Section,const TCHAR* Key,int32& Value)
{
    FString Text;int64 Wide=0;
    if(!Config.GetString(Section,Key,Text) || Text.Len()>11 || !LexTryParseString(Wide,*Text)
        || Wide<MIN_int32 || Wide>MAX_int32 || FString::Printf(TEXT("%lld"),static_cast<long long>(Wide))!=Text) { return false; }
    Value=static_cast<int32>(Wide);return true;
}
void Put(FConfigFile& Config,const TCHAR* Section,const TCHAR* Key,int32 Value)
{ Config.SetString(Section,Key,*FString::FromInt(Value)); }
bool Preservable(const FString& File)
{
    for(const FString& Path:{File,File+TEXT(".bak")})
    {
        const int64 Size=IFileManager::Get().FileSize(*Path);if(Size>FFiskJournalStore::MaxBytes) { return false; }
        if(Size>=0) { FConfigFile C;C.Read(Path);int32 Version=0;if(C.GetInt(Meta,TEXT("Version"),Version) && Version>1) { return false; } }
    }return true;
}
}
FString FFiskJournalStore::LockName(const FString& File)
{
    FString Canonical=FPaths::ConvertRelativePathToFull(File);FPaths::NormalizeFilename(Canonical);
    return FString::Printf(TEXT("FiskelagetJournal-%08X"),GetTypeHash(Canonical.ToLower()));
}
bool FFiskJournalStore::Read(const FString& File,FFiskJournalModel& Model,int32& Generation)
{
    const int64 Bytes=IFileManager::Get().FileSize(*File);
    if(Bytes<0 || Bytes>MaxBytes) { return false; }
    FConfigFile Config;Config.Read(File);int32 Version=0,Count=0;
    if(!Integer(Config,Meta,TEXT("Version"),Version) || Version!=1 || !Integer(Config,Meta,TEXT("Generation"),Generation)
        || Generation<1 || !Integer(Config,Meta,TEXT("Count"),Count) || Count<0 || Count>FFiskJournalModel::Capacity
        || Config.Num()!=Count+1) { return false; }
    TArray<FFiskJournalEntry> Entries;
    for(const auto& Pair:Config)
    {
        if(Pair.Key==Meta) { continue; }
        FFiskJournalEntry E;FString Day,Utc;int32 Sold=0;
        const TCHAR* Section=*Pair.Key;
        if(!Pair.Key.StartsWith(TEXT("Catch.")) || !FGuid::ParseExact(Pair.Key.Mid(6),EGuidFormats::Digits,E.CatchId)
            || !Config.GetString(Section,TEXT("DayId"),Day) || !FGuid::ParseExact(Day,EGuidFormats::Digits,E.DayId)
            || !Integer(Config,Section,TEXT("DayNumber"),E.DayNumber) || !Integer(Config,Section,TEXT("Species"),E.Fish.Species)
            || !Integer(Config,Section,TEXT("Grams"),E.Fish.WeightGrams) || !Integer(Config,Section,TEXT("LengthMm"),E.Fish.LengthMm)
            || !Integer(Config,Section,TEXT("Value"),E.Fish.Value) || !Integer(Config,Section,TEXT("Sold"),Sold) || (Sold!=0 && Sold!=1)
            || !Integer(Config,Section,TEXT("Ground"),E.Context.Ground) || !Integer(Config,Section,TEXT("Bait"),E.Context.Bait)
            || !Integer(Config,Section,TEXT("Depth"),E.Context.Depth) || !Integer(Config,Section,TEXT("Program"),E.Context.Program)
            || !Integer(Config,Section,TEXT("FightMs"),E.Context.FightMs) || !Integer(Config,Section,TEXT("LandedAtMs"),E.Context.LandedAtMs)
            || !Config.GetString(Section,TEXT("LandedUtc"),E.Context.LandedUtc)) { return false; }
        FString Optional;
        if(Config.GetString(Section,TEXT("BoatKind"),Optional) && !Integer(Config,Section,TEXT("BoatKind"),E.Context.BoatKind)) { return false; }
        if(Config.GetString(Section,TEXT("Equipment"),Optional) && !Integer(Config,Section,TEXT("Equipment"),E.Context.Equipment)) { return false; }
        if(Config.GetString(Section,TEXT("HasSeaPoint"),Optional))
        {
            int32 Has=0,X=0,Y=0;
            if(!Integer(Config,Section,TEXT("HasSeaPoint"),Has) || Has!=1 || !Integer(Config,Section,TEXT("SeaX"),X) || !Integer(Config,Section,TEXT("SeaY"),Y)) { return false; }
            E.Context.bHasSeaPoint=true;E.Context.SeaPoint=FVector(X,Y,0);
        }
        E.bSold=Sold==1;Entries.Add(E);
    }
    return Entries.Num()==Count && Model.Restore(Entries);
}
bool FFiskJournalStore::Write(const FString& File,const FFiskJournalModel& Model,int32 Generation)
{
    FConfigFile Config;Put(Config,Meta,TEXT("Version"),1);Put(Config,Meta,TEXT("Generation"),Generation);
    Put(Config,Meta,TEXT("Count"),Model.GetEntries().Num());
    for(const FFiskJournalEntry& E:Model.GetEntries())
    {
        const FString Section=TEXT("Catch.")+E.CatchId.ToString(EGuidFormats::Digits);const TCHAR* S=*Section;
        Config.SetString(S,TEXT("DayId"),*E.DayId.ToString(EGuidFormats::Digits));
        Put(Config,S,TEXT("DayNumber"),E.DayNumber);Put(Config,S,TEXT("Species"),E.Fish.Species);
        Put(Config,S,TEXT("Grams"),E.Fish.WeightGrams);Put(Config,S,TEXT("LengthMm"),E.Fish.LengthMm);Put(Config,S,TEXT("Value"),E.Fish.Value);
        Put(Config,S,TEXT("Sold"),E.bSold?1:0);Put(Config,S,TEXT("Ground"),E.Context.Ground);Put(Config,S,TEXT("Bait"),E.Context.Bait);
        Put(Config,S,TEXT("Depth"),E.Context.Depth);Put(Config,S,TEXT("Program"),E.Context.Program);Put(Config,S,TEXT("FightMs"),E.Context.FightMs);
        Put(Config,S,TEXT("LandedAtMs"),E.Context.LandedAtMs);Config.SetString(S,TEXT("LandedUtc"),*E.Context.LandedUtc);
        Put(Config,S,TEXT("BoatKind"),E.Context.BoatKind);Put(Config,S,TEXT("Equipment"),E.Context.Equipment);
        if(E.Context.bHasSeaPoint) { Put(Config,S,TEXT("HasSeaPoint"),1);Put(Config,S,TEXT("SeaX"),FMath::RoundToInt(E.Context.SeaPoint.X));Put(Config,S,TEXT("SeaY"),FMath::RoundToInt(E.Context.SeaPoint.Y)); }
    }
    Config.Dirty=true;
    return Config.Write(File) && IFileManager::Get().FileSize(*File)<=MaxBytes;
}
EFiskJournalLoad FFiskJournalStore::Load(const FString& File,FFiskJournalModel& Model)
{
    FSystemWideCriticalSection Lock(LockName(File),FTimespan::FromSeconds(1));
    if(!Lock.IsValid() || !Preservable(File)) { return EFiskJournalLoad::Unavailable; }
    FFiskJournalModel Main,Backup;int32 MainGeneration=0,BackupGeneration=0;
    const bool GoodMain=Read(File,Main,MainGeneration),GoodBackup=Read(File+TEXT(".bak"),Backup,BackupGeneration);
    if(GoodMain && (!GoodBackup || MainGeneration>=BackupGeneration)) { Model=MoveTemp(Main);return EFiskJournalLoad::Loaded; }
    if(GoodBackup) { Model=MoveTemp(Backup);return EFiskJournalLoad::Recovered; }
    if(IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")))) { return EFiskJournalLoad::Unavailable; }
    Model=FFiskJournalModel();return EFiskJournalLoad::Empty;
}
bool FFiskJournalStore::Save(const FString& File,FFiskJournalModel& Model)
{
    FSystemWideCriticalSection Lock(LockName(File),FTimespan::FromSeconds(1));
    if(!Lock.IsValid() || !Preservable(File)) { return false; }
    FFiskJournalModel Main,Backup;int32 MainGeneration=0,BackupGeneration=0;
    const bool GoodMain=Read(File,Main,MainGeneration),GoodBackup=Read(File+TEXT(".bak"),Backup,BackupGeneration);
    const bool HadMain=IFileManager::Get().FileExists(*File),HadBackup=IFileManager::Get().FileExists(*(File+TEXT(".bak")));
    if(!GoodMain && !GoodBackup && (HadMain || HadBackup)) { return false; }
    const bool UseMain=GoodMain && (!GoodBackup || MainGeneration>=BackupGeneration);
    FFiskJournalModel Combined=UseMain?Main:GoodBackup?Backup:FFiskJournalModel();
    if(!Combined.Merge(Model)) { return false; }
    const int32 Generation=FMath::Max(GoodMain?MainGeneration:0,GoodBackup?BackupGeneration:0);
    if(Generation==MAX_int32) { return false; }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    const FString Temporary=File+TEXT(".tmp"),BackupTemporary=File+TEXT(".bak.tmp");
    if(!Write(Temporary,Combined,Generation+1)) { return false; }
    // Preserve a validated previous generation; never replace a good backup with a corrupt main file.
    const FString BackupSource=UseMain?File:GoodBackup?File+TEXT(".bak"):Temporary;
    if(IFileManager::Get().Copy(*BackupTemporary,*BackupSource,true,true)!=COPY_OK
        || !IFileManager::Get().Move(*(File+TEXT(".bak")),*BackupTemporary,true,true)
        || !IFileManager::Get().Move(*File,*Temporary,true,true)) { return false; }
    Model=MoveTemp(Combined);return true;
}
