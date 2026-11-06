#include "UI/FiskRigPresets.h"
#include "FiskCareerProfile.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
namespace
{
bool Future(const FString& File)
{
    const int64 Size=IFileManager::Get().FileSize(*File);if(Size>16384) { return true; }if(Size<0) { return false; }
    FConfigFile C;C.Read(File);for(int32 I=1;I<=3;++I) { int32 V=0;const FString S=FString::Printf(TEXT("Fiskelaget.RigPreset%d"),I);if(C.GetInt(*S,TEXT("Version"),V) && V>1) { return true; } }return false;
}
}
bool FFiskRigPresets::Read(const FString& File,FFiskRigPresets& Out)
{
    const int64 Size=IFileManager::Get().FileSize(*File);if(Size<0 || Size>16384) { return false; }FConfigFile C;C.Read(File);
    if(C.Num()>Count || (C.IsEmpty() && Size>0)) { return false; }FFiskRigPresets Candidate;
    for(int32 I=0;I<Count;++I)
    {
        const FString S=FString::Printf(TEXT("Fiskelaget.RigPreset%d"),I+1);auto& Slot=Candidate.Slots[I];Slot.Name=FString::Printf(TEXT("OPPSETT %d"),I+1);
        if(!C.Contains(S)) { continue; }int32 Version=0;auto& R=Slot.Rig;
        if(!C.GetInt(*S,TEXT("Version"),Version) || Version!=1 || !C.GetInt(*S,TEXT("Bait"),R.Bait) || !C.GetInt(*S,TEXT("Depth"),R.Depth)
            || !C.GetInt(*S,TEXT("Program"),R.Program) || !FFiskRigRules::IsValid(R.Bait,R.Depth,R.Program,250)) { return false; }
        FString Name;if(C.GetString(*S,TEXT("Name"),Name)) { if(!FFiskCareerProfile::ValidName(Name)) { return false; }Slot.Name=Name; }Slot.bExists=true;
    }
    int32 Used=0;for(const auto& S:Candidate.Slots) { Used+=S.bExists?1:0; }if(Used!=C.Num()) { return false; }Out=MoveTemp(Candidate);return true;
}
void FFiskRigPresets::Load(const FString& File)
{
    FFiskRigPresets Candidate;
    for(int32 I=0;I<Count;++I) { Slots[I]=FFiskSavedRig();Slots[I].Name=FString::Printf(TEXT("OPPSETT %d"),I+1); }
    bBlocked=Future(File) || Future(File+TEXT(".bak"));if(bBlocked) { return; }
    if(Read(File,Candidate) || Read(File+TEXT(".bak"),Candidate)) { *this=MoveTemp(Candidate);return; }
    bBlocked=IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak")));
}
bool FFiskRigPresets::Write(const FString& File)
{
    if(bBlocked || Future(File) || Future(File+TEXT(".bak"))) { return false; }
    FConfigFile C;for(int32 I=0;I<Count;++I)
    {
        const auto& S=Slots[I];if(!S.bExists) { continue; }const FString Key=FString::Printf(TEXT("Fiskelaget.RigPreset%d"),I+1);
        C.SetString(*Key,TEXT("Version"),TEXT("1"));C.SetString(*Key,TEXT("Bait"),*FString::FromInt(S.Rig.Bait));
        C.SetString(*Key,TEXT("Depth"),*FString::FromInt(S.Rig.Depth));C.SetString(*Key,TEXT("Program"),*FString::FromInt(S.Rig.Program));C.SetString(*Key,TEXT("Name"),*S.Name);
    }
    FFiskRigPresets Old;const bool Main=Read(File,Old),Backup=Read(File+TEXT(".bak"),Old);
    if(!Main && !Backup && (IFileManager::Get().FileExists(*File) || IFileManager::Get().FileExists(*(File+TEXT(".bak"))))) { return false; }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);const FString Temp=File+TEXT(".tmp"),Back=File+TEXT(".bak.tmp");C.Dirty=true;
    if(!C.Write(Temp)) { return false; }const FString Previous=Main?File:Backup?File+TEXT(".bak"):Temp;
    return IFileManager::Get().Copy(*Back,*Previous,true,true)==COPY_OK && IFileManager::Get().Move(*(File+TEXT(".bak")),*Back,true,true) && IFileManager::Get().Move(*File,*Temp,true,true);
}
bool FFiskRigPresets::Save(int32 Slot,const FFiskRigSettings& Rig,const FString& File)
{
    if(Slot<0 || Slot>=Count || !FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,250)) { return false; }
    FSystemWideCriticalSection Lock(FString::Printf(TEXT("FiskelagetRig-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(File).ToLower())),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return false; }
    FFiskRigPresets Candidate=*this;Candidate.Slots[Slot].bExists=true;Candidate.Slots[Slot].Rig=Rig;Candidate.Slots[Slot].Rig.Revision=0;
    if(Candidate.Slots[Slot].Name.IsEmpty()) { Candidate.Slots[Slot].Name=FString::Printf(TEXT("OPPSETT %d"),Slot+1); }
    if(!Candidate.Write(File)) { return false; }*this=MoveTemp(Candidate);return true;
}
bool FFiskRigPresets::Rename(int32 Slot,const FString& Name,const FString& File)
{
    if(Slot<0 || Slot>=Count || !Slots[Slot].bExists || !FFiskCareerProfile::ValidName(Name)) { return false; }
    FSystemWideCriticalSection Lock(FString::Printf(TEXT("FiskelagetRig-%08X"),GetTypeHash(FPaths::ConvertRelativePathToFull(File).ToLower())),FTimespan::FromSeconds(1));if(!Lock.IsValid()) { return false; }
    FFiskRigPresets Candidate=*this;Candidate.Slots[Slot].Name=Name;if(!Candidate.Write(File)) { return false; }*this=MoveTemp(Candidate);return true;
}
