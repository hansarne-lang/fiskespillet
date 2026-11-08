#pragma once
#include "CoreMinimal.h"
#include "Economy/FiskDaySettlement.h"

struct FFiskSonarView
{
    bool bAvailable=false;
    int32 Bottom=0,Near=0,Far=0,BaitDepth=0;
    FString Ground,Structure;
};
struct FFiskBoardRow
{
    int32 Player=INDEX_NONE,Rank=0,Delivered=0,Lost=0,BestGrams=0,Species=-1,Day=0;
    int64 Income=0,Grams=0;
};
namespace FiskNavigation
{
    FFiskSonarView Sonar(int32 Ground,int32 Progress,bool bAboard,int32 BaitDepth);
    bool Revealed(const FFiskSettlementView& View,int32 Day,int32 NowMs);
    TArray<FFiskBoardRow> Standings(const FFiskSettlementView& View,int32 Day,int32 NowMs);
    TArray<FFiskBoardRow> Records(const TArray<FFiskHistoryDay>& History,const FFiskSettlementView& View,int32 Day,int32 NowMs);
    FString NextAction(bool bStarted,int32 Ms,int32 Progress,bool bAboard,bool bCaptain,bool bHost,int32 RodState,bool bRigValid,bool bLandingBusy);
}
