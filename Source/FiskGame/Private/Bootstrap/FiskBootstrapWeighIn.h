#pragma once
#include "CoreMinimal.h"
class AHUD;class UCanvas;

// The shared weigh-in on screen (#183 step 3): the fish on the scale, the last ones weighed and the
// board with landed, weighed and lost fish per participant. Shown to everyone, ashore or aboard, from
// the start of the weigh-in until the last fish has been weighed.
namespace FiskWeighInScreen
{
    void Draw(AHUD& HUD,UCanvas& Canvas);
}
