#include "UI/FiskCrewPanel.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskSeaChart.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"
UFiskCrewPanel::UFiskCrewPanel() { PrimaryComponentTick.bCanEverTick=true; }
void UFiskCrewPanel::BeginPlay()
{
    Super::BeginPlay();
    if(!CastChecked<AFiskBoardingController>(GetOwner())->IsLocalController()) { SetComponentTickEnabled(false);return; }
#if WITH_DEV_AUTOMATION_TESTS
    if(FParse::Param(FCommandLine::Get(),TEXT("FiskCaptainSmoke")))
    {
        FParse::Value(FCommandLine::Get(),TEXT("FiskScreenshotDir="),CaptureDir);
        GetWorld()->GetTimerManager().SetTimer(SmokeTimer,this,&UFiskCrewPanel::Smoke,0.5f,true,3.0f);
    }
#endif
}
bool UFiskCrewPanel::CanOpen() const
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();
    return Boat && PC->PlayerState && Boat->IsAboard(PC->PlayerState->GetPlayerId());
}
void UFiskCrewPanel::Show(bool bShow)
{
    bVisible=bShow;Row=0;Message.Reset();
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());PC->bShowMouseCursor=bShow;
    if(bShow)
    {
        if(PC->GetRod()->GetState()==EFiskRodState::Fighting) { PC->ReleaseRodInputsForMenu(); }
        FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);PC->SetInputMode(Mode);
    }
    else { PC->SetInputMode(FInputModeGameOnly()); }
}
void UFiskCrewPanel::Choose()
{
    AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();if(!Boat || !PC->PlayerState) { return; }
    if(Row>=Boat->GetCrewCount()) { Show(false);return; }
    if(Boat->GetCaptainId()!=PC->PlayerState->GetPlayerId()) { Message=TEXT("Bare kapteinen kan overføre rollen.");return; }
    const int32 Target=Boat->GetCrewMemberId(Row);
    if(Target==PC->PlayerState->GetPlayerId()) { Message=TEXT("Du er allerede kaptein. Velg medspilleren.");return; }
    Message=TEXT("Venter på vertens bekreftelse ...");PC->ServerTransferCaptain(Boat->GetRouteRevision(),Target);
}
bool UFiskCrewPanel::HandleKey(const FInputKeyEventArgs& Args)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(!PC->IsLocalController() || PC->GetFrontEnd()->IsVisible() || PC->GetRigPanel()->IsVisible() || PC->GetSeaChart()->IsVisible()) { return false; }
    const bool Press=Args.Event==IE_Pressed;
    if(Args.Key==EKeys::K && Press) { if(bVisible || CanOpen()) { Show(!bVisible); }return true; }
    if(!bVisible) { return false; }
    if(!Press) { return true; }
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();const int32 Rows=Boat?Boat->GetCrewCount()+1:1;
    if(Args.Key==EKeys::Escape) { Show(false); }
    else if(Args.Key==EKeys::Up) { Row=(Row+Rows-1)%Rows; }
    else if(Args.Key==EKeys::Down || Args.Key==EKeys::Tab) { Row=(Row+1)%Rows; }
    else if(Args.Key==EKeys::Home) { Row=0; }
    else if(Args.Key==EKeys::End) { Row=Rows-1; }
    else if(Args.Key==EKeys::Enter || Args.Key==EKeys::SpaceBar) { Choose(); }
    else if(Args.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;
        if(UGameViewportClient* VP=GetWorld()->GetGameViewport();VP && VP->GetMousePosition(Mouse))
        { for(int32 I=0;I<Buttons.Num();++I) { if(Buttons[I].IsInside(Mouse)) { Row=I;Choose();break; } } }
    }
    return true;
}
void UFiskCrewPanel::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Dt,Type,Tick);
    if(bVisible)
    {
        if(!CanOpen()) { Show(false); }
        else { Row=FMath::Clamp(Row,0,CastChecked<AFiskBoardingController>(GetOwner())->FindStarterBoat()->GetCrewCount()); }
    }
}
void UFiskCrewPanel::Draw(AHUD& HUD,UCanvas& Canvas)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const AFiskStarterBoat* Boat=PC->FindStarterBoat();const AFiskDayState* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!Boat || !PC->PlayerState) { return; }
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);const int32 Self=PC->PlayerState->GetPlayerId();
    const FLinearColor Teal(0.4f,0.9f,0.8f),Muted(0.65f,0.8f,0.83f);
    HUD.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.96f),24*S,28*S,650*S,642*S);
    HUD.DrawText(TEXT("MANNSKAP"),FLinearColor::White,52*S,68*S,nullptr,2.4f*S);
    HUD.DrawText(TEXT("Kapteinen bestemmer fiskeplass og retur."),Muted,52*S,124*S,nullptr,1.1f*S);
    const int32 Seconds=Day && Day->HasStarted()?Day->GetElapsedSeconds():0;
    HUD.DrawText(FString::Printf(TEXT("%02d:%02d  |  Klokken fortsetter"),Seconds/60,Seconds%60),Teal,52*S,164*S,nullptr,1.05f*S);
    Buttons.Reset();
    for(int32 I=0;I<=Boat->GetCrewCount();++I)
    {
        const float Y=(230+82*I)*S;Buttons.Add(FBox2D(FVector2D(52*S,Y),FVector2D(646*S,Y+62*S)));
        HUD.DrawRect(Row==I?FLinearColor(0.17f,0.39f,0.43f):FLinearColor(0.035f,0.12f,0.16f),52*S,Y,594*S,62*S);
        FString Label=TEXT("Tilbake");
        if(I<Boat->GetCrewCount())
        {
            const int32 Id=Boat->GetCrewMemberId(I);const bool Captain=Id==Boat->GetCaptainId();
            Label=FString::Printf(TEXT("%s  -  %s"),Id==Self?TEXT("Deg"):TEXT("Medspiller"),Captain?TEXT("KAPTEIN"):TEXT("Fisker"));
            if(!Captain && Boat->GetCaptainId()==Self) { Label+=TEXT("  |  Gi kapteinsrollen"); }
        }
        HUD.DrawText(Label,FLinearColor::White,70*S,Y+20*S,nullptr,1.15f*S);
    }
    if(!Message.IsEmpty()) { HUD.DrawText(Message,FLinearColor(1,0.85f,0.5f),52*S,520*S,nullptr,0.9f*S); }
    HUD.DrawText(TEXT("Piler: velg. Klikk eller Enter: overfør rollen."),Muted,52*S,570*S,nullptr,1.0f*S);
    HUD.DrawText(TEXT("K / Escape: lukk. Reisen fortsetter ved bytte."),Muted,52*S,607*S,nullptr,1.0f*S);
}
