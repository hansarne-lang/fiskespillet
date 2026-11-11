#pragma once
#include "CoreMinimal.h"
class UWorld;

// Display names for player IDs on boards and panels (#18). The name is the one the server set on the
// player's PlayerState (the boathouse name, or the SpilLan name); a player who has left shows as
// "Spiller <id>".
namespace FiskPlayerNames
{
    FString Name(const UWorld* World,int32 PlayerId);
}
