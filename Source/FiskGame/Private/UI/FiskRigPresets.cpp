#include "UI/FiskRigPresets.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
void FFiskRigPresets::Load(const FString& File)
{
    for(FFiskSavedRig& Slot:Slots) { Slot=FFiskSavedRig(); }
    FConfigFile Config;
    if(!IFileManager::Get().FileExists(*File)) { return; }
    Config.Read(File);
    for(int32 I=0;I<Count;++I)
    {
        const FString Section=FString::Printf(TEXT("Fiskelaget.RigPreset%d"),I+1);
        int32 Version=0;FFiskRigSettings Rig;
        if(Config.GetInt(*Section,TEXT("Version"),Version) && Version==1
            && Config.GetInt(*Section,TEXT("Bait"),Rig.Bait) && Config.GetInt(*Section,TEXT("Depth"),Rig.Depth)
            && Config.GetInt(*Section,TEXT("Program"),Rig.Program) && FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,250))
        { Slots[I].bExists=true;Slots[I].Rig=Rig; }
    }
}
bool FFiskRigPresets::Save(int32 Slot,const FFiskRigSettings& Rig,const FString& File)
{
    if(Slot<0 || Slot>=Count || !FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,250)) { return false; }
    FFiskRigPresets Candidate=*this;
    Candidate.Slots[Slot].bExists=true;Candidate.Slots[Slot].Rig=Rig;Candidate.Slots[Slot].Rig.Revision=0;
    FConfigFile Config;
    for(int32 I=0;I<Count;++I)
    {
        if(!Candidate.Slots[I].bExists) { continue; }
        const FString Section=FString::Printf(TEXT("Fiskelaget.RigPreset%d"),I+1);
        const FFiskRigSettings& Saved=Candidate.Slots[I].Rig;
        Config.SetString(*Section,TEXT("Version"),TEXT("1"));Config.SetString(*Section,TEXT("Bait"),*FString::FromInt(Saved.Bait));
        Config.SetString(*Section,TEXT("Depth"),*FString::FromInt(Saved.Depth));Config.SetString(*Section,TEXT("Program"),*FString::FromInt(Saved.Program));
    }
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    const FString Temporary=File+TEXT(".tmp");Config.Dirty=true;
    if(!Config.Write(Temporary) || !IFileManager::Get().Move(*File,*Temporary,true,true)) { return false; }
    *this=Candidate;return true;
}
