#pragma once

#include "CoreMinimal.h"
class AActor;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

namespace FiskVisual
{
    UStaticMeshComponent* AddPart(AActor* Owner, USceneComponent* Parent, UStaticMesh* Mesh,
        UMaterialInterface* Material, FName Name, const FVector& Location, const FVector& Scale,
        const FLinearColor& Color, float Roughness = 0.4f);
    void SetCylinder(UStaticMeshComponent* Part, const FVector& Start, const FVector& End, float Radius);
}
