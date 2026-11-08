#include "UI/FiskSkipperPanel.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "GameFramework/HUD.h"
#include "Engine/Canvas.h"
void UFiskSkipperPanel::DrawCargo(AHUD& H,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Color=FLinearColor::White){H.DrawText(V,Color,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.98f),24*S,28*S,1232*S,650*S);T(TEXT("FANGSTLAGER OG SLEP"),48,55,2.f);
    const auto* B=CastChecked<AFiskBoardingController>(GetOwner())->FindStarterBoat();if(!B) { T(TEXT("Ingen aktiv båt."),48,160);return; }
    const auto L=FFiskCargoModel::Limits(B->GetKind());const auto& V=B->GetCargoView();const auto M=static_cast<EFiskCargoMode>(V.Mode);
    T(FString::Printf(TEXT("%s | Felles lager for alle om bord"),B->GetKind()==1?TEXT("OFFSHOREBÅT"):TEXT("STARTBÅT")),48,112,1.2f,Teal);
    T(FString::Printf(TEXT("Normalt lager: %d kg"),L.Hold/1000),48,176,1.25f);T(FString::Printf(TEXT("Tillatt overlast: %d kg"),L.Overload/1000),440,176,1.25f);T(FString::Printf(TEXT("Største slepefisk: %d kg"),L.Tow/1000),840,176,1.15f);
    const TCHAR* Status[]={TEXT("Lageret er tomt"),TEXT("Fangst er sikret i lageret"),TEXT("Lageret er fullt"),TEXT("Tillatt overlast – hjemtur"),TEXT("Stor fangst er sikret på slep")};
    T(Status[FMath::Clamp(static_cast<int32>(V.Mode),0,4)],48,256,1.65f,M>=EFiskCargoMode::Full?Gold:Teal);
    if(V.bDiscarded) { T(TEXT("En landet fisk kunne ikke fraktes. Den registreres uten salgsinntekt."),48,306,1.05f,Gold); }
    if(V.RevealedGrams>=0) { T(FString::Printf(TEXT("Fraktet fangst om bord / på slep: %.1f kg"),V.RevealedGrams/1000.),48,360,1.2f); }
    else { T(TEXT("Eksakt fangstvekt, art og verdi avsløres først ved oppgjøret 17:00."),48,360,1.1f,Teal); }
    T(B->GetTripProgress()==AFiskStarterBoat::AtMarina?TEXT("Båten er ved brygga."):V.bReturning?TEXT("Automatisk hjemtur – fisket er avsluttet for hele båten."):TEXT("Du kan fortsette å fiske så lenge lageret har plass."),48,412,1.1f);
    T(TEXT("Fullt lager / tillatt overlast: normal returfart. Slep: 65 prosent av vanlig fart."),48,468,1.05f,Gold);
    T(TEXT("Tidligere last beholdes ved sleping. Ingen flere fisk kan legges til etter tvungen retur."),48,510,1.f);
    T(TEXT("For stor fisk uten last om bord: fisken går tapt; du kan fortsette fisket."),48,548,1.f);
    T(TEXT("Kryss marinagrensen innen 17:00. Sen ankomst taper både lagret fisk og slepefisken."),48,590,1.f,Gold);
    T(TEXT("F11 / Escape: lukk | Klokken går videre mens dette vinduet er åpent"),48,644,1.f,Teal);
}
