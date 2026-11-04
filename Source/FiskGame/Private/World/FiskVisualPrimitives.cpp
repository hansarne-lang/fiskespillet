#include "World/FiskVisualPrimitives.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"

UStaticMeshComponent* FiskVisual::AddPart(AActor* Owner, USceneComponent* Parent, UStaticMesh* Mesh,
    UMaterialInterface* Material, FName Name, const FVector& Location, const FVector& Scale,
    const FLinearColor& Color, float Roughness)
{
    UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(Owner, Name);
    Owner->AddInstanceComponent(Part);
    Part->SetupAttachment(Parent);
    Part->SetMobility(EComponentMobility::Movable);
    Part->SetStaticMesh(Mesh);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Part->SetRelativeLocation(Location);
    Part->SetRelativeScale3D(Scale);
    if (Material)
    {
        UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(Material, Part);
        Tint->SetVectorParameterValue(TEXT("Color"), Color);
        Tint->SetScalarParameterValue(TEXT("Roughness"), Roughness);
        Part->SetMaterial(0, Tint);
    }
    Part->RegisterComponent();
    return Part;
}

void FiskVisual::SetCylinder(UStaticMeshComponent* Part, const FVector& Start, const FVector& End, float Radius)
{
    const FVector Direction = End - Start;
    Part->SetRelativeLocation((Start + End) * 0.5);
    Part->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector, Direction.GetSafeNormal()));
    Part->SetRelativeScale3D(FVector(Radius / 50.0f, Radius / 50.0f, Direction.Length() / 100.0));
}
