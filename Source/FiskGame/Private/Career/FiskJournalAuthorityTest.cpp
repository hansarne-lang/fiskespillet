#include "Career/FiskJournalAuthority.h"
#include "Career/FiskCareerStore.h"
#include "Economy/FiskDaySettlement.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace AuthorityTest
{
struct FFixture
{
    FString Directory=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits));
    FFiskCareerProfile A=FFiskCareerProfile::Create(TEXT("VERT")),B=FFiskCareerProfile::Create(TEXT("GJEST"));
    FFiskSettlementView View;TMap<int32,FGuid> Profiles;
    FFixture()
    {
        FFiskCareerStore::Save(Career(A.Id),A);FFiskCareerStore::Save(Career(B.Id),B);Profiles.Add(256,A.Id);Profiles.Add(257,B.Id);
        FFiskDayLedger Ledger;FFiskCatchContext Context;Context.FightMs=30000;Context.LandedAtMs=200000;Context.LandedUtc=FDateTime(2026,11,6,12,0,0).ToIso8601();
        Ledger.Record(FGuid::NewGuid(),256,200000,FiskCatches::Roll(42,0),Context);Ledger.Record(FGuid::NewGuid(),257,200000,FiskCatches::Roll(84,1),Context);Ledger.Settle(1020000,300000);
        View.DayId=FGuid::NewGuid();View.DayNumber=1;View.bRevealed=true;View.Results=Ledger.GetResults();
    }
    FString Career(const FGuid& Id) const { return FPaths::Combine(Directory,Id.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
    ~FFixture()
    {
        TArray<FString> Files;IFileManager::Get().FindFiles(Files,*(Directory/TEXT("*")),true,false);
        for(const FString& File:Files) { IFileManager::Get().Delete(*(Directory/File)); }IFileManager::Get().DeleteDirectory(*Directory);
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskArchivePrivacyTest,"Fiskelaget.Recovery.ConfirmedOnlyAfterDeadline",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskArchivePrivacyTest::RunTest(const FString& Parameters)
{
    AuthorityTest::FFixture F;F.View.bRevealed=false;TestFalse(TEXT("Unrevealed result never archived"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));
    F.View.bRevealed=true;TestFalse(TEXT("Deadline is mandatory"),FiskJournalAuthority::Commit(F.Directory,F.View,1019999,F.Profiles));
    FFiskJournalModel Read;TestTrue(TEXT("Missing archive is empty"),FiskJournalAuthority::Load(F.Directory,F.A.Id,Read));TestEqual(TEXT("No leaked fish"),Read.GetEntries().Num(),0);
    F.Profiles.Remove(256);TestFalse(TEXT("Unbound catch owner is rejected"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskOfflineArchiveTest,"Fiskelaget.Recovery.OfflineProfilesAndExactlyOnceIncome",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskOfflineArchiveTest::RunTest(const FString& Parameters)
{
    AuthorityTest::FFixture F;TestTrue(TEXT("No connected controller is needed at settlement"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));
    for(int32 Player:{256,257})
    {
        const FGuid Profile=F.Profiles[Player];FFiskJournalModel Journal;TestTrue(TEXT("Own archive readable"),FiskJournalAuthority::Load(F.Directory,Profile,Journal));TestEqual(TEXT("Exactly one own fish"),Journal.GetEntries().Num(),1);
        const FFiskDayResult& Row=*F.View.Results.FindByPredicate([&](const FFiskDayResult& R){return R.PlayerId==Player;});
        TestEqual(TEXT("Own catch only"),Journal.GetEntries()[0].CatchId,Row.Catches[0].CatchId);
        FFiskCareerProfile Career;TestEqual(TEXT("Authority cash durable"),FFiskCareerStore::Load(F.Career(Profile),Career),EFiskCareerLoad::Loaded);
        TestEqual(TEXT("Only own income credited"),Career.Cash,FFiskCareerProfile::InitialCash+static_cast<int64>(Row.SessionBalance)*100);TestEqual(TEXT("One sale receipt"),Career.Revision,2);
    }
    TestTrue(TEXT("Repeated commit is safe"),FiskJournalAuthority::Commit(F.Directory,F.View,1100000,F.Profiles));FFiskCareerProfile Again;FFiskCareerStore::Load(F.Career(F.A.Id),Again);TestEqual(TEXT("No duplicate receipt"),Again.Revision,2);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRecoveryRestartTest,"Fiskelaget.Recovery.RestartAndLostAcknowledgement",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRecoveryRestartTest::RunTest(const FString& Parameters)
{
    AuthorityTest::FFixture F;TestTrue(TEXT("Durable confirmed archive"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));
    FFiskJournalModel Server,Client;TestTrue(TEXT("New process loads archive"),FiskJournalAuthority::Load(F.Directory,F.B.Id,Server));
    TestTrue(TEXT("Client merges fetched batch"),Client.Merge(Server));TestTrue(TEXT("Lost ack resends same batch"),Client.Merge(Server));TestEqual(TEXT("One catch after replay"),Client.GetEntries().Num(),1);
    FFiskCareerProfile Stale=F.B;TestTrue(TEXT("Recovery from archive can rebuild missing income"),FiskJournalAuthority::RecoverIncome(F.Directory,Stale));TestEqual(TEXT("Only one sale"),Stale.Revision,2);
    TestTrue(TEXT("Second income recovery is idempotent"),FiskJournalAuthority::RecoverIncome(F.Directory,Stale));TestEqual(TEXT("No extra sale"),Stale.Revision,2);
    FFiskJournalModel Foreign;TestTrue(TEXT("Different profile has a different archive path"),FiskJournalAuthority::Load(F.Directory,FGuid::NewGuid(),Foreign));TestTrue(TEXT("No foreign catches"),Foreign.GetEntries().IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskArchiveDamageTest,"Fiskelaget.Recovery.PreserveCorruptAndFutureArchives",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskArchiveDamageTest::RunTest(const FString& Parameters)
{
    AuthorityTest::FFixture F;TestTrue(TEXT("Initial commit"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));const FString Path=FiskJournalAuthority::File(F.Directory,F.A.Id);FFiskJournalModel Read;
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Journal]\nVersion=99\n"),*Path);TestFalse(TEXT("Future primary is not replaced by old backup"),FiskJournalAuthority::Load(F.Directory,F.A.Id,Read));TestFalse(TEXT("Future data blocks overwrite"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));
    FFileHelper::SaveStringToFile(TEXT("bad-main"),*Path);FFileHelper::SaveStringToFile(TEXT("bad-backup"),*(Path+TEXT(".bak")));TestFalse(TEXT("Two damaged files preserved"),FiskJournalAuthority::Commit(F.Directory,F.View,1020000,F.Profiles));FString Text;FFileHelper::LoadFileToString(Text,*Path);TestEqual(TEXT("Original damaged bytes retained"),Text,FString(TEXT("bad-main")));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskArchiveNoRetroTest,"Fiskelaget.Recovery.NoRetroactiveClientJournalPayments",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskArchiveNoRetroTest::RunTest(const FString& Parameters)
{
    AuthorityTest::FFixture F;FFiskCareerProfile Legacy=F.A;TestTrue(TEXT("An old career without a v09 server archive is valid"),FiskJournalAuthority::RecoverIncome(F.Directory,Legacy));TestEqual(TEXT("No extra start grant or retroactive money"),Legacy.Cash,F.A.Cash);TestEqual(TEXT("Receipt chain unchanged"),Legacy.Revision,1);
    FFiskJournalModel Personal;Personal.Apply(F.View.DayId,1,1020000,true,F.View.Results[0]);TestTrue(TEXT("Local personal journal can exist"),!Personal.GetEntries().IsEmpty());
    TestTrue(TEXT("Client-only data never enters authority recovery"),FiskJournalAuthority::RecoverIncome(F.Directory,Legacy));TestEqual(TEXT("Still no retroactive payment"),Legacy.Cash,F.A.Cash);return true;
}
#endif
