#include "UI/FiskAnglerPanel.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "FiskFishingGround.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
void UFiskAnglerPanel::DrawSchool(AHUD& H,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,1),24*S,28*S,1232*S,650*S);T(TEXT("FISKESKOLEN"),48,55,2.f);
    T(TEXT("Lokal øving | Ingen fiskedag, penger, fangster eller karriererekorder"),48,107,1.08f,Teal);
    T(bControls?TEXT("KONTROLLØVELSE | Tab: treningskamp | Enter: begynn øvelsen på nytt"):TEXT("TRENINGSKAMP | Tab: kontroller | Piler: art | G: øvingsutstyr | Enter: start / start på nytt"),48,146,.98f);
    if(bControls)
    {
        const TCHAR* Steps[]={TEXT("Trykk venstre museknapp: løft stanga"),TEXT("Slipp venstre museknapp: senk stanga"),TEXT("Trykk høyre museknapp: sveiv"),TEXT("Slipp høyre museknapp: stopp sveiva"),TEXT("Musehjul opp: stram bremsen"),TEXT("Musehjul ned: løsne bremsen"),TEXT("Mellomrom: land når fisken er klar")};
        for(int32 I=0;I<7;++I) { const float Y=212+44*I;H.DrawRect(I==Lesson.Step?FLinearColor(.15f,.34f,.35f):FLinearColor(.035f,.12f,.15f),48*S,Y*S,1120*S,36*S);T(FString::Printf(TEXT("%s %d. %s"),I<Lesson.Step?TEXT("+"):TEXT(" "),I+1,Steps[I]),62,Y+9,1.05f,I<Lesson.Step?Teal:I==Lesson.Step?Gold:FLinearColor::White); }
        T(Lesson.Complete()?TEXT("Kontrollene er utført! Tab åpner en kamp med samme modell som i spillet."):TEXT("Utfør stegene i rekkefølge. Øvelsen endrer ingen ekte stenger."),48,548,1.03f,Gold);
    }
    else
    {
        const auto& Score=Data.Scores[Species*2+(Gear==7?1:0)];T(FString::Printf(TEXT("%s | %s"),FiskCatches::Find(Species)->Name,Gear==7?TEXT("Virtuelt oppgradert utstyr"):TEXT("Standard øvingsutstyr")),48,202,1.4f,Gold);
        T(FString::Printf(TEXT("Øvingsforsøk %d | Fullført %d | Beste fullførte %.1f s"),Score.Attempts,Score.Completed,Score.BestMs/1000.0),48,245,1.02f,Teal);
        T(FString(FiskCatches::FightTip(Species)).Left(115),48,282,.92f);
        const auto& F=Practice.Fight;const bool Running=Practice.HasStarted();const int32 Load=F.GetLoadPercent(),Limit=Practice.Equipment&1?115:100;
        T(Running?FString::Printf(TEXT("Line %.1f m | Brems %d%% | Belastning %d%% | Bruddgrense %d%%"),F.GetLine(),F.GetDragPercent(),Load,Limit):TEXT("Enter starter en treningskamp. Art og utstyr velges før start."),48,338,1.15f);
        H.DrawRect(FLinearColor(.05f,.15f,.18f),48*S,382*S,1120*S,24*S);H.DrawRect(Load>Limit?FLinearColor(1,.3f,.2f):Teal,48*S,382*S,1120*S*FMath::Clamp(Load/130.f,0.f,1.f),24*S);
        H.DrawLine((48+1120*Limit/130.f)*S,378*S,(48+1120*Limit/130.f)*S,412*S,Gold,3*S);
        FString Coach=TEXT("Løft, senk og sveiv i tur. Slipp under utras.");
        if(Practice.IsLanded()) { Coach=TEXT("ØVING FULLFØRT – dette er ingen karrierefangst."); }
        else if(Running && F.GetOutcome()==EFiskFightOutcome::LineBreak) { Coach=TEXT("SNØRET RØK – avlast tidligere. Enter prøver igjen."); }
        else if(Running && F.GetOutcome()==EFiskFightOutcome::Slack) { Coach=TEXT("FOR MYE SLAKK – hold kontakt med fisken. Enter prøver igjen."); }
        else if(Running && F.GetOutcome()==EFiskFightOutcome::ReadyToLand) { Coach=TEXT("KLAR TIL LANDING – trykk mellomrom."); }
        else if(F.IsSurging()) { Coach=TEXT("UTRAS – slipp sveiva og senk stanga."); }
        else if(F.IsRaised() && F.IsReeling()) { Coach=TEXT("SENK STANGA FØR DU SVEIVER – bruk pumpetak."); }
        else if(Load>Limit) { Coach=TEXT("HØY BELASTNING – løsne bremsen eller avlast stanga."); }
        T(Coach,48,438,1.2f,Gold);
        T(FString::Printf(TEXT("Tid %.1f s | Tid med ugunstig belastning %.1f s | Sveiving under utras %.1f s"),Practice.Seconds,Practice.BadLoadSeconds,Practice.SurgeReelSeconds),48,487,1.f,Teal);
        T(TEXT("Venstre: løft | Slipp: senk | Høyre: sveiv | Musehjul: brems | Mellomrom: land"),48,536,1.f);
    }
    T(Message,48,597,.9f,Gold);T(TEXT("F6 / Escape: tilbake til hovedmenyen | Resultater lagres bare som øving på denne profilen"),48,642,.94f,Teal);
}
void UFiskAnglerPanel::DrawWorkbench(AHUD& H,UCanvas& C)
{
    const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=PC->FindStarterBoat();const int32 Ground=B?B->GetGroundId():0;const auto* G=FiskGrounds::Find(Ground);const auto R=FiskAngler::Advice(Ground,Bait);
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("FISKERENS ARBEIDSBENK"),48,55,2.f);
    T(TEXT("Piler: målart | G: agnvalg | 1/2/3: velg egen stang | Enter: råd til valgt | F4: råd til alle"),48,110,.98f,Teal);
    T(FString::Printf(TEXT("Målart: %s"),FiskCatches::Find(Species)->Name),48,163,1.35f,Gold);
    T(FString::Printf(TEXT("Høyest artsandel i katalogen: %s"),FiskGrounds::Find(FiskAngler::BestHabitat(Species))->Name),48,204,1.03f);
    T(FString(FiskCatches::HabitatTip(Species)).Left(110),48,241,.96f,Teal);
    T(TEXT("Dette er katalogråd. Ingen fisk på kroken leses, og ingen art garanteres."),48,277,.94f);
    T(FString::Printf(TEXT("Nåværende habitat: %s | Bunn %d m"),G->Name,G->BottomDepth),48,325,1.12f,Gold);
    T(FString::Printf(TEXT("Råd: %s | %d m | %s"),FFiskRigRules::BaitName(R.Bait),R.Depth,FFiskRigRules::ProgramName(R.Program)),48,364,1.1f,Teal);
    T(TEXT("Dybde innen fiskesjiktet og program som passer agnet gir god tilpasning."),48,401,.9f);
    T(TEXT("Agnvalget endrer ikke artsfordelingen i denne prototypen."),48,431,.91f);
    for(int32 I=0;I<3;++I)
    {
        const auto* Rod=PC->GetRodAt(I);const auto& V=Rod->GetRig();const float Y=164+96*I;const bool Available=I<PC->GetRodCapacity();
        H.DrawRect(I==PC->GetSelectedRod()?FLinearColor(.14f,.3f,.32f):FLinearColor(.03f,.12f,.15f),825*S,Y*S,380*S,86*S);
        T(FString::Printf(TEXT("%d: STANG %d %s"),I+1,I+1,Available?TEXT(""):TEXT("[Ingen egen holder]")),840,Y+12,1.03f,Available?Gold:Teal);
        if(Available) { T(FString::Printf(TEXT("%s | %d m | %s"),FFiskRigRules::BaitName(V.Bait),V.Depth,FFiskRigRules::ProgramName(V.Program)),840,Y+42,.85f);T(Rod->CanConfigure()?TEXT("Kan konfigureres"):TEXT("Aktiv / ventetid – trekk inn først"),840,Y+64,.8f,Teal); }
    }
    const auto& L=Data.Loadouts[Slot];T(FString::Printf(TEXT("EGET STANGSETT %d: %s | %d stenger"),Slot+1,*L.Name,L.Rigs.Num()),48,498,1.12f,Gold);
    T(bRenaming?TEXT("Navn: ")+EditName+TEXT(" | Enter lagrer | Escape avbryter"):TEXT("Tab: bytt stangsett | F5: lagre alle egne oppsett | F8: hent samlet | F2: navn"),48,539,.98f,Teal);
    const FString Feedback=PC->GetActionFeedback();T(Feedback.IsEmpty()?Message:Feedback,48,589,.84f,Gold);
    T(TEXT("Q / Escape: lukk | Stangsett krever riktig antall egne holdere, fersk status og alle stenger trygge."),48,642,.94f);
}
