#include "UI/FiskDeckPanel.h"
#include "UI/FiskNavigationModel.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "DayCycle/FiskSessionRules.h"
#include "FiskFishingGround.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"

namespace FiskDeckStyle
{
const FLinearColor Ink(0.012f,0.045f,0.06f,0.94f),Teal(0.45f,0.94f,0.82f),Muted(0.64f,0.81f,0.85f),Gold(1,0.79f,0.36f);
FString Who(int32 Player,int32 Self) { return Player==Self?TEXT("Deg"):FString::Printf(TEXT("Spiller %d"),Player); }
}
void UFiskDeckPanel::Open(bool Sonar)
{
    bSonar=Sonar;bVisible=true;bRecords=false;Page=0;
    CastChecked<AFiskBoardingController>(GetOwner())->ReleaseRodInputsForMenu();
}
bool UFiskDeckPanel::HandleKey(const FInputKeyEventArgs& Args)
{
    if(!bVisible) { return false; }
    if(Args.Event!=IE_Pressed && Args.Event!=IE_Repeat) { return true; }
    const FKey K=Args.Key;
    if(K==EKeys::Escape || (bSonar && K==EKeys::F7) || (!bSonar && K==EKeys::F8)) { bVisible=false; }
    else if(K==EKeys::F7) { bSonar=true;Page=0; }
    else if(K==EKeys::F8) { bSonar=false;Page=0; }
    else if(K==EKeys::Tab) { bRecords=!bRecords;Page=0; }
    else if(K==EKeys::PageDown || K==EKeys::Down) { Page=FMath::Min(Page+1,999); }
    else if(K==EKeys::PageUp || K==EKeys::Up) { Page=FMath::Max(Page-1,0); }
    return true;
}
void UFiskDeckPanel::Draw(AHUD& HUD,UCanvas& Canvas)
{
    using namespace FiskDeckStyle;
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    auto Text=[&](const FString& T,float X,float Y,float Size=1.0f,FLinearColor C=FLinearColor::White){HUD.DrawText(T,C,X*S,Y*S,nullptr,Size*S);};
    auto Rect=[&](float X,float Y,float W,float H,FLinearColor C){HUD.DrawRect(C,X*S,Y*S,W*S,H*S);};
    Rect(24,28,1232,642,Ink);
    Text(bSonar?TEXT("EKKOLODD"):TEXT("LAN-TAVLE"),52,58,2.0f);
    Text(TEXT("F7: ekkolodd | F8: LAN-tavle | Escape: lukk | Klokken fortsetter"),52,632,1.0f,Muted);
    if(bSonar)
    {
        const bool Aboard=PC->PlayerState && Boat && Boat->IsAboard(PC->PlayerState->GetPlayerId());
        const FFiskSonarView V=FiskNavigation::Sonar(Boat?Boat->GetGroundId():-1,Boat?Boat->GetTripProgress():-2,Aboard,PC->GetRod()->GetRig().Depth);
        Text(TEXT("Habitatprofil fra sjøkartet - ikke en sanntidsskanning av fisk"),52,108,1.05f,Muted);
        if(!V.bAvailable)
        {
            Text(TEXT("Ingen profil her. Gå om bord og reis til fiskeplassen."),68,220,1.3f,Gold);
            Text(TEXT("Bruk M for å se planlagt bunndybde og omtrentlige fiskedybder før avreise."),68,275,1.05f);
            return;
        }
        Text(V.Ground,52,154,1.5f,Teal);
        Text(FString::Printf(TEXT("Bunn %d m | %s"),V.Bottom,*V.Structure),52,200,1.15f);
        Text(FString::Printf(TEXT("Fisketegn ca. %d-%d m"),V.Near,V.Far),52,245,1.1f,Gold);
        Text(FString::Printf(TEXT("Stang %d - agn: %d m"),PC->GetSelectedRod()+1,V.BaitDepth),52,291,1.2f,Teal);
        const bool InBand=V.BaitDepth>=V.Near && V.BaitDepth<=V.Far && V.BaitDepth<=V.Bottom;
        Text(InBand?TEXT("Agnet er innenfor dybdebåndet."):TEXT("Agnet er utenfor dybdebåndet."),52,338,1.0f,InBand?Teal:Gold);
        Text(TEXT("G: juster dybden når stanga er ledig."),52,380,0.95f,Muted);
        Text(TEXT("Ingen garanti for napp eller fangstart."),52,420,0.95f,Muted);
        Rect(610,164,580,404,FLinearColor(0.02f,0.12f,0.18f));
        auto Y=[&](float Depth){return 180+Depth/V.Bottom*350;};
        Rect(610,Y(V.Near),580,Y(V.Far)-Y(V.Near),FLinearColor(0.09f,0.29f,0.29f));
        for(int32 I=0;I<=5;++I)
        {
            const float Depth=V.Bottom*I/5.f,YPos=Y(Depth);
            HUD.DrawLine(610*S,YPos*S,1190*S,YPos*S,FLinearColor(0.17f,0.35f,0.4f),S);
            Text(FString::Printf(TEXT("%d m"),FMath::RoundToInt(Depth)),1130,YPos-18,0.9f,Muted);
        }
        for(int32 I=0;I<7;++I)
        {
            const float X=650+I*65,YPos=Y(V.Near+(V.Far-V.Near)*((I%3)+1)/4.f);
            HUD.DrawLine(X*S,(YPos+4)*S,(X+9)*S,(YPos-5)*S,Gold,2*S);
            HUD.DrawLine((X+9)*S,(YPos-5)*S,(X+18)*S,(YPos+4)*S,Gold,2*S);
        }
        Rect(610,Y(V.Bottom),580,34,FLinearColor(0.35f,0.32f,0.21f));
        const float BaitY=Y(FMath::Clamp(V.BaitDepth,0,V.Bottom));
        HUD.DrawLine(625*S,BaitY*S,1115*S,BaitY*S,Teal,3*S);
        Text(TEXT("AGN"),625,BaitY-22,0.95f,Teal);
        Text(TEXT("Symbolene viser et omtrentlig habitatbånd, ikke fisketelling."),610,586,0.85f,Muted);
        return;
    }
    Text(bRecords?TEXT("Øktens største landede fisk per spiller"):TEXT("Dagens plassering etter salgsinntekt"),52,112,1.2f,Teal);
    Text(TEXT("Tab: bytt visning | Like resultater deler plassering"),52,151,1.0f,Muted);
    TActorIterator<AFiskDaySettlement> Settlement(GetWorld());
    if(!Day || !Day->HasStarted() || !Settlement) { Text(TEXT("Start eller bli med i en økt for å se LAN-tavlen."),52,234,1.2f);return; }
    const int32 Ms=Day->GetElapsedMilliseconds(),Number=Day->GetDayNumber();
    if(!bRecords && !FiskNavigation::Revealed(Settlement->GetView(),Number,Ms))
    { Text(TEXT("Dagens plasseringer og fangster er skjult til 17:00."),52,234,1.3f,Gold);return; }
    const TArray<FFiskBoardRow> Rows=bRecords?FiskNavigation::Records(Settlement->GetHistory(),Settlement->GetView(),Number,Ms)
        :FiskNavigation::Standings(Settlement->GetView(),Number,Ms);
    const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(Rows.Num(),6));Page=FMath::Clamp(Page,0,Pages-1);
    if(Rows.IsEmpty()) { Text(TEXT("Ingen avslørte fangstresultater ennå."),52,234,1.2f); }
    const int32 Self=PC->PlayerState?PC->PlayerState->GetPlayerId():-1;
    for(int32 I=0;I<6 && Page*6+I<Rows.Num();++I)
    {
        const FFiskBoardRow& R=Rows[Page*6+I];const float Y=208+I*57;
        Rect(52,Y,1160,49,R.Player==Self?FLinearColor(0.09f,0.25f,0.27f):FLinearColor(0.025f,0.10f,0.13f));
        Text(FString::Printf(TEXT("%d.  %s"),R.Rank,*Who(R.Player,Self)),68,Y+14,1.1f,R.Player==Self?Teal:FLinearColor::White);
        if(bRecords)
        {
            const FFiskSpecies* Species=FiskCatches::Find(R.Species);
            Text(FString::Printf(TEXT("%s | %.1f kg | dag %d"),Species?Species->Name:TEXT("Ukjent"),R.BestGrams/1000.0,R.Day),410,Y+14,1.1f);
        }
        else { Text(FString::Printf(TEXT("%lld fiskepenger | %d solgt / %d tapt | %.1f kg solgt"),static_cast<long long>(R.Income),R.Delivered,R.Lost,R.Grams/1000.0),410,Y+14,1.0f); }
    }
    Text(FString::Printf(TEXT("Side %d/%d | Piler eller PgUp/PgDn: bla"),Page+1,Pages),52,568,1.0f,Teal);
    Text(bRecords?TEXT("Bekreftede landinger i siste 32 oppgjør. Også fisk tapt ved sen retur teller.")
        :TEXT("Bare dagens salg teller her. Ingen premier eller pengeoverføringer fra tavlen."),52,600,0.95f,Muted);
}
void UFiskDeckPanel::DrawOverlay(AHUD& HUD,UCanvas& Canvas,const AFiskBoardingController& PC)
{
    using namespace FiskDeckStyle;
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    auto Text=[&](const FString& T,float X,float Y,float Size=1.0f,FLinearColor C=FLinearColor::White){HUD.DrawText(T,C,X*S,Y*S,nullptr,Size*S);};
    auto Rect=[&](float X,float Y,float W,float H,FLinearColor C=Ink){HUD.DrawRect(C,X*S,Y*S,W*S,H*S);};
    const AFiskStarterBoat* Boat=PC.FindStarterBoat();const AFiskDayState* Day=PC.GetWorld()->GetGameState<AFiskDayState>();
    const UFiskPrototypeRod* Rod=PC.GetRod();const int32 Self=PC.PlayerState?PC.PlayerState->GetPlayerId():-1;
    const int32 Ms=Day?Day->GetElapsedMilliseconds():0,Progress=Boat?Boat->GetTripProgress():-2;
    const bool Aboard=Boat && Boat->IsAboard(Self),Captain=Boat && Boat->GetCaptainId()==Self;
    const FFiskFishingGround* Ground=FiskGrounds::Find(Boat?Boat->GetGroundId():-1);
    const bool RigValid=Ground && FFiskRigRules::IsValid(Rod->GetRig().Bait,Rod->GetRig().Depth,Rod->GetRig().Program,Ground->BottomDepth);
    Rect(20,20,330,376);Text(TEXT("FISKELAGET"),38,36,1.7f,Teal);
    Text(Ground?Ground->Name:TEXT("Marina"),38,81,1.1f);
    Text(FString::Printf(TEXT("Startbåt | %d/2 om bord | Kaptein: %s"),Boat?Boat->GetCrewCount():0,Captain?TEXT("deg"):!Boat || Boat->GetCaptainId()==INDEX_NONE?TEXT("ingen"):TEXT("medspiller")),38,116,.9f,Muted);
    for(int32 Slot=0;Slot<2;++Slot)
    {
        const UFiskPrototypeRod* CardRod=PC.GetRodAt(Slot);
        const bool Available=Slot<PC.GetRodCapacity(),Selected=Slot==PC.GetSelectedRod();
        const float Y=151+Slot*113;
        Rect(32,Y,306,103,Selected?FLinearColor(.09f,.26f,.28f):FLinearColor(.025f,.08f,.11f));
        Text(FString::Printf(TEXT("%d: STANG %d %s"),Slot+1,Slot+1,Selected?TEXT("[VALGT]"):TEXT("")),44,Y+9,1.0f,Selected?Teal:Muted);
        if(!Available) { Text(Aboard?TEXT("Delt båt: én stang hver"):TEXT("Gå om bord med E"),44,Y+47,.9f,Muted);continue; }
        const EFiskRodState CardState=CardRod->GetState();
        const TCHAR* StateName=CardState==EFiskRodState::Fishing?TEXT("Fisker automatisk"):CardState==EFiskRodState::InitialRun?TEXT("NAPP - FØRSTE UTRAS")
            :CardState==EFiskRodState::Fighting?TEXT("FISKEKAMP") :CardState==EFiskRodState::ReadyToLand?TEXT("KLAR TIL LANDING")
            :CardState==EFiskRodState::Lost?TEXT("Fisk mistet - klar igjen"):CardState==EFiskRodState::Landed?TEXT("Fisk landet"):TEXT("Ledig");
        Text(StateName,44,Y+34,.95f,CardState==EFiskRodState::InitialRun || CardState==EFiskRodState::ReadyToLand?Gold:FLinearColor::White);
        const FFiskRigSettings& R=CardRod->GetRig();
        Text(FString::Printf(TEXT("%s | %d m"),FFiskRigRules::BaitName(R.Bait),R.Depth),44,Y+61,.85f,Teal);
        Text(FString::Printf(TEXT("%s | G: oppsett for valgt stang"),FFiskRigRules::ProgramName(R.Program)),44,Y+83,.75f,Muted);
    }
    Rect(900,20,360,134);
    Text(FString::Printf(TEXT("DAG %d  %02d:%02d"),Day?Day->GetDayNumber():1,Ms/60000,Ms/1000%60),920,36,1.8f);
    const int32 Margin=Boat?FiskGrounds::ReturnMarginMs(Ms,Progress,Boat->GetGroundId()):0;
    const int32 Warning=FiskSession::ReturnWarning(Margin);
    const FLinearColor Risk=Progress==200?Teal:Warning==2?FLinearColor(1,.35f,.3f):Warning==1?Gold:Teal;
    Text(TEXT("ABSOLUTT RETURFRIST 17:00"),920,83,1.0f,Muted);
    Text(Progress==200?TEXT("Tilbake i marinaen"):FString::Printf(TEXT("Returmargin ca. %s%02d:%02d"),Margin<0?TEXT("-"):TEXT(""),FMath::Abs(Margin)/60000,FMath::Abs(Margin)/1000%60),920,116,1.0f,Risk);
    const EFiskRodState State=Rod->GetState();
    const bool Fight=State==EFiskRodState::Fighting || State==EFiskRodState::ReadyToLand || State==EFiskRodState::InitialRun;
    if(Fight && Aboard && Progress==100 && Ms<1020000)
    {
        const int32 Load=Rod->GetLoadPercent();const FLinearColor C=Load>100?FLinearColor(1,.3f,.25f):Load<20?Gold:Teal;
        Rect(400,430,860,186);
        Text(State==EFiskRodState::ReadyToLand?TEXT("KLAR TIL LANDING - MELLOMROM"):Rod->IsSurging() || State==EFiskRodState::InitialRun?TEXT("UTRAS - SLIPP SVEIVA OG SENK STANGA"):TEXT("FISKEKAMP - PUMP OG SVEIV"),420,448,1.3f,State==EFiskRodState::ReadyToLand?Teal:C);
        Text(FString::Printf(TEXT("Line ute: %d m   |   Brems: %d%%   |   Belastning: %d%%"),Rod->GetLineOut(),Rod->GetDragPercent(),Load),420,487,1.15f);
        Rect(420,528,810,18,FLinearColor(.08f,.13f,.16f));
        Rect(420,528,810*FMath::Clamp(Load/120.f,0.f,1.f),18,C);
        HUD.DrawLine(555*S,524*S,555*S,550*S,Gold,2*S);
        HUD.DrawLine(1095*S,524*S,1095*S,550*S,FLinearColor::Red,2*S);
        Text(Load>100?TEXT("BRUDDFARE: avlast stanga eller løsne bremsen med musehjulet."):Load<20?TEXT("SLAKK: løft stanga eller stram bremsen med musehjulet.")
            :TEXT("Venstre: løft | Høyre: sveiv | Musehjul: brems | X: avbryt"),420,570,1.0f,C);
    }
    Rect(20,630,1240,76);Text(TEXT("NESTE HANDLING"),38,642,0.85f,Teal);
    Text(FiskNavigation::NextAction(Day && Day->HasStarted(),Ms,Progress,Aboard,Captain,PC.GetNetMode()==NM_ListenServer,static_cast<int32>(State),RigValid,Ms<Rod->GetAvailableAtMs()),38,672,1.05f);
    Text(TEXT("1/2 stang | C kamera | M kart | K mannskap | F1 hjelp"),38,414,.8f,Muted);
    Text(TEXT("F2 oppgjør | F3 historikk | F4 journal | F7 ekkolodd | F8 LAN-tavle"),38,439,.72f,Muted);
    if(!Fight && Aboard && Progress==100 && Ms<1020000)
    {
        Text(State==EFiskRodState::Fishing?TEXT("STANGA FISKER - VENTER PÅ NAPP"):State==EFiskRodState::Lost?TEXT("FISKEN SLAPP UNNA - F FOR Å PRØVE IGJEN"):TEXT("STANGA ER LEDIG"),38,477,.75f,Gold);
    }
    const FString Feedback=PC.GetActionFeedback();
    if(!Feedback.IsEmpty())
    { Rect(370,178,890,62);Text(Feedback,390,199,.95f,Gold); }
}
