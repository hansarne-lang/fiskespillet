#pragma once
#include "CoreMinimal.h"

struct FFiskUserSettings
{
    bool bSound = true;
    int32 FrameRate = 60;
    bool bMouseRod = true;      // Mouse-steered rod (issue #132), default on
    int32 RodSensitivity = 2;   // 1 low, 2 medium, 3 high
    bool Load(const FString& File);
    bool Save(const FString& File) const;
    static FString DataDirectory();
};
