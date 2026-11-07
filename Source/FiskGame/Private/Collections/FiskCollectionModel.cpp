#include "Collections/FiskCollectionModel.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskRigSettings.h"
#include "Career/FiskSeaPassport.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
const TArray<FFiskCollectionGoal>& FiskCollections::Catalog()
{
    static const TArray<FFiskCollectionGoal> Goals=[](){TArray<FFiskCollectionGoal> G;
        for(int32 I=0;I<FiskCatches::Count;++I) { G.Add({FString(FiskCatches::Find(I)->Name)+TEXT("-samler"),TEXT("Land tre bekreftede fisk av arten"),EFiskCollectionRule::Species,I,3}); }
        for(int32 I=0;I<FiskGrounds::Count;++I) { G.Add({FiskGrounds::Find(I)->Name,TEXT("Land to bekreftede fisk fra habitatet"),EFiskCollectionRule::Ground,I,2}); }
        for(int32 I=0;I<3;++I) { G.Add({FFiskRigRules::BaitName(I),TEXT("Land tre bekreftede fisk med agnet"),EFiskCollectionRule::Bait,I,3}); }
        for(int32 I=0;I<3;++I) { G.Add({FString(TEXT("Program: "))+FFiskRigRules::ProgramName(I),TEXT("Land tre bekreftede fisk med programmet"),EFiskCollectionRule::Program,I,3}); }
        for(int32 W:{3000,10000,25000,50000,100000}) { G.Add({FString::Printf(TEXT("Minst %d kg"),W/1000),TEXT("Én bekreftet landet fisk over vektgrensen"),EFiskCollectionRule::Weight,W,1}); }
        G.Add({TEXT("Ti på kroken"),TEXT("Ti bekreftede landinger"),EFiskCollectionRule::Count,0,10});
        G.Add({TEXT("Trygt i havn"),TEXT("Ti bekreftede solgte fisk"),EFiskCollectionRule::Sold,0,10});
        G.Add({TEXT("Hundre kilo levert"),TEXT("100 kg bekreftet solgt fangst; fremgang i gram"),EFiskCollectionRule::DeliveredWeight,0,100000});
        G.Add({TEXT("Alle elleve"),TEXT("Oppdag alle elleve arter i egen bekreftede journal"),EFiskCollectionRule::Diversity,0,11});return G;
    }();return Goals;
}
int64 FiskCollections::Progress(const FFiskJournalModel& J,int32 Goal)
{
    if(!Catalog().IsValidIndex(Goal)) { return 0; }const auto& G=Catalog()[Goal];int64 N=0;TSet<int32> Species;
    for(const auto& E:J.GetEntries())
    {
        // Journal admission already validates each immutable entry.
        Species.Add(E.Fish.Species);
        switch(G.Rule)
        {
        case EFiskCollectionRule::Species:N+=E.Fish.Species==G.Value;break;
        case EFiskCollectionRule::Ground:N+=E.Context.Ground==G.Value;break;
        case EFiskCollectionRule::Bait:N+=E.Context.Bait==G.Value;break;
        case EFiskCollectionRule::Program:N+=E.Context.Program==G.Value;break;
        case EFiskCollectionRule::Weight:N+=E.Fish.WeightGrams>=G.Value;break;
        case EFiskCollectionRule::Count:++N;break;
        case EFiskCollectionRule::Sold:N+=E.bSold;break;
        case EFiskCollectionRule::DeliveredWeight:N+=E.bSold?E.Fish.WeightGrams:0;break;
        case EFiskCollectionRule::Diversity:break;
        }
    }return FMath::Min(G.Rule==EFiskCollectionRule::Diversity?static_cast<int64>(Species.Num()):N,G.Target);
}
bool FFiskCatchQuery::Valid() const
{ return Species>=-1 && Species<FiskCatches::Count && Ground>=-1 && Ground<FiskGrounds::Count && Status>=0 && Status<=2 && MinGrams>=0 && MinGrams<=250000 && Sort>=0 && Sort<=3; }
TArray<int32> FiskCollections::Search(const FFiskJournalModel& J,const FFiskCatchQuery& Q)
{
    TArray<int32> Rows;if(!Q.Valid()) { return Rows; }const auto& E=J.GetEntries();
    for(int32 I=0;I<E.Num();++I) { const auto& A=E[I];if((Q.Species<0 || A.Fish.Species==Q.Species) && (Q.Ground<0 || A.Context.Ground==Q.Ground) && (Q.Status==0 || (Q.Status==1?A.bSold:!A.bSold)) && A.Fish.WeightGrams>=Q.MinGrams) { Rows.Add(I); } }
    Rows.Sort([&](int32 A,int32 B){const auto& X=E[A];const auto& Y=E[B];
        if(Q.Sort==1 && X.Fish.WeightGrams!=Y.Fish.WeightGrams) { return X.Fish.WeightGrams>Y.Fish.WeightGrams; }
        if(Q.Sort==2 && X.Fish.Value!=Y.Fish.Value) { return X.Fish.Value>Y.Fish.Value; }
        if(Q.Sort==3 && X.Context.FightMs!=Y.Context.FightMs) { return X.Context.FightMs>Y.Context.FightMs; }
        if(X.Context.LandedUtc!=Y.Context.LandedUtc) { return X.Context.LandedUtc>Y.Context.LandedUtc; }return X.CatchId.ToString()<Y.CatchId.ToString();});return Rows;
}
FString FiskCollections::Html(const FFiskJournalModel& J,const FFiskCatchQuery& Q,const TArray<int32>& Pins)
{
    if(!Q.Valid()) { return {}; }FString S=TEXT("<!doctype html><html lang=\"nb\"><meta charset=\"utf-8\"><title>Fangstjakten</title><style>body{font:17px system-ui;max-width:1100px;margin:40px auto;padding:20px;background:#f4f8f7;color:#123e47}td,th{padding:10px;border-bottom:1px solid #abc;text-align:left}table{border-collapse:collapse;width:100%}</style><h1>Min fangstjakt</h1><p>Bare egen bekreftet journal. Ingen pengebonus. Landinger inkluderer tap ved retur; inntekt gjelder solgt fisk.</p><h2>Festede mål</h2><ul>");
    for(int32 Id:Pins) { if(Catalog().IsValidIndex(Id)) { S+=FString::Printf(TEXT("<li>%s: %lld / %lld</li>"),*Catalog()[Id].Name,static_cast<long long>(Progress(J,Id)),static_cast<long long>(Catalog()[Id].Target)); } }
    const auto Rows=Search(J,Q);S+=FString::Printf(TEXT("</ul><h2>%d bekreftede treff</h2><p>Artfilter %d · habitatfilter %d · status %d · minstevekt %d g</p><table><tr><th>Fangst-ID</th><th>Art</th><th>kg</th><th>Habitat</th><th>Status</th><th>Solgt verdi</th></tr>"),Rows.Num(),Q.Species,Q.Ground,Q.Status,Q.MinGrams);
    for(int32 I:Rows) { const auto& E=J.GetEntries()[I];S+=FString::Printf(TEXT("<tr><td>%s</td><td>%s</td><td>%.1f</td><td>%s</td><td>%s</td><td>%d</td></tr>"),*E.CatchId.ToString(EGuidFormats::Digits),FiskCatches::Find(E.Fish.Species)->Name,E.Fish.WeightGrams/1000.0,FiskGrounds::Find(E.Context.Ground)->Name,E.bSold?TEXT("Solgt"):TEXT("Tapt ved retur"),E.bSold?E.Fish.Value:0); }
    return S+TEXT("</table></html>");
}
bool FiskCollections::Export(const FString& Dir,const FFiskJournalModel& J,const FFiskCatchQuery& Q,const TArray<int32>& Pins,FString& File)
{
    if(!Q.Valid() || !IFileManager::Get().MakeDirectory(*Dir,true)) { return false; }const FString Base=FPaths::Combine(Dir,TEXT("FISKELAGET-fangstjakt-")+FGuid::NewGuid().ToString(EGuidFormats::Digits));
    TArray<FFiskJournalEntry> Entries;for(int32 I:Search(J,Q)) { Entries.Add(J.GetEntries()[I]); }FFiskJournalModel Filtered;if(!Filtered.Restore(Entries)) { return false; }
    if(!FFileHelper::SaveStringToFile(FiskSeaPassport::Csv(Filtered),*(Base+TEXT(".csv")),FFileHelper::EEncodingOptions::ForceUTF8) || !FFileHelper::SaveStringToFile(Html(J,Q,Pins),*(Base+TEXT(".html")),FFileHelper::EEncodingOptions::ForceUTF8)) { return false; }File=Base+TEXT(".html");return true;
}
bool FFiskCollectionPins::Valid() const
{ TSet<int32> Seen;if(Values.Num()!=3) { return false; }for(int32 V:Values) { if(V<0 || V>=FiskCollections::Count || Seen.Contains(V)) { return false; }Seen.Add(V); }return true; }
bool FFiskCollectionPins::Set(int32 Slot,int32 Goal)
{ if(Slot<0 || Slot>=3 || Goal<0 || Goal>=FiskCollections::Count || Values.Num()!=3) { return false; }const int32 Existing=Values.Find(Goal);if(Existing>=0) { Swap(Values[Slot],Values[Existing]); }else { Values[Slot]=Goal; }return Valid(); }
