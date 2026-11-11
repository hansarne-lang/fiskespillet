#include "UI/FiskPlayerNames.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

FString FiskPlayerNames::Name(const UWorld* World,int32 PlayerId)
{
    if(const AGameStateBase* State=World?World->GetGameState():nullptr)
    {
        for(const APlayerState* P:State->PlayerArray)
        {
            if(P && P->GetPlayerId()==PlayerId && !P->GetPlayerName().TrimStartAndEnd().IsEmpty()) { return P->GetPlayerName(); }
        }
    }
    return FString::Printf(TEXT("Spiller %d"),PlayerId);
}
