#pragma once
#include "CoreMinimal.h"
namespace FiskBoatLayout
{
    inline int32 Seats(int32 Kind) { return Kind==1?4:2; }
    inline int32 Holders(int32 Kind) { return Kind==1?6:2; }
    inline int32 Rods(int32 Kind,int32 Crew,int32 Seat)
    {
        if(Crew<1 || Crew>Seats(Kind) || Seat<0 || Seat>=Crew) { return 0; }
        if(Kind==0) { return Crew==1?2:1; }
        return FMath::Min(3,6/Crew+(Seat<6%Crew?1:0));
    }
    inline int32 FirstHolder(int32 Kind,int32 Crew,int32 Seat)
    { int32 N=0;for(int32 I=0;I<Seat;++I) { N+=Rods(Kind,Crew,I); }return N; }
    inline FVector RodBase(int32 Holder)
    { return FVector(-230+(Holder/2)*100,(Holder%2==0?-1:1)*97,135); }
}
