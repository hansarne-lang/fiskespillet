#include "Expedition/FiskExpeditionModel.h"
#include "Expedition/FiskExpeditionStore.h"
#include "UI/FiskRigPresets.h"
#include "UI/FiskJournalStore.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace ExpeditionTest
{
struct FFixture
{
    FFiskExpeditionPlan Plan;FFiskSettlementView View;TMap<int32,FGuid> Profiles;TArray<FGuid> Crew;TMap<FString,int32> Records;
    FFixture()
    {
        Plan.DayId=FGuid::NewGuid();Plan.bLocked=true;Plan.RequiredCrew=2;Plan.Species=0;
        View.DayId=Plan.DayId;View.DayNumber=1;View.bRevealed=true;
        for(int32 I=0;I<2;++I) { const FGuid Id=FGuid::NewGuid();Crew.Add(Id);Profiles.Add(256+I,Id); }
    }
    void Catch(int32 Player,int32 Species=0,bool Delivered=true)
    {
        FFiskDayResult Row;Row.PlayerId=Player;FFiskCatchResult C;C.CatchId=FGuid::NewGuid();
        const auto* S=FiskCatches::Find(Species);C.Fish.Species=Species;C.Fish.WeightGrams=S->MinGrams;
        C.Fish.LengthMm=FiskCatches::LengthForWeight(*S,C.Fish.WeightGrams);C.Fish.Value=FiskCatches::ValueForWeight(*S,C.Fish.WeightGrams);
        C.Context.FightMs=30000;C.Context.LandedAtMs=200000;C.Context.LandedUtc=FDateTime(2026,11,6,12,0,0).ToIso8601();
        C.Context.bHasSeaPoint=true;C.Context.SeaPoint=Plan.Point;C.bDelivered=Delivered;Row.Catches.Add(C);
        Row.DeliveredFish=Delivered?1:0;Row.LostFish=Delivered?0:1;Row.SessionBalance=Delivered?C.Fish.Value:0;Row.DeliveredWeightGrams=Delivered?C.Fish.WeightGrams:0;View.Results.Add(Row);
    }
    bool Evaluate() { return FiskExpeditions::Evaluate(Plan,View,1020000,Profiles,Crew,Records); }
};
struct FFiles
{
    FString Path=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT(".ini"));
    FFiles() { IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true); }
    ~FFiles() { for(const FString& S:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(Path+S)); } }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGoalDeadlineTest,"Fiskelaget.Expedition.DeadlineAndLateCatches",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskGoalDeadlineTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFixture F;F.Plan.Goal=0;F.Catch(256);
    TestFalse(TEXT("No result before deadline"),FiskExpeditions::Evaluate(F.Plan,F.View,1019999,F.Profiles,F.Crew,F.Records));
    F.View.bRevealed=false;TestFalse(TEXT("No unrevealed result"),F.Evaluate());F.View.bRevealed=true;TestTrue(TEXT("Delivered habitat catch qualifies"),F.Evaluate());
    F.View.Results[0].Catches[0].bDelivered=false;TestFalse(TEXT("Late return never qualifies"),F.Evaluate());F.View.Results[0].Catches[0].bDelivered=true;
    F.Plan.DayId=FGuid::NewGuid();TestFalse(TEXT("Stale day refused"),F.Evaluate());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGoalVarietyTest,"Fiskelaget.Expedition.VarietyAndCrew",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskGoalVarietyTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFixture F;F.Plan.Goal=1;F.Catch(256);TestFalse(TEXT("One species insufficient"),F.Evaluate());F.Catch(256);TestFalse(TEXT("Repeated species insufficient"),F.Evaluate());
    F.Catch(257,1);TestTrue(TEXT("Two species qualify"),F.Evaluate());F.Plan.Goal=2;TestTrue(TEXT("Two own crew profiles qualify"),F.Evaluate());
    F.Profiles[257]=FGuid::NewGuid();TestFalse(TEXT("Foreign profile excluded"),F.Evaluate());F.Plan.RequiredCrew=1;TestTrue(TEXT("Solo crew goal achievable"),F.Evaluate());
    F.Plan.RequiredCrew=0;TestFalse(TEXT("Invalid crew requirement rejected"),F.Evaluate());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskGoalRecordTest,"Fiskelaget.Expedition.PersonalRecordAndSeaPoint",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskGoalRecordTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFixture F;F.Plan.Goal=3;F.Catch(256);TestTrue(TEXT("First confirmed species record qualifies"),F.Evaluate());
    const auto& C=F.View.Results[0].Catches[0];F.Records.Add(FiskExpeditions::RecordKey(F.Crew[0],0),C.Fish.WeightGrams);
    TestFalse(TEXT("Equal weight is not a new record"),F.Evaluate());F.Records[FiskExpeditions::RecordKey(F.Crew[0],0)]-=1;TestTrue(TEXT("Higher weight qualifies"),F.Evaluate());
    F.Plan.Goal=4;TestTrue(TEXT("Selected sea point qualifies"),F.Evaluate());F.View.Results[0].Catches[0].Context.SeaPoint.X+=501;TestFalse(TEXT("Outside five metres rejected"),F.Evaluate());
    F.View.Results[0].Catches[0].Context.bHasSeaPoint=false;TestFalse(TEXT("Legacy missing location cannot qualify"),F.Evaluate());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskExpeditionPersistenceTest,"Fiskelaget.Expedition.ProfilePersistenceAndReplay",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskExpeditionPersistenceTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFiles F;const FGuid Profile=FGuid::NewGuid();FFiskExpeditionRecord R;R.DayId=FGuid::NewGuid();R.bCompleted=true;R.Goal=3;R.Weather=2;
    TArray<FFiskExpeditionRecord> A={R};TestTrue(TEXT("Save own medal"),FiskExpeditionStore::Save(F.Path,Profile,A));
    TArray<FFiskExpeditionRecord> B;TestTrue(TEXT("Restart loads history"),FiskExpeditionStore::Load(F.Path,Profile,B));TestEqual(TEXT("One record"),B.Num(),1);
    TestTrue(TEXT("Repeated batch is idempotent"),FiskExpeditionStore::Merge(B,{R}));TestTrue(TEXT("Replay persisted"),FiskExpeditionStore::Save(F.Path,Profile,B));TestEqual(TEXT("One medal after replay"),B.Num(),1);
    R.bCompleted=false;TestFalse(TEXT("Conflicting result rejected"),FiskExpeditionStore::Merge(B,{R}));TestFalse(TEXT("Other profile cannot read"),FiskExpeditionStore::Load(F.Path,FGuid::NewGuid(),B));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskExpeditionCorruptTest,"Fiskelaget.Expedition.BackupFutureAndBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskExpeditionCorruptTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFiles F;const FGuid Profile=FGuid::NewGuid();FFiskExpeditionRecord R;R.DayId=FGuid::NewGuid();TArray<FFiskExpeditionRecord> A={R};
    TestTrue(TEXT("Initial backup"),FiskExpeditionStore::Save(F.Path,Profile,A));FFileHelper::SaveStringToFile(TEXT("bad data"),*F.Path);TArray<FFiskExpeditionRecord> B;
    TestTrue(TEXT("Valid backup recovers corrupt main"),FiskExpeditionStore::Load(F.Path,Profile,B));TestTrue(TEXT("Repair from backup"),FiskExpeditionStore::Save(F.Path,Profile,B));
    FFileHelper::SaveStringToFile(FString::Printf(TEXT("[Fiskelaget.Expeditions]\nVersion=1\nProfile=%s\nGeneration=2147483647\nCount=9999\n"),*Profile.ToString(EGuidFormats::Digits)),*F.Path);
    TestTrue(TEXT("Invalid main generation cannot block valid backup repair"),FiskExpeditionStore::Save(F.Path,Profile,B));
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Expeditions]\nVersion=2\n"),*F.Path);TestFalse(TEXT("Future format preserved"),FiskExpeditionStore::Save(F.Path,Profile,B));
    FString Text;FFileHelper::LoadFileToString(Text,*F.Path);TestTrue(TEXT("Future bytes unchanged"),Text.Contains(TEXT("Version=2")));
    B.Reset();for(int32 I=0;I<FiskExpeditions::Capacity;++I) { R.DayId=FGuid::NewGuid();B.Add(R); }R.DayId=FGuid::NewGuid();TestFalse(TEXT("No silent eviction at capacity"),FiskExpeditionStore::Merge(B,{R}));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskNamedRigTest,"Fiskelaget.Expedition.NamedRigMigrationAndPreservation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskNamedRigTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFiles F;FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.RigPreset1]\nVersion=1\nBait=1\nDepth=40\nProgram=1\n"),*F.Path);
    FFiskRigPresets P;P.Load(F.Path);TestTrue(TEXT("Old unnamed rig migrates"),P.Get(0)->bExists);TestEqual(TEXT("Default name"),P.Get(0)->Name,FString(TEXT("OPPSETT 1")));
    TestTrue(TEXT("Rename saved slot"),P.Rename(0,TEXT("REVFISKE"),F.Path));FFiskRigPresets Reload;Reload.Load(F.Path);TestEqual(TEXT("Named rig survives restart"),Reload.Get(0)->Name,FString(TEXT("REVFISKE")));
    TestFalse(TEXT("Empty slot cannot be renamed"),Reload.Rename(1,TEXT("TOM"),F.Path));TestFalse(TEXT("Invalid name rejected"),Reload.Rename(0,TEXT("../escape"),F.Path));
    FFileHelper::SaveStringToFile(TEXT("corrupt"),*F.Path);FFiskRigPresets Backup;Backup.Load(F.Path);TestTrue(TEXT("Rig backup loads"),Backup.Get(0)->bExists);
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.RigPreset1]\nVersion=99\n"),*F.Path);TestFalse(TEXT("Future rig cannot be overwritten"),P.Rename(0,TEXT("NYTT"),F.Path));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPointJournalTest,"Fiskelaget.Expedition.JournalLocationRoundTrip",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPointJournalTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFiles F;ExpeditionTest::FFixture Data;Data.Catch(256);FFiskJournalModel J;
    TestEqual(TEXT("Apply confirmed point"),J.Apply(Data.View.DayId,1,1020000,true,Data.View.Results[0]),EFiskJournalApply::Added);FFiskJournalStore Store;TestTrue(TEXT("Persist location"),Store.Save(F.Path,J));FFiskJournalModel Reload;
    TestEqual(TEXT("Journal reload"),Store.Load(F.Path,Reload),EFiskJournalLoad::Loaded);TestTrue(TEXT("Sea point retained"),Reload.GetEntries().Num()==1 && Reload.GetEntries()[0].Context.bHasSeaPoint && Reload.GetEntries()[0].Context.SeaPoint==Data.Plan.Point);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskFutureJournalTest,"Fiskelaget.Expedition.FutureJournalPreservedWithValidBackup",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskFutureJournalTest::RunTest(const FString& Parameters)
{
    ExpeditionTest::FFiles F;FFiskJournalStore Store;FFiskJournalModel Journal;
    TestTrue(TEXT("Create old valid main and backup"),Store.Save(F.Path,Journal));
    const FString Future=TEXT("[Fiskelaget.Journal]\nVersion=2\nFutureData=keep\n");FFileHelper::SaveStringToFile(Future,*F.Path);
    TestEqual(TEXT("Do not roll back future journal"),Store.Load(F.Path,Journal),EFiskJournalLoad::Unavailable);
    TestFalse(TEXT("Do not overwrite future file with backup"),Store.Save(F.Path,Journal));FString Actual;FFileHelper::LoadFileToString(Actual,*F.Path);
    TestEqual(TEXT("Future bytes preserved"),Actual,Future);return true;
}
#endif
