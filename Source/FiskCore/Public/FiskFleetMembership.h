#pragma once
#include "CoreMinimal.h"

// A voyage belongs to the durable profile, never the temporary connection ID.
class FISKCORE_API FFiskFleetMembership
{
public:
    bool CanJoin(const FGuid& Profile) const { return Profile.IsValid() && !Sailed.Contains(Profile); }
    void Depart(const TArray<FGuid>& Crew) { for(const auto& Profile:Crew) { if(Profile.IsValid()) { Sailed.Add(Profile); } } }
    void ResetForDay() { Sailed.Reset(); }
private:
    TSet<FGuid> Sailed;
};
