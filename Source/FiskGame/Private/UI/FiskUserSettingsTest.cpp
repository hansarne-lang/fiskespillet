#include "UI/FiskUserSettings.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSettingsPersistenceTest,"Fiskelaget.Session.SettingsPersistenceAndIsolation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSettingsPersistenceTest::RunTest(const FString& Parameters)
{
    const FString Root=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString());
    const FString File=FPaths::Combine(Root,TEXT("a.ini")),Other=FPaths::Combine(Root,TEXT("b.ini"));
    FFiskUserSettings First;First.bSound=false;First.FrameRate=30;
    TestTrue(TEXT("Save settings"),First.Save(File));
    FFiskUserSettings Again;TestTrue(TEXT("New object loads settings"),Again.Load(File));
    TestFalse(TEXT("Mute persisted"),Again.bSound);TestEqual(TEXT("FPS persisted"),Again.FrameRate,30);
    FFiskUserSettings Separate;TestFalse(TEXT("Different profile missing"),Separate.Load(Other));
    TestTrue(TEXT("Missing uses sound default"),Separate.bSound);TestEqual(TEXT("Default FPS"),Separate.FrameRate,60);
    TestTrue(TEXT("Independent profile"),Separate.Save(Other));
    Again.Load(File);TestFalse(TEXT("Other save cannot overwrite original"),Again.bSound);
    First.FrameRate=999;TestFalse(TEXT("Reject invalid write"),First.Save(File));
    Again.Load(File);TestEqual(TEXT("Rejected write preserves original"),Again.FrameRate,30);
    IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*Other);IFileManager::Get().DeleteDirectory(*Root);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSettingsInvalidTest,"Fiskelaget.Session.InvalidSettings",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSettingsInvalidTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()+TEXT(".ini"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    for(const FString& Payload:TArray<FString>{TEXT("[Fiskelaget.Settings]\nVersion=2\nSound=0\nFrameRate=30\n"),TEXT("[Fiskelaget.Settings]\nVersion=1\nSound=7\nFrameRate=30\n"),TEXT("[Fiskelaget.Settings]\nVersion=1\nSound=0\nFrameRate=-1\n")})
    {
        FFileHelper::SaveStringToFile(Payload,*File);FFiskUserSettings Settings;
        TestFalse(TEXT("Invalid schema/data ignored"),Settings.Load(File));
        TestTrue(TEXT("Safe sound default"),Settings.bSound);TestEqual(TEXT("Safe FPS default"),Settings.FrameRate,60);
    }
    IFileManager::Get().Delete(*File);return true;
}
#endif
