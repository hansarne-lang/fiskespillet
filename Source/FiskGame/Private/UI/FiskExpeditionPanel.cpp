#include "UI/FiskExpeditionPanel.h"
#include "Expedition/FiskExpeditionState.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskMarinaPanel.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskCrewPanel.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"

bool UFiskExpeditionPanel::HandleKey(const FInputKeyEventArgs& A)
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const bool Press=A.Event==IE_Pressed || A.Event==IE_Repeat;
    if(!bVisible)
    {
        if(!Press || A.Key!=EKeys::J || PC->GetFrontEnd()->IsVisible() || PC->GetMarinaPanel()->IsVisible() || PC->GetSeaChart()->IsVisible()
            || PC->GetCrewPanel()->IsVisible() || PC->GetRigPanel()->IsVisible() || PC->GetJournalPanel()->IsVisible() || PC->GetDeckPanel()->IsVisible()
            || PC->IsCareerVisible() || PC->IsHelpVisible() || PC->IsResultsVisible() || PC->IsHistoryVisible()) { return false; }
        bVisible=true;Page=0;PC->ReleaseRodInputsForMenu();PC->bShowMouseCursor=true;PC->SetInputMode(FInputModeGameAndUI());return true;
    }
    if(!Press) { return true; }
    if(A.Key==EKeys::Escape || A.Key==EKeys::J) { bVisible=false;PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());return true; }
    TActorIterator<AFiskExpeditionState> E(GetWorld());if(!E || !PC->PlayerState) { return true; }const auto& P=E->GetPlan();
    const FKey Numbers[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five};
    for(int32 I=0;I<5;++I) { if(A.Key==Numbers[I]) { PC->ServerExpeditionChoose(P.Revision,I); } }
    if(A.Key==EKeys::T) { PC->ServerExpeditionReady(P.Revision,!P.Ready.Contains(PC->PlayerState->GetPlayerId())); }
    const FKey Messages[]={EKeys::Six,EKeys::Seven,EKeys::Eight};for(int32 I=0;I<3;++I) { if(A.Key==Messages[I]) { PC->ServerExpeditionSay(I); } }
    if(A.Key==EKeys::PageDown) { ++Page; }if(A.Key==EKeys::PageUp) { Page=FMath::Max(0,Page-1); }
    if(A.Key==EKeys::LeftMouseButton)
    {
        FVector2D M;int32 W=0,H=0;PC->GetViewportSize(W,H);const double S=FMath::Min(W/1280.0,H/720.0);
        if(S>0 && GetWorld()->GetGameViewport()->GetMousePosition(M))
        {
            for(int32 I=0;I<5;++I) { if(FBox2D(FVector2D(46*S,(179+43*I)*S),FVector2D(584*S,(217+43*I)*S)).IsInside(M)) { PC->ServerExpeditionChoose(P.Revision,I); } }
            if(FBox2D(FVector2D(650*S,371*S),FVector2D(1195*S,410*S)).IsInside(M)) { PC->ServerExpeditionReady(P.Revision,!P.Ready.Contains(PC->PlayerState->GetPlayerId())); }
        }
    }return true;
}
void UFiskExpeditionPanel::Draw(AHUD& HUD,UCanvas& C)
{
    const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=PC->FindStarterBoat();const auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    TActorIterator<AFiskExpeditionState> E(GetWorld());if(!E || !B || !Day) { return; }const auto& P=E->GetPlan();
    const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);
    const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Col=FLinearColor::White){HUD.DrawText(V,Col,X*S,Y*S,nullptr,Size*S);};
    HUD.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);
    T(TEXT("EKSPEDISJON OG AVREISE"),48,58,2);const int32 Ms=Day->HasStarted()?Day->GetElapsedMilliseconds():0;
    T(FString::Printf(TEXT("Dag %d | %02d:%02d | %s | Dagsklokken fortsetter"),P.DayNumber,Ms/60000,(Ms/1000)%60,FiskExpeditions::WeatherName(P.Weather)),48,112,1.05f,Teal);
    T(P.bLocked?TEXT("Dagens mål er låst etter avreise."):TEXT("Kapteinen velger 1–5 ved brygga. Valg endrer klar-markeringene."),48,150,.9f,Teal);
    for(int32 I=0;I<5;++I)
    {
        HUD.DrawRect(P.Goal==I?FLinearColor(.14f,.34f,.36f):FLinearColor(.03f,.12f,.15f),46*S,(179+43*I)*S,538*S,38*S);
        T(FString::Printf(TEXT("%d  %s%s"),I+1,FiskExpeditions::GoalName(I),P.Goal==I?TEXT("  [VALGT]"):TEXT("")),58,190+43*I,.95f);
    }
    FString Detail;
    switch(P.Goal)
    {
        case 0:Detail=FString::Printf(TEXT("Lever minst én %s fra %s."),FiskCatches::Find(P.Species)->Name,FiskGrounds::Find(P.Ground)->Name);break;
        case 1:Detail=TEXT("Lever minst to ulike arter sammen.");break;
        case 2:Detail=FString::Printf(TEXT("Minst %d ulike fiskere må levere fisk."),P.bLocked?P.RequiredCrew:FMath::Min(2,FMath::Max(1,B->GetCrewCount())));break;
        case 3:Detail=TEXT("Lever en fisk som slår din tidligere vekt i vertens journal.");break;
        default:Detail=TEXT("Lever fisk landet innen 5 m fra dagens valgte sjøpunkt.");break;
    }
    T(Detail,48,417,.85f,Gold);T(TEXT("Bare levert fangst teller. Ingen ekstra pengebonus."),48,449,.9f,Teal);
    const bool Revealed=P.bEvaluated && Ms>=1020000;
    T(Revealed?(P.bCompleted?TEXT("FULLFØRT – medalje til avreisemannskapet!"):TEXT("IKKE FULLFØRT – sent levert fisk teller ikke.")):TEXT("Resultatet åpnes kl. 17:00."),48,485,1.1f,Gold);
    T(FString::Printf(TEXT("Sjøpunkt: %.0f, %.0f m | Reise %d s"),P.Point.X/100,P.Point.Y/100,B->GetTravelMs()/1000),650,178,.98f,Teal);
    const int32 Margin=B->ReturnMarginMs(Ms)/1000;T(B->GetTripProgress()==200?TEXT("Båten er tilbake i marinaen."):FString::Printf(TEXT("Returmargin ca. %s%d:%02d | Frist 17:00"),Margin<0?TEXT("-"):TEXT(""),FMath::Abs(Margin)/60,FMath::Abs(Margin)%60),650,213,1.f,Gold);
    for(int32 I=0;I<B->GetCrewCount();++I) { const int32 Id=B->GetCrewMemberId(I);T(FString::Printf(TEXT("Spiller %d%s: %s"),Id,Id==B->GetCaptainId()?TEXT(" (kaptein)"):TEXT(""),P.Ready.Contains(Id)?TEXT("KLAR"):TEXT("Venter")),650,258+27*I,.95f); }
    HUD.DrawRect(FLinearColor(.14f,.34f,.36f),650*S,371*S,545*S,39*S);T(TEXT("T / klikk: klar eller vent | R brukes etter at panelet er lukket"),661,384,.8f);
    for(int32 I=0;I<3;++I) { T(FString::Printf(TEXT("%d: %s"),I+6,FiskExpeditions::Phrase(I)),650,429+26*I,.88f,Teal); }
    if(P.MessageCode>=0) { T(FString::Printf(TEXT("Spiller %d: %s"),P.MessagePlayer,FiskExpeditions::Phrase(P.MessageCode)),650,521,.9f,Gold); }
    int32 Medals=0;for(const auto& R:GI->Expeditions) { Medals+=R.bCompleted?1:0; }
    T(FString::Printf(TEXT("Din historikk: %d medaljer / %d ekspedisjoner"),Medals,GI->Expeditions.Num()),48,538,.95f,Teal);
    const int32 Pages=FMath::Max(1,FMath::DivideAndRoundUp(GI->Expeditions.Num(),2)),Shown=FMath::Clamp(Page,0,Pages-1);
    for(int32 I=0;I<2;++I) { const int32 Index=GI->Expeditions.Num()-1-Shown*2-I;if(Index>=0) { const auto& R=GI->Expeditions[Index];T(FString::Printf(TEXT("Dag %d | %s | %s"),R.DayNumber,FiskExpeditions::GoalName(R.Goal),R.bCompleted?TEXT("Medalje"):TEXT("Ikke fullført")),48,572+26*I,.85f); } }
    T(TEXT("J / Escape: lukk | PgUp/PgDn: historikk | M: velg sjøpunkt utenfor panelet"),48,646,.9f,Teal);
    T(GI->ExpeditionMessage,650,583,.79f,Teal);T(TEXT("Klarmarkering gir ingen ekstra tid."),650,612,.85f,Teal);
}
