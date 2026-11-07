#include "UI/FiskCollectionPanel.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskUserSettings.h"
#include "Boats/FiskBoardingController.h"
#include "FiskFishingGround.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Misc/Paths.h"
FString UFiskCollectionPanel::File() const
{ return FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Collections-")+Profile.ToString(EGuidFormats::Digits)+TEXT(".ini")); }
void UFiskCollectionPanel::Open()
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());if(Profile!=GI->Career.Id) { Profile=GI->Career.Id;Pins.Load(File(),Profile); }
    bVisible=true;Page=Row=ResultPage=0;Query=FFiskCatchQuery();Message=Pins.bBlocked?TEXT("Målfilen er bevart og blokkert. Samlinger og søk virker fortsatt."):Pins.bRecovered?TEXT("Festede mål hentet fra backup. F5 reparerer hovedfilen."):TEXT("Fremgang hentes fra egen bekreftede journal. Ingen pengebonus.");
    CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu();
}
void UFiskCollectionPanel::SavePins()
{ Message=Pins.Save(File(),Profile)?TEXT("Tre personlige mål er lagret."):TEXT("Lagring ble avvist. Tidligere filer er bevart."); }
bool UFiskCollectionPanel::HandleKey(const FInputKeyEventArgs& A)
{
    if(!bVisible) { return false; }if(A.Event!=IE_Pressed && A.Event!=IE_Repeat) { return true; }const FKey K=A.Key;
    if(K==EKeys::Escape || K==EKeys::J) { bVisible=false;return true; }
    if(K==EKeys::Tab) { Page=(Page+1)%4;ResultPage=0;return true; }
    if(K==EKeys::F5) { SavePins();return true; }
    if(K==EKeys::F10) { const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());FString Path;Message=FiskCollections::Export(FPaths::Combine(FFiskUserSettings::DataDirectory(),TEXT("Exports")),GI->GetJournal(),Page==2?Query:FFiskCatchQuery(),Pins.Values,Path)?TEXT("HTML og CSV er lagret i Fiskelaget/Exports. Søkefiltrene brukes i søkefanen."):TEXT("Rapport kunne ikke lagres.");return true; }
    if(Page<2)
    {
        if(K==EKeys::Up) { Row=(Row+FiskCollections::Count-1)%FiskCollections::Count; }if(K==EKeys::Down) { Row=(Row+1)%FiskCollections::Count; }
        if(K==EKeys::Home) { Row=0; }if(K==EKeys::End) { Row=FiskCollections::Count-1; }
        if(K==EKeys::PageDown) { Row=FMath::Min(Row+6,FiskCollections::Count-1); }if(K==EKeys::PageUp) { Row=FMath::Max(Row-6,0); }
        const FKey Keys[]={EKeys::One,EKeys::Two,EKeys::Three};for(int32 I=0;I<3;++I) { if(K==Keys[I]) { auto Candidate=Pins;if(Candidate.Set(I,Row) && Candidate.Save(File(),Profile)) { Pins=Candidate;Message=TEXT("Valgt samling festet. Et allerede festet mål bytter plass."); }else { Message=TEXT("Målet kunne ikke lagres. Tidligere mål er beholdt."); } } }
    }
    else if(Page==2)
    {
        if(K==EKeys::One) { Query.Species=(Query.Species+2)%(FiskCatches::Count+1)-1;ResultPage=0; }
        if(K==EKeys::Two) { Query.Ground=(Query.Ground+2)%(FiskGrounds::Count+1)-1;ResultPage=0; }
        if(K==EKeys::Three) { Query.Status=(Query.Status+1)%3;ResultPage=0; }
        if(K==EKeys::Four) { const int32 Weights[]={0,5000,10000,25000,50000};int32 Index=0;for(int32 I=0;I<5;++I) { if(Query.MinGrams==Weights[I]) { Index=I; } }Query.MinGrams=Weights[(Index+1)%5];ResultPage=0; }
        if(K==EKeys::Five) { Query.Sort=(Query.Sort+1)%4;ResultPage=0; }
        if(K==EKeys::Home) { Query=FFiskCatchQuery();ResultPage=0; }
        if(K==EKeys::Down || K==EKeys::PageDown) { ++ResultPage; }if(K==EKeys::Up || K==EKeys::PageUp) { ResultPage=FMath::Max(0,ResultPage-1); }
    }return true;
}
void UFiskCollectionPanel::Draw(AHUD& H,UCanvas& C)
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const auto& J=GI->GetJournal();const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Z=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Z*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("FANGSTJAKTEN"),48,55,2.1f);int32 Done=0;for(int32 I=0;I<FiskCollections::Count;++I) { Done+=FiskCollections::Progress(J,I)>=FiskCollections::Catalog()[I].Target; }
    T(FString::Printf(TEXT("%d av 30 samlinger | Bare dine bekreftede fangster etter oppgjøret"),Done),48,108,1.05f,Teal);
    const TCHAR* Tabs[]={TEXT("SAMLINGER"),TEXT("MINE TRE MÅL"),TEXT("FANGSTSØK"),TEXT("OVERSIKT")};T(FString::Printf(TEXT("Tab: %s | J / Escape: lukk | F5: lagre mål | F10: HTML + CSV"),Tabs[Page]),48,148,.98f);
    if(Page<2)
    {
        const int32 First=Page==0?(Row/6)*6:0,Number=Page==0?6:3;
        for(int32 I=0;I<Number;++I)
        {
            const int32 Id=Page==0?First+I:Pins.Values[I];const auto& G=FiskCollections::Catalog()[Id];const auto N=FiskCollections::Progress(J,Id);const float Y=202+I*(Page==0?57:105);
            H.DrawRect(Id==Row?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),48*S,Y*S,1160*S,(Page==0?51:95)*S);
            T(FString::Printf(TEXT("%s %d. %s"),N>=G.Target?TEXT("+"):TEXT(" "),Id+1,*G.Name),60,Y+8,1.1f,N>=G.Target?Gold:Teal);
            T(FString::Printf(TEXT("%lld / %lld%s"),static_cast<long long>(N),static_cast<long long>(G.Target),G.Rule==EFiskCollectionRule::DeliveredWeight?TEXT(" g"):TEXT("")),980,Y+10,.98f,Gold);T(G.Detail,60,Y+31,.84f);
            if(Page==1) { T(FString::Printf(TEXT("Målplass %d | Piler velger ny samling; %d fester valget her"),I+1,I+1),60,Y+62,.9f); }
        }
        T(FString::Printf(TEXT("Valgt: %d. %s | Piler / PgUp/PgDn / Home / End: velg | 1–3: fest"),Row+1,*FiskCollections::Catalog()[Row].Name),48,566,.9f,Gold);
    }
    else if(Page==2)
    {
        const TCHAR* Status[]={TEXT("Alle"),TEXT("Solgt"),TEXT("Tapt")};const TCHAR* Order[]={TEXT("Nyeste"),TEXT("Tyngste"),TEXT("Fangstverdi"),TEXT("Lengste kamp")};
        T(FString::Printf(TEXT("1 Art: %s | 2 Habitat: %s | 3 Status: %s"),Query.Species<0?TEXT("Alle"):FiskCatches::Find(Query.Species)->Name,Query.Ground<0?TEXT("Alle"):FiskGrounds::Find(Query.Ground)->Name,Status[Query.Status]),48,200,.98f,Teal);
        T(FString::Printf(TEXT("4 Minst %.1f kg | 5 Sortering: %s | Home: nullstill"),Query.MinGrams/1000.0,Order[Query.Sort]),48,234,.98f);
        const auto Rows=FiskCollections::Search(J,Query);const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Rows.Num(),4));ResultPage=FMath::Clamp(ResultPage,0,Pages-1);
        for(int32 I=0;I<4 && ResultPage*4+I<Rows.Num();++I) { const auto& E=J.GetEntries()[Rows[ResultPage*4+I]];const float Y=292+59*I;T(FString::Printf(TEXT("%s | %.1f kg | %s | %s"),FiskCatches::Find(E.Fish.Species)->Name,E.Fish.WeightGrams/1000.0,FiskGrounds::Find(E.Context.Ground)->Name,E.bSold?TEXT("Solgt"):TEXT("Tapt ved retur")),48,Y,1.1f,Teal);T(FString::Printf(TEXT("Dag %d | %s UTC | Kamp %.1f s | Solgt verdi %d"),E.DayNumber,*E.Context.LandedUtc.Left(19),E.Context.FightMs/1000.0,E.bSold?E.Fish.Value:0),48,Y+26,.9f); }
        if(Rows.IsEmpty()) { T(TEXT("Ingen bekreftede treff med disse filtrene."),48,315,1.15f,Gold); }T(FString::Printf(TEXT("%d treff | Side %d/%d | PgUp/PgDn: bla"),Rows.Num(),ResultPage+1,Pages),48,566,.95f);
    }
    else
    {
        const auto A=J.Stats();TSet<int32> Species,Grounds;TSet<FGuid> Days;int32 Sold=0;int64 Fight=0;
        for(const auto& E:J.GetEntries()) { Species.Add(E.Fish.Species);Grounds.Add(E.Context.Ground);Days.Add(E.DayId);Sold+=E.bSold;Fight+=E.Context.FightMs; }
        const FString Values[]={FString::Printf(TEXT("%lld landinger | %d solgt | %lld tapt ved retur"),static_cast<long long>(A.Count),Sold,static_cast<long long>(A.Count-Sold)),FString::Printf(TEXT("%.1f kg landet | Største fisk %.1f kg"),A.WeightGrams/1000.0,A.BestGrams/1000.0),FString::Printf(TEXT("%lld fiskepenger fra solgt fangst"),static_cast<long long>(A.Income)),FString::Printf(TEXT("%d / 11 arter | %d / 4 habitater | %d fangstdager"),Species.Num(),Grounds.Num(),Days.Num()),FString::Printf(TEXT("Samlet kamptid %.1f min | Snitt %.1f s per landing"),Fight/60000.0,A.Count?Fight/1000.0/A.Count:0)};
        for(int32 I=0;I<5;++I) { T(Values[I],48,222+62*I,1.3f,I%2?FLinearColor::White:Teal); }
    }
    T(Message,48,603,.88f,Gold);T(TEXT("Fangstjakten gir ingen penger eller fangstbonus. Øving, andres fangster og skjult dagsfiske teller ikke."),48,645,.87f,Teal);
}
