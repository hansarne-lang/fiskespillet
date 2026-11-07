#pragma once
#include "CoreMinimal.h"
#include "World/FiskMarineMesh.h"
struct FFiskSpeciesArt
{
    float Length,Width,Height,Tail,Dorsal,Bill,Eye;
    FLinearColor Body,Accent;
};
namespace FiskSpeciesArt
{
    constexpr int32 Count=15;
    const FFiskSpeciesArt* Find(int32 Species);
    FFiskMarineMesh Body(int32 Species);
    FFiskMarineMesh Fins(int32 Species);
    FVector Eye(int32 Species,int32 Side);
}
