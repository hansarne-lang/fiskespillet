#include "Fishing/FiskCatchCatalog.h"
#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
#include "World/FiskOceanAtlas.h"
#include "Angler/FiskAnglerModel.h"
#include "Career/FiskSeaPassport.h"
#include "Voyage/FiskSavedRoutes.h"
#include "Voyage/FiskVoyagePlan.h"
#include "UI/FiskNavigationNotebook.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMangroveContentTest,"Fiskelaget.Mangrove.ContentIsolationAndSafeRoutes",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMangroveContentTest::RunTest(const FString& Parameters)
{
    const int32 Old[3][7]={{50,10,2,28,8,2,0},{12,40,10,15,12,10,1},{3,15,42,2,12,21,5}};
    for(int32 G=0;G<3;++G) { for(int32 S=0;S<FiskCatches::Count;++S) { TestEqual(TEXT("Old habitat distribution preserved"),FiskCatches::HabitatChance(G,S),S<7?Old[G][S]:0); } }
    TSet<int32> Seen;for(int32 Seed=0;Seed<2048;++Seed) { const auto Fish=FiskCatches::Roll(Seed,3);TestTrue(TEXT("Only valid new habitat species"),FiskCatches::IsValid(Fish) && Fish.Species>=7 && Fish.Species<=10);Seen.Add(Fish.Species); }
    TestEqual(TEXT("All four species available"),Seen.Num(),4);
    for(int32 Mark=9;Mark<FiskAtlas::Count;++Mark) { int32 G=-1,T=0;TestTrue(TEXT("New atlas point is usable"),FiskGrounds::MakeSeaPoint(FiskAtlas::Find(Mark)->Point,G,T));TestEqual(TEXT("New marks use mangrove habitat"),G,3); }
    TestFalse(TEXT("New island rejects destinations"),FiskRouteGeometry::Sea(FiskRouteGeometry::Island(2)));
    const auto P=FiskRouteGeometry::Between(FVector(35000,-60000,0),FVector(52000,-82000,0));TestTrue(TEXT("Route detours around mangrove roots"),P.Num()>2);
    for(int32 I=1;I<P.Num();++I) { TestTrue(TEXT("All detour segments safe"),FiskRouteGeometry::Clear(P[I-1],P[I])); }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMangroveMigrationTest,"Fiskelaget.Mangrove.AnglerV1MigrationPreservesPracticeAndRigs",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMangroveMigrationTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits)),File=FPaths::Combine(Dir,TEXT("angler.ini"));const FGuid Id=FGuid::NewGuid();FFiskAnglerData Old;
    Old.Loadouts[0].Name=TEXT("LEGACY TWELVE");Old.Loadouts[0].Rigs={FiskAngler::Advice(0,0),FiskAngler::Advice(1,2)};
    Old.Scores[13].Attempts=3;Old.Scores[13].Completed=2;Old.Scores[13].BestMs=65432;Old.Scores[13].LastCompleted=FGuid::NewGuid();const auto Last=Old.Scores[13].LastCompleted;
    TestTrue(TEXT("Seed known data"),Old.Save(File,Id));FConfigFile C;C.Read(File);C.SetString(TEXT("Fiskelaget.Angler"),TEXT("Version"),TEXT("1"));
    for(int32 I=14;I<FiskAngler::ScoreCount;++I) { C.Remove(FString::Printf(TEXT("Practice%d"),I)); }C.Dirty=true;TestTrue(TEXT("Write v1 fixture"),C.Write(File));FString Before;FFileHelper::LoadFileToString(Before,*File);
    FFiskAnglerData New;TestTrue(TEXT("Read legacy v1"),New.Load(File,Id));TestEqual(TEXT("All new score rows exist"),New.Scores.Num(),22);TestEqual(TEXT("Named multi-rod preset preserved"),New.Loadouts[0].Name,Old.Loadouts[0].Name);TestEqual(TEXT("Second rod preserved"),New.Loadouts[0].Rigs[1].Depth,Old.Loadouts[0].Rigs[1].Depth);TestEqual(TEXT("Old completed count"),New.Scores[13].Completed,2);TestEqual(TEXT("Old best time"),New.Scores[13].BestMs,65432);TestTrue(TEXT("Old attempt identity"),New.Scores[13].LastCompleted==Last);
    for(int32 I=14;I<22;++I) { TestEqual(TEXT("New species start without invented progress"),New.Scores[I].Attempts,0); }
    FString Unchanged;FFileHelper::LoadFileToString(Unchanged,*File);TestEqual(TEXT("Reading does not rewrite old file"),Unchanged,Before);TestTrue(TEXT("Explicit save upgrades"),New.Save(File,Id));FString Backup;FFileHelper::LoadFileToString(Backup,*(File+TEXT(".bak")));TestEqual(TEXT("Original v1 preserved in backup"),Backup,Before);
    FFiskAnglerData Reload;TestTrue(TEXT("Reload upgraded file"),Reload.Load(File,Id));TestEqual(TEXT("Completion survives upgrade"),Reload.Scores[13].Completed,2);
    for(const FString& S:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(File+S)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskMangroveHistoryTest,"Fiskelaget.Mangrove.HistoricalPointsSurviveNewLandAndHabitat",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskMangroveHistoryTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits));IFileManager::Get().MakeDirectory(*Dir,true);
    const FVector Land=FiskRouteGeometry::Island(2);TestTrue(TEXT("Previously valid sea retained for records"),FiskRouteGeometry::Sea(Land,2));TestFalse(TEXT("Current route cannot use new land"),FFiskVoyageRoute::Valid({Land}));
    FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;E.Fish=FiskCatches::Roll(1,0);E.Context.Ground=2;E.Context.Depth=30;E.Context.LandedAtMs=200000;E.Context.FightMs=30000;E.Context.LandedUtc=TEXT("2026-11-06T12:00:00.000Z");E.Context.bHasSeaPoint=true;E.Context.SeaPoint=Land;E.bSold=true;
    FFiskJournalModel J;TestTrue(TEXT("Old confirmed catch remains readable"),J.Restore({E}));
    FFiskSavedRoutes Routes;const FGuid Id=FGuid::NewGuid();const FString RouteFile=Dir/TEXT("routes.ini");TestTrue(TEXT("Initialize old route slots"),Routes.Load(RouteFile,Id));Routes.Slots[0].Points={Land};TestTrue(TEXT("Historical named route can persist"),Routes.Save(RouteFile,Id));FFiskSavedRoutes Reload;TestTrue(TEXT("Historical named route reloads"),Reload.Load(RouteFile,Id));TestTrue(TEXT("Exact historic point preserved"),Reload.Slots[0].Points[0]==Land);TestFalse(TEXT("Loading never makes land navigable"),FFiskVoyageRoute::Valid(Reload.Slots[0].Points));
    const FString PlaceFile=Dir/TEXT("places.ini");FConfigFile Old;Old.SetString(TEXT("Fiskelaget.Places"),TEXT("Version"),TEXT("2"));Old.SetString(TEXT("Place1"),TEXT("Ground"),TEXT("2"));Old.SetString(TEXT("Place1"),TEXT("Name"),TEXT("OLD WATER"));Old.SetString(TEXT("Place1"),TEXT("X"),TEXT("42000"));Old.SetString(TEXT("Place1"),TEXT("Y"),TEXT("-66000"));Old.Dirty=true;TestTrue(TEXT("Write historical place"),Old.Write(PlaceFile));
    FFiskNavigationNotebook Places;TestTrue(TEXT("Old habitat label still reads after new nearest habitat"),Places.Load(PlaceFile));TestTrue(TEXT("Keep old coordinates"),Places.Get(0) && Places.Get(0)->Point==Land);TestFalse(TEXT("New save cannot select land"),Places.SavePoint(1,Land,TEXT("LAND"),PlaceFile));TestTrue(TEXT("Can add a valid mangrove favorite without erasing old point"),Places.SavePoint(1,FiskGrounds::Find(3)->Position,TEXT("MANGROVE"),PlaceFile));FFiskNavigationNotebook New;TestTrue(TEXT("New ground 3 reloads"),New.Load(PlaceFile));TestEqual(TEXT("New label retained"),New.Get(1)->Ground,3);TestTrue(TEXT("Old place retained"),New.Get(0)->Point==Land);
    for(const FString& P:{RouteFile,RouteFile+TEXT(".bak"),PlaceFile}) { IFileManager::Get().Delete(*P); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
#endif
