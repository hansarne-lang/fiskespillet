#include "Bootstrap/FiskBootstrapHUD.h"
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
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DayCycle/FiskDayState.h"
#include "Economy/FiskDaySettlement.h"
#include "DayCycle/FiskSessionRules.h"
#include "UI/FiskJournalPanel.h"

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
        Lines={TEXT("Spill starter dagen alene. Venner kan koble til samme LAN-økt."),
            TEXT("E: gå om bord. Kapteinen velger plass med M. Fra 02:00 starter R turen."),
            TEXT("G: agn, dybde og agnprogram. 1/2/3 velger lagringsplass i oppsettspanelet."),
            TEXT("F5 lagrer bekreftet oppsett på PC-en. F9 henter det til servervalidering."),
            TEXT("F: sett ut stanga ved fiskeplassen. X: trekk inn / avbryt egen kamp."),
            TEXT("Kamp: løft med venstre mus, slipp for å senke. Sveiv med høyre mus."),
            TEXT("Musehjul: brems. Slipp sveiva under utras. Mellomrom lander når klart."),
            TEXT("R: kapteinens retur. K: mannskap og kapteinsbytte. Lukk paneler før R."),
            TEXT("Senest 17:00: kryss marinasonen. Resultater og utbetaling vises først da."),
            TEXT("F2: dagsoppgjør. F3: økthistorikk. F4: lagret journal/rekorder. N: neste dag etter 20:00.")};
        for(int32 I=0;I<Lines.Num();++I) { HUD.DrawText(Lines[I],FLinearColor::White,52*S,(126+44*I)*S,nullptr,1.05f*S); }
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
                    const FString Who=Row.PlayerId==Self?TEXT("Deg"):FString::Printf(TEXT("Spiller %d"),Row.PlayerId);
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
        HUD.DrawText(TEXT("Siste 32 oppgjør i denne økten. Nullstilles når økten avsluttes. Ingen lagret karriere."),FLinearColor(0.6f,0.85f,0.85f),52*S,626*S,nullptr,0.95f*S);
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
            const FString Who=Row.PlayerId==Self?TEXT("Deg"):FString::Printf(TEXT("Spiller %d"),Row.PlayerId);
            Lines.Add(FString::Printf(TEXT("%s: levert %d / tapt %d | levert vekt %.1f kg | %d fiskepenger"),
                *Who,Row.DeliveredFish,Row.LostFish,Row.DeliveredWeightGrams/1000.0,Row.SessionBalance));
            for(const FFiskCatchResult& Catch:Row.Catches)
            {
                const FFiskSpecies* Species=FiskCatches::Find(Catch.Fish.Species);
                Lines.Add(FString::Printf(TEXT("  %s | %.1f kg | %.1f cm | %d fiskepenger | %s"),Species?Species->Name:TEXT("Ukjent"),
                    Catch.Fish.WeightGrams/1000.0,Catch.Fish.LengthMm/10.0,Catch.bDelivered?Catch.Fish.Value:0,
                    Catch.bDelivered?TEXT("LEVERT"):TEXT("TAPT - sen retur")));
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
        const FString Feedback=PC->GetActionFeedback();
        if(!Feedback.IsEmpty())
        {
            const float S=FMath::Min(Canvas->ClipX/1280.f,Canvas->ClipY/720.f);
            DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.96f),24*S,Canvas->ClipY-42*S,1232*S,36*S);
            DrawText(Feedback,FLinearColor(1,0.86f,0.5f),40*S,Canvas->ClipY-34*S,nullptr,0.95f*S);
        }
        if(PC->GetJournalPanel()->IsVisible()) { PC->GetJournalPanel()->Draw(*this,*Canvas);return; }
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
    const float Left = 30.0f;
    const float Top = 24.0f;
    DrawRect(FLinearColor(0.01f, 0.025f, 0.04f, 0.78f), 14.0f, 12.0f, 560.0f, 455.0f);
    DrawText(TEXT("FISKELAGET"), FLinearColor::White, Left, Top, nullptr, 2.0f);
    DrawText(TEXT("M: Kart  K: Mannskap  F1: Hjelp  F2: Oppgjør  F3: Historikk  F4: Journal"), FLinearColor::White, Left, Top + 65.0f, nullptr, 0.80f);

    if (!StarterBoat.IsValid())
    {
        TActorIterator<AFiskStarterBoat> Boats(GetWorld());
        if (Boats)
        {
            StarterBoat = *Boats;
        }
    }

    if (const AFiskStarterBoat* Boat = StarterBoat.Get())
    {
        const APlayerController* PC = GetOwningPlayerController();
        const int32 PlayerId = PC && PC->PlayerState ? PC->PlayerState->GetPlayerId() : INDEX_NONE;
        const int32 Progress = Boat->GetTripProgress();
        const bool bAboard = Boat->IsAboard(PlayerId);
        const TCHAR* Location = bAboard
            ? TEXT("Du er om bord")
                        : Progress == AFiskStarterBoat::AtDock
                ? TEXT("Ved brygga: Trykk E for å gå om bord")
                : TEXT("Du er ved brygga");
        const int32 CaptainId = Boat->GetCaptainId();
        const TCHAR* Captain = CaptainId == INDEX_NONE ? TEXT("Ingen")
            : CaptainId == PlayerId ? TEXT("Deg") : TEXT("Medspiller");
        DrawText(FString::Printf(TEXT("Startbåt  %d/%d   Kaptein: %s"), Boat->GetCrewCount(),
            AFiskStarterBoat::Capacity, Captain), FLinearColor::White, Left, Top + 135.0f);
        DrawText(Location, FLinearColor::White, Left, Top + 160.0f);

        FString Route;
        if (Progress == AFiskStarterBoat::AtDock)
        {
            Route = CaptainId != INDEX_NONE && CaptainId == PlayerId
                ? TEXT("Ved brygga. Fra 02:00: Trykk R for å dra til fiskeplassen")
                : TEXT("Ved brygga. Kapteinen starter turen");
        }
        else if (Progress == AFiskStarterBoat::AtMarina)
        {
            Route = TEXT("Tilbake ved brygga");
        }
        else if (Progress > AFiskStarterBoat::AtFishingSpot)
        {
            Route = FString::Printf(TEXT("På vei hjem: %d%%"), Progress - AFiskStarterBoat::AtFishingSpot);
        }
        else if (Progress == AFiskStarterBoat::AtFishingSpot)
        {
            Route = CaptainId != INDEX_NONE && CaptainId == PlayerId
                ? TEXT("Framme ved fiskeplassen. Trykk R for å dra hjem")
                : TEXT("Framme ved fiskeplassen");
        }
        else
        {
            Route = FString::Printf(TEXT("Underveis til fiskeplassen: %d%%"), Progress);
        }
        DrawText(Route, FLinearColor::White, Left, Top + 185.0f);
        const FFiskFishingGround* Ground=FiskGrounds::Find(Boat->GetGroundId());
        if(Ground) { DrawText(Ground->Name,FLinearColor(0.5f,0.9f,0.8f),Left,Top+50.0f,nullptr,0.85f); }
    }

    if (const AFiskBoardingController* RigOwner = Cast<AFiskBoardingController>(GetOwningPlayerController()))
    {
        const FFiskRigSettings& Rig=RigOwner->GetRod()->GetRig();
        DrawText(FString::Printf(TEXT("%s / %d m / %s"),FFiskRigRules::BaitName(Rig.Bait),Rig.Depth,FFiskRigRules::ProgramName(Rig.Program)),FLinearColor::White,Left,Top+430.0f,nullptr,0.95f);
        if (RigOwner->CanOpenRig()) { DrawText(TEXT("G: Velg agn, dybde og program"),FLinearColor(0.5f,0.95f,0.8f),Left,Top+380.0f); }
        const AFiskStarterBoat* RigBoat=RigOwner->FindStarterBoat();
        const FFiskFishingGround* Ground=FiskGrounds::Find(RigBoat?RigBoat->GetGroundId():0);
        if(Ground && !FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,Ground->BottomDepth))
        { DrawText(TEXT("For dyp rigg for plassen. Trykk G før utsetting."),FLinearColor(1,0.7f,0.3f),Left,Top+240.0f); }
    }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (Day == nullptr)
    {
        return;
    }

    if (!Day->HasStarted())
    {
        const TCHAR* Message = GetNetMode() == NM_Standalone
            ? TEXT("Start en LAN-vert for å spille.")
            : TEXT("Starter fiskedagen ...");
        DrawText(Message, FLinearColor::White, Left, Top + 100.0f);
        return;
    }

    const int32 Elapsed = Day->GetElapsedSeconds();
    const TCHAR* Phase = TEXT("Forberedelser");
    switch (AFiskDayState::GetPhase(Elapsed))
    {
    case EFiskDayPhase::Preparation:
        break;
    case EFiskDayPhase::FishingAndReturn:
        Phase = TEXT("Fiske og retur");
        break;
    case EFiskDayPhase::Settlement:
        Phase = TEXT("Dagsoppgjør");
        break;
    case EFiskDayPhase::Finished:
        Phase = TEXT("Fiskedagen er ferdig");
        break;
    }

    DrawText(FString::Printf(TEXT("Dag %d | %02d:%02d  %s"), Day->GetDayNumber(), Elapsed / 60, Elapsed % 60, Phase),
        FLinearColor::White, Left, Top + 100.0f);
    if(Elapsed>=1200)
    { DrawText(GetNetMode()==NM_ListenServer?TEXT("N: Neste fiskedag | F3: Økthistorikk"):TEXT("Venter på vertens neste fiskedag. F3: Historikk"),FLinearColor(0.5f,0.95f,0.8f),Left,Top+240.0f,nullptr,0.95f); }


    for (TActorIterator<AFiskDaySettlement> Settlement(GetWorld()); Settlement; ++Settlement)
    {
        const FFiskSettlementView& View = Settlement->GetView();
        if (!View.bRevealed || View.DayNumber!=Day->GetDayNumber())
        {
            DrawText(TEXT("Dagens fangstinntekt vises først ved 17:00. F3: tidligere oppgjør"), FLinearColor::White, Left, Top + 355.0f,nullptr,0.85f);
        }
        else
        {
            float RowY = Top + 355.0f;
            for (const FFiskDayResult& Row : View.Results)
            {
                DrawText(FString::Printf(TEXT("Spiller %d: levert %d, tapt %d, dagsinntekt %d"),
                    Row.PlayerId, Row.DeliveredFish, Row.LostFish, Row.SessionBalance), FLinearColor::White, Left, RowY);
                RowY += 24.0f;
            }
        }
    }

    const AFiskStarterBoat* Boat = StarterBoat.Get();
    const AFiskBoardingController* PC = Cast<AFiskBoardingController>(GetOwningPlayerController());
    if (!Boat || !PC || !PC->PlayerState || !Boat->IsAboard(PC->PlayerState->GetPlayerId())
        || Boat->GetTripProgress() != AFiskStarterBoat::AtFishingSpot
        || AFiskDayState::GetPhase(Elapsed) != EFiskDayPhase::FishingAndReturn)
    {
        return;
    }

    FString RodText = TEXT("Stang 1: Trykk F for å sette ut");
    switch (PC->GetRod()->GetState())
    {
    case EFiskRodState::Ready:
        break;
    case EFiskRodState::Fishing:
        RodText = TEXT("Stang 1: Fisker");
        break;
    case EFiskRodState::InitialRun:
        RodText = TEXT("Stang 1: Napp! Fisken tar line");
        break;
    case EFiskRodState::Fighting:
        RodText = FString::Printf(TEXT("Stang 1: Kamp - %d m line ute"), PC->GetRod()->GetLineOut());
        DrawText(TEXT("Hold venstre: løft. Slipp: senk. Hold høyre: sveiv"),
            FLinearColor::White, Left, Top + 240.0f);
        break;
    case EFiskRodState::Cancelled:
        RodText = TEXT("Stanga er trukket inn. Trykk F for ny utsetting");
        break;
    case EFiskRodState::Lost:
        RodText = PC->GetRod()->GetLoss() == EFiskRodLoss::LineBreak
            ? TEXT("Lina røk! Trykk F for å fiske igjen") : TEXT("Slakk line - fisken løsnet. Trykk F igjen");
        break;
    case EFiskRodState::ReadyToLand:
        RodText = TEXT("Stang 1: Klar til landing. Trykk mellomrom");
        break;
    case EFiskRodState::Landed:
        RodText = Day->GetElapsedMilliseconds() < PC->GetRod()->GetAvailableAtMs()
            ? TEXT("Fisk landet. Gjør klar stanga ...")
            : TEXT("Fisk landet. Trykk F for ny utsetting");
        break;
    }
    DrawText(RodText, FLinearColor::White, Left, Top + 215.0f);
    DrawText(TEXT("X: Trekk inn egen stang / avbryt egen kamp"), FLinearColor::White, Left, Top + 405.0f);
    const UFiskPrototypeRod* Rod = PC->GetRod();
    if (Rod->GetState() == EFiskRodState::Fighting || Rod->GetState() == EFiskRodState::ReadyToLand)
    {
        const int32 Load = Rod->GetLoadPercent();
        const FLinearColor Color = Load > 100 ? FLinearColor::Red : Load < 20 ? FLinearColor::Yellow : FLinearColor(0.2f, 0.85f, 0.65f);
        DrawText(FString::Printf(TEXT("Musehjul: brems %d%%   Linebelastning %d%%"), Rod->GetDragPercent(), Load),
            Color, Left, Top + 265.0f);
        DrawRect(FLinearColor(0.08f, 0.12f, 0.16f), Left, Top + 292.0f, 460.0f, 10.0f);
        DrawRect(Color, Left, Top + 292.0f, 460.0f * FMath::Clamp(Load / 100.0f, 0.0f, 1.0f), 10.0f);
        const TCHAR* Hint = Load > 100 ? TEXT("BRUDDFARE - slipp opp eller løsne bremsen")
            : Load < 20 ? TEXT("SLAKK LINE - løft stanga eller stram bremsen")
            : Rod->IsSurging() ? TEXT("UTRAS - fisken tar line. Senk stanga og slipp sveiva")
            : TEXT("Pump: løft uten sveiving, senk og sveiv inn");
        DrawText(Hint, Color, Left, Top + 312.0f);
    }
    const int32 Margin=FiskGrounds::ReturnMarginMs(Day->GetElapsedMilliseconds(),100,Boat->GetGroundId());
    const int32 Warning=FiskSession::ReturnWarning(Margin);
    const int32 Seconds=FMath::Max(0,Margin)/1000;
    DrawText(Warning==2?TEXT("RETURVARSEL: Beregnet ankomst etter 17:00")
        :FString::Printf(TEXT("Returfrist 17:00 | margin ved retur nå: %02d:%02d"),Seconds/60,Seconds%60),
        Warning==2?FLinearColor::Red:Warning==1?FLinearColor::Yellow:FLinearColor::White,Left,Top+332.0f,nullptr,0.95f);
}
