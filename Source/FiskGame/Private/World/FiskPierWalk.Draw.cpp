#include "World/FiskPierWalk.h"
#include "FiskPierRules.h"
#include "FiskPierGuide.h"
#include "Boats/FiskBoardingController.h"
#include "World/FiskMarinaHub.h"
#include "UI/FiskMarinaPanel.h"
#include "DayCycle/FiskDayState.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"
bool UFiskPierWalk::Draw(AHUD& H,UCanvas& C) const
{
    if(!bEnabled || HasMenu()) { return false; }const auto* P=CastChecked<AFiskBoardingController>(GetOwner());const auto* Hub=P->GetMarinaPanel()->FindHub();
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Col=FLinearColor::White){H.DrawText(V,Col,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.95f),24*S,28*S,390*S,130*S);T(TEXT("BRYGGELIV"),44,49,1.7f);T(TEXT("Gå på bryggene og besøk fiskelaget."),44,93,.95f,Teal);
    if(const auto* D=GetWorld()->GetGameState<AFiskDayState>()) { T(FString::Printf(TEXT("Dag %d | %02d:%02d | Oppgjør 17:00"),D->GetDayNumber(),D->GetElapsedSeconds()/60,D->GetElapsedSeconds()%60),44,126,.85f,Gold); }
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.95f),24*S,556*S,1232*S,122*S);
    FString Action=TEXT("Følg brygga til et naust, strandbaren eller båten.");
    if(FiskPier::NearBar(Position)) { Action=TEXT("E: besøk strandbaren | B: barens kamera og tavle"); }
    else if(FiskPier::NearBoat(Position)) { Action=TEXT("E: gå om bord i valgt båt | Tab: velg båt og mannskap"); }
    else if(Hub) { for(int32 I=0;I<Hub->GetResidents().Num();++I) { if(FVector::Dist2D(Position,FiskPier::HouseDoor(I))<390) { Action=TEXT("E: besøk ")+Hub->GetResidents()[I].Identity.HouseName+TEXT(" / ")+Hub->GetResidents()[I].Identity.DisplayName;break; } } }
    T(Action,44,578,1.1f,Gold);T(TEXT("WASD: gå / overta | Shift: løp | F10: havneguide | C: gåkamera | Piler / hjul: dreie / avstand"),44,616,.86f);
    T(TEXT("T / Escape: avslutt | H: naust | B: bar | M: musikk | 1–5: hils | Home: nullstill kamera"),44,648,.86f,Teal);
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.92f),24*S,174*S,650*S,62*S);
    T(FString::Printf(TEXT("Kamera: %s | F10: velg mål, Enter: start gåhjelp"),CameraMode==1?TEXT("NÆR"):CameraMode==2?TEXT("OVERSIKT"):TEXT("FØLG")),42,185,.85f,Teal);
    const FString Status=GuideStatus.IsEmpty()?TEXT("Gå selv, eller la guiden følge bryggene for deg."):GuideStatus;
    T(Status.Len()>92?Status.Left(89)+TEXT("…"):Status,42,211,.8f,bGuiding?Gold:FLinearColor::White);
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.96f),972*S,28*S,284*S,340*S);T(TEXT("MARINA"),990,46,1.2f,Teal);
    auto Map=[&](FVector V){return FVector2D(996+(300-V.X)/9300.*240,342-(V.Y+3200)/13500.*260);};
    for(const auto& A:FiskPier::Areas) { const auto Lo=Map(FVector(A.X1,A.Y0,130)),Hi=Map(FVector(A.X0,A.Y1,130));H.DrawRect(FLinearColor(.36f,.26f,.15f),Lo.X*S,Hi.Y*S,(Hi.X-Lo.X)*S,(Lo.Y-Hi.Y)*S); }
    for(int32 I=0;I<12;++I) { const auto Q=Map(FiskPier::HouseDoor(I));H.DrawRect(Teal,(Q.X-3)*S,(Q.Y-3)*S,6*S,6*S);T(FString::FromInt(I+1),Q.X-4,Q.Y-14,.58f,Teal); }
    if(bGuiding && GuideRoute.IsValidIndex(GuideIndex)) { FVector A=Position;for(int32 I=GuideIndex;I<GuideRoute.Num();++I) { const auto U=Map(A),V=Map(GuideRoute[I]);H.DrawLine(U.X*S,U.Y*S,V.X*S,V.Y*S,Gold,2*S);A=GuideRoute[I]; }T(FString::Printf(TEXT("Mål: %d | %.0f m"),GuideChoice,FVector::Dist2D(Position,GuideRoute.Last())/100),990,73,.76f,Gold); }
    if(Hub) { for(const auto& R:Hub->GetResidents()) { if(R.bWalking && R.PlayerId>=0) { const auto Q=Map(R.WalkPosition);H.DrawRect(FLinearColor::White,(Q.X-2)*S,(Q.Y-2)*S,4*S,4*S); } } }
    const auto Bar=Map(FVector(-4200,-2700,130)),Boat=Map(FVector(-550,0,130)),Self=Map(Position);H.DrawRect(FLinearColor(.9f,.5f,.25f),(Bar.X-4)*S,(Bar.Y-4)*S,8*S,8*S);H.DrawRect(FLinearColor(.4f,.7f,1),(Boat.X-4)*S,(Boat.Y-4)*S,8*S,8*S);H.DrawRect(Gold,(Self.X-4)*S,(Self.Y-4)*S,8*S,8*S);
    T(TEXT("Gul: deg / rute | Hvit: venner"),990,350,.7f,Gold);
    if(Hub && !Hub->GetMessages().IsEmpty() && GetWorld()->GetRealTimeSeconds()<GreetingUntil) { const auto& M=Hub->GetMessages().Last();H.DrawRect(FLinearColor(.012f,.045f,.06f,.94f),24*S,250*S,650*S,48*S);T((M.Name+TEXT(": ")+AFiskMarinaHub::Phrase(M.Code)).Left(88),42,266,.85f,Teal); }
    if(bGuidePanel)
    {
        H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),420*S,28*S,536*S,516*S);T(TEXT("HAVNEGUIDE"),442,46,1.5f,Gold);
        T(TEXT("Piler opp/ned: velg | Enter: gå | F10 / Escape: lukk"),442,83,.8f,Teal);
        for(int32 I=0;I<FiskPier::DestinationCount;++I) { const float Y=117+I*27;if(I==GuideChoice) { H.DrawRect(FLinearColor(.13f,.29f,.3f),434*S,(Y-5)*S,508*S,25*S); }T(DestinationName(I).Left(64),446,Y,.79f,I==GuideChoice?Gold:FLinearColor::White); }
        T(TEXT("Guiden stopper ved målet. E åpner besøk / ombordstigning."),442,511,.74f,Teal);
    }
    return true;
}
