#pragma once
#include "UI/FiskJournalModel.h"
#include "Expedition/FiskExpeditionModel.h"
struct FFiskSeaMilestone { const TCHAR* Name;const TCHAR* Detail;int64 Current=0;int64 Target=1;bool Complete() const { return Current>=Target; } };
namespace FiskSeaPassport
{
    TArray<FFiskSeaMilestone> Build(const FFiskJournalModel& Journal,const TArray<FFiskExpeditionRecord>& Expeditions);
    FString Csv(const FFiskJournalModel& Journal);
    bool Export(const FString& Directory,const FFiskJournalModel& Journal,FString& File);
}
