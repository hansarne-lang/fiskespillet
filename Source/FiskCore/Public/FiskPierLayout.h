#pragma once
#include "CoreMinimal.h"
// Shared floor-plan dimensions in centimetres, used by visuals and authority movement.
namespace FiskPier
{
    inline constexpr double BodyRadius=30;
    inline constexpr int32 StoolCount=12,TableCount=5,PostCount=13;
    inline FVector Stool(int32 I) { return FVector(-5000+(I%6)*300,-2280-(I/6)*380,130); }
    inline FVector Table(int32 I) { return FVector(-5400+I*600,-3150,130); }
    inline FVector Post(int32 I) { return FVector(-1750,-3000+I*1000,130); }
    inline bool FurnitureClear(FVector P)
    {
        for(int32 I=0;I<StoolCount;++I) { if(FVector::DistSquared2D(P,Stool(I))<FMath::Square(32.5+BodyRadius)) { return false; } }
        for(int32 I=0;I<TableCount;++I) { if(FVector::DistSquared2D(P,Table(I))<FMath::Square(125.+BodyRadius)) { return false; } }
        for(int32 I=0;I<PostCount;++I) { if(FVector::DistSquared2D(P,Post(I))<FMath::Square(11.+BodyRadius)) { return false; } }
        return true;
    }
}
