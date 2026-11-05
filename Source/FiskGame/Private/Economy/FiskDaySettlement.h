#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Economy/FiskDayLedger.h"
#include "FiskDaySettlement.generated.h"

USTRUCT()
struct FFiskSettlementView
{
    GENERATED_BODY()
    UPROPERTY() bool bRevealed = false;
    UPROPERTY() TArray<FFiskDayResult> Results;
};

UCLASS()
class AFiskDaySettlement final : public AInfo
{
    GENERATED_BODY()
public:
    AFiskDaySettlement();
    bool RegisterCatch(const FGuid& CatchId, int32 PlayerId, const FFiskCatchData& Fish);
    const FFiskSettlementView& GetView() const { return View; }
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
    FFiskDayLedger Ledger;
    UPROPERTY(ReplicatedUsing = OnRep_View)
    FFiskSettlementView View;
    UFUNCTION() void OnRep_View();
};
