#include "Career/FiskSeaPassport.h"
#include "World/FiskOceanAtlas.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskRigSettings.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
TArray<FFiskSeaMilestone> FiskSeaPassport::Build(const FFiskJournalModel& J,const TArray<FFiskExpeditionRecord>& Expeditions)
{
    TSet<int32> Species,Habitats,Baits,Marks,Goals,Weather;TSet<FGuid> Days;int64 Sold=0,Grams=0,Offshore=0,Upgraded=0;
    for(const auto& E:J.GetEntries())
    {
        if(!E.IsValid()) { continue; }Species.Add(E.Fish.Species);Habitats.Add(E.Context.Ground);Baits.Add(E.Context.Bait);Days.Add(E.DayId);
        Sold+=E.bSold?1:0;Grams+=E.bSold?E.Fish.WeightGrams:0;Offshore+=E.Context.BoatKind==1?1:0;Upgraded+=E.Context.Equipment==7?1:0;
        if(E.Context.bHasSeaPoint) { const int32 Mark=FiskAtlas::Nearest(E.Context.SeaPoint);if(Mark>=0) { Marks.Add(Mark); } }
    }
    int64 Medals=0;TSet<FGuid> Seen;
    for(const auto& E:Expeditions) { if(E.IsValid() && E.bCompleted && !Seen.Contains(E.DayId)) { Seen.Add(E.DayId);++Medals;Goals.Add(E.Goal);Weather.Add(E.Weather); } }
    return {
        {TEXT("Første fangst"),TEXT("Én bekreftet landet fisk"),J.GetEntries().Num(),1},
        {TEXT("Artsutforsker"),TEXT("Tre ulike bekreftede arter"),Species.Num(),3},
        {TEXT("Hele artsboken"),TEXT("Alle sju arter oppdaget"),Species.Num(),7},
        {TEXT("Trygg levering"),TEXT("Ti fisk solgt ved rettidig retur"),Sold,10},
        {TEXT("Hundre kilo"),TEXT("100 kg bekreftet levert fangst"),Grams,100000},
        {TEXT("Tre havmiljøer"),TEXT("Fangst fra alle tre habitatene"),Habitats.Num(),3},
        {TEXT("Agnmester"),TEXT("Bekreftet fangst med tre agntyper"),Baits.Num(),3},
        {TEXT("Kjentmann"),TEXT("Bekreftet fangst ved fem atlasmerker"),Marks.Num(),5},
        {TEXT("Offshorefisker"),TEXT("Fem bekreftede fangster i offshorebåt"),Offshore,5},
        {TEXT("Fullt rigget"),TEXT("Fangst med alle tre utstyrsoppgraderingene"),Upgraded,1},
        {TEXT("Ekspedisjonsveteran"),TEXT("Fem fullførte ekspedisjoner"),Medals,5},
        {TEXT("Allværsfisker"),TEXT("Fullført ekspedisjon i alle tre værtypene"),Weather.Num(),3}
    };
}
FString FiskSeaPassport::Csv(const FFiskJournalModel& J)
{
    FString Text=TEXT("Fangst-ID;Dag-ID;Dag;UTC;Art;Gram;Millimeter;Solgt;Salgsverdi;Habitat;Agn;Dybde_m;Program;Kamp_ms;Sjøpunkt_X_cm;Sjøpunkt_Y_cm\r\n");
    const auto Rows=J.Select(EFiskJournalSort::Date,0,-1);
    for(int32 Index:Rows)
    {
        const auto& E=J.GetEntries()[Index];if(!E.IsValid()) { continue; }
        Text+=FString::Printf(TEXT("%s;%s;%d;%s;%s;%d;%d;%s;%d;%s;%s;%d;%s;%d;%s;%s\r\n"),
            *E.CatchId.ToString(EGuidFormats::Digits),*E.DayId.ToString(EGuidFormats::Digits),E.DayNumber,*E.Context.LandedUtc,FiskCatches::Find(E.Fish.Species)->Name,
            E.Fish.WeightGrams,E.Fish.LengthMm,E.bSold?TEXT("Ja"):TEXT("Nei"),E.bSold?E.Fish.Value:0,FiskGrounds::Find(E.Context.Ground)->Name,
            FFiskRigRules::BaitName(E.Context.Bait),E.Context.Depth,FFiskRigRules::ProgramName(E.Context.Program),E.Context.FightMs,
            E.Context.bHasSeaPoint?*FString::FromInt(FMath::RoundToInt(E.Context.SeaPoint.X)):TEXT(""),E.Context.bHasSeaPoint?*FString::FromInt(FMath::RoundToInt(E.Context.SeaPoint.Y)):TEXT(""));
    }return Text;
}
bool FiskSeaPassport::Export(const FString& Directory,const FFiskJournalModel& J,FString& File)
{
    if(!IFileManager::Get().MakeDirectory(*Directory,true)) { return false; }
    const FString Candidate=FPaths::Combine(Directory,TEXT("FISKELAGET-journal-")+FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S-"))+FGuid::NewGuid().ToString(EGuidFormats::Digits).Left(8)+TEXT(".csv"));
    if(IFileManager::Get().FileExists(*Candidate) || !FFileHelper::SaveStringToFile(Csv(J),*Candidate,FFileHelper::EEncodingOptions::ForceUTF8)) { return false; }File=Candidate;return true;
}
