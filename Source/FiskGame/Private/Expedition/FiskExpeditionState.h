#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Expedition/FiskExpeditionModel.h"
#include "FiskExpeditionState.generated.h"
class AFiskStarterBoat;
UCLASS()
class AFiskExpeditionState final : public AInfo
{
    GENERATED_BODY()
public:
    AFiskExpeditionState();
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    const FFiskExpeditionPlan& GetPlan() const { return Plan; }
    void Refresh();
    bool Choose(int32 Player,int32 Revision,int32 Goal);
    bool SetReady(int32 Player,int32 Revision,bool Ready);
    bool Say(int32 Player,int32 Code);
    bool Depart();
    bool Archive(const FFiskSettlementView& View,int32 NowMs);
private:
    UPROPERTY(ReplicatedUsing=OnRep_Plan) FFiskExpeditionPlan Plan;
    UFUNCTION() void OnRep_Plan();
    TArray<FGuid> Participants;
    TMap<FString,int32> Records;
    int32 RouteRevision=-1;
};
