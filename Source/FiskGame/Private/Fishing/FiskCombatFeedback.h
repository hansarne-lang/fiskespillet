#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskCombatFeedback.generated.h"
class UAudioComponent;
class USoundWaveProcedural;

namespace FiskReelSound
{
    constexpr int32 SampleRate = 22050;
    void Fill(TArray<int16>& Samples, int32 Count, bool bActive, bool bRun, int32 Load, uint64& Offset);
}

UCLASS()
class UFiskCombatFeedback final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskCombatFeedback();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    UPROPERTY() TObjectPtr<USoundWaveProcedural> Wave;
    UPROPERTY() TObjectPtr<UAudioComponent> Audio;
    uint64 SampleOffset = 0, QueuedBytes = 0;
    bool bReportedPlayback = false;
};
