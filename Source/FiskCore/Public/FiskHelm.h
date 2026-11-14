#pragma once
#include "CoreMinimal.h"
#include "FiskRouteGeometry.h"

// Hand-steered boat (issue #176). Pure rules: the captain's throttle and rudder move the boat in
// fixed time steps, so the result does not depend on frame rate. Islands and the chart edge stop
// the boat. Units: centimetres, seconds, radians. Heading 0 points out to sea (+X).
namespace FiskHelm
{
    inline constexpr double StepSeconds=0.05;
    inline constexpr double BaseSpeed=20000.0/12.0;   // Same speed as the charted trip: 200 m in 12 s
    inline constexpr double OffshoreShare=1.25;       // Offshore boat (kind 1), as on charted trips
    inline constexpr double TowShare=0.65;            // Towing a big fish, as in the cargo rules
    inline constexpr double ReverseShare=0.25;
    inline constexpr double SpinUpSeconds=4.0;        // Standstill to full speed
    inline constexpr double CoastSeconds=6.0;         // Full speed to standstill with the throttle released
    inline constexpr double TurnRate=0.5;             // Radians per second at full speed
    inline constexpr double StillSpeed=40.0;          // At or below this the boat counts as stopped
    inline constexpr double LongestAdvance=1.0;       // A stalled frame never moves the boat more than this

    struct FState
    {
        FVector Position=FVector::ZeroVector;
        double Heading=0.0;
        double Speed=0.0;      // Negative when reversing
        double Pending=0.0;    // Time not yet simulated
    };

    inline double MaxSpeed(int32 Kind,bool bTowing) { return BaseSpeed*(Kind==1?OffshoreShare:1.0)*(bTowing?TowShare:1.0); }
    // Throttle and rudder arrive from a client: anything not finite counts as released.
    inline double Clean(double Value) { return FMath::IsFinite(Value)?FMath::Clamp(Value,-1.0,1.0):0.0; }
    inline FVector Forward(const FState& S) { return FVector(FMath::Cos(S.Heading),FMath::Sin(S.Heading),0.0); }
    inline bool Still(const FState& S) { return FMath::Abs(S.Speed)<=StillSpeed; }
    // Fishing needs a stopped boat on open sea, outside the marina.
    inline bool CanFish(const FState& S) { return Still(S) && FiskRouteGeometry::Sea(S.Position); }

    inline void Step(FState& S,double Throttle,double Rudder,int32 Kind,bool bTowing)
    {
        const double Max=MaxSpeed(Kind,bTowing),T=Clean(Throttle),R=Clean(Rudder);
        const double Target=T>=0.0?T*Max:T*Max*ReverseShare;
        const double Change=Max/(T==0.0?CoastSeconds:SpinUpSeconds)*StepSeconds;
        S.Speed+=FMath::Clamp(Target-S.Speed,-Change,Change);
        // The rudder needs water flowing past it: no turning at a standstill, mirrored in reverse.
        S.Heading=FMath::UnwindRadians(S.Heading+R*TurnRate*(S.Speed/Max)*StepSeconds);
        FVector Next=S.Position+Forward(S)*S.Speed*StepSeconds;Next.Z=0.0;
        if(FiskRouteGeometry::Navigable(Next)) { S.Position=Next; }
        else { S.Speed=0.0; }   // Aground on an island or at the chart edge: back off or turn away
    }

    inline void Advance(FState& S,double Throttle,double Rudder,int32 Kind,bool bTowing,double Seconds)
    {
        if(!FMath::IsFinite(Seconds) || Seconds<=0.0) { return; }
        S.Pending+=FMath::Min(Seconds,LongestAdvance);
        while(S.Pending+1.e-9>=StepSeconds) { S.Pending-=StepSeconds;Step(S,Throttle,Rudder,Kind,bTowing); }
    }
}
