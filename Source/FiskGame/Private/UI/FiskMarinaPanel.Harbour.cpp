#include "UI/FiskMarinaPanel.h"
#include "Boats/FiskBoardingController.h"
#include "Boats/FiskStarterBoat.h"
#include "UI/FiskGameInstance.h"
#include "Fishing/FiskCatchCatalog.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
const AFiskStarterBoat* UFiskMarinaPanel::FollowBoat() const
{ const auto Boats=CastChecked<AFiskBoardingController>(GetOwner())->FleetBoats();return Boats.IsEmpty()?nullptr:Boats[FMath::Clamp(FollowIndex,0,Boats.Num()-1)]; }
void UFiskMarinaPanel::DrawGallery(AHUD& H,UCanvas& C)
{
    const float S=FMath::Min(C.ClipX/1280.f,C.ClipY/720.f);const FLinearColor Teal(.5f,.9f,.84f),Gold(1,.8f,.4f);
    auto T=[&](const FString& V,float X,float Y,float Size=1.f,FLinearColor Col=FLinearColor::White){H.DrawText(V,Col,X*S,Y*S,nullptr,Size*S);};
    H.DrawRect(FLinearColor(.012f,.045f,.06f,.97f),24*S,28*S,610*S,650*S);T(TEXT("HAVETS 3D-ARTSGALLERI"),46,60,1.65f);
    const auto* F=FiskCatches::Find(Species);if(!F) { return; }const auto* GI=CastChecked<UFiskGameInstance>(GetWorld()->GetGameInstance());
    T(FString::Printf(TEXT("%02d / 15  %s"),Species+1,F->Name),46,122,1.5f,Gold);T(TEXT("Offentlig katalogmodell – stilisert 3D"),46,174,.95f,Teal);
    T(FiskCatches::HabitatTip(Species),46,233,.85f);T(FiskCatches::FightTip(Species),46,281,.85f);
    T(FString::Printf(TEXT("%.1f–%.1f kg | $%d/kg i testøkonomien"),F->MinGrams/1000.,F->MaxGrams/1000.,F->PricePerKg),46,343,1.f,Gold);
    const auto* Best=GI->GetJournal().Best(Species);T(Best?FString::Printf(TEXT("Din bekreftede rekord: %.1f kg / %.1f cm"),Best->Fish.WeightGrams/1000.,Best->Fish.LengthMm/10.):TEXT("Ingen bekreftet rekord av denne arten ennå."),46,405,.95f,Teal);
    T(TEXT("Dagens hemmelige fisk brukes ikke i galleriet."),46,468,.89f);T(TEXT("Nye rekorder vises først etter oppgjøret 17:00."),46,498,.86f,Teal);
    T(TEXT("Piler / PgUp/PgDn: art | A/D: roter"),46,565,.95f);T(TEXT("Musehjul: avstand | Home: nullstill"),46,595,.95f);T(TEXT("V: artsbok | I / Escape: lukk | Klokken går"),46,642,.87f,Teal);
}
