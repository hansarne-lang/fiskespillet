#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiskOceanScene.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class AFiskOceanScene final : public AActor
{
    GENERATED_BODY()
public:
    AFiskOceanScene();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Plane;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TObjectPtr<UMaterialInterface> WaterMaterial;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Ripples;
};
