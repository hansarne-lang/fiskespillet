#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FiskBoatPresentation.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UProceduralMeshComponent;

UCLASS()
class UFiskBoatPresentation final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskBoatPresentation();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> RodSections;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Wake;
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> TowFish;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> TowRope;
    FVector SmoothedWorldPosition = FVector::ZeroVector;
#if WITH_DEV_AUTOMATION_TESTS
    FString ScreenshotDirectory;
    TSet<int32> CapturedStages;
    void CaptureTestStage();
#endif
};
