#pragma once
#include "CoreMinimal.h"
namespace FiskVoyage
{
    inline bool CanRelocate(bool Captain,bool Fresh,bool AtSpot,int32 NowMs,bool AllRodsSafe,bool ValidPoint)
    { return Captain && Fresh && AtSpot && NowMs>=120000 && NowMs<1020000 && AllRodsSafe && ValidPoint; }
    inline bool Unanimous(const TArray<int32>& Crew,const TArray<int32>& Votes)
    { if(Crew.IsEmpty() || Crew.Num()>4) { return false; }for(int32 Id:Crew) { if(Id<0 || !Votes.Contains(Id)) { return false; } }return true; }
    inline int32 Crossing(int32 Started,int32 Duration,double Length)
    {
        if(Started<0 || Duration<1 || !FMath::IsFinite(Length) || Length<0) { return -1; }
        const int64 Value=static_cast<int64>(Started)+FMath::CeilToInt(Duration*(1.0-500.0/FMath::Max(Length,500.0)));
        return Value>MAX_int32?-1:static_cast<int32>(Value);
    }
}
