#include "World/FiskOceanAtlas.h"
#include "FiskRouteGeometry.h"
#include "FiskVoyageRules.h"
#include "FiskFishingGround.h"
#include "Career/FiskSeaPassport.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskAtlasRoutesTest,"Fiskelaget.Exploration.AtlasAndInterIslandRoutes",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskAtlasRoutesTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Invalid mark safely rejected"),!FiskAtlas::Find(-1) && !FiskAtlas::Find(FiskAtlas::Count));
    for(int32 A=0;A<FiskAtlas::Count;++A)
    {
        int32 Habitat=-1,Time=0;const FVector From=FiskAtlas::Find(A)->Point;
        TestTrue(TEXT("Atlas point has valid habitat and home time"),FiskGrounds::MakeSeaPoint(From,Habitat,Time) && Habitat>=0 && Habitat<FiskGrounds::Count && Time>=4000);
        TestEqual(TEXT("Location finds own atlas mark"),FiskAtlas::Nearest(From),A);
        for(int32 B=0;B<FiskAtlas::Count;++B)
        {
            const FVector To=FiskAtlas::Find(B)->Point;const auto Path=FiskRouteGeometry::Between(From,To);
            TestTrue(TEXT("All atlas transfers have routes"),Path.Num()>=2);
            if(Path.Num()<2) { continue; }TestTrue(TEXT("Exact route endpoints"),Path[0]==From && Path.Last()==To);
            for(int32 I=1;I<Path.Num();++I) { TestTrue(TEXT("Each segment clears islands"),FiskRouteGeometry::Clear(Path[I-1],Path[I])); }
            TestTrue(TEXT("Interpolated arrival is exact"),FiskRouteGeometry::Position(Path,1)==To);
        }
    }
    const auto Home=FiskRouteGeometry::Between(FVector(1000,400,0),FVector::ZeroVector);TestTrue(TEXT("An interrupted near-dock departure can return"),Home.Num()>=2);
    TestTrue(TEXT("Island cannot be a route origin"),FiskRouteGeometry::Between(FiskRouteGeometry::Island(0),FVector::ZeroVector).IsEmpty());
    TestTrue(TEXT("Invalid floating point rejected"),FiskRouteGeometry::Between(FVector(MAX_dbl,0,0),FVector::ZeroVector).IsEmpty());return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskVoyageGuardTest,"Fiskelaget.Exploration.TransferPermissionsAndDeadline",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskVoyageGuardTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Captain may relocate safely"),FiskVoyage::CanRelocate(true,true,true,120000,true,true));
    TestFalse(TEXT("Passenger rejected"),FiskVoyage::CanRelocate(false,true,true,120000,true,true));
    TestFalse(TEXT("Stale route rejected"),FiskVoyage::CanRelocate(true,false,true,120000,true,true));
    TestFalse(TEXT("Only at fishing spot"),FiskVoyage::CanRelocate(true,true,false,120000,true,true));
    TestFalse(TEXT("An active rod prevents relocation"),FiskVoyage::CanRelocate(true,true,true,200000,false,true));
    TestFalse(TEXT("Invalid sea point rejected"),FiskVoyage::CanRelocate(true,true,true,200000,true,false));
    TestFalse(TEXT("Preparation is not fishing time"),FiskVoyage::CanRelocate(true,true,true,119999,true,true));
    TestFalse(TEXT("Deadline cannot be extended"),FiskVoyage::CanRelocate(true,true,true,1020000,true,true));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskVoyageVoteTest,"Fiskelaget.Exploration.UnanimousPresentationOnly",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskVoyageVoteTest::RunTest(const FString& Parameters)
{
    const TArray<int32> Crew={256,257,258,259};TestFalse(TEXT("Three votes not enough"),FiskVoyage::Unanimous(Crew,{256,257,258}));
    TestTrue(TEXT("All crew agree"),FiskVoyage::Unanimous(Crew,Crew));TestFalse(TEXT("Foreign vote does not replace a member"),FiskVoyage::Unanimous(Crew,{256,257,258,260}));
    TestTrue(TEXT("Solo can choose own presentation"),FiskVoyage::Unanimous({256},{256}));TestFalse(TEXT("Empty boat has no vote"),FiskVoyage::Unanimous({},{}));
    TestEqual(TEXT("Full return retains exact crossing time"),FiskVoyage::Crossing(1000000,40000,40000),1039500);
    TestEqual(TEXT("At boundary crosses now"),FiskVoyage::Crossing(1020000,1000,500),1020000);
    TestEqual(TEXT("Invalid duration rejected"),FiskVoyage::Crossing(0,0,40000),-1);return true;
}
namespace PassportTest
{
FFiskJournalEntry Entry(int32 Seed,int32 Species,int32 Ground)
{
    FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;const auto* S=FiskCatches::Find(Species);
    E.Fish.Species=Species;E.Fish.WeightGrams=S->MinGrams;E.Fish.LengthMm=FiskCatches::LengthForWeight(*S,E.Fish.WeightGrams);E.Fish.Value=FiskCatches::ValueForWeight(*S,E.Fish.WeightGrams);
    E.Context.Ground=Ground;E.Context.Depth=30;E.Context.Bait=Seed%3;E.Context.LandedAtMs=200000;E.Context.FightMs=30000;E.Context.LandedUtc=FDateTime(2026,11,6,12,0,0).ToIso8601();
    E.Context.BoatKind=1;E.Context.Equipment=7;E.Context.bHasSeaPoint=true;E.Context.SeaPoint=FiskAtlas::Find(Seed%9)->Point;E.bSold=true;return E;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskPassportTest,"Fiskelaget.Exploration.ConfirmedPassportCollections",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskPassportTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel Empty;const auto None=FiskSeaPassport::Build(Empty,{});TestEqual(TEXT("Fourteen milestones"),None.Num(),14);for(const auto& R:None) { TestFalse(TEXT("No unconfirmed progress"),R.Complete()); }
    TArray<FFiskJournalEntry> Entries;for(int32 I=0;I<10;++I) { Entries.Add(PassportTest::Entry(I,I%7,I%3)); }
    FFiskJournalModel J;TestTrue(TEXT("Valid historic fixture"),J.Restore(Entries));TArray<FFiskExpeditionRecord> Records;
    for(int32 I=0;I<5;++I) { FFiskExpeditionRecord R;R.DayId=FGuid::NewGuid();R.Goal=I;R.Weather=I%3;R.bCompleted=true;Records.Add(R); }
    const FFiskExpeditionRecord Duplicate=Records[0];Records.Add(Duplicate);const auto Rows=FiskSeaPassport::Build(J,Records);
    TestTrue(TEXT("Species, sold fish and atlas collections earned"),Rows[1].Complete() && Rows[2].Complete() && Rows[3].Complete() && Rows[7].Complete());
    TestTrue(TEXT("All-weather expedition collection"),Rows[11].Complete());TestEqual(TEXT("Duplicate medal never advances twice"),Rows[10].Current,static_cast<int64>(5));
    for(auto& E:Entries) { E.Context.bHasSeaPoint=false;E.bSold=false; }TestTrue(TEXT("Legacy entries still load"),J.Restore(Entries));const auto Legacy=FiskSeaPassport::Build(J,{});
    TestEqual(TEXT("Missing old coordinates never invent visits"),Legacy[7].Current,static_cast<int64>(0));TestEqual(TEXT("Lost catch does not advance delivered weight"),Legacy[4].Current,static_cast<int64>(0));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCsvTest,"Fiskelaget.Exploration.JournalCsvExport",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCsvTest::RunTest(const FString& Parameters)
{
    const FString Dir=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Automation"),FGuid::NewGuid().ToString(EGuidFormats::Digits));FFiskJournalModel J;FString File;
    auto E=PassportTest::Entry(0,0,0);TestTrue(TEXT("Confirmed entry"),J.Restore({E}));TestTrue(TEXT("Create CSV"),FiskSeaPassport::Export(Dir,J,File));
    FString Text;FFileHelper::LoadFileToString(Text,*File);TestTrue(TEXT("Own catch ID exported"),Text.Contains(E.CatchId.ToString(EGuidFormats::Digits)));
    TArray<FString> Lines;Text.ParseIntoArrayLines(Lines);TestEqual(TEXT("Header and one record"),Lines.Num(),2);TArray<FString> Columns;Lines[1].ParseIntoArray(Columns,TEXT(";"),false);TestEqual(TEXT("Sixteen stable columns"),Columns.Num(),16);
    FString Other;TestTrue(TEXT("Export creates a fresh name"),FiskSeaPassport::Export(Dir,J,Other));TestTrue(TEXT("Previous export preserved"),Other!=File && IFileManager::Get().FileExists(*File));
    IFileManager::Get().Delete(*File);IFileManager::Get().Delete(*Other);IFileManager::Get().DeleteDirectory(*Dir);return true;
}
#endif
