#pragma once
#include "CoreMinimal.h"
#include "FiskRodSelection.generated.h"

USTRUCT()
struct FFiskRodSelection
{
    GENERATED_BODY()
    UPROPERTY() int32 Slot=0;
    UPROPERTY() int32 Revision=0;
    UPROPERTY() int32 Day=1;
    bool Matches(const FFiskRodSelection& Other) const
    { return Slot==Other.Slot && Revision==Other.Revision && Day==Other.Day; }
    static int32 Capacity(int32 Crew) { return Crew==1?2:Crew==2?1:0; }
    bool Select(int32 ExpectedRevision,int32 ExpectedDay,int32 Target,int32 Available)
    {
        if(ExpectedRevision!=Revision || ExpectedDay!=Day || Target<0 || Target>=Available || Revision==MAX_int32) { return false; }
        if(Target!=Slot) { Slot=Target;++Revision; }return true;
    }
    void Reset(int32 NewDay)
    { Slot=0;Day=NewDay;if(Revision<MAX_int32) { ++Revision; } }
};
