#pragma once
#include "CoreMinimal.h"
class AActor;class USceneComponent;class UProceduralMeshComponent;class UMaterialInterface;
struct FFiskMarineMesh
{
    TArray<FVector> Vertices,Normals;
    TArray<FVector2D> UV;
    TArray<int32> Triangles;
    bool Valid() const;
    void Finish();
};
namespace FiskMarine
{
    FFiskMarineMesh Hull();
    FFiskMarineMesh Deck();
    FFiskMarineMesh Gunwale();
    FFiskMarineMesh Fish();
    double Wave(double X,double Y,double Time,double Strength);
    FFiskMarineMesh Water(double Time,double Strength);
    UProceduralMeshComponent* Add(AActor* Owner,USceneComponent* Parent,FName Name,const FFiskMarineMesh& Mesh,UMaterialInterface* Material,FLinearColor Color,float Roughness=.4f);
    void UpdateWater(UProceduralMeshComponent* Component,double Time,double Strength);
}
