#include "Fishing/FiskCatchCatalog.h"
#include "FiskFishingGround.h"
#include "World/FiskOceanAtlas.h"
#include "Angler/FiskAnglerModel.h"
#include "Collections/FiskCollectionModel.h"
#include "Career/FiskSeaPassport.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDeepContentTest,"Fiskelaget.Deepsea.PreservedDistributionsAndNewContent",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskDeepContentTest::RunTest(const FString& Parameters)
{
    const int32 Old[4][11]={{50,10,2,28,8,2,0},{12,40,10,15,12,10,1},{3,15,42,2,12,21,5},{0,0,0,0,0,0,0,35,25,25,15}};
    for(int32 G=0;G<4;++G) { for(int32 S=0;S<FiskCatches::Count;++S) { TestEqual(TEXT("Every old distribution retained"),FiskCatches::HabitatChance(G,S),S<11?Old[G][S]:0); } }
    TSet<int32> Species;for(int32 Seed=0;Seed<2048;++Seed) { const auto F=FiskCatches::Roll(Seed,4);TestTrue(TEXT("Valid deep catch only"),FiskCatches::IsValid(F) && F.Species>=11 && F.Species<15);Species.Add(F.Species); }TestEqual(TEXT("All four deep species"),Species.Num(),4);
    for(int32 Mark=12;Mark<FiskAtlas::Count;++Mark) { int32 Habitat=-1,Travel=0;TestTrue(TEXT("New mark has valid route"),FiskGrounds::MakeSeaPoint(FiskAtlas::Find(Mark)->Point,Habitat,Travel));TestEqual(TEXT("New mark uses deep habitat"),Habitat,4); }
    TestEqual(TEXT("Old species goal ID stable"),FiskCollections::Catalog()[10].Value,10);TestTrue(TEXT("Old habitat goal ID stable"),FiskCollections::Catalog()[11].Rule==EFiskCollectionRule::Ground && FiskCollections::Catalog()[11].Value==0);TestTrue(TEXT("Old count goal stable"),FiskCollections::Catalog()[26].Rule==EFiskCollectionRule::Count);TestEqual(TEXT("Old diversity target retained"),FiskCollections::Catalog()[29].Target,int64(11));TestEqual(TEXT("New species appended"),FiskCollections::Catalog()[30].Value,11);TestEqual(TEXT("New habitat appended"),FiskCollections::Catalog()[34].Value,4);TestEqual(TEXT("New complete catalog target"),FiskCollections::Catalog()[35].Target,int64(15));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDeepAnglerMigrationTest,"Fiskelaget.Deepsea.AnglerV2MigrationPreservesLastSpeciesAndLoadout",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskDeepAnglerMigrationTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Automation")/FGuid::NewGuid().ToString(),File=Dir/TEXT("angler.ini");const FGuid Id=FGuid::NewGuid();FFiskAnglerData D;D.Loadouts[0].Name=TEXT("V14 DEEP");D.Loadouts[0].Rigs={FiskAngler::Advice(3,1)};D.Scores[21].Attempts=4;D.Scores[21].Completed=2;D.Scores[21].BestMs=65432;D.Scores[21].LastCompleted=FGuid::NewGuid();const auto Last=D.Scores[21].LastCompleted;
    TestTrue(TEXT("Seed data"),D.Save(File,Id));FConfigFile C;C.Read(File);C.SetString(TEXT("Fiskelaget.Angler"),TEXT("Version"),TEXT("2"));for(int32 I=22;I<FiskAngler::ScoreCount;++I) { C.Remove(FString::Printf(TEXT("Practice%d"),I)); }C.Dirty=true;C.Write(File);FString Before;FFileHelper::LoadFileToString(Before,*File);
    FFiskAnglerData E;TestTrue(TEXT("Read actual v2 shape"),E.Load(File,Id));TestEqual(TEXT("Thirty rows after migration"),E.Scores.Num(),30);TestEqual(TEXT("Old last completion retained"),E.Scores[21].Completed,2);TestTrue(TEXT("Attempt identity retained"),E.Scores[21].LastCompleted==Last);TestEqual(TEXT("Named loadout retained"),E.Loadouts[0].Name,D.Loadouts[0].Name);for(int32 I=22;I<30;++I) { TestEqual(TEXT("No invented new practice"),E.Scores[I].Attempts,0); }
    TestTrue(TEXT("Explicit upgrade"),E.Save(File,Id));FString Backup;FFileHelper::LoadFileToString(Backup,*(File+TEXT(".bak")));TestEqual(TEXT("Exact v2 backup"),Backup,Before);FFiskAnglerData Reload;TestTrue(TEXT("Reload v3"),Reload.Load(File,Id));TestEqual(TEXT("Best time survives"),Reload.Scores[21].BestMs,65432);IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*(File+TEXT(".bak")));IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDeepPinsMigrationTest,"Fiskelaget.Deepsea.CollectionV1MigrationPreservesOldIds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskDeepPinsMigrationTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Automation")/FGuid::NewGuid().ToString(),File=Dir/TEXT("pins.ini");const FGuid Id=FGuid::NewGuid();FFiskCollectionPins P;P.Values={29,1,26};TestTrue(TEXT("Seed known old IDs"),P.Save(File,Id));FConfigFile C;C.Read(File);C.SetString(TEXT("Fiskelaget.Collections"),TEXT("Version"),TEXT("1"));C.Dirty=true;C.Write(File);FString Before;FFileHelper::LoadFileToString(Before,*File);
    FFiskCollectionPins Q;TestTrue(TEXT("Old pins read"),Q.Load(File,Id));TestTrue(TEXT("Old IDs unchanged"),Q.Values==P.Values);TestTrue(TEXT("Can pin new deep goal"),Q.Set(0,35));TestTrue(TEXT("Explicit save upgrades"),Q.Save(File,Id));FString Backup;FFileHelper::LoadFileToString(Backup,*(File+TEXT(".bak")));TestEqual(TEXT("Old pin file preserved exactly"),Backup,Before);FFiskCollectionPins R;TestTrue(TEXT("Read new format"),R.Load(File,Id));TestEqual(TEXT("New pin persisted"),R.Values[0],35);IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*(File+TEXT(".bak")));IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskDeepPassportTest,"Fiskelaget.Deepsea.PassportSpeciesFamiliesStaySeparate",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskDeepPassportTest::RunTest(const FString& Parameters)
{
    TArray<FFiskJournalEntry> Entries;for(int32 I=11;I<15;++I) { FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;const auto& S=*FiskCatches::Find(I);E.Fish.Species=I;E.Fish.WeightGrams=S.MinGrams;E.Fish.LengthMm=S.MinLengthMm;E.Fish.Value=FiskCatches::ValueForWeight(S,S.MinGrams);E.bSold=true;E.Context.Ground=4;E.Context.Depth=90;E.Context.LandedAtMs=400000;E.Context.FightMs=100000;E.Context.LandedUtc=TEXT("2026-11-07T12:00:00.000Z");Entries.Add(E); }
    FFiskJournalModel J;TestTrue(TEXT("Confirmed deep fixtures"),J.Restore(Entries));const auto Rows=FiskSeaPassport::Build(J,{});TestEqual(TEXT("Sixteen milestones"),Rows.Num(),16);TestEqual(TEXT("Deep catches never credit mangrove species"),Rows[12].Current,int64(0));TestTrue(TEXT("All deep species milestone"),Rows[14].Complete());TestEqual(TEXT("New deep collection"),FiskCollections::Progress(J,34),int64(2));TestEqual(TEXT("New diversity collection"),FiskCollections::Progress(J,35),int64(4));return true;
}
#endif
