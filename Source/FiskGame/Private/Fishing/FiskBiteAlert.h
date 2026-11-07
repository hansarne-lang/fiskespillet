#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskBiteAlert.generated.h"
class UAudioComponent; class USoundWaveProcedural;
namespace FiskBiteSound
{
    constexpr int32 SampleRate=22050;
    constexpr double Duration=.75;
    int16 Sample(uint64 Offset,int32 Slot);
}
UCLASS()
class UFiskBiteAlert final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskBiteAlert();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    int32 GetAlertCount() const { return AlertCount; }
    int32 GetConsumedCount() const { return ConsumedCount; }
private:
    struct FPulse { uint64 Offset=0;int32 Slot=0; };
    struct FPlayback { uint64 Marker=0;int32 Slot=0;FGuid Cast; };
    UPROPERTY() TObjectPtr<USoundWaveProcedural> Wave;
    UPROPERTY() TObjectPtr<UAudioComponent> Audio;
    TArray<FPulse> Pulses;
    TArray<FPlayback> PendingPlayback;
    FGuid Seen[3];
    uint64 QueuedBytes=0;
    int32 AlertCount=0,ConsumedCount=0;
};
