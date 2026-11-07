#include "Fishing/FiskFishingHelp.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "World/FiskPierWalk.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"

namespace
{
bool HasFish(const UFiskPrototypeRod* Rod)
{
    return Rod && (Rod->GetState()==EFiskRodState::InitialRun || Rod->GetState()==EFiskRodState::Fighting || Rod->GetState()==EFiskRodState::ReadyToLand);
}
}
UFiskFishingHelp::UFiskFishingHelp()
{
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.TickInterval=.1f;
}
bool UFiskFishingHelp::HasGameView() const
{
    const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    return PC->IsLocalController() && !PC->GetPierWalk()->IsEnabled() && !PC->GetPierWalk()->HasMenu();
}
void UFiskFishingHelp::ToggleAssist()
{
#if !UE_BUILD_SHIPPING
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(!PC->IsLocalController()) { return; }
    if(bAssisting) { StopAssist(true);return; }
    if(!HasGameView() || !PC->PlayerState || PC->IsRodSelectionPending()) { return; }
    const auto* Boat=PC->FindStarterBoat();
    if(!Boat || !Boat->IsAboard(PC->PlayerState->GetPlayerId()) || Boat->GetTripProgress()!=AFiskStarterBoat::AtFishingSpot)
    { PC->ClientActionFeedback_Implementation(TEXT("Å: AI-testhjelp brukes etter napp ved fiskeplassen."));return; }
    TargetSlot=PC->GetSelectedRod();
    if(!HasFish(PC->GetRodAt(TargetSlot)))
    {
        TargetSlot=-1;
        for(int32 I=0;I<PC->GetRodCapacity();++I) { if(HasFish(PC->GetRodAt(I))) { TargetSlot=I;break; } }
    }
    if(TargetSlot<0) { PC->ClientActionFeedback_Implementation(TEXT("Vent på napp. Å lar AI forsøke å lande én fisk for deg."));return; }
    TargetCast=PC->GetRodAt(TargetSlot)->GetCastId();
    PC->ReleaseRodInputsForMenu();
    bAssisting=true;Pilot=FFiskTestPilot();NextControlAt=0;
    bWaitingSelection=PC->GetSelectedRod()!=TargetSlot;
    SelectionUntil=GetWorld()->GetRealTimeSeconds()+5;
    if(bWaitingSelection) { PC->RequestRodSelection(TargetSlot); }
    PC->ClientActionFeedback_Implementation(FString::Printf(TEXT("AI-TESTHJELP: forsøker å lande stang %d. Å eller manuell kamp stopper hjelpen."),TargetSlot+1));
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TEST_ASSIST_STARTED slot=%d cast=%s"),TargetSlot,*TargetCast.ToString(EGuidFormats::Digits));
#endif
}
void UFiskFishingHelp::StopAssist(bool Notify)
{
    if(!bAssisting) { return; }
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    if(PC->GetSelectedRod()==TargetSlot && PC->GetRod()->GetCastId()==TargetCast) { PC->RequestLower();PC->RequestStopReel(); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TEST_ASSIST_STOPPED slot=%d"),TargetSlot);
    bAssisting=bWaitingSelection=false;TargetSlot=-1;TargetCast.Invalidate();
    if(Notify) { PC->ClientActionFeedback_Implementation(TEXT("AI-testhjelp av. Du styrer fiskekampen.")); }
}
void UFiskFishingHelp::RequestReturn()
{
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());
    const auto* Boat=PC->FindStarterBoat();const auto* Day=GetWorld()->GetGameState<AFiskDayState>();
    if(!Boat || !Day || !PC->PlayerState) { return; }
    if(Boat->GetCaptainId()!=PC->PlayerState->GetPlayerId()) { PC->ClientActionFeedback_Implementation(TEXT("Kapteinen styrer hjemreisen. R / Tilbake til marina brukes av kapteinen."));return; }
    if(Boat->GetTripProgress()>100) { PC->ClientActionFeedback_Implementation(TEXT("Båten er allerede på vei hjem eller tilbake ved marinaen."));return; }
    if(Boat->GetTripProgress()<0) { PC->ClientActionFeedback_Implementation(TEXT("Båten er allerede ved brygga."));return; }
    StopAssist();PC->ReleaseRodInputsForMenu();
    PC->ServerReturnToMarina(Boat->GetRouteRevision(),Day->GetDayNumber(),Boat->GetLegSerial());
}
bool UFiskFishingHelp::HandleKey(const FInputKeyEventArgs& Args)
{
    const bool Press=Args.Event==IE_Pressed,Release=Args.Event==IE_Released;
    if(Press && Args.Key==EKeys::LeftMouseButton) { bButtonHeld=false; }
    if(Release && Args.Key==EKeys::LeftMouseButton && bButtonHeld) { bButtonHeld=false;return true; }
    if(!HasGameView()) { return false; }
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* Boat=PC->FindStarterBoat();
#if !UE_BUILD_SHIPPING
    if(FiskQuickControls::IsAssistKey(Args.Key)) { if(Press) { ToggleAssist(); }return true; }
#endif
    if(Press && Args.Key==EKeys::R && Boat && Boat->GetTripProgress()>=0)
    { RequestReturn();return true; }
    if(Press && Args.Key==EKeys::LeftMouseButton && Boat && PC->PlayerState && Boat->IsAboard(PC->PlayerState->GetPlayerId()))
    {
        auto* View=GetWorld()->GetGameViewport();FVector2D Mouse,Size;
        if(View && View->GetMousePosition(Mouse))
        {
            View->GetViewportSize(Size);const float S=FMath::Min(Size.X/1280.f,Size.Y/720.f);
            if(Boat->GetTripProgress()>=0 && Boat->GetTripProgress()<200 && FiskQuickControls::ReturnBox(S).IsInside(Mouse))
            { bButtonHeld=true;RequestReturn();return true; }
#if !UE_BUILD_SHIPPING
            if(Boat->GetTripProgress()==100 && FiskQuickControls::AssistBox(S).IsInside(Mouse))
            { bButtonHeld=true;ToggleAssist();return true; }
#endif
        }
    }
    const bool Manual=Args.Key==EKeys::LeftMouseButton || Args.Key==EKeys::RightMouseButton || Args.Key==EKeys::MouseScrollUp || Args.Key==EKeys::MouseScrollDown || Args.Key==EKeys::SpaceBar || Args.Key==EKeys::X || Args.Key==EKeys::One || Args.Key==EKeys::Two || Args.Key==EKeys::Three;
    if(Press && Manual && bAssisting) { StopAssist(true); }
    return false;
}
void UFiskFishingHelp::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);
    auto* PC=CastChecked<AFiskBoardingController>(GetOwner());if(!PC->IsLocalController()) { return; }
    const auto* Boat=PC->FindStarterBoat();const bool Aboard=Boat && PC->PlayerState && Boat->IsAboard(PC->PlayerState->GetPlayerId());
    if(HasGameView())
    {
        if(Aboard && (!bManagedCursor || !PC->bShowMouseCursor))
        {
            FInputModeGameAndUI Mode;Mode.SetHideCursorDuringCapture(false);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(Mode);PC->bShowMouseCursor=true;bManagedCursor=true;
            // A chart click must not become a return-button click on the first pump.
            int32 Width=0,Height=0;PC->GetViewportSize(Width,Height);
            PC->SetMouseLocation(Width/2,Height/2);bButtonHeld=false;
        }
        else if(!Aboard && bManagedCursor) { PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());bManagedCursor=false; }
    }
    else { bManagedCursor=false;bButtonHeld=false; }
#if !UE_BUILD_SHIPPING
    if(!bAssisting) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();
    if(bWaitingSelection)
    {
        if(Now>SelectionUntil) { StopAssist(true);return; }
        if(PC->IsRodSelectionPending()) { return; }
        bWaitingSelection=false;
    }
    const auto* Rod=PC->GetRod();
    if(!Aboard || !Boat || Boat->GetTripProgress()!=100 || PC->GetSelectedRod()!=TargetSlot || Rod->GetCastId()!=TargetCast || !HasFish(Rod))
    {
        if(Rod->GetCastId()==TargetCast && Rod->GetState()==EFiskRodState::Landed)
        { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_TEST_ASSIST_LANDED slot=%d cast=%s"),TargetSlot,*TargetCast.ToString(EGuidFormats::Digits));PC->ClientActionFeedback_Implementation(TEXT("AI-testhjelpen landet fisken. Vanlig oppgjør ved 17:00 gjelder.")); }
        StopAssist();return;
    }
    if(Now<NextControlAt || Rod->GetState()==EFiskRodState::InitialRun) { return; }NextControlAt=Now+.09;
    auto Act=[&](EFiskRodAction Action){PC->ServerActOnSelectedRod(Action,PC->Selection,TargetCast);};
    if(Rod->GetState()==EFiskRodState::ReadyToLand) { Act(EFiskRodAction::Land);return; }
    const auto Intent=Pilot.Step(Delta,Rod->IsSurging(),Rod->GetLoadPercent(),Rod->GetDragPercent());
    if(Rod->IsRaised()!=Intent.bRaise) { Act(Intent.bRaise?EFiskRodAction::Lift:EFiskRodAction::Lower); }
    if(Rod->IsReeling()!=Intent.bReel) { Act(Intent.bReel?EFiskRodAction::Reel:EFiskRodAction::StopReel); }
    if(Intent.DragStep) { Act(Intent.DragStep>0?EFiskRodAction::DragUp:EFiskRodAction::DragDown); }
#endif
}
void UFiskFishingHelp::Draw(AHUD& HUD,UCanvas& Canvas) const
{
    if(!HasGameView()) { return; }
    const auto* PC=CastChecked<AFiskBoardingController>(GetOwner());const auto* Boat=PC->FindStarterBoat();
    if(!Boat || !PC->PlayerState || !Boat->IsAboard(PC->PlayerState->GetPlayerId())) { return; }
    const float S=FMath::Min(Canvas.ClipX/1280.f,Canvas.ClipY/720.f);const int32 Progress=Boat->GetTripProgress();
    const FLinearColor Ink(.015f,.065f,.085f,.98f),Teal(.5f,.95f,.85f),Gold(1,.85f,.4f);
    if(Progress>=0 && Progress<200)
    {
        HUD.DrawRect(Ink,900*S,166*S,360*S,62*S);
        HUD.DrawText(Progress>100?TEXT("PÅ VEI HJEM"):TEXT("TILBAKE TIL MARINA  [R]"),Teal,916*S,177*S,nullptr,1.05f*S);
        HUD.DrawText(Boat->GetCaptainId()==PC->PlayerState->GetPlayerId()?TEXT("Klikk / R. Aktivt fiske avbrytes. Vanlig reisetid."):TEXT("Kapteinen styrer hjemreisen."),Gold,916*S,205*S,nullptr,.74f*S);
    }
#if !UE_BUILD_SHIPPING
    if(Progress==100)
    {
        HUD.DrawRect(Ink,900*S,242*S,360*S,66*S);
        HUD.DrawText(bAssisting?TEXT("AI FISKER FOR DEG  [Å: stopp]"):TEXT("AI-TESTHJELP  [Å]"),Gold,916*S,254*S,nullptr,1.05f*S);
        HUD.DrawText(bAssisting?FString::Printf(TEXT("Stang %d. Manuell kamp tar over igjen."),TargetSlot+1):TEXT("Etter napp: klikk / Å for ett forsøk på landing."),Teal,916*S,283*S,nullptr,.75f*S);
    }
#endif
}
