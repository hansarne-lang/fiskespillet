#include "UI/FiskNavigationNotebook.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace
{
FString PlacesTestFile() { return FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(),TEXT("Places.ini")); }
void CleanPlaces(const FString& File)
{
    IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*(File+TEXT(".tmp")));
    IFileManager::Get().DeleteDirectory(*FPaths::GetPath(File));
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPlaceRoundTripTest,"Fiskelaget.Navigation.PlacesPersistMergeAndDelete",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPlaceRoundTripTest::RunTest(const FString& Parameters)
{
    const FString File=PlacesTestFile(),Other=PlacesTestFile();FFiskNavigationNotebook A,B,C;
    TestTrue(TEXT("Fresh profile"),A.Load(File));
    TestTrue(TEXT("Save named ground"),A.Save(0,1,TEXT("Min skrent"),File));
    TestTrue(TEXT("Second process does not erase another slot"),B.Save(5,2,TEXT("TUR 2"),File));
    TestTrue(TEXT("Reload"),C.Load(File));
    TestNotNull(TEXT("First slot merged"),C.Get(0));TestNotNull(TEXT("Last slot saved"),C.Get(5));
    if(C.Get(0)) { TestEqual(TEXT("Name retained"),C.Get(0)->Name,FString(TEXT("Min skrent"))); }
    TestTrue(TEXT("Independent profile"),C.Load(Other));TestNull(TEXT("No shared slots"),C.Get(0));
    TestTrue(TEXT("Rename"),A.Save(0,1,TEXT("NYTT NAVN"),File));
    TestTrue(TEXT("Clear slot"),A.Clear(5,File));TestTrue(TEXT("Reload clear"),C.Load(File));
    TestNull(TEXT("Cleared"),C.Get(5));TestNotNull(TEXT("Other slot kept"),C.Get(0));
    CleanPlaces(File);CleanPlaces(Other);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPlaceValidationTest,"Fiskelaget.Navigation.PlacesValidationAndFailedWrite",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPlaceValidationTest::RunTest(const FString& Parameters)
{
    const FString File=PlacesTestFile();FFiskNavigationNotebook N;
    TestFalse(TEXT("Bad ground"),N.Save(0,3,TEXT("Test"),File));
    TestFalse(TEXT("Bad slot"),N.Save(6,0,TEXT("Test"),File));
    TestFalse(TEXT("Newline"),FFiskNavigationNotebook::ValidName(TEXT("A\nB")));
    TestFalse(TEXT("INI injection"),FFiskNavigationNotebook::ValidName(TEXT("[Place2]")));
    TestFalse(TEXT("Empty"),FFiskNavigationNotebook::ValidName(TEXT("")));
    TestFalse(TEXT("Whitespace padding"),FFiskNavigationNotebook::ValidName(TEXT(" A ")));
    TestFalse(TEXT("Too long"),FFiskNavigationNotebook::ValidName(FString::ChrN(25,'A')));
    TestTrue(TEXT("Norwegian names"),FFiskNavigationNotebook::ValidName(TEXT("Øya-2")));
    TestTrue(TEXT("Seed valid"),N.Save(0,0,TEXT("Revet"),File));
    const FString Blocked=File/TEXT("cannot-create.ini");
    TestFalse(TEXT("Parent is file"),N.Save(0,1,TEXT("Feil"),Blocked));
    TestEqual(TEXT("Failed write preserves memory"),N.Get(0)->Ground,0);
    CleanPlaces(File);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPlaceCorruptTest,"Fiskelaget.Navigation.PlacesCorruptAndFuturePreserved",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPlaceCorruptTest::RunTest(const FString& Parameters)
{
    const FString File=PlacesTestFile();IFileManager::Get().MakeDirectory(*FPaths::GetPath(File),true);
    for(const FString& Bad:TArray<FString>{TEXT("truncated"),TEXT("[Fiskelaget.Places]\nVersion=3\n"),TEXT("[Fiskelaget.Places]\nVersion=1\n[Place1]\nGround=999\nName=Bad\n")})
    {
        FFileHelper::SaveStringToFile(Bad,*File);FFiskNavigationNotebook N;
        TestFalse(TEXT("Invalid load"),N.Load(File));TestTrue(TEXT("Blocked status"),N.IsBlocked());
        TestFalse(TEXT("Invalid file not replaced"),N.Save(0,0,TEXT("Test"),File));
        FString After;FFileHelper::LoadFileToString(After,*File);TestEqual(TEXT("Bad bytes represented unchanged"),After,Bad);
    }
    CleanPlaces(File);return true;
}
#endif
