#pragma once
#include "CoreMinimal.h"
#include "Algo/StableSort.h"

// Shared weigh-in when everyone is ashore (issue #183, owner decision 2026-11-14). Pure rules: the order
// on the scale (smallest first, largest last), how long each fish stays on the scale, how many have been
// weighed after a given time, and when the weigh-in starts.
namespace FiskWeighIn
{
    inline constexpr double SmallSeconds=3.0;     // Most fish
    inline constexpr double LargeSeconds=5.0;     // The second to fourth largest
    inline constexpr double LargestSeconds=8.0;   // The day's biggest fish, last
    inline constexpr double MaxTotalSeconds=120.0;// A long day is weighed faster, never slower than this
    inline constexpr double MinSeconds=1.0;       // ...but every fish is on the scale at least this long
    inline constexpr int32 EarliestStartMs=120000;// Nobody can be home from sea before 02:00

    // Positions on the scale for fish of the given weights: smallest first, equal weights in the order
    // they were landed (stable). Returns indices into Grams.
    inline TArray<int32> Order(const TArray<int32>& Grams)
    {
        TArray<int32> Result;Result.Reserve(Grams.Num());for(int32 I=0;I<Grams.Num();++I) { Result.Add(I); }
        Algo::StableSort(Result,[&Grams](int32 A,int32 B){return Grams[A]<Grams[B];});
        return Result;
    }

    // Seconds the fish at Position (0 = first, smallest) stays on the scale out of Count fish.
    inline double StepSeconds(int32 Position,int32 Count)
    {
        if(Count<=0 || Position<0 || Position>=Count) { return 0.0; }
        const int32 FromTop=Count-1-Position;
        const double Base=FromTop==0?LargestSeconds:FromTop<=3?LargeSeconds:SmallSeconds;
        double Total=0;for(int32 I=0;I<Count;++I) { const int32 Top=Count-1-I;Total+=Top==0?LargestSeconds:Top<=3?LargeSeconds:SmallSeconds; }
        return Total<=MaxTotalSeconds?Base:FMath::Max(MinSeconds,Base*MaxTotalSeconds/Total);
    }

    inline double TotalSeconds(int32 Count)
    { double Total=0;for(int32 I=0;I<Count;++I) { Total+=StepSeconds(I,Count); }return Total; }

    // Fish put on the scale after Elapsed seconds: the first goes on at once. Count means all are weighed
    // and the last has had its full time, so the result can be shown.
    inline int32 OnScale(double Elapsed,int32 Count)
    {
        if(Count<=0 || !FMath::IsFinite(Elapsed) || Elapsed<0) { return 0; }
        double At=0;for(int32 I=0;I<Count;++I) { if(Elapsed<At) { return I; }At+=StepSeconds(I,Count); }
        return Count;
    }
    inline bool Finished(double Elapsed,int32 Count)
    { return Count<=0 || (FMath::IsFinite(Elapsed) && Elapsed>=TotalSeconds(Count)); }

    // The weigh-in starts when every boat that went out is home again (after 02:00), or at the deadline.
    // Boats that never left the dock do not count as out.
    inline bool ShouldStart(int32 NowMs,int32 DeadlineMs,bool bAnyAway,bool bAnyHome)
    { return NowMs>=DeadlineMs || (NowMs>=EarliestStartMs && !bAnyAway && bAnyHome); }
}
