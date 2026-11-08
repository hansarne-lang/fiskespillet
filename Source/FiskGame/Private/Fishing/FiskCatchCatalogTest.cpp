#include "Fishing/FiskCatchCatalog.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCatchCatalogTest,"Fiskelaget.Content.SpeciesAndDeterminism",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCatchCatalogTest::RunTest(const FString& Parameters)
{
    for(int32 Ground=0;Ground<5;++Ground)
    {
        TSet<int32> Weights;int32 Counts[FiskCatches::Count]={};
        for(int32 Seed=0;Seed<1024;++Seed)
        {
            const FFiskCatchData A=FiskCatches::Roll(Seed,Ground),B=FiskCatches::Roll(Seed,Ground);
            TestTrue(TEXT("Generated catch is valid"),FiskCatches::IsValid(A));
            TestTrue(TEXT("Only habitat-supported species"),FiskCatches::HabitatChance(Ground,A.Species)>0);
            ++Counts[A.Species];
            TestEqual(TEXT("Same seed retains species"),A.Species,B.Species);
            TestEqual(TEXT("Same cast retains weight"),A.WeightGrams,B.WeightGrams);
            TestEqual(TEXT("Same cast retains length"),A.LengthMm,B.LengthMm);
            TestEqual(TEXT("Same cast retains value"),A.Value,B.Value);
            Weights.Add(A.WeightGrams);
        }
        TestTrue(TEXT("Different casts vary"),Weights.Num()>10);
        int32 Sum=0;for(int32 Species=0;Species<FiskCatches::Count;++Species)
        { const int32 Chance=FiskCatches::HabitatChance(Ground,Species);Sum+=Chance;TestTrue(TEXT("Observed habitat distribution within sampling tolerance"),FMath::Abs(Counts[Species]-1024.0*Chance/100)<80); }
        TestEqual(TEXT("Distribution conserves probability"),Sum,100);
    }
    TestFalse(TEXT("Unknown habitat rejected"),FiskCatches::IsValid(FiskCatches::Roll(7,-1)));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FFiskCatchValidationTest,"Fiskelaget.Content.CatchValidation",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FFiskCatchValidationTest::RunTest(const FString& Parameters)
{
    for(int32 Id=0;Id<FiskCatches::Count;++Id)
    {
        const FFiskSpecies& Species=*FiskCatches::Find(Id);
        for(int32 Grams:{Species.MinGrams,Species.MaxGrams})
        {
            FFiskCatchData Fish;Fish.Species=Id;Fish.WeightGrams=Grams;
            Fish.LengthMm=FiskCatches::LengthForWeight(Species,Grams);Fish.Value=FiskCatches::ValueForWeight(Species,Grams);
            TestTrue(TEXT("Inclusive species boundaries"),FiskCatches::IsValid(Fish));
            FFiskCatchData Bad=Fish;++Bad.Value;TestFalse(TEXT("Forged price"),FiskCatches::IsValid(Bad));
            Bad=Fish;--Bad.LengthMm;TestFalse(TEXT("Forged size"),FiskCatches::IsValid(Bad));
        }
        FFiskCatchData Bad=FiskCatches::Roll(1,Id);Bad.WeightGrams=MAX_int32;
        TestFalse(TEXT("Extreme weight fails before arithmetic"),FiskCatches::IsValid(Bad));
    }
    return true;
}
#endif
