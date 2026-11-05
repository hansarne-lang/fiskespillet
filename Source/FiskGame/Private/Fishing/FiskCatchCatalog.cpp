#include "Fishing/FiskCatchCatalog.h"
const FFiskSpecies* FiskCatches::Find(int32 Species)
{
    static const FFiskSpecies Catalog[] = {
        {TEXT("Snapper"),2000,20000,300,900,5},
        {TEXT("Amberjack"),10000,50000,600,1600,8},
        {TEXT("Gulfinnet tunfisk"),30000,120000,1100,2200,12}
    };
    return Species>=0 && Species<3 ? &Catalog[Species] : nullptr;
}
int32 FiskCatches::LengthForWeight(const FFiskSpecies& Species,int32 Grams)
{
    return Species.MinLengthMm+static_cast<int32>(static_cast<int64>(Grams-Species.MinGrams)
        *(Species.MaxLengthMm-Species.MinLengthMm)/(Species.MaxGrams-Species.MinGrams));
}
int32 FiskCatches::ValueForWeight(const FFiskSpecies& Species,int32 Grams)
{
    return static_cast<int32>((static_cast<int64>(Grams)*Species.PricePerKg+500)/1000);
}
FFiskCatchData FiskCatches::Roll(int32 Seed,int32 GroundId)
{
    FFiskCatchData Fish;
    Fish.Species=GroundId;
    if(!Find(GroundId)) { return Fish; }
    FRandomStream Random(Seed);
    // A habitat suggests a species; it must not reveal the confirmed catch.
    static const int32 Alternative[]={1,0,1};
    Fish.Species=Random.RandRange(0,99)<80?GroundId:Alternative[GroundId];
    const FFiskSpecies* Species=Find(Fish.Species);
    Fish.WeightGrams=Random.RandRange(Species->MinGrams/100,Species->MaxGrams/100)*100;
    Fish.LengthMm=LengthForWeight(*Species,Fish.WeightGrams);
    Fish.Value=ValueForWeight(*Species,Fish.WeightGrams);
    return Fish;
}
bool FiskCatches::IsValid(const FFiskCatchData& Fish)
{
    const FFiskSpecies* Species=Find(Fish.Species);
    return Species && Fish.WeightGrams>=Species->MinGrams && Fish.WeightGrams<=Species->MaxGrams
        && Fish.LengthMm==LengthForWeight(*Species,Fish.WeightGrams)
        && Fish.Value==ValueForWeight(*Species,Fish.WeightGrams);
}
