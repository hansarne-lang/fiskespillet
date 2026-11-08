#include "World/FiskPierWalk.h"
#include "FiskPierGuide.h"
#include "Boats/FiskBoardingController.h"
#include "UI/FiskMarinaPanel.h"
#include "World/FiskMarinaHub.h"
#include "Engine/World.h"
FString UFiskPierWalk::DestinationName(int32 I) const
{
    if(I==0) { return TEXT("Båt / ombordstigning"); }if(I==1) { return TEXT("Strandbaren"); }
    const auto* P=CastChecked<AFiskBoardingController>(GetOwner());const auto* H=P->GetMarinaPanel()->FindHub();
    if(H && H->GetResidents().IsValidIndex(I-2)) { const auto& R=H->GetResidents()[I-2];return FString::Printf(TEXT("Naust %02d: %s / %s"),I-1,*R.Identity.HouseName,*R.Identity.DisplayName); }
    return FString::Printf(TEXT("Naust %02d / ledig plass"),I-1);
}
void UFiskPierWalk::StopGuide(const FString& Reason)
{ if(bGuiding) { UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_STOP choice=%d reason=%s"),GuideChoice,*Reason); }bGuiding=false;GuideRoute.Reset();GuideIndex=0;GuideStatus=Reason; }
void UFiskPierWalk::StartGuide()
{
    ClearKeys();GuideRoute=FiskPier::GuidePath(Position,FiskPier::Destination(GuideChoice));GuideIndex=1;bGuidePanel=false;bGuiding=GuideRoute.Num()>1;
    GuideLastPosition=Position;GuideMovedAt=GetWorld()->GetRealTimeSeconds();GuideStatus=bGuiding?TEXT("Gåhjelp til ")+DestinationName(GuideChoice):TEXT("Ingen trygg gangvei. Flytt deg litt og prøv igjen.");
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_START choice=%d points=%d enabled=%d"),GuideChoice,GuideRoute.Num(),bGuiding);
}
FVector2D UFiskPierWalk::GuideAxis(double Now)
{
    if(!bEnabled || HasMenu() || !GuideRoute.IsValidIndex(GuideIndex)) { StopGuide(TEXT("Gåhjelp avsluttet."));return FVector2D::ZeroVector; }
    if(FVector::DistSquared2D(Position,GuideLastPosition)>25) { GuideMovedAt=Now;GuideLastPosition=Position; }
    if(Now-GuideMovedAt>3) { StopGuide(TEXT("Gåhjelpen stoppet ved en hindring. Bruk WASD eller velg målet på nytt."));return FVector2D::ZeroVector; }
    while(GuideRoute.IsValidIndex(GuideIndex) && FVector::Dist2D(Position,GuideRoute[GuideIndex])<14) { ++GuideIndex; }
    if(!GuideRoute.IsValidIndex(GuideIndex)) { StopGuide(TEXT("Fremme ved ")+DestinationName(GuideChoice)+TEXT(". Trykk E for å besøke / gå om bord."));UE_LOG(LogTemp,Display,TEXT("FISKELAGET_GUIDE_ARRIVED choice=%d"),GuideChoice);return FVector2D::ZeroVector; }
    const FVector V=GuideRoute[GuideIndex]-Position;const double D=V.Size2D();
    // Slow down near each bend to avoid cutting the corner or oscillating on replicated positions.
    return FVector2D(V.X,V.Y).GetSafeNormal()*FMath::Min(1.,D/70.);
}
