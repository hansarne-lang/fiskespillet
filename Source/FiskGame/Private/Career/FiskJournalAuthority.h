#pragma once
#include "CoreMinimal.h"
#include "UI/FiskJournalStore.h"
#include "FiskCareerProfile.h"
struct FFiskSettlementView;

// Contains only server-confirmed records. Client journals are never imported here.
namespace FiskJournalAuthority
{
    constexpr int32 BatchSize=16;
    FString File(const FString& Directory,const FGuid& Profile);
    bool Load(const FString& Directory,const FGuid& Profile,FFiskJournalModel& Journal);
    bool RecoverIncome(const FString& Directory,FFiskCareerProfile& Career);
    bool Commit(const FString& Directory,const FFiskSettlementView& View,int32 NowMs,const TMap<int32,FGuid>& Profiles);
}
