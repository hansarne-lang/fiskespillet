#include "Voyage/FiskSkipperTools.h"
#include "Voyage/FiskVoyagePlan.h"
#include "World/FiskOceanAtlas.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskTourTemplatesTest,"Fiskelaget.SkipperTools.SixValidToursCoverAtlas",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskTourTemplatesTest::RunTest(const FString&)
{
    TSet<int32> Covered;for(int32 I=0;I<FiskSkipperTools::TemplateCount;++I) { const auto P=FiskSkipperTools::Template(I);TestTrue(TEXT("Valid bounded tour"),!P.IsEmpty() && FFiskVoyageRoute::Valid(P));for(auto V:P) { Covered.Add(FiskAtlas::Nearest(V)); } }
    TestEqual(TEXT("Every atlas mark is reachable"),Covered.Num(),FiskAtlas::Count);TestTrue(TEXT("Unknown tours are empty"),FiskSkipperTools::Template(-1).IsEmpty() && FiskSkipperTools::Template(6).IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskTourBudgetTest,"Fiskelaget.SkipperTools.BudgetDepartureReturnAndDeadline",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskTourBudgetTest::RunTest(const FString&)
{
    const TArray<FVector> P={FiskAtlas::Find(0)->Point};const auto A=FiskSkipperTools::Budget(P,0,120000,180000),B=FiskSkipperTools::Budget(P,1,120000,180000);
    TestTrue(TEXT("Known tour"),A.Valid);TestEqual(TEXT("Selected fishing time"),A.FishingMs,static_cast<int64>(180000));TestEqual(TEXT("Departure and both legs"),A.HomeAtMs,120000+A.SailingMs+180000);TestEqual(TEXT("Deadline margin"),A.MarginMs,1020000-A.HomeAtMs);TestTrue(TEXT("Purchased boat time advantage"),B.SailingMs<A.SailingMs);
    const auto Edge=FiskSkipperTools::Budget(P,0,1020000-static_cast<int32>(A.SailingMs),0);TestEqual(TEXT("Exact 17:00"),Edge.MarginMs,static_cast<int64>(0));TestEqual(TEXT("One millisecond late"),FiskSkipperTools::Budget(P,0,1020001-static_cast<int32>(A.SailingMs),0).MarginMs,static_cast<int64>(-1));
    TestEqual(TEXT("Wait for 02:00"),FiskSkipperTools::Budget(P,0,0,180000).HomeAtMs,A.HomeAtMs);TestFalse(TEXT("Invalid boat"),FiskSkipperTools::Budget(P,2,0,0).Valid);TestFalse(TEXT("Invalid stop time"),FiskSkipperTools::Budget(P,0,0,600001).Valid);TestFalse(TEXT("No invented empty tour"),FiskSkipperTools::Budget({},0,0,0).Valid);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskTourOptimizeTest,"Fiskelaget.SkipperTools.ShortestRoundTripPreservesStops",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskTourOptimizeTest::RunTest(const FString&)
{
    const auto P=FiskSkipperTools::Template(5),Q=FiskSkipperTools::Optimize(P);TestEqual(TEXT("Same number of stops"),Q.Num(),P.Num());for(auto V:P) { TestTrue(TEXT("Every requested stop retained"),Q.Contains(V)); }
    TestTrue(TEXT("Never longer"),FiskSkipperTools::Budget(Q,0,120000,0).DistanceCm<=FiskSkipperTools::Budget(P,0,120000,0).DistanceCm+.01);TestTrue(TEXT("Deterministic ties"),FiskSkipperTools::Optimize(P)==Q);
    TestTrue(TEXT("Duplicate rejected"),FiskSkipperTools::Optimize({P[0],P[0]}).IsEmpty());TestTrue(TEXT("Land rejected"),FiskSkipperTools::Optimize({FVector(110000,45000,0)}).IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCameraLimitsTest,"Fiskelaget.SkipperTools.PhotoPresetValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCameraLimitsTest::RunTest(const FString&)
{
    FFiskPhotoPresets P;TestTrue(TEXT("Three safe defaults"),P.Valid());P.Views[0].Yaw=std::numeric_limits<double>::quiet_NaN();TestFalse(TEXT("NaN refused"),P.Valid());P=FFiskPhotoPresets();P.Views[2].Distance=5001;TestFalse(TEXT("Too far refused"),P.Valid());P.Views[2].Distance=5000;TestTrue(TEXT("Inclusive bounds"),P.Valid());P.Views.Pop();TestFalse(TEXT("Missing slot refused"),P.Valid());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCameraDiskTest,"Fiskelaget.SkipperTools.PhotoRecoveryProfileAndFutureGuards",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCameraDiskTest::RunTest(const FString&)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits)),File=Dir/TEXT("camera.ini");const FGuid Id=FGuid::NewGuid();FFiskPhotoPresets A,B;
    TestTrue(TEXT("New profile"),A.Load(File,Id));A.Views[0]={10,25,2000,55};TestTrue(TEXT("First view saved"),A.Save(File,Id));A.Views[0].Yaw=90;TestTrue(TEXT("Second generation"),A.Save(File,Id));TestTrue(TEXT("Reload"),B.Load(File,Id));TestEqual(TEXT("Exact saved yaw"),B.Views[0].Yaw,90.0);
    TestFalse(TEXT("Foreign profile cannot read"),B.Load(File,FGuid::NewGuid()));TestFalse(TEXT("Foreign profile cannot write"),A.Save(File,FGuid::NewGuid()));FFileHelper::SaveStringToFile(TEXT("bad"),*File);TestTrue(TEXT("Backup recovers"),B.Load(File,Id));TestTrue(TEXT("Recovery flag"),B.bRecovered);TestEqual(TEXT("Prior generation kept"),B.Views[0].Yaw,10.0);TestTrue(TEXT("Explicit repair"),B.Save(File,Id));
    FFileHelper::SaveStringToFile(TEXT("bad"),*File);FFileHelper::SaveStringToFile(TEXT("bad-backup"),*(File+TEXT(".bak")));TestFalse(TEXT("Two damaged files blocked"),B.Load(File,Id));TestFalse(TEXT("Cannot overwrite corrupt history"),A.Save(File,Id));FString Text;FFileHelper::LoadFileToString(Text,*File);TestEqual(TEXT("Primary preserved"),Text,FString(TEXT("bad")));
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Camera]\nVersion=2\n"),*File);TestFalse(TEXT("Future format blocked"),B.Load(File,Id));TestFalse(TEXT("Future format preserved"),A.Save(File,Id));
    for(const auto& Tail:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(File+Tail)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
namespace { FFiskJournalEntry SkipperToolsEntry(FGuid Day,int32 Number,int32 Species,bool Sold)
{ FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=Day;E.DayNumber=Number;const auto& S=*FiskCatches::Find(Species);E.Fish.Species=Species;E.Fish.WeightGrams=S.MinGrams;E.Fish.LengthMm=FiskCatches::LengthForWeight(S,E.Fish.WeightGrams);E.Fish.Value=FiskCatches::ValueForWeight(S,E.Fish.WeightGrams);E.bSold=Sold;E.Context.Ground=0;E.Context.Depth=30;E.Context.Bait=0;E.Context.FightMs=30000;E.Context.LandedAtMs=200000;E.Context.LandedUtc=TEXT("2026-11-07T12:00:00.000Z");return E; } }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskTripCompareTest,"Fiskelaget.SkipperTools.CompareConfirmedTripsAndExport",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskTripCompareTest::RunTest(const FString&)
{
    const FGuid A=FGuid::NewGuid(),B=FGuid::NewGuid();const auto E=SkipperToolsEntry(A,1,0,true),F=SkipperToolsEntry(A,1,1,false),G=SkipperToolsEntry(B,2,2,true);FFiskJournalModel J;TestTrue(TEXT("Two valid days"),J.Restore({E,F,G}));const auto Days=FiskTripBook::Days(J);const int32 X=Days.IndexOfByPredicate([&](const auto& D){return D.Id==A;}),Y=1-X;
    TestEqual(TEXT("Only sold contributes income"),Days[X].Income,static_cast<int64>(E.Fish.Value));TestEqual(TEXT("Lost still landed"),Days[X].Grams,static_cast<int64>(E.Fish.WeightGrams+F.Fish.WeightGrams));TestEqual(TEXT("Lost count"),Days[X].Lost,1);
    const auto Html=FiskSkipperTools::CompareHtml(J,X,Y);TestTrue(TEXT("Both confirmed day IDs"),Html.Contains(A.ToString(EGuidFormats::Digits)) && Html.Contains(B.ToString(EGuidFormats::Digits)));TestTrue(TEXT("Same day rejected"),FiskSkipperTools::CompareHtml(J,X,X).IsEmpty());TestTrue(TEXT("Missing day rejected"),FiskSkipperTools::CompareHtml(J,0,2).IsEmpty());
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits));FString One,Two;TestTrue(TEXT("Export"),FiskSkipperTools::ExportComparison(Dir,J,X,Y,One));TestTrue(TEXT("Second unique export"),FiskSkipperTools::ExportComparison(Dir,J,X,Y,Two));TestTrue(TEXT("Previous export preserved"),One!=Two && IFileManager::Get().FileExists(*One));IFileManager::Get().Delete(*One);IFileManager::Get().Delete(*Two);IFileManager::Get().DeleteDirectory(*Dir);return true;
}
#endif
