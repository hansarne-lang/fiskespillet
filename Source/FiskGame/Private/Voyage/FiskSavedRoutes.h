#pragma once
#include "CoreMinimal.h"
struct FFiskSavedRoute { FString Name;TArray<FVector> Points; };
struct FFiskSavedRoutes
{
    TArray<FFiskSavedRoute> Slots;
    bool bBlocked=false,bRecovered=false;
    bool Load(const FString& File,const FGuid& Profile);
    bool Save(const FString& File,const FGuid& Profile) const;
    static bool Valid(const TArray<FFiskSavedRoute>& Slots);
};
