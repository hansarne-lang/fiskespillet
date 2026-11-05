#include "Economy/FiskCatchContext.h"
#include "FiskFishingGround.h"
#include "Fishing/FiskRigSettings.h"

bool FFiskCatchContext::IsValid(bool bRequireTimestamp) const
{
    const FFiskFishingGround* Place=FiskGrounds::Find(Ground);
    if(!Place || !FFiskRigRules::IsValid(Bait,Depth,Program,Place->BottomDepth)
        || LandedAtMs<0 || LandedAtMs>=1020000 || FightMs<0 || FightMs>LandedAtMs) { return false; }
    if(LandedUtc.IsEmpty()) { return !bRequireTimestamp; }
    FDateTime Time;
    return LandedUtc.Len()<=32 && FDateTime::ParseIso8601(*LandedUtc,Time) && Time.ToIso8601()==LandedUtc;
}
bool FFiskCatchContext::operator==(const FFiskCatchContext& Other) const
{
    return Ground==Other.Ground && Bait==Other.Bait && Depth==Other.Depth && Program==Other.Program
        && FightMs==Other.FightMs && LandedAtMs==Other.LandedAtMs && LandedUtc==Other.LandedUtc;
}
