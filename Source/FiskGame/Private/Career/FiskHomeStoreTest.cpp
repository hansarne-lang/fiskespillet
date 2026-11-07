#include "Career/FiskCareerStore.h"
#include "Career/FiskMarinaStore.h"
#include "FiskMarinaTypes.h"
#include "Fishing/FiskFightModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace HomeTest
{
struct FFiles { FString Dir=FPaths::ProjectSavedDir()/TEXT("Automation")/FGuid::NewGuid().ToString();FString Career=Dir/TEXT("Career.ini"),House=Dir/TEXT("House.ini");~FFiles(){IFileManager::Get().DeleteDirectory(*Dir,false,true);} };
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeMigrationTest,"Fiskelaget.Home.VersionedStoresAndMigration",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeMigrationTest::RunTest(const FString& Parameters)
{
    HomeTest::FFiles F;auto P=FFiskCareerProfile::Create(TEXT("NAUST"));FFiskMarinaIdentity V,W;FFiskCareerProfile Read;
    TestTrue(TEXT("Old profile and free house saved"),FFiskCareerStore::Save(F.Career,P) && FFiskMarinaStore::Save(F.House,P.Id,V));
    FConfigFile C;C.Read(F.Career);int32 Version=0;C.GetInt(TEXT("Fiskelaget.Career"),TEXT("Version"),Version);TestEqual(TEXT("Legacy career format kept without purchases"),Version,1);
    P.Buy(FGuid::NewGuid(),P.Revision,6);P.Buy(FGuid::NewGuid(),P.Revision,9);P.Buy(FGuid::NewGuid(),P.Revision,12);V.Lighting=V.Pier=V.Display=3;
    TestTrue(TEXT("Premium career and identity saved"),FFiskCareerStore::Save(F.Career,P) && FFiskMarinaStore::Save(F.House,P.Id,V));
    TestEqual(TEXT("Reload career"),FFiskCareerStore::Load(F.Career,Read),EFiskCareerLoad::Loaded);TestEqual(TEXT("Reload house"),FFiskMarinaStore::Load(F.House,P.Id,W),EFiskMarinaLoad::Loaded);TestTrue(TEXT("Fields and money retained"),W==V && Read.Extends(P) && W.IsOwnedBy(Read));
    for(const FString& Path:TArray<FString>{F.Career,F.Career+TEXT(".bak"),F.House}) { FConfigFile K;K.Read(Path);K.GetInt(Path.Contains(TEXT("House"))?TEXT("Fiskelaget.Marina"):TEXT("Fiskelaget.Career"),TEXT("Version"),Version);TestEqual(TEXT("Premium format2 including promoted backup"),Version,2); }return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeWriteFailureTest,"Fiskelaget.Home.PurchaseWriteFailureAndRecovery",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeWriteFailureTest::RunTest(const FString& Parameters)
{
    HomeTest::FFiles F;auto P=FFiskCareerProfile::Create(TEXT("NAUST"));TestTrue(TEXT("Initial committed"),FFiskCareerStore::Save(F.Career,P));auto Next=P;const auto Cmd=FGuid::NewGuid();Next.Buy(Cmd,1,4);
    IFileManager::Get().MakeDirectory(*(F.Career+TEXT(".tmp")));TestFalse(TEXT("Actual blocked temporary path"),FFiskCareerStore::Save(F.Career,Next));FFiskCareerProfile Read;FFiskCareerStore::Load(F.Career,Read);TestEqual(TEXT("No committed debit"),Read.Cash,P.Cash);
    IFileManager::Get().DeleteDirectory(*(F.Career+TEXT(".tmp")));TestTrue(TEXT("Retry commits"),FFiskCareerStore::Save(F.Career,Next));TestTrue(TEXT("Same command never repeats debit"),Next.Buy(Cmd,1,4));TestEqual(TEXT("One debit"),Next.Cash,2975000LL);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeFutureTest,"Fiskelaget.Home.FuturePremiumFilesPreserved",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeFutureTest::RunTest(const FString& Parameters)
{
    HomeTest::FFiles F;auto P=FFiskCareerProfile::Create(TEXT("NAUST"));P.Buy(FGuid::NewGuid(),P.Revision,4);TestTrue(TEXT("Save current premium"),FFiskCareerStore::Save(F.Career,P));
    const FString Future=TEXT("[Fiskelaget.Career]\nVersion=99\nFutureMoney=preserve\n");FFileHelper::SaveStringToFile(Future,*F.Career);FFiskCareerProfile Read;
    TestEqual(TEXT("Future primary blocks older backup rollback"),FFiskCareerStore::Load(F.Career,Read),EFiskCareerLoad::Blocked);TestFalse(TEXT("No overwrite of future career"),FFiskCareerStore::Save(F.Career,P));FString Kept;FFileHelper::LoadFileToString(Kept,*F.Career);TestEqual(TEXT("Future bytes intact"),Kept,Future);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeNoBonusTest,"Fiskelaget.Home.CosmeticsDoNotChangeFight",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeNoBonusTest::RunTest(const FString& Parameters)
{
    FFiskFightModel Base,Premium;Base.Reset(18,60,0,0);Premium.Reset(18,60,0,8176);
    for(int32 I=0;I<1200;++I) { const bool Lift=(I/20)%2==0;Base.SetRaised(Lift);Premium.SetRaised(Lift);Base.SetReeling(!Lift);Premium.SetReeling(!Lift);Base.Advance(.1);Premium.Advance(.1); }
    TestEqual(TEXT("Cosmetics do not grant gear"),Premium.GetOutcome(),Base.GetOutcome());TestEqual(TEXT("Identical line"),Premium.GetLine(),Base.GetLine());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskHomeBoundsTest,"Fiskelaget.Home.PremiumIdentityBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskHomeBoundsTest::RunTest(const FString& Parameters)
{
    auto P=FFiskCareerProfile::Create(TEXT("NAUST"));for(int32 I=4;I<13;++I) { P.Buy(FGuid::NewGuid(),P.Revision,I); }
    for(int32 L=-1;L<=4;++L) { for(int32 B=-1;B<=4;++B) { for(int32 T=-1;T<=4;++T) { FFiskMarinaIdentity V;V.Lighting=L;V.Pier=B;V.Display=T;TestEqual(TEXT("All valid combinations require owned tiers"),V.IsOwnedBy(P),L>=0 && L<=3 && B>=0 && B<=3 && T>=0 && T<=3); } } }
    return true;
}
#endif
