#include "UI/FiskSeaChart.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
#include "FiskRouteGeometry.h"
UFiskSeaChart::UFiskSeaChart() { PrimaryComponentTick.bCanEverTick=true; }
void UFiskSeaChart::BeginPlay()
{
    Super::BeginPlay();
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(!PC->IsLocalController()) { SetComponentTickEnabled(false);return; }
#if WITH_DEV_AUTOMATION_TESTS
    if(FParse::Param(FCommandLine::Get(),TEXT("FiskChartSmoke")))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),CaptureDir);
        GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskSeaChart::Smoke,0.5f,true,3.0f);
    }
#endif
}
bool UFiskSeaChart::CanOpen() const
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    return PC->PlayerState && Boat && Boat->IsAboard(PC->PlayerState->GetPlayerId());
}
void UFiskSeaChart::Show(bool bShow)
{
    bVisible=bShow;
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    PC->bShowMouseCursor=bShow;
    if(bShow)
    {
        Row=PC->FindStarterBoat()->GetGroundId();Message.Reset();
        if(PC->GetRod()->GetState()==EFiskRodState::Fighting) { PC->ReleaseRodInputsForMenu(); }
        FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);
    }
    else { PC->SetInputMode(FInputModeGameOnly()); }
}
void UFiskSeaChart::Choose()
{
    if(Row==3) { Show(false);return; }
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    if(!Boat || !PC->PlayerState) { return; }
    if(Boat->GetCaptainId()!=PC->PlayerState->GetPlayerId()) { Message=TEXT("Bare kapteinen kan velge plass.");return; }
    if(Boat->GetTripProgress()!=AFiskStarterBoat::AtDock) { Message=TEXT("Ruten er låst til båtturen er avsluttet.");return; }
    Message=TEXT("Venter på bekreftelse ...");PC->ServerSelectGround(Boat->GetRouteRevision(),Row);
}
bool UFiskSeaChart::HandleKey(const FInputKeyEventArgs& Args)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(!PC->IsLocalController() || PC->GetFrontEnd()->IsVisible() || PC->GetRigPanel()->IsVisible()) { return false; }
    if(bVisible && bRenaming) { return HandleFavorites(Args); }
    const bool bPress=Args.Event==IE_Pressed;
    if(Args.Key==EKeys::M && bPress) { if(bVisible || CanOpen()) { Show(!bVisible); }return true; }
    if(!bVisible) { return false; }
    if(bPress && Args.Key==EKeys::Tab) { bFavorites=!bFavorites;Message.Reset();return true; }
    if(bFavorites) { return HandleFavorites(Args); }
    if(!bPress) { return true; }
    if(Args.Key==EKeys::Escape) { Show(false); }
    else if(Args.Key==EKeys::Up) { Row=(Row+3)%4; }
    else if(Args.Key==EKeys::Down) { Row=(Row+1)%4; }
    else if(Args.Key==EKeys::Home) { Row=0; }
    else if(Args.Key==EKeys::End) { Row=3; }
    else if(Args.Key==EKeys::Enter || Args.Key==EKeys::SpaceBar) { Choose(); }
    else if(Args.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;
        if(UGameViewportClient* VP=GetWorld()->GetGameViewport();VP && VP->GetMousePosition(Mouse))
        {
            for(int32 I=0;I<Buttons.Num();++I) { if(Buttons[I].IsInside(Mouse)) { Row=I;Choose();return true; } }
            for(int32 I=0;I<Points.Num();++I) { if(Points[I].IsInside(Mouse)) { Row=I;Choose();return true; } }
            int32 W=0,H=0;PC->GetViewportSize(W,H);const float S=FMath::Min(W/1280.f,H/720.f);
            if(S>0 && FBox2D(FVector2D(610*S,156*S),FVector2D(1224*S,554*S)).IsInside(Mouse))
            {
                AFiskBoardingController* MutablePC=CastChecked<AFiskBoardingController>(GetOwner());const AFiskStarterBoat* Boat=PC->FindStarterBoat();
                const FVector SeaPoint(FMath::RoundToDouble((Mouse.X/S-638)/560*150000),FMath::RoundToDouble((355-Mouse.Y/S)/180*100000),0);
                if(Boat) { MutablePC->ServerSelectSeaPoint(Boat->GetRouteRevision(),SeaPoint); }return true;
            }
        }
    }
    return true;
}
void UFiskSeaChart::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);if(bVisible && !CanOpen()) { Show(false); }
}
void UFiskSeaChart::Draw(AHUD& HUD,UCanvas& Canvas)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!Boat || !Day) { return; }
    if(bFavorites) { DrawFavorites(HUD,Canvas);return; }
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);
    const FLinearColor Teal(0.4f,0.9f,0.8f),Muted(0.65f,0.8f,0.83f);
    HUD.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.96f),24*S,28*S,1232*S,650*S);
    HUD.DrawText(TEXT("SJØKART"),FLinearColor::White,52*S,58*S,nullptr,2.5f*S);
    const int32 Seconds=Day->HasStarted()?Day->GetElapsedSeconds():0;
    HUD.DrawText(FString::Printf(TEXT("%02d:%02d  |  Dagsklokken fortsetter"),Seconds/60,Seconds%60),Muted,680*S,90*S,nullptr,1.1f*S);
    HUD.DrawText(TEXT("Tab: plasser / lagrede favoritter | Kapteinen velger ved brygga"),Muted,52*S,110*S,nullptr,1.0f*S);
    Buttons.Reset();Points.Reset();
    for(int32 I=0;I<3;++I)
    {
        const FFiskFishingGround& G=*FiskGrounds::Find(I);
        const float Y=(170+96*I)*S;
        Buttons.Add(FBox2D(FVector2D(52*S,Y),FVector2D(550*S,Y+82*S)));
        HUD.DrawRect(I==Row?FLinearColor(0.17f,0.39f,0.43f):FLinearColor(0.035f,0.12f,0.16f),52*S,Y,498*S,82*S);
        HUD.DrawText(FString::Printf(TEXT("%s%s"),G.Name,I==Boat->GetGroundId()?TEXT("  [VALGT]"):TEXT("")),I==Boat->GetGroundId()?Teal:FLinearColor::White,68*S,Y+10*S,nullptr,1.15f*S);
        HUD.DrawText(FString::Printf(TEXT("Reise %d s  |  Bunn %d m"),G.TravelMs/1000,G.BottomDepth),FLinearColor::White,68*S,Y+37*S,nullptr,1.0f*S);
        HUD.DrawText(FString::Printf(TEXT("Fisketegn omtrent %d-%d m"),G.FishDepthMin,G.FishDepthMax),Muted,68*S,Y+59*S,nullptr,0.9f*S);
    }
    auto Point=[&](FVector Position){return FVector2D((638+Position.X/150000.0*560)*S,(355-Position.Y/100000.0*180)*S);};
    const FVector2D Marina=Point(FVector::ZeroVector);
    HUD.DrawRect(FLinearColor(0.025f,0.19f,0.25f),610*S,156*S,614*S,398*S);
    HUD.DrawRect(FLinearColor(0.33f,0.36f,0.21f),610*S,250*S,30*S,155*S);
    for(int32 I=0;I<2;++I)
    {
        const FVector2D P=Point(FiskRouteGeometry::Island(I));const double R=FiskRouteGeometry::Radius(I);
        Canvas.K2_DrawPolygon(nullptr,P,FVector2D((R+2000)/150000*560*S,(R+2000)/100000*180*S),32,FLinearColor(.18f,.3f,.29f));
        Canvas.K2_DrawPolygon(nullptr,P,FVector2D(R/150000*560*S,R/100000*180*S),32,FLinearColor(.52f,.43f,.24f));
    }
    const TArray<FVector> RoutePath=FiskRouteGeometry::Path(Boat->GetDestination());
    for(int32 I=1;I<RoutePath.Num();++I) { const FVector2D A=Point(RoutePath[I-1]),B=Point(RoutePath[I]);HUD.DrawLine(A.X,A.Y,B.X,B.Y,Teal,3*S); }
    const FVector2D Target=Point(Boat->GetDestination());HUD.DrawRect(FLinearColor(1,.7f,.25f),Target.X-5*S,Target.Y-5*S,10*S,10*S);
    for(int32 I=0;I<3;++I)
    {
        const FFiskFishingGround& G=*FiskGrounds::Find(I);const FVector2D Pos=Point(G.Position);
        const FLinearColor Color=I==Boat->GetGroundId()?Teal:Muted;
        HUD.DrawLine(Marina.X,Marina.Y,Pos.X,Pos.Y,Color,I==Boat->GetGroundId()?3*S:1*S);
        HUD.DrawRect(Color,Pos.X-7*S,Pos.Y-7*S,14*S,14*S);
        Points.Add(FBox2D(Pos-FVector2D(14*S,14*S),Pos+FVector2D(14*S,14*S)));
        HUD.DrawText(G.Name,Color,Pos.X+16*S,Pos.Y-15*S,nullptr,0.85f*S);
    }
    HUD.DrawRect(FLinearColor::White,Marina.X-5*S,Marina.Y-5*S,10*S,10*S);
    HUD.DrawText(TEXT("Marina"),FLinearColor::White,Marina.X-25*S,Marina.Y+20*S,nullptr,1.0f*S);
    const FVector2D BoatPos=Point(Boat->GetActorLocation());
    HUD.DrawRect(FLinearColor(1,0.8f,0.3f),BoatPos.X-3*S,BoatPos.Y-3*S,6*S,6*S);
    HUD.DrawText(TEXT("Båt"),FLinearColor(1,0.8f,0.3f),BoatPos.X+9*S,BoatPos.Y+8*S,nullptr,0.9f*S);
    HUD.DrawText(TEXT("N"),Muted,1184*S,176*S,nullptr,1.15f*S);
    const FFiskFishingGround& Selected=*FiskGrounds::Find(Boat->GetGroundId());
    const int32 Ms=Day->HasStarted()?Day->GetElapsedMilliseconds():0;
    const int32 Margin=Boat->ReturnMarginMs(Ms)/1000;
    const bool bHome=Boat->GetTripProgress()==AFiskStarterBoat::AtMarina;
    const FLinearColor Risk=bHome?Teal:Margin<=0?FLinearColor(1,0.35f,0.25f):Margin<=60?FLinearColor(1,0.75f,0.25f):Teal;
    HUD.DrawText(FString::Printf(TEXT("Valgt rute: %s"),Selected.Name),Teal,52*S,486*S,nullptr,1.15f*S);
    HUD.DrawText(FString::Printf(TEXT("Punkt %.0f, %.0f m | Reise %d s"),Boat->GetDestination().X/100,Boat->GetDestination().Y/100,Boat->GetTravelMs()/1000),FLinearColor::White,52*S,519*S,nullptr,.9f*S);
    HUD.DrawText(bHome?TEXT("Båten er tilbake i marinaen."):FString::Printf(TEXT("Returfrist 17:00 | Margin ca. %s%d:%02d"),Margin<0?TEXT("-"):TEXT(""),FMath::Abs(Margin)/60,FMath::Abs(Margin)%60),Risk,610*S,575*S,nullptr,1.2f*S);
    HUD.DrawText(TEXT("Klikk i sjøen for eget punkt. Øyer og marina avvises."),Muted,610*S,611*S,nullptr,0.85f*S);
    const FString Hint=!Message.IsEmpty()?Message:Boat->GetTripProgress()!=AFiskStarterBoat::AtDock?TEXT("Ruten er låst. Lukk kartet før R for retur."):TEXT("Piler: velg rad. Enter eller klikk: velg plass.");
    HUD.DrawText(Hint,FLinearColor(1,0.85f,0.5f),52*S,559*S,nullptr,0.95f*S);
    Buttons.Add(FBox2D(FVector2D(52*S,615*S),FVector2D(550*S,651*S)));
    HUD.DrawRect(Row==3?FLinearColor(0.17f,0.39f,0.43f):FLinearColor(0.035f,0.12f,0.16f),52*S,615*S,498*S,36*S);
    HUD.DrawText(TEXT("Tilbake  (M / Escape)"),FLinearColor::White,68*S,624*S,nullptr,1.0f*S);
}
