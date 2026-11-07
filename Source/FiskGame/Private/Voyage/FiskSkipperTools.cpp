#include "Voyage/FiskSkipperTools.h"
#include "Voyage/FiskVoyagePlan.h"
#include "World/FiskOceanAtlas.h"
#include "FiskRouteGeometry.h"
#include "FiskFishingGround.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
const TCHAR* FiskSkipperTools::TemplateName(int32 Id)
{ static const TCHAR* Names[]={TEXT("Revrunden"),TEXT("Mangroveekspedisjonen"),TEXT("Skrentene"),TEXT("Ytterhavet"),TEXT("Dyphavsrunden"),TEXT("Store havturen")};return Id>=0 && Id<TemplateCount?Names[Id]:TEXT("Ukjent tur"); }
TArray<FVector> FiskSkipperTools::Template(int32 Id)
{
    static const TArray<TArray<int32>> Marks={{0,3,4},{9,10,11},{1,5,6},{2,7,8},{12,13,14,15},{0,9,5,2,12}};TArray<FVector> P;
    if(Marks.IsValidIndex(Id)) { for(int32 M:Marks[Id]) { P.Add(FiskAtlas::Find(M)->Point); } }return P;
}
FFiskRouteBudget FiskSkipperTools::Budget(const TArray<FVector>& P,int32 Kind,int32 Start,int32 Stop)
{
    FFiskRouteBudget B;if(!FFiskVoyageRoute::Valid(P) || P.IsEmpty() || Kind<0 || Kind>1 || Start<0 || Start>1200000 || Stop<0 || Stop>600000) { return B; }
    FVector From=FVector::ZeroVector;int32 LastHome=0;
    for(const FVector& To:P)
    {
        int32 G=0,Home=0;if(!FiskGrounds::MakeSeaPoint(To,G,Home)) { return B; }const auto Path=FiskRouteGeometry::Between(From,To);if(Path.IsEmpty()) { return B; }
        const auto& Ground=*FiskGrounds::Find(G);const double Length=FiskRouteGeometry::Length(Path);const int32 Base=FMath::Clamp(FMath::RoundToInt(Ground.TravelMs*Length/Ground.Position.Size()),4000,150000);
        B.SailingMs+=Kind==1?FMath::Max(1,Base*4/5):Base;B.DistanceCm+=Length;LastHome=Kind==1?FMath::Max(1,Home*4/5):Home;From=To;
    }
    B.SailingMs+=LastHome;B.DistanceCm+=FiskRouteGeometry::Length(FiskRouteGeometry::Between(From,FVector::ZeroVector));B.FishingMs=static_cast<int64>(P.Num())*Stop;B.HomeAtMs=FMath::Max(Start,120000)+B.SailingMs+B.FishingMs;B.MarginMs=1020000-B.HomeAtMs;B.Valid=true;return B;
}
TArray<FVector> FiskSkipperTools::Optimize(const TArray<FVector>& P)
{
    if(!FFiskVoyageRoute::Valid(P)) { return {}; }TArray<FVector> Best=P,Work;TArray<bool> Used;Used.Init(false,P.Num());double BestLength=1.e30;
    TFunction<void()> Visit=[&](){if(Work.Num()==P.Num()) { double D=0;FVector From=FVector::ZeroVector;for(const auto& To:Work) { D+=FiskRouteGeometry::Length(FiskRouteGeometry::Between(From,To));From=To; }D+=FiskRouteGeometry::Length(FiskRouteGeometry::Between(From,FVector::ZeroVector));if(D<BestLength-.01) { BestLength=D;Best=Work; }return; }
        for(int32 I=0;I<P.Num();++I) { if(!Used[I]) { Used[I]=true;Work.Add(P[I]);Visit();Work.Pop();Used[I]=false; } }};Visit();return Best;
}
FString FiskSkipperTools::CompareHtml(const FFiskJournalModel& J,int32 A,int32 B)
{
    const auto Days=FiskTripBook::Days(J);if(!Days.IsValidIndex(A) || !Days.IsValidIndex(B) || A==B) { return {}; }const auto& X=Days[A];const auto& Y=Days[B];
    FString S=TEXT("<!doctype html><html lang=\"nb\"><meta charset=\"utf-8\"><title>Tursammenligning</title><style>body{font:18px system-ui;max-width:1000px;margin:40px auto;padding:20px;color:#123e47;background:#f4f8f7}td,th{padding:14px;border-bottom:1px solid #abc;text-align:left}table{border-collapse:collapse;width:100%}</style><h1>Mine bekreftede fangstturer</h1>");
    S+=FString::Printf(TEXT("<p>Tur A: dag %d (%s) · Tur B: dag %d (%s)</p><table><tr><th>Mål</th><th>A</th><th>B</th></tr>"),X.Number,*X.Id.ToString(EGuidFormats::Digits),Y.Number,*Y.Id.ToString(EGuidFormats::Digits));
    S+=FString::Printf(TEXT("<tr><td>Landinger</td><td>%d</td><td>%d</td></tr><tr><td>Solgt / tapt</td><td>%d / %d</td><td>%d / %d</td></tr><tr><td>Landet kg</td><td>%.1f</td><td>%.1f</td></tr><tr><td>Solgt inntekt</td><td>%lld</td><td>%lld</td></tr><tr><td>Største kg</td><td>%.1f</td><td>%.1f</td></tr><tr><td>Arter / habitater</td><td>%d / %d</td><td>%d / %d</td></tr><tr><td>Kamp min</td><td>%.1f</td><td>%.1f</td></tr></table>"),X.Count,Y.Count,X.Sold,X.Lost,Y.Sold,Y.Lost,X.Grams/1000.,Y.Grams/1000.,static_cast<long long>(X.Income),static_cast<long long>(Y.Income),X.Best/1000.,Y.Best/1000.,X.Species.Num(),X.Grounds.Num(),Y.Species.Num(),Y.Grounds.Num(),X.FightMs/60000.,Y.FightMs/60000.);
    return S+TEXT("<p>Bare egen bekreftet journal. Landet vekt inkluderer tap ved retur; penger gjelder bare solgt fisk. Sammenligningen gir ingen bonus.</p></html>");
}
bool FiskSkipperTools::ExportComparison(const FString& Dir,const FFiskJournalModel& J,int32 A,int32 B,FString& File)
{
    const FString Text=CompareHtml(J,A,B);if(Text.IsEmpty() || !IFileManager::Get().MakeDirectory(*Dir,true)) { return false; }const FString P=FPaths::Combine(Dir,TEXT("FISKELAGET-sammenligning-")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT(".html"));if(!FFileHelper::SaveStringToFile(Text,*P,FFileHelper::EEncodingOptions::ForceUTF8)) { return false; }File=P;return true;
}
