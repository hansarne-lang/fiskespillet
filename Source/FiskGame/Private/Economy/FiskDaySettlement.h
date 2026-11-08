#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Economy/FiskDayLedger.h"
#include "Economy/FiskSessionHistory.h"
#include "FiskDaySettlement.generated.h"

USTRUCT()
struct FFiskSettlementView
{
    GENERATED_BODY()
    UPROPERTY() int32 DayNumber = 1;
    UPROPERTY() FGuid DayId;
    UPROPERTY() bool bRevealed = false;
    UPROPERTY() TArray<FFiskDayResult> Results;
};

UCLASS()
class AFiskDaySettlement final : public AInfo
{
    GENERATED_BODY()
public:
    AFiskDaySettlement();
    bool RegisterCatch(const FGuid& CatchId, int32 PlayerId, const FFiskCatchData& Fish,const FFiskCatchContext& Context=FFiskCatchContext());
    virtual void BeginPlay() override;
    const FFiskSettlementView& GetView() const { return View; }
    const TArray<FFiskHistoryDay>& GetHistory() const { return HistoryView; }
    void ResetForDay(int32 DayNumber);
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
    TMap<FGuid,TWeakObjectPtr<class AFiskStarterBoat>> CatchBoats;
    FFiskDayLedger Ledger;
    FFiskSessionHistory History;
    UPROPERTY(Replicated) TArray<FFiskHistoryDay> HistoryView;
    UPROPERTY(ReplicatedUsing = OnRep_View)
    FFiskSettlementView View;
    UFUNCTION() void OnRep_View();
};
