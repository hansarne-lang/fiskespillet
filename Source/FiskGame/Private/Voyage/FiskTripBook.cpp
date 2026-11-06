#include "Voyage/FiskTripBook.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskRigSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
void FFiskPhotoView::Clamp()
{
    Yaw=FMath::IsFinite(Yaw)?FMath::UnwindDegrees(Yaw):0;Pitch=FMath::IsFinite(Pitch)?FMath::Clamp(Pitch,5.0,70.0):20;
    Distance=FMath::IsFinite(Distance)?FMath::Clamp(Distance,800.0,5000.0):1800;Fov=FMath::IsFinite(Fov)?FMath::Clamp(Fov,35.0,100.0):60;
}
FVector FFiskPhotoView::Offset(double BoatYaw) const
{ FFiskPhotoView P=*this;P.Clamp();return FRotator(-P.Pitch,BoatYaw+P.Yaw,0).Vector()*-P.Distance; }
TArray<FFiskTripSummary> FiskTripBook::Days(const FFiskJournalModel& J)
{
    TArray<FFiskTripSummary> Result;
    for(const auto& E:J.GetEntries())
    {
        if(!E.IsValid()) { continue; }int32 I=Result.IndexOfByPredicate([&](const auto& D){return D.Id==E.DayId;});
        if(I<0) { I=Result.AddDefaulted();Result[I].Id=E.DayId;Result[I].Number=E.DayNumber;Result[I].Utc=E.Context.LandedUtc; }
        auto& D=Result[I];++D.Count;D.Sold+=E.bSold?1:0;D.Lost+=E.bSold?0:1;D.Grams+=E.Fish.WeightGrams;D.Income+=E.bSold?E.Fish.Value:0;
        D.Best=FMath::Max(D.Best,E.Fish.WeightGrams);D.FightMs+=E.Context.FightMs;D.Species.Add(E.Fish.Species);D.Grounds.Add(E.Context.Ground);D.Baits.Add(E.Context.Bait);
        if(E.Context.LandedUtc>D.Utc) { D.Utc=E.Context.LandedUtc; }
    }
    Result.Sort([](const auto& A,const auto& B){return A.Utc==B.Utc?A.Id.ToString()<B.Id.ToString():A.Utc>B.Utc;});return Result;
}
TArray<int32> FiskTripBook::Rows(const FFiskJournalModel& J,const FGuid& Day)
{ TArray<int32> Out;for(int32 I:J.Select(EFiskJournalSort::Date,0,-1)) { if(J.GetEntries()[I].IsValid() && J.GetEntries()[I].DayId==Day) { Out.Add(I); } }return Out; }
namespace { FString Esc(FString V) { return V.Replace(TEXT("&"),TEXT("&amp;")).Replace(TEXT("<"),TEXT("&lt;")).Replace(TEXT(">"),TEXT("&gt;")).Replace(TEXT("\""),TEXT("&quot;")); } }
FString FiskTripBook::Html(const FFiskJournalModel& J,const FGuid& Day)
{
    const auto Summaries=Days(J);const auto* D=Summaries.FindByPredicate([&](const auto& S){return S.Id==Day;});if(!D) { return {}; }
    FString S=TEXT("<!doctype html><html lang=\"nb\"><meta charset=\"utf-8\"><title>FISKELAGET – turrapport</title><style>body{font:17px system-ui;max-width:1100px;margin:40px auto;padding:20px;color:#123e47;background:#f4f8f7}table{border-collapse:collapse;width:100%}td,th{padding:12px;text-align:left;border-bottom:1px solid #abc}h1{color:#095c68}</style><h1>Min bekreftede fisketur</h1>");
    S+=FString::Printf(TEXT("<p>Dag %d · %s UTC</p><p>%d landet · %d solgt · %d tapt ved retur · %.1f kg landet · %lld fiskepenger</p><table><tr><th>Art</th><th>kg</th><th>cm</th><th>Status</th><th>Habitat</th><th>Agn / dybde</th><th>Kamp</th></tr>"),D->Number,*Esc(D->Utc.Left(19)),D->Count,D->Sold,D->Lost,D->Grams/1000.0,static_cast<long long>(D->Income));
    for(int32 I:Rows(J,Day)) { const auto& E=J.GetEntries()[I];S+=FString::Printf(TEXT("<tr><td>%s</td><td>%.1f</td><td>%.1f</td><td>%s</td><td>%s</td><td>%s / %d m</td><td>%.1f s</td></tr>"),*Esc(FiskCatches::Find(E.Fish.Species)->Name),E.Fish.WeightGrams/1000.0,E.Fish.LengthMm/10.0,E.bSold?TEXT("Solgt"):TEXT("Tapt ved retur"),*Esc(FiskGrounds::Find(E.Context.Ground)->Name),*Esc(FFiskRigRules::BaitName(E.Context.Bait)),E.Context.Depth,E.Context.FightMs/1000.0); }
    return S+TEXT("</table><p>Bare egen bekreftet journal. Ingen andre spilleres fangster eller uoppgjorte resultater. Landet vekt inkluderer fangst som gikk tapt ved retur; inntekt gjelder bare solgt fisk.</p></html>");
}
bool FiskTripBook::Export(const FString& Dir,const FFiskJournalModel& J,const FGuid& Day,FString& File)
{
    const FString Text=Html(J,Day);if(Text.IsEmpty() || !IFileManager::Get().MakeDirectory(*Dir,true)) { return false; }
    const FString P=FPaths::Combine(Dir,TEXT("FISKELAGET-tur-")+Day.ToString(EGuidFormats::Digits)+TEXT("-")+FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8)+TEXT(".html"));
    if(IFileManager::Get().FileExists(*P) || !FFileHelper::SaveStringToFile(Text,*P,FFileHelper::EEncodingOptions::ForceUTF8)) { return false; }File=P;return true;
}
