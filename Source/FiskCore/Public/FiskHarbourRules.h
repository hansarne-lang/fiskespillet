#pragma once
#include "CoreMinimal.h"
namespace FiskHarbour
{
    inline bool CanChoose(bool Ready,bool Own,bool Dock,int32 Place) { return Ready && Own && Dock && Place>=0 && Place<=2; }
    inline int32 PublicPlace(bool Connected,bool Sea,int32 Choice) { return !Connected?-1:Sea?3:FMath::Clamp(Choice,0,2); }
    inline FVector MeetingPoint(int32 Slot,int32 Place) { const int32 I=FMath::Clamp(Slot,0,11);return Place==2?FVector(-5000+(I%6)*300,-2420-(I/6)*380,135):FVector(-1000+(I%4)*260,-1200-(I/4)*280,120); }
}
