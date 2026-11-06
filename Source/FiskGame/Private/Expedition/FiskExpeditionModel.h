#pragma once
#include "CoreMinimal.h"
#include "Economy/FiskDaySettlement.h"
#include "FiskExpeditionModel.generated.h"

USTRUCT()
struct FFiskExpeditionRecord
{
    GENERATED_BODY()
    UPROPERTY() FGuid DayId;
    UPROPERTY() int32 DayNumber=1;
    UPROPERTY() int32 Goal=0;
    UPROPERTY() int32 Weather=0;
    UPROPERTY() bool bCompleted=false;
    bool IsValid() const;
    bool operator==(const FFiskExpeditionRecord& B) const;
};

USTRUCT()
struct FFiskExpeditionPlan
{
    GENERATED_BODY()
    UPROPERTY() FGuid DayId;
    UPROPERTY() int32 DayNumber=1;
    UPROPERTY() int32 Weather=0;
    UPROPERTY() int32 Goal=1;
    UPROPERTY() int32 Ground=0;
    UPROPERTY() int32 Species=0;
    UPROPERTY() FVector Point=FVector(20000,0,0);
    UPROPERTY() int32 RequiredCrew=1;
    UPROPERTY() int32 Revision=0;
    UPROPERTY() bool bLocked=false;
    UPROPERTY() bool bEvaluated=false;
    UPROPERTY() bool bCompleted=false;
    UPROPERTY() TArray<int32> Ready;
    UPROPERTY() int32 MessagePlayer=INDEX_NONE;
    UPROPERTY() int32 MessageCode=-1;
    UPROPERTY() int32 MessageSerial=0;
};

namespace FiskExpeditions
{
    constexpr int32 Capacity=512;
    const TCHAR* WeatherName(int32 Id);
    const TCHAR* GoalName(int32 Id);
    const TCHAR* Phrase(int32 Id);
    bool Evaluate(const FFiskExpeditionPlan& Plan,const FFiskSettlementView& View,int32 NowMs,
        const TMap<int32,FGuid>& Profiles,const TArray<FGuid>& Participants,const TMap<FString,int32>& Records);
    FString RecordKey(const FGuid& Profile,int32 Species);
}
