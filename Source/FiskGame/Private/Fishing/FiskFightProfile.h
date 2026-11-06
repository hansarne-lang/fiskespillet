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
        if(Species==3) { return {.85f,.7f,.95f,.8f}; }
        if(Species==4) { return {1.15f,.65f,1.15f,1.05f}; }
        if(Species==5) { return {1.6f,.55f,.95f,.95f}; }
        if(Species==6) { return {1.5f,1.6f,.6f,.75f}; }
        if(Species==7) { return {1.45f,.6f,1.05f,1.f}; }
        if(Species==8) { return {1.25f,1.4f,.72f,.85f}; }
        if(Species==9) { return {1.1f,.7f,1.2f,1.1f}; }
        if(Species==10) { return {.95f,.9f,1.05f,1.f}; }
        return {};
    }
};
