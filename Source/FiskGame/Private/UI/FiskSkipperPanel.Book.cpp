#include "UI/FiskSkipperPanel.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskRigSettings.h"
#include "FiskFishingGround.h"
#include "FiskRouteGeometry.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
void UFiskSkipperPanel::DrawBook(AHUD& H,UCanvas& C)
{
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const auto& J=GI->GetJournal();const auto Days=FiskTripBook::Days(J);const auto All=J.Stats();
    DayRow=FMath::Clamp(DayRow,0,FMath::Max(0,Days.Num()-1));const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("MIN FISKEDAGBOK"),48,55,2.f);
    T(FString::Printf(TEXT("%d fangstturer | %lld landet | %.1f kg landet | %lld fiskepenger fra solgt fangst"),Days.Num(),static_cast<long long>(All.Count),All.WeightGrams/1000.0,static_cast<long long>(All.Income)),48,106,1.05f,Teal);
    const TCHAR* Tabs[]={TEXT("TURER"),TEXT("ARTER"),TEXT("HABITATER"),TEXT("AGN"),TEXT("SAMMENLIGN")};T(FString::Printf(TEXT("Tab: %s | Piler: velg tur | PgUp/PgDn: bla | F10: eksporter valgt tur"),Tabs[Tab]),48,148,.98f);
    if(Tab==4)
    {
        T(TEXT("Piler velger tur | 1 setter A | 2 setter B | F10 eksporterer sammenligning"),48,200,1.f,Gold);
        if(Days.Num()<2) { T(TEXT("Du trenger to ulike bekreftede fangstturer. Nye fangster vises først etter 17:00."),48,280,1.05f,Teal); }
        else
        {
            CompareA=FMath::Clamp(CompareA,0,Days.Num()-1);CompareB=FMath::Clamp(CompareB,0,Days.Num()-1);const auto& A=Days[CompareA];const auto& B=Days[CompareB];
            T(FString::Printf(TEXT("Valgt rad %d: dag %d | A: dag %d (%d) | B: dag %d (%d)"),DayRow+1,Days[DayRow].Number,A.Number,CompareA+1,B.Number,CompareB+1),48,242,1.05f,Teal);
            if(CompareA==CompareB) { T(TEXT("Velg to ulike turer med piler og 1/2."),48,292,1.1f,Gold); }
            const FString Values[]={FString::Printf(TEXT("Landinger       %d                    %d"),A.Count,B.Count),FString::Printf(TEXT("Solgt / tapt    %d / %d               %d / %d"),A.Sold,A.Lost,B.Sold,B.Lost),FString::Printf(TEXT("Landet kg       %.1f                 %.1f"),A.Grams/1000.,B.Grams/1000.),FString::Printf(TEXT("Solgt inntekt   %lld                 %lld"),static_cast<long long>(A.Income),static_cast<long long>(B.Income)),FString::Printf(TEXT("Største kg      %.1f                 %.1f"),A.Best/1000.,B.Best/1000.),FString::Printf(TEXT("Arter/habitater %d / %d               %d / %d"),A.Species.Num(),A.Grounds.Num(),B.Species.Num(),B.Grounds.Num()),FString::Printf(TEXT("Kamp min        %.1f                 %.1f"),A.FightMs/60000.,B.FightMs/60000.)};
            for(int32 I=0;I<7;++I) { T(Values[I],48,318+32*I,1.08f,I%2?FLinearColor::White:Teal); }
        }
        T(Message,48,601,.92f,Gold);T(TEXT("Y / Escape: lukk | Bare egen bekreftet journal; tapt fangst gir ingen inntekt"),48,644,.88f,Teal);return;
    }
    if(Tab==0)
    {
        if(Days.IsEmpty()) { T(TEXT("Ingen bekreftede fangstturer ennå. Dagens nye fangster vises først etter oppgjøret 17:00."),48,222,1.08f); }
        else
        {
            for(int32 I=(DayRow/5)*5;I<FMath::Min((DayRow/5+1)*5,Days.Num());++I)
            {
                const auto& D=Days[I];const float Y=202+(I%5)*65;H.DrawRect(I==DayRow?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),48*S,Y*S,300*S,59*S);
                T(FString::Printf(TEXT("Dag %d | %s"),D.Number,*D.Utc.Left(10)),60,Y+8,1.02f,I==DayRow?Gold:Teal);T(FString::Printf(TEXT("%d fisk | %.1f kg | %lld solgt"),D.Count,D.Grams/1000.0,static_cast<long long>(D.Income)),60,Y+34,.88f);
            }
            const auto& D=Days[DayRow];T(FString::Printf(TEXT("Dag %d – bekreftet fangsttur"),D.Number),385,201,1.25f,Gold);
            T(FString::Printf(TEXT("%d arter | %d solgt / %d tapt | %.1f kg landet"),D.Species.Num(),D.Sold,D.Lost,D.Grams/1000.0),385,239,1.f,Teal);
            T(FString::Printf(TEXT("Inntekt %lld | Største %.1f kg | Samlet kamptid %.1f min"),static_cast<long long>(D.Income),D.Best/1000.0,D.FightMs/60000.0),385,272,.96f);
            const auto Rows=FiskTripBook::Rows(J,D.Id);const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Rows.Num(),4));DetailPage=FMath::Clamp(DetailPage,0,Pages-1);
            for(int32 I=0;I<4 && DetailPage*4+I<Rows.Num();++I)
            {
                const auto& E=J.GetEntries()[Rows[DetailPage*4+I]];const float Y=332+50*I;
                T(FString::Printf(TEXT("%s | %.1f kg | %s"),FiskCatches::Find(E.Fish.Species)->Name,E.Fish.WeightGrams/1000.0,E.bSold?TEXT("Solgt"):TEXT("Ikke solgt")),385,Y,1.02f);
                T(FString::Printf(TEXT("%s | %s / %d m | Kamp %.1f s"),FiskGrounds::Find(E.Context.Ground)->Name,FFiskRigRules::BaitName(E.Context.Bait),E.Context.Depth,E.Context.FightMs/1000.0),385,Y+24,.83f,Teal);
            }
            T(FString::Printf(TEXT("Fangstside %d/%d | Dags-ID %s"),DetailPage+1,Pages,*D.Id.ToString(EGuidFormats::Digits).Left(8)),385,558,.9f);
            auto Point=[&](FVector P){return FVector2D((956+P.X/150000*240)*S,(418-P.Y/100000*110)*S);};
            H.DrawRect(FLinearColor(.025f,.19f,.25f),935*S,310*S,280*S,230*S);
            for(int32 I=0;I<FiskRouteGeometry::IslandCount;++I) { C.K2_DrawPolygon(nullptr,Point(FiskRouteGeometry::Island(I)),FVector2D(FiskRouteGeometry::Radius(I)/150000*240*S,FiskRouteGeometry::Radius(I)/100000*110*S),24,FLinearColor(.52f,.43f,.24f)); }
            int32 Plotted=0;for(int32 I:Rows) { const auto& E=J.GetEntries()[I];if(!E.Context.bHasSeaPoint) { continue; }const auto P=Point(E.Context.SeaPoint);H.DrawRect(Gold,P.X-3*S,P.Y-3*S,6*S,6*S);++Plotted; }
            T(FString::Printf(TEXT("%d bekreftede sjøpunkt"),Plotted),945,548,.85f,Teal);
        }
    }
    else
    {
        const int32 Count=Tab==1?FiskCatches::Count:Tab==2?FiskGrounds::Count:3,Pages=FMath::DivideAndRoundUp(Count,5);DetailPage=FMath::Clamp(DetailPage,0,Pages-1);
        T(TEXT("Samlet statistikk fra hele din bekreftede journal"),48,202,1.2f,Gold);
        for(int32 I=DetailPage*5;I<FMath::Min((DetailPage+1)*5,Count);++I)
        {
            FFiskTripSummary A;for(const auto& E:J.GetEntries()) { if(!E.IsValid() || (Tab==1?E.Fish.Species:Tab==2?E.Context.Ground:E.Context.Bait)!=I) { continue; }++A.Count;A.Sold+=E.bSold?1:0;A.Grams+=E.Fish.WeightGrams;A.Income+=E.bSold?E.Fish.Value:0;A.Best=FMath::Max(A.Best,E.Fish.WeightGrams); }
            const TCHAR* Name=Tab==1?FiskCatches::Find(I)->Name:Tab==2?FiskGrounds::Find(I)->Name:FFiskRigRules::BaitName(I);const float Y=255+(I%5)*58;
            T(Name,48,Y,1.2f,Teal);T(FString::Printf(TEXT("%d landet / %d solgt | %.1f kg landet | Største %.1f kg | %lld fiskepenger"),A.Count,A.Sold,A.Grams/1000.0,A.Best/1000.0,static_cast<long long>(A.Income)),385,Y+2,1.02f);
        }
        T(FString::Printf(TEXT("Side %d/%d"),DetailPage+1,Pages),48,558,.95f);
    }
    T(Message,48,601,.92f,Gold);T(TEXT("Y / Escape: lukk | Bare din bekreftede journal | Landet vekt inkluderer tap ved retur; penger gjelder solgt fisk"),48,644,.88f,Teal);
}
