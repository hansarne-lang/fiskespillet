#pragma once
#include "CoreMinimal.h"
#include "FiskMarinaTypes.h"

enum class EFiskMarinaLoad : uint8 { Empty,Loaded,Recovered,Blocked };
class FFiskMarinaStore
{
public:
    static EFiskMarinaLoad Load(const FString& File,const FGuid& Profile,FFiskMarinaIdentity& Identity);
    static bool Save(const FString& File,const FGuid& Profile,const FFiskMarinaIdentity& Identity);
};
