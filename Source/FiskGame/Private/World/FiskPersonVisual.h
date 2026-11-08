#pragma once
#include "CoreMinimal.h"
class AActor;class USceneComponent;class UStaticMesh;class UMaterialInterface;class UStaticMeshComponent;class UTextRenderComponent;
struct FFiskPersonVisual
{
    USceneComponent* Root=nullptr;
    UStaticMeshComponent* Upper[2]={nullptr,nullptr};
    UStaticMeshComponent* Lower[2]={nullptr,nullptr};
    UStaticMeshComponent* Legs[2]={nullptr,nullptr};
    UStaticMeshComponent* Feet[2]={nullptr,nullptr};
    TArray<UStaticMeshComponent*> Shirts;
    FLinearColor ShirtColor=FLinearColor::Black;
    void SetShirt(FLinearColor Color);
    UTextRenderComponent* Label=nullptr;
    void Build(AActor* Actor,USceneComponent* Parent,FName Name,UStaticMesh* Sphere,UStaticMesh* Cube,UStaticMesh* Cylinder,UMaterialInterface* Material,FLinearColor Shirt);
    void Pose(double Time,int32 Activity,bool Wave,const FString& Name,FVector Camera,float Walk=0,FVector CameraUp=FVector::UpVector);
};
