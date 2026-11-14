#include "UI/FiskJournalModel.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
namespace
{
FFiskCatchContext Context()
{
    FFiskCatchContext C;C.LandedAtMs=200000;C.FightMs=50000;C.LandedUtc=FDateTime(2026,11,5,11,0,0).ToIso8601();return C;
}
FFiskDayResult Receipt(bool Delivered=true)
{
    FFiskDayLedger Ledger;Ledger.Record(FGuid::NewGuid(),7,200000,FFiskCatchData(),Context());
    Ledger.Settle(1020000,Delivered?900000:1020001);return Ledger.GetResults()[0];
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalPrivacyTest,"Fiskelaget.Journal.PrivacyMetadataAndIdempotence",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalPrivacyTest::RunTest(const FString& Parameters)
{
    FFiskDayLedger Ledger;const FGuid CatchId=FGuid::NewGuid(),Day=FGuid::NewGuid();
    const FFiskCatchContext Detail=Context();
    TestTrue(TEXT("Record private metadata"),Ledger.Record(CatchId,7,200000,FFiskCatchData(),Detail));
    TestTrue(TEXT("No early result details"),Ledger.GetResults().IsEmpty());Ledger.Settle(1020000,900000);
    const FFiskDayResult Row=Ledger.GetResults()[0];FFiskJournalModel Journal;
    TestTrue(TEXT("Metadata unchanged through settlement"),Row.Catches[0].Context==Detail);
    TestTrue(TEXT("Closed reveal gate"),Journal.Apply(Day,1,1020000,false,Row)==EFiskJournalApply::Rejected);
    TestTrue(TEXT("Invalid time refused"),Journal.Apply(Day,1,-1,true,Row)==EFiskJournalApply::Rejected);
    TestTrue(TEXT("One accepted publication"),Journal.Apply(Day,1,1020000,true,Row)==EFiskJournalApply::Added);
    TestTrue(TEXT("Retry is unchanged"),Journal.Apply(Day,1,1200000,true,Row)==EFiskJournalApply::Unchanged);
    TestEqual(TEXT("Retry does not duplicate"),Journal.GetEntries().Num(),1);
    TestTrue(TEXT("Stored day and catch identity"),Journal.GetEntries()[0].DayId==Day && Journal.GetEntries()[0].CatchId==CatchId);
    TestTrue(TEXT("Own context preserved"),Journal.GetEntries()[0].Context==Detail);
    FFiskDayResult Conflicting=Row;Conflicting.Catches[0].Context.Program=1;
    TestTrue(TEXT("Conflicting replacement denied"),Journal.Apply(Day,1,1020000,true,Conflicting)==EFiskJournalApply::Rejected);
    TestEqual(TEXT("Original income retained"),Journal.Stats().Income,int64(Row.SessionBalance));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalStatsTest,"Fiskelaget.Journal.SortFilterAndLostRecords",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalStatsTest::RunTest(const FString& Parameters)
{
    TArray<FFiskJournalEntry> Data;
    const int32 Grams[]={2000,10000,30000},Lengths[]={300,600,1100},Prices[]={10,80,360};
    for(int32 S=0;S<3;++S)
    {
        FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=S+1;E.Context=Context();
        E.Context.LandedUtc=FDateTime(2026,11,5+S,11,0,0).ToIso8601();E.Fish.Species=S;E.Fish.WeightGrams=Grams[S];
        E.Fish.LengthMm=Lengths[S];E.Fish.Value=Prices[S];E.bSold=S!=1;Data.Add(E);
    }
    FFiskJournalModel Journal;TestTrue(TEXT("Restore valid journal"),Journal.Restore(Data));
    const FFiskJournalStats Stats=Journal.Stats();
    TestEqual(TEXT("All landed count including lost"),Stats.Count,int64(3));
    TestEqual(TEXT("All landed weight including lost"),Stats.WeightGrams,int64(42000));
    TestEqual(TEXT("Income only sold"),Stats.Income,int64(370));
    TestEqual(TEXT("Largest fish"),Stats.BestGrams,30000);
    TestEqual(TEXT("Lost fish keeps species record"),Journal.Best(1)->Fish.WeightGrams,10000);
    TestEqual(TEXT("Date descending"),Journal.Select(EFiskJournalSort::Date,0,-1)[0],2);
    TestEqual(TEXT("Weight descending"),Journal.Select(EFiskJournalSort::Weight,0,-1)[0],2);
    TestEqual(TEXT("Species alphabetical"),Journal.Select(EFiskJournalSort::Species,0,-1)[0],1);
    TestEqual(TEXT("Sold filter"),Journal.Select(EFiskJournalSort::Date,1,-1).Num(),2);
    TestEqual(TEXT("Lost filter"),Journal.Select(EFiskJournalSort::Date,2,-1).Num(),1);
    TestEqual(TEXT("Species filter"),Journal.Select(EFiskJournalSort::Date,0,0).Num(),1);
    TestEqual(TEXT("Combined empty filter"),Journal.Select(EFiskJournalSort::Date,1,1).Num(),0);
    FFiskJournalModel Late;const FFiskDayResult Lost=Receipt(false);
    TestTrue(TEXT("Late receipt accepted as lost"),Late.Apply(FGuid::NewGuid(),1,1020000,true,Lost)==EFiskJournalApply::Added);
    TestEqual(TEXT("Late receipt has zero income"),Late.Stats().Income,int64(0));TestEqual(TEXT("Late record retained"),Late.Stats().BestGrams,2000);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskJournalAtomicTest,"Fiskelaget.Journal.AtomicBatchAndCapacity",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskJournalAtomicTest::RunTest(const FString& Parameters)
{
    FFiskJournalModel Journal;FFiskDayResult Batch=Receipt();FFiskCatchResult Bad=Batch.Catches[0];Bad.CatchId=FGuid::NewGuid();Bad.Context.Depth=999;
    Batch.Catches.Add(Bad);Batch.DeliveredFish=2;Batch.DeliveredWeightGrams=4000;Batch.SessionBalance=20;
    TestTrue(TEXT("Invalid member rejects whole batch"),Journal.Apply(FGuid::NewGuid(),1,1020000,true,Batch)==EFiskJournalApply::Rejected);
    TestTrue(TEXT("No partial update"),Journal.GetEntries().IsEmpty());
    FFiskDayResult Duplicate=Receipt();const FFiskCatchResult Repeated=Duplicate.Catches[0];Duplicate.Catches.Add(Repeated);
    Duplicate.DeliveredFish=2;Duplicate.DeliveredWeightGrams=4000;Duplicate.SessionBalance=20;
    TestTrue(TEXT("Duplicate inside receipt rejected"),Journal.Apply(FGuid::NewGuid(),1,1020000,true,Duplicate)==EFiskJournalApply::Rejected);
    TArray<FFiskJournalEntry> Full;Full.Reserve(FFiskJournalModel::Capacity);
    for(int32 I=0;I<FFiskJournalModel::Capacity;++I)
    { FFiskJournalEntry E;E.CatchId=FGuid::NewGuid();E.DayId=FGuid::NewGuid();E.DayNumber=1;E.Context=Context();E.bSold=true;Full.Add(E); }
    TestTrue(TEXT("Exact capacity allowed"),Journal.Restore(Full));
    TestTrue(TEXT("No silent eviction on overflow"),Journal.Apply(FGuid::NewGuid(),2,1020000,true,Receipt())==EFiskJournalApply::Rejected);
    TestEqual(TEXT("Existing records kept"),Journal.GetEntries().Num(),FFiskJournalModel::Capacity);
    return true;
}
#endif
