#pragma once
#include "CoreMinimal.h"
#include "Fishing/FiskRigSettings.h"

struct FFiskSavedRig { bool bExists=false; FFiskRigSettings Rig; };
class FFiskRigPresets
{
public:
    static constexpr int32 Count=3;
    const FFiskSavedRig* Get(int32 Slot) const { return Slot>=0 && Slot<Count ? &Slots[Slot] : nullptr; }
    void Load(const FString& File);
    bool Save(int32 Slot,const FFiskRigSettings& Rig,const FString& File);
private:
    FFiskSavedRig Slots[Count];
};
