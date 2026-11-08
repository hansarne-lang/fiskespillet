#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
bool FiskGrounds::MakeSeaPoint(FVector Point,int32& Habitat,int32& TravelMs)
{
    const TArray<FVector> Path=FiskRouteGeometry::Path(Point);if(Path.IsEmpty()) { return false; }
    Habitat=0;for(int32 I=1;I<Count;++I) { if(FVector::DistSquared(Point,Find(I)->Position)<FVector::DistSquared(Point,Find(Habitat)->Position)) { Habitat=I; } }
    const FFiskFishingGround& G=*Find(Habitat);
    TravelMs=FMath::Clamp(FMath::RoundToInt(G.TravelMs*FiskRouteGeometry::Length(Path)/G.Position.Size()),4000,150000);return true;
}
const FFiskFishingGround* FiskGrounds::Find(int32 Id)
{
    static const FFiskFishingGround Grounds[] = {
        {TEXT("Korallrevet"),FVector(20000,0,0),12000,60,20,40},
        {TEXT("Undervannsskrenten"),FVector(60000,24000,0),40000,120,60,90},
        {TEXT("Åpent hav"),FVector(100000,-75000,0),75000,250,20,60},
        {TEXT("Mangrovebanken"),FVector(35000,-60000,0),42000,80,20,50},
        {TEXT("Dyphavsbanken"),FVector(140000,85000,0),110000,400,80,120}
    };
    return Id>=0 && Id<Count ? &Grounds[Id] : nullptr;
}
int32 FiskGrounds::CrossingMs(int32 ReturnStartMs,int32 Id)
{
    const FFiskFishingGround* Ground=Find(Id);
    if(!Ground || ReturnStartMs<0) { return -1; }
    const int32 LegMs=FMath::CeilToInt(Ground->TravelMs*(1.0-500.0/Ground->Position.Size()));
    return ReturnStartMs>MAX_int32-LegMs ? -1 : ReturnStartMs+LegMs;
}
int32 FiskGrounds::ReturnMarginMs(int32 NowMs,int32 Progress,int32 Id)
{
    const FFiskFishingGround* Ground=Find(Id);
    if(!Ground || NowMs<0 || Progress<-1 || Progress>200) { return MIN_int32; }
    // Conservative estimate to the dock center; actual circle crossing is earlier.
    int64 HomeMs=NowMs;
    if(Progress==-1) { HomeMs=FMath::Max(NowMs,120000)+2LL*Ground->TravelMs; }
    else if(Progress<100) { HomeMs+=FMath::DivideAndRoundUp<int64>((100LL-Progress)*Ground->TravelMs,100)+Ground->TravelMs; }
    else if(Progress==100) { HomeMs+=Ground->TravelMs; }
    else if(Progress<200) { const int64 Left=Progress==101?100:200-Progress;HomeMs+=FMath::DivideAndRoundUp<int64>(Left*Ground->TravelMs,100); }
    return static_cast<int32>(FMath::Clamp<int64>(1020000LL-HomeMs,MIN_int32,MAX_int32));
}
