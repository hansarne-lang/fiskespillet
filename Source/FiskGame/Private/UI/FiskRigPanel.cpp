#include "UI/FiskRigPanel.h"
#include "UI/FiskFrontEnd.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

UFiskRigPanel::UFiskRigPanel() { PrimaryComponentTick.bCanEverTick = true; }
void UFiskRigPanel::BeginPlay()
{
    Super::BeginPlay();
    const AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    if (!PC->IsLocalController()) { SetComponentTickEnabled(false); return; }
#if WITH_DEV_AUTOMATION_TESTS
    if (FParse::Param(FCommandLine::Get(), TEXT("FiskRigSmoke")))
    {
        FParse::Value(FCommandLine::Get(), TEXT("FiskScreenshotDir="), CaptureDir);
        GetWorld()->GetTimerManager().SetTimer(SmokeTimer, this, &UFiskRigPanel::Smoke, 1.0f, true, 3.0f);
    }
#endif
}
void UFiskRigPanel::Show(bool bShow)
{
    bVisible = bShow; Row = 0;
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    PC->bShowMouseCursor = bShow;
    if (bShow)
    {
        PC->ReleaseRodInputsForMenu();
        FInputModeGameAndUI Mode; Mode.SetHideCursorDuringCapture(false); Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); PC->SetInputMode(Mode);
    }
    else { PC->SetInputMode(FInputModeGameOnly()); }
}
void UFiskRigPanel::Change(int32 Direction)
{
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    const FFiskRigSettings& Rig = PC->GetRod()->GetRig();
    int32 Bait=Rig.Bait, Depth=Rig.Depth, Program=Rig.Program;
    if (Row==0) { Bait=(Bait+Direction+3)%3; }
    else if (Row==1) { Depth=10+((Depth/10-1+Direction+6)%6)*10; }
    else if (Row==2) { Program=(Program+Direction+3)%3; }
    else { Show(false); return; }
    PC->RequestRig(Bait,Depth,Program);
}
bool UFiskRigPanel::HandleKey(const FInputKeyEventArgs& Args)
{
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    if (!PC->IsLocalController() || PC->GetFrontEnd()->IsVisible()) { return false; }
    const bool bPress = Args.Event==IE_Pressed;
    if (Args.Key==EKeys::G && bPress)
    {
        if (bVisible || PC->CanOpenRig()) { Show(!bVisible); }
        return true;
    }
    if (!bVisible) { return false; }
    if (!bPress) { return true; }
    if (Args.Key==EKeys::Escape) { Show(false); }
    else if (Args.Key==EKeys::Up) { Row=(Row+3)%4; }
    else if (Args.Key==EKeys::Down || Args.Key==EKeys::Tab) { Row=(Row+1)%4; }
    else if (Args.Key==EKeys::Home) { Row=0; }
    else if (Args.Key==EKeys::Left) { Change(-1); }
    else if (Args.Key==EKeys::Right || Args.Key==EKeys::Enter || Args.Key==EKeys::SpaceBar) { Change(1); }
    else if (Args.Key==EKeys::LeftMouseButton)
    {
        FVector2D Mouse;
        if (UGameViewportClient* VP=GetWorld()->GetGameViewport(); VP && VP->GetMousePosition(Mouse))
        {
            for (int32 I=0;I<Buttons.Num();++I) { if (Buttons[I].IsInside(Mouse)) { Row=I;Change(1);break; } }
        }
    }
    return true;
}
void UFiskRigPanel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick)
{
    Super::TickComponent(DeltaTime,TickType,Tick);
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if (bVisible && !PC->CanOpenRig()) { Show(false); }
}
void UFiskRigPanel::Draw(AHUD& HUD,UCanvas& Canvas)
{
    const AFiskBoardingController* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const FFiskRigSettings& Rig=PC->GetRod()->GetRig();
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f), X=52*S, W=460*S;
    HUD.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.96f),24*S,28*S,516*S,642*S);
    HUD.DrawText(TEXT("STANGOPPSETT"),FLinearColor::White,X,70*S,nullptr,2.1f*S);
    HUD.DrawText(TEXT("Revplassen - bunndybde 60 m"),FLinearColor(0.55f,0.85f,0.8f),X,126*S,nullptr,1.15f*S);
    HUD.DrawText(TEXT("Fisketegn ved 20-40 m"),FLinearColor::White,X,158*S,nullptr,1.1f*S);
    const FString Labels[] = {FString(TEXT("Agn: "))+FFiskRigRules::BaitName(Rig.Bait),FString::Printf(TEXT("Fiskedybde: %d m"),Rig.Depth),FString(TEXT("Program: "))+FFiskRigRules::ProgramName(Rig.Program),TEXT("Ferdig")};
    Buttons.Reset();
    for (int32 I=0;I<4;++I)
    {
        FVector2D A(X,(220+66*I)*S), B(X+W,(272+66*I)*S); Buttons.Add(FBox2D(A,B));
        HUD.DrawRect(I==Row?FLinearColor(0.18f,0.45f,0.46f):FLinearColor(0.035f,0.13f,0.16f),A.X,A.Y,W,52*S);
        HUD.DrawText(Labels[I],FLinearColor::White,A.X+16*S,A.Y+14*S,nullptr,1.2f*S);
    }
    HUD.DrawText(FFiskRigRules::QualityName(Rig),FLinearColor(0.65f,0.95f,0.75f),X,510*S,nullptr,1.2f*S);
    HUD.DrawText(TEXT("Piler: velg/endre. Klikk eller Enter: neste valg."),FLinearColor::White,X,552*S,nullptr,0.9f*S);
    HUD.DrawText(TEXT("G / Escape: lukk. Klokken fortsetter."),FLinearColor::White,X,580*S,nullptr,0.9f*S);
    HUD.DrawText(TEXT("Valgene bekreftes av verten. Trekk inn før endring."),FLinearColor(0.7f,0.8f,0.8f),X,616*S,nullptr,0.85f*S);
}
