#pragma once
#include "CoreMinimal.h"

struct FFiskUserSettings
{
    bool bSound = true;
    int32 FrameRate = 60;
    bool Load(const FString& File);
    bool Save(const FString& File) const;
    static FString DataDirectory();
};
