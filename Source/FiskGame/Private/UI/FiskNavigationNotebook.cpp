#include "UI/FiskNavigationNotebook.h"
#include "FiskFishingGround.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"

bool FFiskNavigationNotebook::ValidName(const FString& Name)
{
    if(Name.IsEmpty() || Name.Len()>NameLimit || Name!=Name.TrimStartAndEnd()) { return false; }
    for(TCHAR C:Name) { if(!FChar::IsAlnum(C) && C!=' ' && C!='-' && C!='_') { return false; } }
    return true;
}
const FFiskSavedPlace* FFiskNavigationNotebook::Get(int32 Slot) const
{ return Slot>=0 && Slot<Count && Slots[Slot].Ground!=INDEX_NONE?&Slots[Slot]:nullptr; }
bool FFiskNavigationNotebook::Read(const FString& File,FFiskNavigationNotebook& Result)
{
    if(IFileManager::Get().FileSize(*File)<0 || IFileManager::Get().FileSize(*File)>16384) { return false; }
    FConfigFile Config;Config.Read(File);FString Version;
    if(!Config.GetString(TEXT("Fiskelaget.Places"),TEXT("Version"),Version) || (Version!=TEXT("1") && Version!=TEXT("2"))) { return false; }
    FFiskNavigationNotebook Candidate;
    for(int32 I=0;I<Count;++I)
    {
        const FString Section=FString::Printf(TEXT("Place%d"),I+1);
        if(!Config.Contains(Section)) { continue; }
        FString Ground,Name;
        if(!Config.GetString(*Section,TEXT("Ground"),Ground) || (Ground!=TEXT("0") && Ground!=TEXT("1") && Ground!=TEXT("2"))
            || !Config.GetString(*Section,TEXT("Name"),Name) || !ValidName(Name)) { return false; }
        Candidate.Slots[I]={FCString::Atoi(*Ground),Name};
        Candidate.Slots[I].Point=FiskGrounds::Find(Candidate.Slots[I].Ground)->Position;
        if(Version==TEXT("2"))
        {
            FString X,Y;int32 PX=0,PY=0,Habitat=0,Duration=0;
            if(!Config.GetString(*Section,TEXT("X"),X) || !Config.GetString(*Section,TEXT("Y"),Y)
                || X.Len()>8 || Y.Len()>8 || !LexTryParseString(PX,*X) || !LexTryParseString(PY,*Y)
                || FString::FromInt(PX)!=X || FString::FromInt(PY)!=Y) { return false; }
            Candidate.Slots[I].Point=FVector(PX,PY,0);
            if(!FiskGrounds::MakeSeaPoint(Candidate.Slots[I].Point,Habitat,Duration) || Habitat!=Candidate.Slots[I].Ground) { return false; }
        }
    }
    for(const auto& Pair:Config)
    {
        if(Pair.Key==TEXT("Fiskelaget.Places")) { continue; }
        bool Known=false;for(int32 I=0;I<Count;++I) { Known|=Pair.Key==FString::Printf(TEXT("Place%d"),I+1); }
        if(!Known) { return false; }
    }
    Result=Candidate;return true;
}
bool FFiskNavigationNotebook::Load(const FString& File)
{
    *this=FFiskNavigationNotebook();
    if(!IFileManager::Get().FileExists(*File)) { return true; }
    FFiskNavigationNotebook Candidate;
    if(!Read(File,Candidate)) { bBlocked=true;return false; }
    *this=Candidate;return true;
}
bool FFiskNavigationNotebook::Commit(int32 Slot,const FFiskSavedPlace& Place,const FString& File)
{
    if(Slot<0 || Slot>=Count) { return false; }
    FString Canonical=FPaths::ConvertRelativePathToFull(File);FPaths::NormalizeFilename(Canonical);
    FSystemWideCriticalSection Lock(FString::Printf(TEXT("FiskelagetPlaces-%08X"),GetTypeHash(Canonical.ToLower())),FTimespan::FromSeconds(1));
    if(!Lock.IsValid()) { return false; }
    // Re-read under lock so another process's other slots are preserved.
    FFiskNavigationNotebook Candidate;
    if(IFileManager::Get().FileExists(*File) && !Read(File,Candidate)) { bBlocked=true;return false; }
    Candidate.Slots[Slot]=Place;
    FConfigFile Config;Config.SetString(TEXT("Fiskelaget.Places"),TEXT("Version"),TEXT("2"));
    for(int32 I=0;I<Count;++I)
    {
        if(!Candidate.Get(I)) { continue; }
        const FString Section=FString::Printf(TEXT("Place%d"),I+1);
        Config.SetString(*Section,TEXT("Ground"),*FString::FromInt(Candidate.Slots[I].Ground));
        Config.SetString(*Section,TEXT("Name"),*Candidate.Slots[I].Name);
        Config.SetString(*Section,TEXT("X"),*FString::FromInt(FMath::RoundToInt(Candidate.Slots[I].Point.X)));
        Config.SetString(*Section,TEXT("Y"),*FString::FromInt(FMath::RoundToInt(Candidate.Slots[I].Point.Y)));
    }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    const FString Temporary=File+TEXT(".tmp");Config.Dirty=true;
    if(!Config.Write(Temporary) || !IFileManager::Get().Move(*File,*Temporary,true,true)) { return false; }
    *this=Candidate;return true;
}
bool FFiskNavigationNotebook::Save(int32 Slot,int32 Ground,const FString& Name,const FString& File)
{ return FiskGrounds::Find(Ground) && SavePoint(Slot,FiskGrounds::Find(Ground)->Position,Name,File); }
bool FFiskNavigationNotebook::SavePoint(int32 Slot,FVector Point,const FString& Name,const FString& File)
{
    int32 Habitat=0,Duration=0;if(Point.ContainsNaN()) { return false; }
    Point=FVector(FMath::RoundToDouble(Point.X),FMath::RoundToDouble(Point.Y),Point.Z);
    return ValidName(Name) && FiskGrounds::MakeSeaPoint(Point,Habitat,Duration) && Commit(Slot,{Habitat,Name,Point},File);
}
bool FFiskNavigationNotebook::Clear(int32 Slot,const FString& File)
{ return Commit(Slot,FFiskSavedPlace(),File); }
