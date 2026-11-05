#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskJourneyAudio.generated.h"
class USoundWaveProcedural;
class UAudioComponent;
UCLASS()
class UFiskJourneyAudio final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskJourneyAudio();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
private:
    UPROPERTY() TObjectPtr<USoundWaveProcedural> Wave;
    UPROPERTY() TObjectPtr<UAudioComponent> Audio;
    uint64 Offset=0,Queued=0;
    uint32 Noise=1234567;
    bool bReported=false;
};
