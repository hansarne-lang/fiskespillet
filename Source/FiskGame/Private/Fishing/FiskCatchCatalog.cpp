#include "Fishing/FiskCatchCatalog.h"
const FFiskSpecies* FiskCatches::Find(int32 Species)
{
    static const FFiskSpecies Catalog[] = {
        {TEXT("Snapper"),2000,20000,300,900,5},
        {TEXT("Amberjack"),10000,50000,600,1600,8},
        {TEXT("Gulfinnet tunfisk"),30000,120000,1100,2200,12},
        {TEXT("Grouper"),4000,40000,400,1300,9},
        {TEXT("Mahi-mahi"),3000,30000,500,1600,10},
        {TEXT("Wahoo"),8000,70000,800,2000,15},
        {TEXT("Blå marlin"),60000,250000,1800,4000,25}
    };
    return Species>=0 && Species<Count ? &Catalog[Species] : nullptr;
}
int32 FiskCatches::HabitatChance(int32 Ground,int32 Species)
{
    static constexpr int32 Chances[3][Count]={{50,10,2,28,8,2,0},{12,40,10,15,12,10,1},{3,15,42,2,12,21,5}};
    return Ground>=0 && Ground<3 && Find(Species)?Chances[Ground][Species]:0;
}
const TCHAR* FiskCatches::HabitatTip(int32 Species)
{
    static const TCHAR* Tips[]={TEXT("Korallrev | 20-40 m | Naturlig agnfisk, rolig"),TEXT("Undervannsskrent | 60-90 m | Agnfisk / metalljigg"),TEXT("Åpent hav | 20-60 m | Blekksprut, pulserende"),TEXT("Rev og skrent | 30-90 m | Naturlig agnfisk, rolig"),TEXT("Revkant og åpent vann | 20-50 m | Blekksprut, aktivt"),TEXT("Åpent hav | 20-60 m | Metalljigg, aktivt"),TEXT("Sjeldent i åpent hav | 30-70 m | Blekksprut / agnfisk")};
    return Find(Species)?Tips[Species]:TEXT("Ukjent art");
}
const TCHAR* FiskCatches::FightTip(int32 Species)
{
    static const TCHAR* Tips[]={TEXT("Korte utras og lett pumping"),TEXT("Kraftige dykk og jevn motstand"),TEXT("Lange utras og høy utholdenhet"),TEXT("Tunge, korte dykk; hold kontroll på slakk"),TEXT("Raske, hyppige retningsendringer"),TEXT("Svært raske, korte utras"),TEXT("Eksplosive, lange utras og hard motstand")};
    return Find(Species)?Tips[Species]:TEXT("Ukjent kampmønster");
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
    if(GroundId<0 || GroundId>2) { Fish.Species=-1;return Fish; }
    FRandomStream Random(Seed);
    // A habitat suggests a species; it must not reveal the confirmed catch.
    int32 Roll=Random.RandRange(0,99);
    for(int32 I=0;I<Count;++I) { Roll-=HabitatChance(GroundId,I);if(Roll<0) { Fish.Species=I;break; } }
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
