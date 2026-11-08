#pragma once
#include "World/FiskMarineMesh.h"
namespace FiskCoast
{
    inline constexpr int32 Sectors=64,Rings=8;
    FFiskMarineMesh Terrain(int32 Island,int32 Band);
    FFiskMarineMesh Foam(int32 Island);
    FVector Decoration(int32 Island,int32 Index,double Fraction);
}
