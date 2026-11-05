#pragma once
#include "CoreMinimal.h"
#include "FiskCatchContext.generated.h"

USTRUCT()
struct FFiskCatchContext
{
    GENERATED_BODY()
    UPROPERTY() int32 Ground = 0;
    UPROPERTY() int32 Bait = 0;
    UPROPERTY() int32 Depth = 30;
    UPROPERTY() int32 Program = 0;
    UPROPERTY() int32 FightMs = 0;
    UPROPERTY() int32 LandedAtMs = 0;
    UPROPERTY() FString LandedUtc;
    bool IsValid(bool bRequireTimestamp) const;
    bool operator==(const FFiskCatchContext& Other) const;
};
