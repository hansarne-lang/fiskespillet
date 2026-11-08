#include "World/FiskPierWalk.h"
#include "FiskPierRules.h"
#include "FiskPierGuide.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "World/FiskMarinaHub.h"
#include "UI/FiskGameInstance.h"
#include "UI/FiskMarinaPanel.h"
#include "UI/FiskFrontEnd.h"
#include "UI/FiskCrewPanel.h"
#include "UI/FiskSeaChart.h"
#include "UI/FiskRigPanel.h"
#include "UI/FiskJournalPanel.h"
#include "UI/FiskDeckPanel.h"
#include "UI/FiskSkipperPanel.h"
#include "UI/FiskAnglerPanel.h"
#include "UI/FiskCollectionPanel.h"
#include "UI/FiskExpeditionPanel.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
UFiskPierWalk::UFiskPierWalk() { PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f;SetIsReplicatedByDefault(true); }
void UFiskPierWalk::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const { Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(UFiskPierWalk,bEnabled,COND_OwnerOnly);DOREPLIFETIME_CONDITION(UFiskPierWalk,Position,COND_OwnerOnly); }
void UFiskPierWalk::ClearKeys() { bUp=bDown=bLeft=bRight=bSprint=false; }
bool UFiskPierWalk::HasMenu() const
{
    const auto* P=CastChecked<AFiskBoardingController>(GetOwner());
    return P->IsFleetVisible() || P->GetFrontEnd()->IsVisible() || P->GetMarinaPanel()->IsVisible() || P->GetCrewPanel()->IsVisible() || P->GetSeaChart()->IsVisible() || P->GetRigPanel()->IsVisible() || P->GetJournalPanel()->IsVisible() || P->GetDeckPanel()->IsVisible() || P->GetSkipperPanel()->IsVisible() || P->GetAnglerPanel()->IsVisible() || P->GetCollectionPanel()->IsVisible() || P->GetExpeditionPanel()->IsVisible() || P->IsCareerVisible() || P->IsHelpVisible() || P->IsResultsVisible() || P->IsHistoryVisible();
}
void UFiskPierWalk::ServerMode(FGuid Profile,bool Enable)
{
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());if(!P->HasAuthority()) { return; }
    const auto* B=P->FindStarterBoat();const bool Aboard=B && P->PlayerState && B->IsAboard(P->PlayerState->GetPlayerId());
    const bool Own=P->IsCareerReady() && P->GetServerCareer().Id==Profile;
    const bool Allowed=Own && (!Enable || FiskPier::CanStart(true,true,P->GetMarinaPanel()->IsAtDock(),Aboard));
    if(Allowed) { bEnabled=Enable;++Generation;LastSerial=0;ServerAxis=FVector2D::ZeroVector;LastInputAt=-1;Speed=0;if(!FiskPier::Inside(Position)) { Position=FiskPier::Spawn(); }if(auto* H=P->GetMarinaPanel()->FindHub();H && P->PlayerState) { H->SetWalking(P->PlayerState->GetPlayerId(),bEnabled,Position,Facing,0); }P->ForceNetUpdate(); }
    P->ClientPierMode(bEnabled,Position,Generation);
    if(!Allowed) { P->ClientActionFeedback(TEXT("Gå i land ved brygga først: Tab, L, Tab. T åpner bryggeliv.")); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_MODE accepted=%d enabled=%d generation=%d"),Allowed,bEnabled,Generation);
}
void UFiskPierWalk::ReceiveMode(bool Enable,FVector P,int32 G) { if(G!=Generation) { Serial=0; }bEnabled=Enable;Position=P;Generation=G;bPending=false;if(!Enable) { ClearKeys();bGuidePanel=false;StopGuide(TEXT("Bryggeliv avsluttet.")); } }
void UFiskPierWalk::ServerInput(FGuid Profile,int32 G,int32 S,FVector2D A,bool Sprint)
{
    const auto* P=CastChecked<AFiskBoardingController>(GetOwner());
    if(!P->HasAuthority() || !FiskPier::AcceptInput(bEnabled,P->IsCareerReady() && P->GetServerCareer().Id==Profile,G,Generation,S,LastSerial,A)) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_INPUT_REJECTED"));return; }
    LastSerial=S;ServerAxis=A.GetClampedToMaxSize(1.);bServerSprint=Sprint;LastInputAt=GetWorld()->GetRealTimeSeconds();
}
void UFiskPierWalk::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);auto* P=CastChecked<AFiskBoardingController>(GetOwner());const double Now=GetWorld()->GetRealTimeSeconds();
    if(P->HasAuthority() && bEnabled)
    {
        const auto* B=P->FindStarterBoat();if(!P->IsCareerReady() || !P->PlayerState || (B && B->IsAboard(P->PlayerState->GetPlayerId()))) { bEnabled=false;++Generation;ServerAxis=FVector2D::ZeroVector;Speed=0;if(auto* H=P->GetMarinaPanel()->FindHub();H && P->PlayerState) { H->SetWalking(P->PlayerState->GetPlayerId(),false,Position,Facing,0); }P->ClientPierMode(false,Position,Generation);P->ForceNetUpdate();UE_LOG(LogTemp,Display,TEXT("FISKELAGET_PIER_AUTO_STOP"));return; }
        const FVector Old=Position;const FVector2D A=Now-LastInputAt<=.4?ServerAxis:FVector2D::ZeroVector;Position=FiskPier::Step(Position,A,Delta,bServerSprint);Speed=Delta>0?FVector::Dist2D(Old,Position)/Delta:0;
        if(Speed>1) { Facing=FMath::RadiansToDegrees(FMath::Atan2(Position.Y-Old.Y,Position.X-Old.X)); }
        if(auto* H=P->GetMarinaPanel()->FindHub()) { H->SetWalking(P->PlayerState->GetPlayerId(),true,Position,Facing,Speed); }
    }
    if(P->IsLocalController() && bEnabled && Now>=NextSend)
    {
        if(const auto* H=P->GetMarinaPanel()->FindHub();H && !H->GetMessages().IsEmpty() && H->GetMessages().Last().Serial>GreetingSerial) { GreetingSerial=H->GetMessages().Last().Serial;GreetingUntil=Now+10; }
        NextSend=Now+.1;if(HasMenu()) { ClearKeys();bGuidePanel=false;StopGuide(TEXT("Gåhjelp stoppet av meny.")); }
        const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());FVector2D A((bLeft?1.:0)-(bRight?1.:0),(bUp?1.:0)-(bDown?1.:0));
        if(bGuidePanel) { A=FVector2D::ZeroVector; }else if(bGuiding) { A=GuideAxis(Now); }
        P->ServerPierInput(GI->Career.Id,Generation,++Serial,A,bSprint || bGuiding);
    }
}
bool UFiskPierWalk::HandleKey(const FInputKeyEventArgs& A)
{
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());const bool Press=A.Event==IE_Pressed,Release=A.Event==IE_Released;
    if(bDispatching || HasMenu()) { return false; }
    if(Press && A.Key==EKeys::T)
    { const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());ClearKeys();bGuidePanel=false;StopGuide(TEXT("Gåhjelp avsluttet."));bPending=true;P->ReleaseRodInputsForMenu();P->ServerPierMode(GI->Career.Id,!bEnabled);return true; }
    if(!bEnabled && !bPending) { return false; }
    if(Press && A.Key==EKeys::F10) { ClearKeys();StopGuide(TEXT("Velg et mål i havneguiden."));bGuidePanel=!bGuidePanel;return true; }
    if(bGuidePanel)
    {
        if(Press && (A.Key==EKeys::Up || A.Key==EKeys::Down)) { GuideChoice=(GuideChoice+(A.Key==EKeys::Up?13:1))%FiskPier::DestinationCount;return true; }
        if(Press && A.Key==EKeys::Enter) { StartGuide();return true; }
        if(Press && A.Key==EKeys::Escape) { bGuidePanel=false;return true; }
        if(A.Key==EKeys::W || A.Key==EKeys::A || A.Key==EKeys::S || A.Key==EKeys::D) { return true; }
    }
    if(Press && (A.Key==EKeys::W || A.Key==EKeys::A || A.Key==EKeys::S || A.Key==EKeys::D || A.Key==EKeys::E || A.Key==EKeys::Escape)) { StopGuide(TEXT("Du har overtatt gangen.")); }
    const bool Held=Press || A.Event==IE_Repeat;
    if(A.Key==EKeys::W) { bUp=Held;return true; }if(A.Key==EKeys::S) { bDown=Held;return true; }if(A.Key==EKeys::A) { bLeft=Held;return true; }if(A.Key==EKeys::D) { bRight=Held;return true; }if(A.Key==EKeys::LeftShift || A.Key==EKeys::RightShift) { bSprint=Held;return true; }
    if(Press && A.Key==EKeys::E) { Interact();return true; }
    if(Press && A.Key==EKeys::M) { P->GetMarinaPanel()->ToggleHarbourMusic();return true; }
    if(Press && A.Key==EKeys::C) { CameraMode=(CameraMode+1)%3;return true; }
    const FKey Greetings[]={EKeys::One,EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five};for(int32 I=0;I<5;++I) { if(Press && A.Key==Greetings[I]) { P->ServerMarinaSay(I);return true; } }
    if(Press && A.Key==EKeys::Left) { CameraYaw-=15;return true; }if(Press && A.Key==EKeys::Right) { CameraYaw+=15;return true; }
    if(Press && A.Key==EKeys::MouseScrollUp) { Zoom=FMath::Clamp(Zoom-.1f,.7f,1.6f);return true; }if(Press && A.Key==EKeys::MouseScrollDown) { Zoom=FMath::Clamp(Zoom+.1f,.7f,1.6f);return true; }
    if(Press && A.Key==EKeys::Home) { Zoom=1;CameraYaw=0;CameraMode=0;return true; }
    if(Press && A.Key==EKeys::Escape) { ClearKeys();P->ServerPierMode(CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance())->Career.Id,false);return true; }
    return false;
}
void UFiskPierWalk::Interact()
{
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());ClearKeys();auto* H=P->GetMarinaPanel()->FindHub();if(!H) { return; }
    auto Key=[&](FKey K){bDispatching=true;FInputKeyEventArgs A;A.InputDevice=FInputDeviceId::CreateFromInternalId(0);A.Key=K;A.Event=IE_Pressed;P->InputKey(A);A.Event=IE_Released;P->InputKey(A);bDispatching=false;};
    if(FiskPier::NearBar(Position)) { Key(EKeys::B);return; }
    for(int32 I=0;I<H->GetResidents().Num();++I) { if(FVector::Dist2D(Position,FiskPier::HouseDoor(I))<390) { Key(EKeys::H);for(int32 J=0;J<12 && P->GetMarinaPanel()->GetHouseIndex()!=I;++J) { Key(EKeys::PageDown); }return; } }
    if(FiskPier::NearBoat(Position)) { const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());P->ServerPierMode(GI->Career.Id,false);Key(EKeys::E);return; }
    P->ClientActionFeedback(TEXT("Følg brygga til et naust, strandbaren eller ombordstigningen. Minikartet viser veien."));
}
