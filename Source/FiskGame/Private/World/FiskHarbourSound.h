#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskHarbourSound.generated.h"
namespace FiskHarbourAudio { float Sample(uint64 Offset,bool Music); }
class USoundWaveProcedural;class UAudioComponent;
UCLASS()
class UFiskHarbourSound final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskHarbourSound();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool HasConsumedAudio() const { return bReported; }
private:
    UPROPERTY() TObjectPtr<USoundWaveProcedural> Wave;
    UPROPERTY() TObjectPtr<UAudioComponent> Audio;
    uint64 Offset=0,Queued=0;
    bool bReported=false,bMusic=true;
};
