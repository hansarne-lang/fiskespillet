#pragma once
#include "CoreMinimal.h"

// Public value contract. World supplies the immutable prototype catalog.
struct FFiskFishingGround
{
    const TCHAR* Name;
    FVector Position;
    int32 TravelMs;
    int32 BottomDepth;
    int32 FishDepthMin;
    int32 FishDepthMax;
};
namespace FiskGrounds
{
    constexpr int32 Count = 3;
    FISKGAME_API const FFiskFishingGround* Find(int32 Id);
    FISKGAME_API int32 CrossingMs(int32 ReturnStartMs, int32 Id);
    FISKGAME_API int32 ReturnMarginMs(int32 NowMs, int32 Progress, int32 Id);
    FISKGAME_API bool MakeSeaPoint(FVector Point,int32& Habitat,int32& TravelMs);
}
