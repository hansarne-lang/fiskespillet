#include "Boats/Controller/FiskHelmControl.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/Controller/FiskBoatCommands.h"
#include "FiskHelm.h"
#include "FiskRouteGeometry.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerState.h"

UFiskHelmControl::UFiskHelmControl() { PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=.05f; }

const AFiskStarterBoat* UFiskHelmControl::OwnBoat(bool& bCaptain) const
{
    const auto* P=CastChecked<AFiskBoardingController>(GetOwner());const auto* B=P->FindStarterBoat();
    const int32 Self=P->PlayerState?P->PlayerState->GetPlayerId():INDEX_NONE;
    bCaptain=B && Self!=INDEX_NONE && B->IsAboard(Self) && B->GetCaptainId()==Self;
    return B && Self!=INDEX_NONE && B->IsAboard(Self)?B:nullptr;
}

bool UFiskHelmControl::HandleKey(const FInputKeyEventArgs& A)
{
    if(A.Key!=EKeys::W && A.Key!=EKeys::S && A.Key!=EKeys::A && A.Key!=EKeys::D) { return false; }
    bool bCaptain=false;const AFiskStarterBoat* B=OwnBoat(bCaptain);
    if(!B) { return false; }
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());
    if(!bCaptain) { if(A.Event==IE_Pressed) { P->ClientActionFeedback(TEXT("Bare kapteinen styrer båten. K viser mannskapet.")); }return true; }
    const bool Press=A.Event==IE_Pressed,Release=A.Event==IE_Released;
    if(A.Key==EKeys::A && (Press || Release)) { bLeft=Press; }
    if(A.Key==EKeys::D && (Press || Release)) { bRight=Press; }
    if(Press || A.Event==IE_Repeat)
    {
        if(A.Key==EKeys::W) { Lever=FMath::Min(1.f,Lever+LeverStep); }
        if(A.Key==EKeys::S) { Lever=FMath::Max(-1.f,Lever-LeverStep); }
    }
    NextSend=0;   // Send the change at once
    return true;
}

void UFiskHelmControl::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);
    auto* P=CastChecked<AFiskBoardingController>(GetOwner());if(!P->IsLocalController()) { return; }
    bool bCaptain=false;const AFiskStarterBoat* B=OwnBoat(bCaptain);
    const bool bUnder=B && B->IsUnderHelm();
    // The chart's autopilot, R, a stop or a new day took the boat from the hand: the lever goes to zero.
    if((bWasUnderHelm && !bUnder) || !bCaptain || (B && B->GetTripProgress()==AFiskStarterBoat::AtMarina)) { Lever=0;bLeft=bRight=false; }
    bWasUnderHelm=bUnder;
    if(!B || !bCaptain) { return; }
    const double Now=GetWorld()->GetRealTimeSeconds();
    if(Now<NextSend || (!bUnder && Lever==0 && GetRudder()==0)) { return; }
    NextSend=Now+.1;
    P->GetBoatCommands()->ServerHelm(++Serial,Lever,GetRudder());
}

void UFiskHelmControl::Draw(AHUD& H,UCanvas& C) const
{
    bool bCaptain=false;const AFiskStarterBoat* B=OwnBoat(bCaptain);
    if(!B) { return; }
    const int32 Trip=B->GetTripProgress();
    const bool bShow=B->IsUnderHelm() || Trip==AFiskStarterBoat::AtFishingSpot || (Trip==AFiskStarterBoat::AtDock && bCaptain);
    if(!bShow) { return; }
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const float X=24*S,Y=C.ClipY-132*S;
    H.DrawRect(FLinearColor(0.015f,0.055f,0.075f,0.9f),X,Y,560*S,82*S);
    const FVector Here=B->GetActorLocation();
    const double Home=FiskRouteGeometry::Length(FiskRouteGeometry::Between(Here,FVector::ZeroVector));
    const int32 HomeMinutes=FiskHelm::TravelMs(Home,B->GetKind(),false)/1000;   // One real second is one clock minute
    const float Speed=B->IsUnderHelm()?B->GetHelmSpeed():0.f;
    const int32 Course=(FMath::RoundToInt(B->GetActorRotation().Yaw)%360+360)%360;
    const FString LeverText=bCaptain?FString::Printf(TEXT("Gass %+d %%"),FMath::RoundToInt(Lever*100)):TEXT("Kapteinen styrer");
    H.DrawText(FString::Printf(TEXT("ROR  %s  |  fart %.1f knop  |  kurs %03d°"),*LeverText,Speed/51.444f,Course),FLinearColor(1,0.86f,0.5f),X+14*S,Y+8*S,nullptr,1.0f*S);
    H.DrawText(FString::Printf(TEXT("Hjem %.1f km, ca. %d:%02d med full fart"),Home/100000.0,HomeMinutes/60,HomeMinutes%60),FLinearColor::White,X+14*S,Y+32*S,nullptr,.95f*S);
    H.DrawText(bCaptain?TEXT("W/S gass (S under null rygger)  A/D ror  Slipp gassen for å fiske  R hjem"):TEXT("Stanga settes ut når båten ligger stille."),FLinearColor(0.5f,0.9f,0.8f),X+14*S,Y+56*S,nullptr,.85f*S);
}
