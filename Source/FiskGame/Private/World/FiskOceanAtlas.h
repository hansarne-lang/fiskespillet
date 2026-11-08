#pragma once
#include "CoreMinimal.h"
struct FFiskAtlasMark { const TCHAR* Name;FVector Point;const TCHAR* Tip; };
namespace FiskAtlas
{
    constexpr int32 Count=16;
    const FFiskAtlasMark* Find(int32 Index);
    int32 Nearest(FVector Point,double Radius=500);
}
