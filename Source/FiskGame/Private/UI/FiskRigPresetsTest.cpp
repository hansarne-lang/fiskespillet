#include "UI/FiskRigPresets.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPresetPersistenceTest,"Fiskelaget.Content.PresetPersistence",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPresetPersistenceTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()+TEXT(".ini"));
    FFiskRigPresets Store;FFiskRigSettings Rig;Rig.Bait=2;Rig.Depth=100;Rig.Program=2;Rig.Revision=17;
    TestTrue(TEXT("Save deep rig"),Store.Save(0,Rig,File));
    Rig.Bait=1;Rig.Depth=40;Rig.Program=1;
    TestTrue(TEXT("Save another slot"),Store.Save(2,Rig,File));
    FFiskRigPresets Reloaded;Reloaded.Load(File);
    TestTrue(TEXT("New instance loads saved rig"),Reloaded.Get(0)->bExists && Reloaded.Get(0)->Rig.Depth==100);
    TestFalse(TEXT("Empty slot remains empty"),Reloaded.Get(1)->bExists);
    TestEqual(TEXT("Session revision is not persisted"),Reloaded.Get(0)->Rig.Revision,0);
    TestEqual(TEXT("Independent slot"),Reloaded.Get(2)->Rig.Bait,1);
    TestFalse(TEXT("Deep saved rig still fails shallow-ground validation"),FFiskRigRules::IsValid(2,100,2,60));
    TestFalse(TEXT("Invalid slot rejected"),Reloaded.Save(-1,Rig,File));
    Rig.Depth=MAX_int32;TestFalse(TEXT("Invalid data rejected"),Reloaded.Save(0,Rig,File));
    FFiskRigPresets Again;Again.Load(File);TestEqual(TEXT("Rejected write preserves old file"),Again.Get(0)->Rig.Depth,100);
    IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*(File+TEXT(".tmp")));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPresetCorruptionTest,"Fiskelaget.Content.PresetCorruption",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPresetCorruptionTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()+TEXT(".ini"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.RigPreset1]\nVersion=1\nBait=999\nDepth=40\nProgram=0\n[Fiskelaget.RigPreset2]\nVersion=2\nBait=0\nDepth=30\nProgram=0\n"),*File);
    FFiskRigPresets Store;Store.Load(File);
    TestFalse(TEXT("Corrupt bait ignored"),Store.Get(0)->bExists);
    TestFalse(TEXT("Unknown version ignored"),Store.Get(1)->bExists);
    TestTrue(TEXT("Out of bounds access is safe"),Store.Get(3)==nullptr);
    IFileManager::Get().Delete(*File);Store.Load(File);
    TestFalse(TEXT("Missing file does not invent data"),Store.Get(0)->bExists);
    return true;
}
#endif
