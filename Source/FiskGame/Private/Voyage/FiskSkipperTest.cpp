#include "Voyage/FiskVoyagePlan.h"
#include "Voyage/FiskSavedRoutes.h"
#include "Voyage/FiskTripBook.h"
#include "World/FiskOceanAtlas.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRouteBoundsTest,"Fiskelaget.Skipper.BoundedRevisionedPlan",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRouteBoundsTest::RunTest(const FString& Parameters)
{
    TArray<FVector> P;TestTrue(TEXT("Empty plan can be cleared"),FFiskVoyageRoute::Valid(P));for(int32 I=0;I<5;++I) { P.Add(FiskAtlas::Find(I)->Point); }
    TestTrue(TEXT("Five valid stops"),FFiskVoyageRoute::Valid(P));P.Add(FiskAtlas::Find(5)->Point);TestFalse(TEXT("Six rejected"),FFiskVoyageRoute::Valid(P));P.Pop();
    P[4]=P[0];TestFalse(TEXT("Duplicate rejected"),FFiskVoyageRoute::Valid(P));P[4]=FVector::ZeroVector;TestFalse(TEXT("Marina is not a fishing stop"),FFiskVoyageRoute::Valid(P));
    P[4]=FVector(110000,45000,0);TestFalse(TEXT("Island rejected"),FFiskVoyageRoute::Valid(P));FFiskVoyageRoute R;R.Day=2;R.Revision=8;
    TestTrue(TEXT("Fresh command"),R.Matches(2,8));TestFalse(TEXT("Old day"),R.Matches(1,8));TestFalse(TEXT("Old revision"),R.Matches(2,7));R.Revision=MAX_int32;TestFalse(TEXT("Overflow refused"),R.Matches(2,MAX_int32));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskRouteDiskTest,"Fiskelaget.Skipper.RoutePersistenceRecoveryAndPreservation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskRouteDiskTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits)),File=Dir/TEXT("routes.ini");const FGuid Id=FGuid::NewGuid();FFiskSavedRoutes A;
    TestTrue(TEXT("Fresh empty store"),A.Load(File,Id));A.Slots[0].Name=TEXT("REEF TOUR");A.Slots[0].Points={FiskAtlas::Find(0)->Point,FiskAtlas::Find(5)->Point};TestTrue(TEXT("First save"),A.Save(File,Id));
    A.Slots[0].Name=TEXT("SECOND TOUR");TestTrue(TEXT("Second generation"),A.Save(File,Id));FFiskSavedRoutes B;TestTrue(TEXT("Reload"),B.Load(File,Id));TestEqual(TEXT("Exact own name"),B.Slots[0].Name,FString(TEXT("SECOND TOUR")));
    FFiskSavedRoutes Other;TestFalse(TEXT("Foreign profile cannot load"),Other.Load(File,FGuid::NewGuid()));TestFalse(TEXT("Foreign profile cannot overwrite"),A.Save(File,FGuid::NewGuid()));
    FFileHelper::SaveStringToFile(TEXT("damaged-primary"),*File);TestTrue(TEXT("Real prior backup recovers"),B.Load(File,Id));TestTrue(TEXT("Recovered flag"),B.bRecovered);TestEqual(TEXT("Backup is previous generation"),B.Slots[0].Name,FString(TEXT("REEF TOUR")));TestTrue(TEXT("Explicit repair"),B.Save(File,Id));
    FFileHelper::SaveStringToFile(TEXT("bad-primary"),*File);FFileHelper::SaveStringToFile(TEXT("bad-backup"),*(File+TEXT(".bak")));TestFalse(TEXT("Two damaged files block"),B.Load(File,Id));TestFalse(TEXT("Do not replace corrupted history"),A.Save(File,Id));FString Text;FFileHelper::LoadFileToString(Text,*File);TestEqual(TEXT("Primary preserved"),Text,FString(TEXT("bad-primary")));
    FFileHelper::SaveStringToFile(TEXT("[Fiskelaget.Routes]\nVersion=2\n"),*File);TestFalse(TEXT("Future version preserved"),B.Load(File,Id));TestFalse(TEXT("Future store cannot be overwritten"),A.Save(File,Id));
    for(const FString& Tail:{FString(),FString(TEXT(".bak")),FString(TEXT(".tmp")),FString(TEXT(".bak.tmp"))}) { IFileManager::Get().Delete(*(File+Tail)); }IFileManager::Get().DeleteDirectory(*Dir);return true;
}
namespace { FFiskJournalEntry BookEntry(FGuid Day,int32 Species,bool Sold)
{
    FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=Day;E.DayNumber=1;const auto& S=*FiskCatches::Find(Species);E.Fish.Species=Species;E.Fish.WeightGrams=S.MinGrams;E.Fish.LengthMm=FiskCatches::LengthForWeight(S,E.Fish.WeightGrams);E.Fish.Value=FiskCatches::ValueForWeight(S,E.Fish.WeightGrams);E.bSold=Sold;
    E.Context.Ground=0;E.Context.Depth=30;E.Context.Bait=0;E.Context.FightMs=30000;E.Context.LandedAtMs=200000;E.Context.LandedUtc=TEXT("2026-11-06T12:00:00.000Z");return E;
} }
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskBookStatsTest,"Fiskelaget.Skipper.ConfirmedTripAggregation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskBookStatsTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel J;TestTrue(TEXT("No imagined days"),FiskTripBook::Days(J).IsEmpty());const FGuid A=FGuid::NewGuid(),B=FGuid::NewGuid();const auto E=BookEntry(A,0,true),F=BookEntry(A,1,false),G=BookEntry(B,2,true);
    TestTrue(TEXT("Valid own fixture"),J.Restore({E,F,G}));const auto Days=FiskTripBook::Days(J);TestEqual(TEXT("Group by GUID, not reused day number"),Days.Num(),2);
    const auto* D=Days.FindByPredicate([&](const auto& V){return V.Id==A;});TestNotNull(TEXT("Selected day exists"),D);if(D) { TestEqual(TEXT("Both landed fish"),D->Count,2);TestEqual(TEXT("Sold only income"),D->Income,static_cast<int64>(E.Fish.Value));TestEqual(TEXT("One lost"),D->Lost,1);TestEqual(TEXT("Two species"),D->Species.Num(),2);TestEqual(TEXT("Actual fight time"),D->FightMs,static_cast<int64>(60000)); }
    TestEqual(TEXT("Own day rows only"),FiskTripBook::Rows(J,A).Num(),2);TestTrue(TEXT("Unknown day cannot export data"),FiskTripBook::Html(J,FGuid::NewGuid()).IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskReportTest,"Fiskelaget.Skipper.PersonalHtmlReport",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskReportTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel J;const FGuid A=FGuid::NewGuid(),B=FGuid::NewGuid();J.Restore({BookEntry(A,0,true),BookEntry(B,6,true)});const FString Html=FiskTripBook::Html(J,A);
    TestTrue(TEXT("UTF8 Norwegian document"),Html.Contains(TEXT("charset=\"utf-8\"")) && Html.Contains(TEXT("lang=\"nb\"")));TestFalse(TEXT("Other day fish not included"),Html.Contains(FiskCatches::Find(6)->Name));TestFalse(TEXT("No remote scripts"),Html.Contains(TEXT("<script")));
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits));FString One,Two;
    TestTrue(TEXT("Export selected day"),FiskTripBook::Export(Dir,J,A,One));TestTrue(TEXT("Fresh second export"),FiskTripBook::Export(Dir,J,A,Two));TestTrue(TEXT("Do not overwrite"),One!=Two && IFileManager::Get().FileExists(*One));IFileManager::Get().Delete(*One);IFileManager::Get().Delete(*Two);IFileManager::Get().DeleteDirectory(*Dir);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPhotoBoundsTest,"Fiskelaget.Skipper.LocalPhotoCameraBounds",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPhotoBoundsTest::RunTest(const FString& Parameters)
{
    FFiskPhotoView A,B;A.Distance=-1;A.Pitch=180;A.Fov=200;A.Yaw=1080;A.Clamp();TestEqual(TEXT("Safe near limit"),A.Distance,800.0);TestEqual(TEXT("No flipped camera"),A.Pitch,70.0);TestEqual(TEXT("Bounded field of view"),A.Fov,100.0);
    TestEqual(TEXT("Second local view unchanged"),B.Distance,1800.0);TestTrue(TEXT("Finite offset"),!A.Offset(90).ContainsNaN());TestTrue(TEXT("Rotation follows boat"),!B.Offset(0).Equals(B.Offset(90)));A.Distance=MAX_dbl;A.Clamp();TestEqual(TEXT("Far limit"),A.Distance,5000.0);return true;
}
#endif
