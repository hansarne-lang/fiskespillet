#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiskOceanScene.generated.h"

class UProceduralMeshComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;
class UDirectionalLightComponent;
class UExponentialHeightFogComponent;

UCLASS()
class AFiskOceanScene final : public AActor
{
    GENERATED_BODY()
public:
    AFiskOceanScene();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<UProceduralMeshComponent> WaveSurface;
    UPROPERTY() TObjectPtr<UDirectionalLightComponent> Daylight;
    UPROPERTY() TObjectPtr<UExponentialHeightFogComponent> Fog;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Rain;
    int32 ReportedWeather=-1;
    UPROPERTY() TObjectPtr<UStaticMesh> Plane;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Birds;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Flyers;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Wake;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Traffic;
};
