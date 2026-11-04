#pragma once
#include "CoreMinimal.h"
#include "FiskFishVisualState.generated.h"

UENUM()
enum class EFiskFishVisualPhase : uint8
{
    Hidden,
    Fighting,
    ReadyToLand,
    Landing
};

USTRUCT()
struct FFiskFishVisualState
{
    GENERATED_BODY()
    UPROPERTY() EFiskFishVisualPhase Phase = EFiskFishVisualPhase::Hidden;
    UPROPERTY() int32 LineMeters = 0;
    UPROPERTY() int32 LandingAtMs = -1;

    bool operator!=(const FFiskFishVisualState& Other) const
    {
        return Phase != Other.Phase || LineMeters != Other.LineMeters || LandingAtMs != Other.LandingAtMs;
    }
};
