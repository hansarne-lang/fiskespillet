#pragma once
#include "CoreMinimal.h"
#include "FiskCatchCatalog.generated.h"

// Server-private until included in the final settlement at 17:00.
USTRUCT()
struct FFiskCatchData
{
    GENERATED_BODY()
    UPROPERTY() int32 Species = 0;
    UPROPERTY() int32 WeightGrams = 2000;
    UPROPERTY() int32 LengthMm = 300;
    UPROPERTY() int32 Value = 10;
};

struct FFiskSpecies
{
    const TCHAR* Name;
    int32 MinGrams, MaxGrams, MinLengthMm, MaxLengthMm, PricePerKg;
};

namespace FiskCatches
{
    const FFiskSpecies* Find(int32 Species);
    FFiskCatchData Roll(int32 Seed, int32 GroundId);
    bool IsValid(const FFiskCatchData& Fish);
    int32 LengthForWeight(const FFiskSpecies& Species, int32 Grams);
    int32 ValueForWeight(const FFiskSpecies& Species, int32 Grams);
}
