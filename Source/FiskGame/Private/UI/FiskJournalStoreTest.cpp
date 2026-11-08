#include "UI/FiskJournalStore.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace
{
FString NewFile()
{ return FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(),TEXT("Journal.ini")); }
FFiskJournalModel Sample()
{
    FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;E.bSold=true;
    E.Context.LandedAtMs=200000;E.Context.FightMs=50000;E.Context.LandedUtc=FDateTime(2026,11,5,11,0,0).ToIso8601();
    FFiskJournalModel M;M.Restore({E});return M;
}
void Clean(const FString& File)
{
    for(const FString& Suffix:TArray<FString>{TEXT(""),TEXT(".bak"),TEXT(".tmp"),TEXT(".bak.tmp")})
    { IFileManager::Get().Delete(*(File+Suffix)); }
    IFileManager::Get().DeleteDirectory(*FPaths::GetPath(File));
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalDiskTest,"Fiskelaget.Journal.DiskRoundTripAndIsolation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalDiskTest::RunTest(const FString& Parameters)
{
    const FString File=NewFile(),Other=NewFile();FFiskJournalStore Store;FFiskJournalModel Original=Sample(),Loaded;
    TestTrue(TEXT("Save valid personal journal"),Store.Save(File,Original));
    FFiskJournalStore NewProcessStore;TestTrue(TEXT("Reload journal"),NewProcessStore.Load(File,Loaded)==EFiskJournalLoad::Loaded);
    TestEqual(TEXT("Single record"),Loaded.GetEntries().Num(),1);
    TestTrue(TEXT("Full record unchanged"),Loaded.GetEntries()[0]==Original.GetEntries()[0]);
    TestTrue(TEXT("New file starts separate"),NewProcessStore.Load(Other,Loaded)==EFiskJournalLoad::Empty);
    TestTrue(TEXT("No foreign records"),Loaded.GetEntries().IsEmpty());
    TestTrue(TEXT("Backup created on first save"),IFileManager::Get().FileExists(*(File+TEXT(".bak"))));
    Clean(File);Clean(Other);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalBackupTest,"Fiskelaget.Journal.BackupRecoveryAndRepair",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalBackupTest::RunTest(const FString& Parameters)
{
    const FString File=NewFile();FFiskJournalStore Store;FFiskJournalModel Original=Sample(),Loaded;
    TestTrue(TEXT("Initial save"),Store.Save(File,Original));
    FFileHelper::SaveStringToFile(TEXT("truncated"),*File);
    TestTrue(TEXT("Corrupt main recovers backup"),Store.Load(File,Loaded)==EFiskJournalLoad::Recovered);
    TestTrue(TEXT("Recovered data correct"),Loaded.GetEntries()[0]==Original.GetEntries()[0]);
    TestTrue(TEXT("Repair main without copying corrupt content over backup"),Store.Save(File,Loaded));
    TestTrue(TEXT("Repaired primary loads normally"),Store.Load(File,Loaded)==EFiskJournalLoad::Loaded);
    IFileManager::Get().Delete(*File);
    TestTrue(TEXT("Missing main also recovers backup"),Store.Load(File,Loaded)==EFiskJournalLoad::Recovered);
    TestEqual(TEXT("No new duplicate after recovery"),Loaded.GetEntries().Num(),1);
    Clean(File);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalCorruptTest,"Fiskelaget.Journal.CorruptFilesAndWriteFailure",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalCorruptTest::RunTest(const FString& Parameters)
{
    const FString File=NewFile();FFiskJournalStore Store;FFiskJournalModel Memory=Sample();Store.Save(File,Memory);
    FFileHelper::SaveStringToFile(TEXT("bad-main"),*File);FFileHelper::SaveStringToFile(TEXT("bad-backup"),*(File+TEXT(".bak")));
    TestTrue(TEXT("Both bad reported unavailable"),Store.Load(File,Memory)==EFiskJournalLoad::Unavailable);
    TestEqual(TEXT("Memory is not silently cleared"),Memory.GetEntries().Num(),1);
    TestFalse(TEXT("Corrupt files not overwritten by new data"),Store.Save(File,Memory));
    FString Main,Backup;FFileHelper::LoadFileToString(Main,*File);FFileHelper::LoadFileToString(Backup,*(File+TEXT(".bak")));
    TestEqual(TEXT("Bad main retained for manual recovery"),Main,FString(TEXT("bad-main")));
    TestEqual(TEXT("Bad backup retained"),Backup,FString(TEXT("bad-backup")));
    const FString Blocker=FPaths::Combine(FPaths::GetPath(File),TEXT("not-a-directory"));
    FFileHelper::SaveStringToFile(TEXT("preserve-me"),*Blocker);
    TestFalse(TEXT("Failed write is not reported saved"),Store.Save(FPaths::Combine(Blocker,TEXT("Journal.ini")),Memory));
    FFileHelper::LoadFileToString(Main,*Blocker);TestEqual(TEXT("Unrelated blocker retained"),Main,FString(TEXT("preserve-me")));
    IFileManager::Get().Delete(*Blocker);Clean(File);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalMergeTest,"Fiskelaget.Journal.StaleWritersAndSchemaBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalMergeTest::RunTest(const FString& Parameters)
{
    const FString File=NewFile();FFiskJournalStore A,B;FFiskJournalModel One=Sample(),Two=Sample(),Loaded;
    TestTrue(TEXT("First writer"),A.Save(File,One));
    TestTrue(TEXT("Stale writer merges confirmed records from disk"),B.Save(File,Two));
    TestEqual(TEXT("Both catches survive"),Two.GetEntries().Num(),2);
    TestTrue(TEXT("Older writer retry does not remove newer catch"),A.Save(File,One));
    A.Load(File,Loaded);TestEqual(TEXT("No duplicates or lost concurrent data"),Loaded.GetEntries().Num(),2);
    FString Text;FFileHelper::LoadFileToString(Text,*File);Text.ReplaceInline(TEXT("Version=1"),TEXT("Version=99"));
    FFileHelper::SaveStringToFile(Text,*File);IFileManager::Get().Delete(*(File+TEXT(".bak")));
    TestTrue(TEXT("Unknown schema rejected"),A.Load(File,Loaded)==EFiskJournalLoad::Unavailable);
    TestFalse(TEXT("Unknown schema preserved"),A.Save(File,Loaded));
    FFileHelper::SaveStringToFile(FString::ChrN(FFiskJournalStore::MaxBytes+1,TEXT('A')),*File,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    TestTrue(TEXT("Oversized input rejected before parsing"),A.Load(File,Loaded)==EFiskJournalLoad::Unavailable);
    TestFalse(TEXT("Oversized original not silently replaced"),A.Save(File,Loaded));
    Clean(File);return true;
}
#endif
