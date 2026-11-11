#include "Bootstrap/FiskBootstrapHUD.h"
#include "UI/FiskPlayerNames.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskPierWalk.h"
#include "UI/FiskExpeditionPanel.h"
#include "Engine/Canvas.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "FiskFishingGround.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Fishing/FiskFishingHelp.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskSessionRules.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskAnglerPanel.h"
#include "UI/FiskCollectionPanel.h"

namespace
{
void DrawContentPanel(AHUD& HUD,UCanvas& Canvas,const AFiskBoardingController& PC)
{
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    HUD.DrawRect(FLinearColor(0.012f,0.045f,0.06f,0.98f),24*S,28*S,1232*S,642*S);
    TArray<FString> Lines;
    if(PC.IsHelpVisible())
    {
        HUD.DrawText(TEXT("HJELP OG KONTROLLER"),FLinearColor::White,52*S,60*S,nullptr,2.1f*S);
        Lines={TEXT("Spill starter dagen alene. Opptil tolv spillere i samme LAN-økt. Tab: flåte. B/V: havnekamera. I/V ved marinaen: 3D-arter."),
            TEXT("T i land: bryggeliv / WASD / Shift / E i nærheten. E: gå om bord. M: fiskeplass. R: avreise fra 02:00. C: tre faste kameravinkler."),
            TEXT("G: agn, dybde og agnprogram. 1/2/3 velger lagringsplass i oppsettspanelet."),
            TEXT("F5 lagrer bekreftet oppsett på PC-en. F9 henter det til servervalidering."),
            TEXT("1/2/3: velg egen stang. F: sett ut. X: trekk inn / avbryt valgt stang."),
            TEXT("P: marinabutikk, lagret saldo, utstyr og båtvalg. Kjøp ved brygga."),
            TEXT("Kamp: før musa opp for å pumpe og ned for å sveive, eller hold venstre/høyre knapp."),
#if !UE_BUILD_SHIPPING
            TEXT("Musehjul: brems. Slipp under utras. Mellomrom lander. Å: AI-testhjelp etter napp."),
#else
            TEXT("Musehjul: brems. Slipp under utras. Mellomrom lander når klart."),
#endif
            TEXT("R / Tilbake til marina: kapteinens retur. Klikk knappen i spillvisningen. K: mannskap."),
            TEXT("Senest 17:00: kryss marinasonen. Resultater og utbetaling vises først da."),
            TEXT("F2/F3: oppgjør/historikk. F4: journal. F7: ekkolodd. F8: LAN-tavle. N: neste dag."),
            TEXT("H/U/B/I: naust, utstyrsrom, strandbar og artsbok. J: ekspedisjon. L: sjøpass."),
            TEXT("Q: fiskeassistent | O: turplan | Y: dagbok | F9: foto | F11: last/slep")};
        for(int32 I=0;I<Lines.Num();++I) { HUD.DrawText(Lines[I],FLinearColor::White,52*S,(126+36*I)*S,nullptr,1.05f*S); }
        HUD.DrawText(TEXT("F1 / Escape: lukk. Dagen slutter ved 20:00."),FLinearColor(0.5f,0.9f,0.8f),52*S,620*S,nullptr,1.0f*S);
        return;
    }
    if(PC.IsHistoryVisible())
    {
        HUD.DrawText(TEXT("ØKTHISTORIKK"),FLinearColor::White,52*S,60*S,nullptr,2.0f*S);
        TActorIterator<AFiskDaySettlement> Settlement(PC.GetWorld());
        int64 Income=0, Delivered=0, Lost=0; int32 Best=0;
        const int32 Self=PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE;
        if(Settlement)
        {
            const TArray<FFiskHistoryDay>& Days=Settlement->GetHistory();
            for(int32 D=Days.Num()-1;D>=0;--D)
            {
                Lines.Add(FString::Printf(TEXT("Dag %d - oppgjort"),Days[D].Number));
                if(Days[D].Rows.IsEmpty()) { Lines.Add(TEXT("  Ingen registrerte fangster.")); }
                for(const FFiskHistoryRow& Row:Days[D].Rows)
                {
                    const FFiskSpecies* Species=FiskCatches::Find(Row.BestSpecies);
                    const FString Who=Row.PlayerId==Self?TEXT("Deg"):FiskPlayerNames::Name(PC.GetWorld(),Row.PlayerId);
                    Lines.Add(FString::Printf(TEXT("  %s: levert %d / tapt %d | %.1f kg levert | %d fiskepenger"),*Who,Row.Delivered,Row.Lost,Row.DeliveredGrams/1000.0,Row.Income));
                    Lines.Add(FString::Printf(TEXT("  Største landet: %s %.1f kg"),Species?Species->Name:TEXT("Ingen"),Row.BestGrams/1000.0));
                    if(Row.PlayerId==Self) { Income+=Row.Income;Delivered+=Row.Delivered;Lost+=Row.Lost;Best=FMath::Max(Best,Row.BestGrams); }
                }
            }
        }
        HUD.DrawText(FString::Printf(TEXT("Deg i viste dager: %lld levert / %lld tapt | %lld fiskepenger | største %.1f kg"),
            static_cast<long long>(Delivered),static_cast<long long>(Lost),static_cast<long long>(Income),Best/1000.0),FLinearColor(0.5f,0.9f,0.8f),52*S,116*S,nullptr,1.05f*S);
        if(Lines.IsEmpty()) { Lines.Add(TEXT("Ingen oppgjorte dager ennå. Dagens resultater kommer først ved 17:00.")); }
        const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Lines.Num(),9));
        const int32 Page=FMath::Clamp(PC.GetResultPage(),0,Pages-1);
        for(int32 I=0;I<9 && Page*9+I<Lines.Num();++I)
        { HUD.DrawText(Lines[Page*9+I],FLinearColor::White,52*S,(172+43*I)*S,nullptr,1.05f*S); }
        HUD.DrawText(FString::Printf(TEXT("Side %d/%d | PgUp/PgDn eller piler: bla | F3 / Escape: lukk"),Page+1,Pages),FLinearColor(0.5f,0.9f,0.8f),52*S,582*S,nullptr,1.0f*S);
        HUD.DrawText(TEXT("Siste 32 oppgjør i økten. P viser lagret karriere. F4 viser varig journal."),FLinearColor(0.6f,0.85f,0.85f),52*S,626*S,nullptr,0.95f*S);
        return;
    }
    HUD.DrawText(TEXT("DAGSOPPGJØR OG FANGSTER"),FLinearColor::White,52*S,60*S,nullptr,2.0f*S);
    TActorIterator<AFiskDaySettlement> Settlement(PC.GetWorld());
    const AFiskDayState* CurrentDay=PC.GetWorld()->GetGameState<AFiskDayState>();
    if(!Settlement || !CurrentDay || Settlement->GetView().DayNumber!=CurrentDay->GetDayNumber() || !Settlement->GetView().bRevealed)
    {
        HUD.DrawText(TEXT("Fangstart, mål, antall og verdi avsløres samlet ved 17:00."),FLinearColor::White,52*S,160*S,nullptr,1.15f*S);
        HUD.DrawText(TEXT("Ingen fangstresultater vises før den felles avsløringen."),FLinearColor(0.6f,0.85f,0.85f),52*S,210*S,nullptr,1.0f*S);
    }
    else
    {
        const int32 Self=PC.PlayerState?PC.PlayerState->GetPlayerId():INDEX_NONE;
        for(const FFiskDayResult& Row:Settlement->GetView().Results)
        {
            const FString Who=Row.PlayerId==Self?TEXT("Deg"):FiskPlayerNames::Name(PC.GetWorld(),Row.PlayerId);
            Lines.Add(FString::Printf(TEXT("%s: levert %d / tapt %d | levert vekt %.1f kg | %d fiskepenger"),
                *Who,Row.DeliveredFish,Row.LostFish,Row.DeliveredWeightGrams/1000.0,Row.SessionBalance));
            for(const FFiskCatchResult& Catch:Row.Catches)
            {
                const FFiskSpecies* Species=FiskCatches::Find(Catch.Fish.Species);
                Lines.Add(FString::Printf(TEXT("  %s | %.1f kg | %.1f cm | %d fiskepenger | %s"),Species?Species->Name:TEXT("Ukjent"),
                    Catch.Fish.WeightGrams/1000.0,Catch.Fish.LengthMm/10.0,Catch.bDelivered?Catch.Fish.Value:0,
                    Catch.bDelivered?TEXT("LEVERT"):TEXT("IKKE SOLGT")));
            }
        }
        if(Lines.IsEmpty()) { Lines.Add(TEXT("Ingen fangster registrert denne dagen.")); }
        constexpr int32 PerPage=9;
        const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Lines.Num(),PerPage));
        const int32 Page=FMath::Clamp(PC.GetResultPage(),0,Pages-1);
        for(int32 I=0;I<PerPage && Page*PerPage+I<Lines.Num();++I)
        { HUD.DrawText(Lines[Page*PerPage+I],FLinearColor::White,52*S,(138+46*I)*S,nullptr,1.05f*S); }
        HUD.DrawText(FString::Printf(TEXT("Side %d/%d | PgUp/PgDn eller piler: bla"),Page+1,Pages),FLinearColor(0.5f,0.9f,0.8f),52*S,582*S,nullptr,1.0f*S);
    }
    const TCHAR* Footer=CurrentDay && CurrentDay->GetElapsedSeconds()>=1200
        ? (PC.GetNetMode()==NM_ListenServer?TEXT("N: start neste fiskedag | F3: historikk | F2 / Escape: lukk")
            :TEXT("Venter på vertens neste dag. F3: historikk | F2 / Escape: lukk"))
        :TEXT("F2 / Escape: lukk. F3: økthistorikk. Beløpene gjelder dagens oppgjør.");
    HUD.DrawText(Footer,FLinearColor(0.6f,0.85f,0.85f),52*S,626*S,nullptr,0.95f*S);
}
}

void AFiskBootstrapHUD::DrawHUD()
{
    Super::DrawHUD();
    if (Canvas == nullptr)
    {
        return;
    }

    if (const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetOwningPlayerController()))
    {
        if(PC->IsFleetVisible()) { const_cast<AFiskBoardingController*>(PC)->DrawFleet(*this,*Canvas);return; }
        if(PC->GetCollectionPanel()->IsVisible()) { PC->GetCollectionPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetAnglerPanel()->IsVisible()) { PC->GetAnglerPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetSkipperPanel()->IsVisible()) { PC->GetSkipperPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetExpeditionPanel()->IsVisible()) { PC->GetExpeditionPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetMarinaPanel()->IsVisible()) { PC->GetMarinaPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetPierWalk()->Draw(*this,*Canvas)) { return; }
        const FString Feedback=PC->GetActionFeedback();
        if(!Feedback.IsEmpty() && !PC->IsCareerVisible())
        {
            const float S=FMath::Min(Canvas->ClipX/1280.f,Canvas->ClipY/720.f);
            DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.96f),24*S,Canvas->ClipY-42*S,1232*S,36*S);
            DrawText(Feedback,FLinearColor(1,0.86f,0.5f),40*S,Canvas->ClipY-34*S,nullptr,0.95f*S);
        }
        if(PC->IsCareerVisible()) { const_cast<AFiskBoardingController*>(PC)->DrawCareer(*this,*Canvas);return; }
        if(PC->GetJournalPanel()->IsVisible()) { PC->GetJournalPanel()->Draw(*this,*Canvas);return; }
        if(PC->GetDeckPanel()->IsVisible()) { PC->GetDeckPanel()->Draw(*this,*Canvas);return; }
        if(PC->IsHelpVisible() || PC->IsResultsVisible() || PC->IsHistoryVisible()) { DrawContentPanel(*this,*Canvas,*PC);return; }
        if (PC->GetCrewPanel() && PC->GetCrewPanel()->IsVisible())
        {
            PC->GetCrewPanel()->Draw(*this,*Canvas);return;
        }
        if (PC->GetSeaChart() && PC->GetSeaChart()->IsVisible())
        {
            PC->GetSeaChart()->Draw(*this, *Canvas);
            return;
        }
        if (PC->GetRigPanel() && PC->GetRigPanel()->IsVisible())
        {
            PC->GetRigPanel()->Draw(*this, *Canvas);
            return;
        }
        if (PC->GetFrontEnd() && PC->GetFrontEnd()->IsVisible())
        {
            PC->GetFrontEnd()->Draw(*this, *Canvas);
            return;
        }
    }
    if(const AFiskBoardingController* PC=Cast<AFiskBoardingController>(GetOwningPlayerController()))
    { UFiskDeckPanel::DrawOverlay(*this,*Canvas,*PC);PC->GetFishingHelp()->Draw(*this,*Canvas);PC->GetAnglerPanel()->DrawNotice(*this,*Canvas); }
}
