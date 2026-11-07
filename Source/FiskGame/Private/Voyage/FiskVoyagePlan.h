#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiskVoyagePlan.generated.h"
USTRUCT()
struct FFiskVoyageRoute
{
    GENERATED_BODY()
    UPROPERTY() TArray<FVector> Points;
    UPROPERTY() int32 Revision=0;
    UPROPERTY() int32 Day=1;
    UPROPERTY() int32 Next=0;
    UPROPERTY() int32 Active=-1;
    static bool Valid(const TArray<FVector>& Points,bool bHistorical=false);
    bool Matches(int32 ExpectedDay,int32 ExpectedRevision) const;
};
class AFiskStarterBoat;
UCLASS()
class AFiskVoyagePlan final : public AActor
{
    GENERATED_BODY()
public:
    AFiskVoyagePlan();
    const FFiskVoyageRoute& GetRoute() const { return Route; }
    bool Replace(int32 Player,int32 Day,int32 Revision,const TArray<FVector>& Points,FString& Message);
    bool NextStop(int32 Player,int32 Day,int32 Revision,int32 BoatRevision,FString& Message);
    void Refresh();
    void InitializeRevision(int32 Base) { Route.Revision=Base; }
    virtual void Tick(float Delta) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
private:
    UPROPERTY(ReplicatedUsing=OnRep_Route) FFiskVoyageRoute Route;
    UFUNCTION() void OnRep_Route();
};
