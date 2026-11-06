#pragma once
#include "Expedition/FiskExpeditionModel.h"

namespace FiskExpeditionStore
{
    FString File(const FString& Directory,const FGuid& Profile);
    bool Merge(TArray<FFiskExpeditionRecord>& Into,const TArray<FFiskExpeditionRecord>& Entries);
    bool Load(const FString& File,const FGuid& Profile,TArray<FFiskExpeditionRecord>& Entries);
    bool Save(const FString& File,const FGuid& Profile,TArray<FFiskExpeditionRecord>& Entries);
}
