#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskFishPresentation.generated.h"

class UStaticMesh;
class UMaterialInterface;
class USceneComponent;

UCLASS()
class UFiskFishPresentation final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskFishPresentation();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    bool GetLineTarget(int32 Seat, FVector& Target) const;
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cone;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> FishRoots;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> TailRoots;
    bool Visible[2] = {false, false};
    bool NearReported[2] = {false, false};
    bool LandingReported[2] = {false, false};
    bool HiddenReported[2] = {false, false};
#if WITH_DEV_AUTOMATION_TESTS
    FString ScreenshotDirectory;
    bool bCapturedNear = false;
    bool bCapturedLanding = false;
    void CaptureTestPhase(int32 Seat, bool bLanding, double LandingAge);
#endif
};
