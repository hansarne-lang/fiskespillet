#include "Boats/FiskBoardingController.h"
#include "Fishing/FiskPrototypeRod.h"
#include "Boats/FiskStarterBoat.h"
#include "DayCycle/FiskDayState.h"
#include "FiskFishingGround.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
FFiskRigBundle AFiskBoardingController::MakeRigBundle(const TArray<FFiskRigSettings>& Rigs) const
{
    FFiskRigBundle B;const auto* D=GetWorld()->GetGameState<AFiskDayState>();B.Day=D?D->GetDayNumber():0;B.Rigs=Rigs;
    for(int32 I=0;I<GetRodCapacity();++I) { B.Casts.Add(GetRodAt(I)->GetCastId());B.Revisions.Add(GetRodAt(I)->GetRig().Revision); }return B;
}
void AFiskBoardingController::ServerApplyRigBundle_Implementation(FFiskRigBundle Request)
{
    const auto* B=FindStarterBoat();const auto* D=GetWorld()->GetGameState<AFiskDayState>();const int32 Count=GetRodCapacity();
    TArray<FGuid> Casts;TArray<FFiskRigSettings> Current,Prepared;TArray<bool> Safe;
    for(int32 I=0;I<Count;++I) { const auto* R=GetRodAt(I);Casts.Add(R->GetCastId());Current.Add(R->GetRig());Safe.Add(R->CanConfigure()); }
    const auto* G=B?FiskGrounds::Find(B->GetGroundId()):nullptr;
    const bool OK=IsCareerReady() && PlayerState && D && D->HasStarted() && D->GetElapsedMilliseconds()<1020000 && B && G && B->IsAboard(PlayerState->GetPlayerId())
        && (B->GetTripProgress()==AFiskStarterBoat::AtDock || B->GetTripProgress()==AFiskStarterBoat::AtFishingSpot) && FiskAngler::Prepare(Request,D->GetDayNumber(),Count,G->BottomDepth,Casts,Current,Safe,Prepared);
    if(OK)
    { for(int32 I=0;I<Count;++I) { const_cast<UFiskPrototypeRod*>(GetRodAt(I))->Rig=Prepared[I]; }ForceNetUpdate(); }
    UE_LOG(LogTemp,Display,TEXT("FISKELAGET_RIG_BUNDLE accepted=%d player=%d count=%d"),OK,PlayerState?PlayerState->GetPlayerId():-1,Count);ClientRigBundleReply(OK);
}
void AFiskBoardingController::ClientRigBundleReply_Implementation(bool OK)
{ ClientActionFeedback_Implementation(OK?TEXT("Stangsettet er anvendt samlet på dine egne stenger."):TEXT("Ingen stenger ble endret. Bruk et ferskt sett for riktig antall egne stenger, trekk inn alle, og prøv ved brygga eller fiskeplassen.")); }
