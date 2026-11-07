#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "World/FiskPersonVisual.h"
#include "FiskCrewFigures.generated.h"
UCLASS()
class UFiskCrewFigures final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskCrewFigures();
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    TArray<FFiskPersonVisual> Figures;
    bool bReported=false;
};
