#include "Collections/FiskCollectionModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
namespace CollectionTest
{
FFiskJournalEntry Entry(int32 Species,int32 Ground,bool Sold,int32 Weight=0)
{
    FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;const auto& S=*FiskCatches::Find(Species);E.Fish.Species=Species;E.Fish.WeightGrams=Weight?Weight:S.MinGrams;E.Fish.LengthMm=FiskCatches::LengthForWeight(S,E.Fish.WeightGrams);E.Fish.Value=FiskCatches::ValueForWeight(S,E.Fish.WeightGrams);E.bSold=Sold;E.Context.Ground=Ground;E.Context.Depth=30;E.Context.LandedAtMs=200000;E.Context.FightMs=30000;E.Context.LandedUtc=TEXT("2026-11-07T12:00:00.000Z");return E;
}
void Clean(const FString& File) { for(const FString& S:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(File+S)); }IFileManager::Get().DeleteDirectory(*FPaths::GetPath(File)); }
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionPrivacyTest,"Fiskelaget.Collections.CatalogAndConfirmedOnly",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionPrivacyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Thirty-six named objectives"),FiskCollections::Catalog().Num(),FiskCollections::Count);FFiskJournalModel J;for(int32 I=0;I<FiskCollections::Count;++I) { TestEqual(TEXT("Empty journal has no invented progress"),FiskCollections::Progress(J,I),int64(0)); }
    auto E=CollectionTest::Entry(9,3,true,10100);TestTrue(TEXT("Own confirmed fixture"),J.Restore({E}));TestEqual(TEXT("Permit collection"),FiskCollections::Progress(J,9),int64(1));TestEqual(TEXT("Mangrove collection"),FiskCollections::Progress(J,14),int64(1));TestEqual(TEXT("No other species credited"),FiskCollections::Progress(J,0),int64(0));TestEqual(TEXT("Delivered grams"),FiskCollections::Progress(J,28),int64(10100));TestEqual(TEXT("Completion never exceeds target"),FiskCollections::Progress(J,21),int64(1));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionLostTest,"Fiskelaget.Collections.LostCatchNeverPaysOrAdvancesDeliveredWeight",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionLostTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel J;auto E=CollectionTest::Entry(9,3,false,10100);J.Restore({E});TestEqual(TEXT("Lost landing counts for species"),FiskCollections::Progress(J,9),int64(1));TestEqual(TEXT("Not sold"),FiskCollections::Progress(J,27),int64(0));TestEqual(TEXT("Not delivered weight"),FiskCollections::Progress(J,28),int64(0));TestEqual(TEXT("No sale income"),J.Stats().Income,int64(0));TestTrue(TEXT("Report does not claim sale price"),FiskCollections::Html(J,{},{}).Contains(TEXT("Tapt ved retur</td><td>0</td>")));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionSearchTest,"Fiskelaget.Collections.CombinedFiltersAndStableSorting",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionSearchTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel J;auto A=CollectionTest::Entry(9,3,true,10100),B=CollectionTest::Entry(9,3,false,15000),C=CollectionTest::Entry(0,0,true,2000);J.Restore({A,B,C});FFiskCatchQuery Q;Q.Species=9;Q.Ground=3;Q.MinGrams=10000;Q.Status=1;auto Rows=FiskCollections::Search(J,Q);TestTrue(TEXT("All criteria intersect"),Rows.Num()==1 && Rows[0]==0);Q.Status=2;Rows=FiskCollections::Search(J,Q);TestTrue(TEXT("Lost filter"),Rows.Num()==1 && Rows[0]==1);Q.Status=0;Q.Sort=1;Rows=FiskCollections::Search(J,Q);TestTrue(TEXT("Heaviest first"),Rows.Num()==2 && Rows[0]==1);Q.Species=99;TestTrue(TEXT("Invalid query cannot widen results"),FiskCollections::Search(J,Q).IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionPinTest,"Fiskelaget.Collections.ThreeDistinctPinsAndSwap",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionPinTest::RunTest(const FString& Parameters)
{ FFiskCollectionPins P;TestTrue(TEXT("Default pins valid"),P.Valid());TestTrue(TEXT("Swap existing goal"),P.Set(0,11));TestTrue(TEXT("No duplicate after swap"),P.Valid() && P.Values[0]==11 && P.Values[1]==0);TestFalse(TEXT("Invalid slot"),P.Set(3,1));TestFalse(TEXT("Invalid goal"),P.Set(0,FiskCollections::Count));P.Values[2]=P.Values[1];TestFalse(TEXT("Duplicate store invalid"),P.Valid());return true; }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionDiskTest,"Fiskelaget.Collections.ProfileBindingBackupAndFuturePreservation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionDiskTest::RunTest(const FString& Parameters)
{
    const FString File=FPaths::ProjectSavedDir()/TEXT("Automation")/FGuid::NewGuid().ToString()/TEXT("pins.ini");const auto Id=FGuid::NewGuid();FFiskCollectionPins P,Q;TestTrue(TEXT("Existing profile gets new defaults"),P.Load(File,Id));P.Set(0,29);TestTrue(TEXT("Save first generation"),P.Save(File,Id));P.Set(1,1);TestTrue(TEXT("Save second generation"),P.Save(File,Id));TestTrue(TEXT("Reload own pins"),Q.Load(File,Id));TestEqual(TEXT("Exact stored selection"),Q.Values[1],1);TestFalse(TEXT("Foreign profile rejected"),Q.Load(File,FGuid::NewGuid()));TestFalse(TEXT("Foreign writer rejected"),P.Save(File,FGuid::NewGuid()));FFileHelper::SaveStringToFile(TEXT("bad-primary"),*File);TestTrue(TEXT("Actual previous backup"),Q.Load(File,Id));TestTrue(TEXT("Recovery marked"),Q.bRecovered);TestEqual(TEXT("Backup is previous pins"),Q.Values[1],11);TestTrue(TEXT("Explicit repair"),Q.Save(File,Id));FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Collections]\nVersion=3\n"),*File);TestFalse(TEXT("Future primary blocks"),P.Load(File,Id));TestFalse(TEXT("Future primary not overwritten"),Q.Save(File,Id));CollectionTest::Clean(File);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCollectionExportTest,"Fiskelaget.Collections.FilteredHtmlCsvExportAndUniqueNames",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCollectionExportTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Automation")/FGuid::NewGuid().ToString();FFiskJournalModel J;auto A=CollectionTest::Entry(9,3,true,10100),B=CollectionTest::Entry(0,0,true);J.Restore({A,B});FFiskCatchQuery Q;Q.Ground=3;FString File,Other;TestTrue(TEXT("Both export formats"),FiskCollections::Export(Dir,J,Q,{9,14,26},File));FString Html,Csv;FFileHelper::LoadFileToString(Html,*File);const auto CsvFile=FPaths::ChangeExtension(File,TEXT("csv"));FFileHelper::LoadFileToString(Csv,*CsvFile);TestTrue(TEXT("Matching catch included"),Html.Contains(A.CatchId.ToString(EGuidFormats::Digits)) && Csv.Contains(A.CatchId.ToString(EGuidFormats::Digits)));TestFalse(TEXT("Filtered catch excluded"),Html.Contains(B.CatchId.ToString(EGuidFormats::Digits)) || Csv.Contains(B.CatchId.ToString(EGuidFormats::Digits)));TestFalse(TEXT("No executable scripts or remote resources"),Html.Contains(TEXT("<script")) || Html.Contains(TEXT("https://")));TestTrue(TEXT("Fresh names preserve prior report"),FiskCollections::Export(Dir,J,Q,{},Other) && Other!=File);IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*CsvFile);IFileManager::Get().Delete(*Other);IFileManager::Get().Delete(*FPaths::ChangeExtension(Other,TEXT("csv")));IFileManager::Get().DeleteDirectory(*Dir);return true;
}
#endif
