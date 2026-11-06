#include "Career/FiskMarinaStore.h"
#include "Fishing/FiskCatchCatalog.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMarinaIdentityTest,"Fiskelaget.Marina.IdentityValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMarinaIdentityTest::RunTest(const FString& Parameters)
{
    FFiskMarinaIdentity V;TestTrue(TEXT("Default valid"),V.IsValid());V.DisplayName=TEXT("HIBPR KAPTEIN");V.HouseName=TEXT("PALMEHYTTA");TestTrue(TEXT("Names accept shortcut letters"),V.IsValid());
    V.DisplayName=TEXT("../other");TestFalse(TEXT("Path/control characters rejected"),V.IsValid());V.DisplayName=TEXT("VERT");V.Color=6;TestFalse(TEXT("Palette bound"),V.IsValid());V.Color=0;V.Style=-1;TestFalse(TEXT("Style bound"),V.IsValid());V.Style=0;V.Decor=3;TestFalse(TEXT("Decor bound"),V.IsValid());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMarinaStorageTest,"Fiskelaget.Marina.AtomicPersistenceAndRecovery",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMarinaStorageTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()),File=Dir/TEXT("Marina.ini");const FGuid Profile=FGuid::NewGuid();FFiskMarinaIdentity V,Read;
    TestEqual(TEXT("No profile yet"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Empty);TestTrue(TEXT("Initial persist"),FFiskMarinaStore::Save(File,Profile,V));V.HouseName=TEXT("PALMEHAVN");V.Color=4;V.Style=2;V.Decor=1;
    TestTrue(TEXT("Cosmetic change"),FFiskMarinaStore::Save(File,Profile,V));TestEqual(TEXT("Fresh read"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Loaded);TestTrue(TEXT("Exact fields retained"),Read==V);
    TestEqual(TEXT("Career binding rejects other profile"),FFiskMarinaStore::Load(File,FGuid::NewGuid(),Read),EFiskMarinaLoad::Blocked);
    TestFalse(TEXT("Other career cannot overwrite"),FFiskMarinaStore::Save(File,FGuid::NewGuid(),V));
    FFileHelper::SaveStringToFile(TEXT("bad-primary"),*File);TestEqual(TEXT("Backup recovery"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Recovered);TestTrue(TEXT("Recovered value can repair"),FFiskMarinaStore::Save(File,Profile,Read));
    TestEqual(TEXT("Repaired primary readable"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Loaded);
    for(const TCHAR* S:{TEXT(""),TEXT(".bak"),TEXT(".tmp"),TEXT(".bak.tmp")}) { IFileManager::Get().Delete(*(File+S)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMarinaCorruptTest,"Fiskelaget.Marina.PreserveInvalidAndFutureData",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMarinaCorruptTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()),File=Dir/TEXT("Marina.ini");IFileManager::Get().MakeDirectory(*Dir,true);const FGuid Profile=FGuid::NewGuid();FFiskMarinaIdentity V;
    FFileHelper::SaveStringToFile(TEXT("primary-preserve"),*File);FFileHelper::SaveStringToFile(TEXT("backup-preserve"),*(File+TEXT(".bak")));
    TestEqual(TEXT("Two invalid files blocked"),FFiskMarinaStore::Load(File,Profile,V),EFiskMarinaLoad::Blocked);TestFalse(TEXT("No overwrite"),FFiskMarinaStore::Save(File,Profile,V));FString Text;FFileHelper::LoadFileToString(Text,*File);TestEqual(TEXT("Primary untouched"),Text,FString(TEXT("primary-preserve")));
    IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*(File+TEXT(".bak")));TestTrue(TEXT("Valid profile before future backup"),FFiskMarinaStore::Save(File,Profile,V));FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Marina]\nVersion=99\n"),*(File+TEXT(".bak")));
    TestEqual(TEXT("Future backup does not silently roll back"),FFiskMarinaStore::Load(File,Profile,V),EFiskMarinaLoad::Blocked);TestFalse(TEXT("Future data never overwritten"),FFiskMarinaStore::Save(File,Profile,V));
    for(const TCHAR* S:{TEXT(""),TEXT(".bak"),TEXT(".tmp"),TEXT(".bak.tmp")}) { IFileManager::Get().Delete(*(File+S)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMarinaRevisionRecoveryTest,"Fiskelaget.Marina.ValidatedRevisionRecovery",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMarinaRevisionRecoveryTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString()),File=Dir/TEXT("Marina.ini");
    const FGuid Profile=FGuid::NewGuid();FFiskMarinaIdentity Original,Changed,Read;Changed.HouseName=TEXT("NYTT NAUST");
    auto Cleanup=[&](){for(const TCHAR* S:{TEXT(""),TEXT(".bak"),TEXT(".tmp"),TEXT(".bak.tmp")}) { IFileManager::Get().Delete(*(File+S)); }};
    for(const TCHAR* BadRevision:{TEXT("2147483647"),TEXT("2147483646"),TEXT("-1"),TEXT("9999999999999999")})
    {
        Cleanup();TestTrue(TEXT("Fresh baseline"),FFiskMarinaStore::Save(File,Profile,Original));TestTrue(TEXT("Second generation"),FFiskMarinaStore::Save(File,Profile,Changed));
        FConfigFile Corrupt;Corrupt.Read(File);Corrupt.SetString(TEXT("Fiskelaget.Marina"),TEXT("Revision"),BadRevision);Corrupt.SetString(TEXT("Fiskelaget.Marina"),TEXT("Color"),TEXT("99"));Corrupt.Dirty=true;TestTrue(TEXT("Write corrupt primary"),Corrupt.Write(File));
        TestEqual(TEXT("Recover known-good backup"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Recovered);TestTrue(TEXT("Backup fields retained"),Read==Original);
        TestTrue(TEXT("Invalid primary revision cannot prevent repair"),FFiskMarinaStore::Save(File,Profile,Read));
        TestEqual(TEXT("Repaired primary validates"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Loaded);
        FConfigFile Repaired;Repaired.Read(File);int32 Revision=0;Repaired.GetInt(TEXT("Fiskelaget.Marina"),TEXT("Revision"),Revision);TestEqual(TEXT("Revision derives only from valid backup"),Revision,2);
        Corrupt.Dirty=true;TestTrue(TEXT("Write corrupt backup"),Corrupt.Write(File+TEXT(".bak")));
        TestTrue(TEXT("Invalid backup revision cannot prevent update"),FFiskMarinaStore::Save(File,Profile,Changed));
        TestEqual(TEXT("Updated primary validates"),FFiskMarinaStore::Load(File,Profile,Read),EFiskMarinaLoad::Loaded);TestTrue(TEXT("Latest valid values retained"),Read==Changed);
    }
    Cleanup();IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskSevenSpeciesTest,"Fiskelaget.Content.SevenSpeciesCoverage",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskSevenSpeciesTest::RunTest(const FString& Parameters)
{
    TSet<int32> Seen;for(int32 Ground=0;Ground<3;++Ground) { for(int32 Seed=0;Seed<10000;++Seed) { const FFiskCatchData Fish=FiskCatches::Roll(Seed,Ground);if(!FiskCatches::IsValid(Fish)) { AddError(TEXT("Invalid generated fish"));return false; }Seen.Add(Fish.Species); } }
    TestEqual(TEXT("All seven species generated"),Seen.Num(),7);TestEqual(TEXT("Legacy snapper price retained"),FiskCatches::Find(0)->PricePerKg,5);TestEqual(TEXT("Legacy tuna price retained"),FiskCatches::Find(2)->PricePerKg,12);TestNull(TEXT("Unknown species"),FiskCatches::Find(7));return true;
}
#endif
