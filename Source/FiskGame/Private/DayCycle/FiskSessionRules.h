#pragma once
#include "CoreMinimal.h"

namespace FiskSession
{
    constexpr int32 MaxDay = 9999;
    inline bool CanAdvance(bool bLocalListenHost, int32 Day, int32 ExpectedDay, int32 NowMs, bool bSettled)
    {
        return bLocalListenHost && Day > 0 && Day < MaxDay && Day == ExpectedDay
            && NowMs >= 1200000 && bSettled;
    }
    // Warning uses only public time/route data, never catch weight or value.
    inline int32 ReturnWarning(int32 MarginMs)
    {
        return MarginMs < 0 ? 2 : MarginMs <= 60000 ? 1 : 0;
    }
}
