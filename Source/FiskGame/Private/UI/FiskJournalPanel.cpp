#include "UI/FiskJournalPanel.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskRigSettings.h"
#include "FiskFishingGround.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Career/FiskSeaPassport.h"
#include "UI/FiskUserSettings.h"
#include "Misc/Paths.h"

void UFiskJournalPanel::Toggle()
{
    bVisible=!bVisible;bPassport=false;Page=0;CachedRevision=-1;
    if(bVisible) { CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu(); }
}
void UFiskJournalPanel::TogglePassport()
{ bVisible=true;bPassport=true;Page=0;CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu(); }
bool UFiskJournalPanel::HandleKey(const FInputKeyEventArgs& Args)
{
    if(!bVisible) { return false; }
    if(Args.Event!=IE_Pressed && Args.Event!=IE_Repeat) { return true; }
    const FKey Key=Args.Key;
    if(Key==EKeys::L) { if(bPassport) { bVisible=false; }else { bPassport=true; }return true; }
    if(Key==EKeys::F10 && Args.Event==IE_Pressed)
    {
        FString File;const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
        const bool Saved=FiskSeaPassport::Export(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Exports")),GI->GetJournal(),File);
        ExportMessage=Saved?TEXT("Hele din bekreftede journal er lagret i Fiskelaget/Exports som CSV."):TEXT("Eksport kunne ikke lagres. Journalen er bevart.");
        UE_LOG(LogTemp,Display,TEXT("FISKELAGET_JOURNAL_EXPORT success=%d records=%d file=%s"),Saved,GI->GetJournal().GetEntries().Num(),*File);return true;
    }
    if(Key==EKeys::Escape || Key==EKeys::F4) { bVisible=false;return true; }
    if(Key==EKeys::F5)
    { CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->RetryJournalSave();return true; }
    if(Key==EKeys::F6) { bRecords=!bRecords;Page=0; }
    if(Key==EKeys::One) { Sort=EFiskJournalSort::Date; }
    if(Key==EKeys::Two) { Sort=EFiskJournalSort::Species; }
    if(Key==EKeys::Three) { Sort=EFiskJournalSort::Weight; }
    if(Key==EKeys::Four) { StatusFilter=(StatusFilter+1)%3; }
    if(Key==EKeys::Five) { SpeciesFilter=(SpeciesFilter+2)%(FiskCatches::Count+1)-1; }
    if(Key==EKeys::PageDown || Key==EKeys::Down) { Page=FMath::Min(Page+1,1024); }
    else if(Key==EKeys::PageUp || Key==EKeys::Up) { Page=FMath::Max(Page-1,0); }
    else if(Key==EKeys::One || Key==EKeys::Two || Key==EKeys::Three || Key==EKeys::Four || Key==EKeys::Five)
    { CachedRevision=-1;Page=0; }
    return true;
}
void UFiskJournalPanel::Draw(AHUD& HUD,UCanvas& Canvas)
{
    if(bPassport) { DrawPassport(HUD,Canvas);return; }
    const UFiskGameInstance* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    const FFiskJournalModel& Journal=GI->GetJournal();
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    HUD.DrawRect(FLinearColor(0.012f,0.045f,0.06f,0.98f),24*S,28*S,1232*S,642*S);
    HUD.DrawText(bRecords?TEXT("MINE ARTSREKORDER"):TEXT("MIN FANGSTJOURNAL"),FLinearColor::White,52*S,60*S,nullptr,2.0f*S);
    const FFiskJournalStats Stats=Journal.Stats();
    HUD.DrawText(FString::Printf(TEXT("Landet: %lld fisk | %.1f kg | Salgsinntekt: %lld | Største: %.1f kg"),
        static_cast<long long>(Stats.Count),Stats.WeightGrams/1000.0,static_cast<long long>(Stats.Income),Stats.BestGrams/1000.0),
        FLinearColor(0.5f,0.9f,0.8f),52*S,112*S,nullptr,1.05f*S);
    if(bRecords)
    {
        const int32 RecordPages=(FiskCatches::Count+2)/3;Page=FMath::Clamp(Page,0,RecordPages-1);
        for(int32 Species=Page*3;Species<FMath::Min(Page*3+3,FiskCatches::Count);++Species)
        {
            const FFiskJournalEntry* Best=Journal.Best(Species);const FFiskJournalStats PerSpecies=Journal.Stats(Species);
            const float Y=(186+104*(Species%3))*S;
            HUD.DrawText(FiskCatches::Find(Species)->Name,FLinearColor::White,52*S,Y,nullptr,1.4f*S);
            const FString Detail=Best?FString::Printf(TEXT("Største landet: %.1f kg / %.1f cm | %lld fisk totalt"),
                Best->Fish.WeightGrams/1000.0,Best->Fish.LengthMm/10.0,static_cast<long long>(PerSpecies.Count)):TEXT("Ingen bekreftet fangst ennå.");
            HUD.DrawText(Detail,FLinearColor(0.65f,0.85f,0.85f),52*S,Y+38*S,nullptr,1.05f*S);
        }
        HUD.DrawText(FString::Printf(TEXT("Artsrekorder side %d/%d | PgUp/PgDn: bla | Landede rekorder beholdes også ved sen retur."),Page+1,RecordPages),FLinearColor::White,52*S,530*S,nullptr,0.95f*S);
    }
    else
    {
        if(CachedRevision!=GI->GetJournalRevision())
        { Selected=Journal.Select(Sort,StatusFilter,SpeciesFilter);CachedRevision=GI->GetJournalRevision(); }
        const TCHAR* Status=StatusFilter==0?TEXT("Alle"):StatusFilter==1?TEXT("Solgt"):TEXT("Tapt ved retur");
        const TCHAR* Species=SpeciesFilter<0?TEXT("Alle arter"):FiskCatches::Find(SpeciesFilter)->Name;
        const TCHAR* SortName=Sort==EFiskJournalSort::Date?TEXT("Dato"):Sort==EFiskJournalSort::Species?TEXT("Art"):TEXT("Vekt");
        HUD.DrawText(FString::Printf(TEXT("1: Dato  2: Art  3: Vekt | Sortert: %s | 4: %s | 5: %s"),SortName,Status,Species),FLinearColor::White,52*S,154*S,nullptr,1.0f*S);
        const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Selected.Num(),5));Page=FMath::Clamp(Page,0,Pages-1);
        if(Selected.IsEmpty()) { HUD.DrawText(TEXT("Ingen poster i dette filteret. Nye fangster kommer først ved oppgjøret 17:00."),FLinearColor::White,52*S,216*S,nullptr,1.05f*S); }
        for(int32 I=0;I<5 && Page*5+I<Selected.Num();++I)
        {
            const FFiskJournalEntry& E=Journal.GetEntries()[Selected[Page*5+I]];const float Y=(204+68*I)*S;
            const FFiskFishingGround* Ground=FiskGrounds::Find(E.Context.Ground);
            HUD.DrawText(FString::Printf(TEXT("%s | %.1f kg | %.1f cm | %s | %d fiskepenger"),FiskCatches::Find(E.Fish.Species)->Name,
                E.Fish.WeightGrams/1000.0,E.Fish.LengthMm/10.0,E.bSold?TEXT("SOLGT"):TEXT("TAPT VED RETUR"),E.bSold?E.Fish.Value:0),FLinearColor::White,52*S,Y,nullptr,1.1f*S);
            HUD.DrawText(FString::Printf(TEXT("%s UTC | %s | %s / %d m / %s | Kamp %.1f s"),*E.Context.LandedUtc.Left(19).Replace(TEXT("T"),TEXT(" ")),
                Ground->Name,FFiskRigRules::BaitName(E.Context.Bait),E.Context.Depth,FFiskRigRules::ProgramName(E.Context.Program),E.Context.FightMs/1000.0),
                FLinearColor(0.65f,0.85f,0.85f),52*S,Y+28*S,nullptr,0.90f*S);
            HUD.DrawText(FString::Printf(TEXT("%s | Snøre %s | Snelle %s | Stang %s"),E.Context.BoatKind==1?TEXT("Offshorebåt"):TEXT("Startbåt"),
                E.Context.Equipment&1?TEXT("oppgradert"):TEXT("standard"),E.Context.Equipment&2?TEXT("oppgradert"):TEXT("standard"),E.Context.Equipment&4?TEXT("oppgradert"):TEXT("standard")),
                FLinearColor(.45f,.8f,.75f),52*S,Y+47*S,nullptr,.76f*S);
        }
        HUD.DrawText(FString::Printf(TEXT("Side %d/%d | PgUp/PgDn eller piler: bla | F6: artsrekorder"),Page+1,Pages),FLinearColor(0.5f,0.9f,0.8f),52*S,560*S,nullptr,1.0f*S);
    }
    HUD.DrawText(ExportMessage.IsEmpty()?GI->GetJournalMessage():ExportMessage,GI->IsJournalPending()?FLinearColor(1,0.8f,0.5f):FLinearColor(0.6f,0.85f,0.85f),52*S,598*S,nullptr,0.95f*S);
    HUD.DrawText(TEXT("F4 / Escape: lukk | F5: lagre igjen | F6: rekorder | L: sjøpass | F10: eksporter hele journalen"),FLinearColor::White,52*S,636*S,nullptr,0.9f*S);
}
void UFiskJournalPanel::DrawPassport(AHUD& HUD,UCanvas& C)
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const auto Rows=FiskSeaPassport::Build(GI->GetJournal(),GI->Expeditions);
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){HUD.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("MITT SJØPASS"),48,55,2.1f);
    int32 Done=0;for(const auto& R:Rows) { Done+=R.Complete()?1:0; }
    T(FString::Printf(TEXT("%d av 14 milepæler | Bekreftede opplevelser, ingen pengebonus"),Done),48,108,1.1f,Teal);
    T(TEXT("Dagens nye fremgang vises først etter oppgjøret 17:00."),48,143,.95f);
    for(int32 I=0;I<Rows.Num();++I)
    {
        const auto& R=Rows[I];const float X=48+(I/7)*600,Y=184+(I%7)*56;
        HUD.DrawRect(R.Complete()?FLinearColor(.12f,.3f,.29f):FLinearColor(.03f,.12f,.15f),X*S,Y*S,568*S,51*S);
        const FString Progress=I==4?FString::Printf(TEXT("%.1f / 100 kg"),FMath::Min(R.Current,R.Target)/1000.0):FString::Printf(TEXT("%lld / %lld"),static_cast<long long>(FMath::Min(R.Current,R.Target)),static_cast<long long>(R.Target));
        T(FString(R.Complete()?TEXT("+ "):TEXT("  "))+R.Name,X+12,Y+6,.97f,R.Complete()?Gold:Teal);T(Progress,X+420,Y+8,.85f);
        T(R.Detail,X+12,Y+29,.78f);
    }
    T(ExportMessage,48,598,.9f,Gold);T(TEXT("L / F4 / Escape: lukk | F10: eksporter bekreftet journal | Gamle poster uten sjøpunkt gir ingen atlasmerker"),48,638,.85f,Teal);
}
