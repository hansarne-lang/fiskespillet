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
    for(const FString& Payload:TArray<FString>{TEXT("[Fiskelaget.Settings]\nVersion=2\nSound=0\nFrameRate=30\n"),TEXT("[Fiskelaget.Settings]\nVersion=3\nSound=0\nFrameRate=30\nMouseRod=1\nRodSensitivity=2\n"),
        TEXT("[Fiskelaget.Settings]\nVersion=2\nSound=0\nFrameRate=30\nMouseRod=5\nRodSensitivity=2\n"),TEXT("[Fiskelaget.Settings]\nVersion=2\nSound=0\nFrameRate=30\nMouseRod=0\nRodSensitivity=9\n"),TEXT("[Fiskelaget.Settings]\nVersion=1\nSound=7\nFrameRate=30\n"),TEXT("[Fiskelaget.Settings]\nVersion=1\nSound=0\nFrameRate=-1\n")})
    {
        FFileHelper::SaveStringToFile(Payload,*File);FFiskUserSettings Settings;
        TestFalse(TEXT("Invalid schema/data ignored"),Settings.Load(File));
        TestTrue(TEXT("Safe sound default"),Settings.bSound);TestEqual(TEXT("Safe FPS default"),Settings.FrameRate,60);
        TestTrue(TEXT("Safe mouse rod default"),Settings.bMouseRod);TestEqual(TEXT("Safe sensitivity default"),Settings.RodSensitivity,2);
    }
    IFileManager::Get().Delete(*File);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRodSettingsTest,"Fiskelaget.Session.RodControlSettings",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRodSettingsTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()+TEXT(".ini"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    FFiskUserSettings Defaults;TestTrue(TEXT("Mouse-steered rod is the default"),Defaults.bMouseRod);TestEqual(TEXT("Medium sensitivity"),Defaults.RodSensitivity,2);
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Settings]\nVersion=1\nSound=0\nFrameRate=30\n"),*File);
    FFiskUserSettings Old;TestTrue(TEXT("Version 1 file still loads"),Old.Load(File));
    TestFalse(TEXT("Version 1 sound kept"),Old.bSound);TestTrue(TEXT("Version 1 gets mouse rod"),Old.bMouseRod);TestEqual(TEXT("Version 1 gets medium"),Old.RodSensitivity,2);
    Old.bMouseRod=false;Old.RodSensitivity=3;TestTrue(TEXT("Save version 2"),Old.Save(File));
    FFiskUserSettings New;TestTrue(TEXT("Version 2 loads"),New.Load(File));
    TestFalse(TEXT("Buttons choice persisted"),New.bMouseRod);TestEqual(TEXT("Sensitivity persisted"),New.RodSensitivity,3);TestFalse(TEXT("Sound still persisted"),New.bSound);
    New.RodSensitivity=0;TestFalse(TEXT("Reject invalid sensitivity"),New.Save(File));
    FFiskUserSettings Again;Again.Load(File);TestEqual(TEXT("Rejected write preserves original"),Again.RodSensitivity,3);
    IFileManager::Get().Delete(*File);return true;
}
#endif
