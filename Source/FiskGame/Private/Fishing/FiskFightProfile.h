#pragma once
#include "CoreMinimal.h"

// Private server tuning. Species and stamina are never added to a rod's network view.
struct FFiskFightProfile
{
    float RunSpeed=1,RunDuration=1,FatigueDrain=1,Pump=1;
    static FFiskFightProfile ForSpecies(int32 Species)
    {
        if(Species==0) { return {0.8f,0.85f,1.25f,1.2f}; }
        if(Species==2) { return {1.3f,1.35f,0.75f,0.85f}; }
        return {};
    }
};
