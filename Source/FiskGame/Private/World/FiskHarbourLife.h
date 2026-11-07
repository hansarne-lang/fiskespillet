#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "World/FiskPersonVisual.h"
#include "FiskGreetingClock.h"
#include "FiskHarbourLife.generated.h"
UCLASS()
class UFiskHarbourLife final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskHarbourLife();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    int32 GetVisiblePeople() const { return VisiblePeople; }
    int32 GetBuiltSpecies() const { return BuiltSpecies; }
    FLinearColor PersonColor(FGuid Profile) const;
private:
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TObjectPtr<USceneComponent> GalleryRoot;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> GalleryParts;
    TArray<FFiskPersonVisual> People;
    FFiskGreetingClock Greetings;
    int32 BuiltSpecies=-1,VisiblePeople=0,Generation=0;
    void BuildGallery(int32 Species);
};
