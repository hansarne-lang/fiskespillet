#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/EngineBaseTypes.h"
#include "FiskGameInstance.generated.h"
class UNetDriver;
UCLASS()
class UFiskGameInstance final : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    void ApplySettings(bool bSave);
    bool bSound = true;
    int32 FrameRate = 60;
    FString ConnectionMessage;
#if WITH_DEV_AUTOMATION_TESTS
    int32 MenuSmokeStep = 0;
    double SmokeAt = 0.0;
    int32 SmokeClockAt = 0;
#endif
private:
    void NetworkFailure(UWorld* World, UNetDriver* Driver, ENetworkFailure::Type Type, const FString& Error);
    void TravelFailure(UWorld* World, ETravelFailure::Type Type, const FString& Error);
};
