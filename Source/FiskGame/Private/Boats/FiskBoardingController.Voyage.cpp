#include "Boats/FiskBoardingController.h"
#include "Voyage/FiskVoyagePlan.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
void AFiskBoardingController::ServerVoyageReplace_Implementation(int32 D,int32 R,const TArray<FVector>& Points)
{
    auto* P=FindVoyage();FString Message=TEXT("Turplanen er ikke klar.");
    const bool OK=IsCareerReady() && PlayerState && P && P->Replace(PlayerState->GetPlayerId(),D,R,Points,Message);
    ClientActionFeedback(Message);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_REPLACE accepted=%d player=%d"),OK,PlayerState?PlayerState->GetPlayerId():-1);
}
void AFiskBoardingController::ServerVoyageNext_Implementation(int32 D,int32 R,int32 BR)
{
    auto* P=FindVoyage();FString Message=TEXT("Turplanen er ikke klar.");
    const bool OK=IsCareerReady() && PlayerState && P && P->NextStop(PlayerState->GetPlayerId(),D,R,BR,Message);
    ClientActionFeedback(Message);UE_LOG(LogTemp,Display,TEXT("FISKELAGET_VOYAGE_NEXT accepted=%d player=%d"),OK,PlayerState?PlayerState->GetPlayerId():-1);
}
