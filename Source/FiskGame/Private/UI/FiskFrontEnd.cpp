#include "UI/FiskFrontEnd.h"
#include "UI/FiskGameInstance.h"
#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "TimerManager.h"

UFiskFrontEnd::UFiskFrontEnd() { PrimaryComponentTick.bCanEverTick = true; }
void UFiskFrontEnd::BeginPlay()
{
    Super::BeginPlay();
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    if (!PC->IsLocalController()) { SetComponentTickEnabled(false); return; }
    bLocalFrontEnd = true;
    UFiskGameInstance* GI = CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    Model.bSound = GI->bSound; Model.FrameRate = GI->FrameRate;
    GI->ApplySettings(false);
    Model.Show(GetNetMode() == NM_Standalone ? EFiskMenuPage::Home : EFiskMenuPage::Closed);
    Model.Message = GI->ConnectionMessage; GI->ConnectionMessage.Reset();
    SyncInput();
#if WITH_DEV_AUTOMATION_TESTS
    FParse::Value(FCommandLine::Get(), TEXT("FiskMenuSmoke="), SmokeCase);
    FParse::Value(FCommandLine::Get(), TEXT("FiskScreenshotDir="), ScreenshotDirectory);
    if (!SmokeCase.IsEmpty()) { GetWorld()->GetTimerManager().SetTimer(SmokeTimer, this, &UFiskFrontEnd::RunMenuSmoke, 1.0f, true, 2.0f); }
#endif
}
void UFiskFrontEnd::SyncInput()
{
    APlayerController* PC = CastChecked<APlayerController>(GetOwner());
    PC->bShowMouseCursor = IsVisible();
    if (IsVisible())
    {
        FInputModeGameAndUI Mode; Mode.SetHideCursorDuringCapture(false); Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
    }
    else { bControl = bShift = false; PC->SetInputMode(FInputModeGameOnly()); }
}
void UFiskFrontEnd::CancelConnection(const FString& Message)
{
    if (GEngine)
    {
        // ClientTravel queues a URL before PendingNetGame exists. Cancel both stages.
        if (FWorldContext* Context = GEngine->GetWorldContextFromWorld(GetWorld())) { Context->TravelURL.Reset(); }
        GEngine->CancelPending(GetWorld());
    }
    Model.Show(EFiskMenuPage::Home); Model.Message = Message;
    SyncInput();
}
void UFiskFrontEnd::Dispatch(EFiskMenuAction Action)
{
    UFiskGameInstance* GI = CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    if (Action == EFiskMenuAction::Host)
    {
        GI->ConnectionMessage.Reset();
        Model.Show(EFiskMenuPage::Connecting); Model.Message = TEXT("Starter vert ..."); ConnectingAt = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_HOST"));
        UGameplayStatics::OpenLevel(this, TEXT("/Engine/Maps/Entry"), true, TEXT("listen"));
    }
    else if (Action == EFiskMenuAction::Join)
    {
        FString Endpoint;
        if (!FFiskMenuModel::ParseAddress(Model.Address, Endpoint)) { return; }
        GI->ConnectionMessage.Reset();
        Model.Show(EFiskMenuPage::Connecting); Model.Message = TEXT("Kobler til vennen din ..."); ConnectingAt = FPlatformTime::Seconds();
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_JOIN endpoint=%s"), *Endpoint);
        PC->ClientTravel(Endpoint, TRAVEL_Absolute);
    }
    else if (Action == EFiskMenuAction::Cancel) { CancelConnection(TEXT("Tilkoblingen ble avbrutt.")); UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_CANCELLED")); }
    else if (Action == EFiskMenuAction::Sound || Action == EFiskMenuAction::FrameRate)
    { GI->bSound = Model.bSound; GI->FrameRate = Model.FrameRate; GI->ApplySettings(true); }
    else if (Action == EFiskMenuAction::Leave)
    {
        GI->ConnectionMessage.Reset();
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_LEAVE"));
        UGameplayStatics::OpenLevel(this, TEXT("/Engine/Maps/Entry"), true);
    }
    else if (Action == EFiskMenuAction::Quit)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_QUIT"));
        // A pending travel may temporarily detach Player from the old controller.
        // The game engine owns process exit; PIE keeps the editor's usual quit path.
        if (!GIsEditor && GEngine) { GEngine->Exec(GetWorld(), TEXT("QUIT"), *GLog); }
        else { UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false); }
    }
    SyncInput();
}
bool UFiskFrontEnd::HandleKey(const FInputKeyEventArgs& Args)
{
    AFiskBoardingController* PC = CastChecked<AFiskBoardingController>(GetOwner());
    if (!bLocalFrontEnd) { return false; }
    const FKey Key = Args.Key;
    const bool bPressed = Args.Event == IE_Pressed || Args.Event == IE_Repeat;
    if (Key == EKeys::Escape && Args.Event == IE_Pressed)
    {
        if (!IsVisible())
        {
            if (PC->GetRod()->GetState() == EFiskRodState::Fighting) { PC->ReleaseRodInputsForMenu(); }
            Model.Show(EFiskMenuPage::Session); SyncInput();
        }
        else { Dispatch(Model.Back()); }
        return true;
    }
    if (!IsVisible()) { return false; }
    if (Key == EKeys::LeftControl || Key == EKeys::RightControl) { bControl = Args.Event != IE_Released; return true; }
    if (Key == EKeys::LeftShift || Key == EKeys::RightShift) { bShift = Args.Event != IE_Released; return true; }
    if (!bPressed) { return true; }
    if (Key == EKeys::Up) { Model.Move(-1); }
    else if (Key == EKeys::Down || Key == EKeys::Tab) { Model.Move(Key == EKeys::Tab && bShift ? -1 : 1); }
    else if (Key == EKeys::Home) { Model.Selected = 0; }
    else if (Key == EKeys::End) { Model.Selected = FMath::Max(0, Model.Rows() - 1); }
    else if (Key == EKeys::Enter || Key == EKeys::SpaceBar) { Dispatch(Model.Activate()); }
    else if (Key == EKeys::LeftMouseButton)
    { FVector2D Position; if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport()) { if (Viewport->GetMousePosition(Position)) { Click(Position); } } }
    else if (Key == EKeys::BackSpace) { Model.Backspace(); }
    else if (Key == EKeys::A && bControl && Model.Page == EFiskMenuPage::Join && Model.Selected == 0) { Model.Address.Reset(); }
    else
    {
        const FKey Digits[] = {EKeys::Zero,EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine};
        const FKey Numpad[] = {EKeys::NumPadZero,EKeys::NumPadOne,EKeys::NumPadTwo,EKeys::NumPadThree,EKeys::NumPadFour,EKeys::NumPadFive,EKeys::NumPadSix,EKeys::NumPadSeven,EKeys::NumPadEight,EKeys::NumPadNine};
        for (int32 I = 0; I < 10; ++I) { if (Key == Digits[I] || Key == Numpad[I]) { Model.Edit(TEXT('0') + I); } }
        if (Key == EKeys::Period) { Model.Edit(bShift ? TEXT(':') : TEXT('.')); }
        if (Key == EKeys::Decimal) { Model.Edit(TEXT('.')); }
        if (Key == EKeys::Semicolon && bShift) { Model.Edit(TEXT(':')); }
    }
    return true;
}
void UFiskFrontEnd::Click(FVector2D Position)
{
    for (int32 I = 0; I < Buttons.Num(); ++I)
    {
        if (Buttons[I].IsInside(Position))
        {
            Model.Selected = I;
            if (Model.Page == EFiskMenuPage::Join && I == 0) { return; }
            UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_POINTER row=%d"), I);
            Dispatch(Model.Activate()); return;
        }
    }
}
void UFiskFrontEnd::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick)
{
    Super::TickComponent(DeltaTime, TickType, Tick);
    UFiskGameInstance* GI = CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    if (!GI->ConnectionMessage.IsEmpty() && (GetNetMode() == NM_Standalone || Model.Page == EFiskMenuPage::Connecting))
    {
        const bool bConnecting = Model.Page == EFiskMenuPage::Connecting;
        const FString Message = GI->ConnectionMessage; GI->ConnectionMessage.Reset(); CancelConnection(Message);
        if (bConnecting) { UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_CONNECTION_FAILED")); }
    }
    if (Model.Page == EFiskMenuPage::Connecting && FPlatformTime::Seconds() - ConnectingAt >= 15.0)
    {
        CancelConnection(TEXT("Fant ikke verten. Kontroller adressen og at verten er startet."));
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_MENU_CONNECTION_FAILED"));
    }
    if (IsVisible())
    {
        FVector2D Current;
        UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
        if (Viewport && Viewport->GetMousePosition(Current))
        {
            if (LastMouse.X >= 0 && FVector2D::Distance(Current, LastMouse) > 1.0f)
            { for (int32 I = 0; I < Buttons.Num(); ++I) { if (Buttons[I].IsInside(Current)) { Model.Selected = I; } } }
            LastMouse = Current;
        }
    }
}
FString UFiskFrontEnd::Label(int32 Row) const
{
    if (Model.Page == EFiskMenuPage::Home)
    { const TCHAR* Labels[] = {TEXT("Spill"),TEXT("Bli med venner"),TEXT("Innstillinger"),TEXT("Avslutt")}; return Labels[Row]; }
    if (Model.Page == EFiskMenuPage::Session)
    { const TCHAR* Labels[] = {TEXT("Fortsett"),TEXT("Innstillinger"),TEXT("Til hovedmeny"),TEXT("Avslutt")}; return Labels[Row]; }
    if (Model.Page == EFiskMenuPage::Join)
    { return Row == 0 ? Model.Address + (Model.Selected == 0 ? TEXT(" |") : TEXT("")) : Row == 1 ? TEXT("Koble til") : TEXT("Tilbake"); }
    if (Model.Page == EFiskMenuPage::Settings)
    { return Row == 0 ? (Model.bSound ? TEXT("Hovedlyd: På") : TEXT("Hovedlyd: Av")) : Row == 1 ? FString::Printf(TEXT("Bildefrekvens: %d"), Model.FrameRate) : TEXT("Tilbake"); }
    return TEXT("Avbryt");
}
void UFiskFrontEnd::Draw(AHUD& HUD, UCanvas& Canvas)
{
    const float S = FMath::Min(Canvas.ClipX / 1280.0f, Canvas.ClipY / 720.0f);
    const float X = 52 * S, Width = 424 * S, RowY = 258 * S, Step = 66 * S;
    HUD.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.94f), 24*S, 28*S, 486*S, 642*S);
    HUD.DrawRect(FLinearColor(0.25f,0.85f,0.75f), X, 64*S, 62*S, 4*S);
    HUD.DrawText(TEXT("FISKELAGET"),FLinearColor::White,X,90*S,nullptr,2.7f*S);
    HUD.DrawText(TEXT("Karibia venter."),FLinearColor(0.68f,0.83f,0.84f),X,145*S,nullptr,1.3f*S);
    const TCHAR* Title = Model.Page == EFiskMenuPage::Join ? TEXT("BLI MED VENNER - IP-ADRESSE")
        : Model.Page == EFiskMenuPage::Settings ? TEXT("INNSTILLINGER") : Model.Page == EFiskMenuPage::Session ? TEXT("MENY")
        : Model.Page == EFiskMenuPage::Connecting ? TEXT("ET ØYEBLIKK ...") : TEXT("VELKOMMEN OM BORD");
    HUD.DrawText(Title,FLinearColor(0.38f,0.87f,0.77f),X,212*S,nullptr,1.0f*S);
    Buttons.Reset();
    for (int32 I = 0; I < Model.Rows(); ++I)
    {
        const bool Selected = I == Model.Selected;
        const FVector2D Start(X,RowY+Step*I), Size(Width,52*S);
        Buttons.Add(FBox2D(Start,Start+Size));
        HUD.DrawRect(Selected ? FLinearColor(0.18f,0.45f,0.46f,1) : FLinearColor(0.035f,0.13f,0.16f,1),Start.X,Start.Y,Size.X,Size.Y);
        if (Selected) { HUD.DrawRect(FLinearColor(0.55f,0.94f,0.79f),Start.X,Start.Y,4*S,Size.Y); }
        HUD.DrawText(Label(I),FLinearColor::White,Start.X+18*S,Start.Y+14*S,nullptr,1.3f*S);
    }
    if (!Model.Message.IsEmpty()) { HUD.DrawText(Model.Message,FLinearColor(1,0.8f,0.5f),X,548*S,nullptr,0.85f*S); }
    else if (Model.Page == EFiskMenuPage::Home) { HUD.DrawText(TEXT("Spill starter en vert. Venner kan koble til på LAN."),FLinearColor(0.65f,0.78f,0.80f),X,548*S,nullptr,0.9f*S); }
    else if (Model.Page == EFiskMenuPage::Join) { HUD.DrawText(TEXT("Skriv vertens adresse. Ctrl+A erstatter teksten."),FLinearColor(0.65f,0.78f,0.80f),X,548*S,nullptr,0.9f*S); }
    if (GetNetMode() != NM_Standalone) { HUD.DrawText(TEXT("Spillet fortsetter mens menyen er åpen."),FLinearColor(1,0.8f,0.5f),X,582*S,nullptr,0.9f*S); }
    HUD.DrawText(TEXT("Mus eller piltaster / Tab   Enter: velg   Esc: tilbake"),FLinearColor(0.65f,0.78f,0.80f),X,626*S,nullptr,0.85f*S);
    if (!bReadyReported) { bReadyReported = true; UE_LOG(LogTemp,Display,TEXT("FISKELAGET_MENU_READY")); }
}
