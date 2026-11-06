#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiskMarinaTypes.h"
#include "FiskMarinaHub.generated.h"
class UStaticMesh;
class UMaterialInterface;
class UStaticMeshComponent;
class UTextRenderComponent;
UCLASS()
class AFiskMarinaHub final : public AActor
{
    GENERATED_BODY()
public:
    AFiskMarinaHub();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& Out) const override;
    virtual void CalcCamera(float DeltaTime,FMinimalViewInfo& Out) override;
    bool Upsert(const FGuid& Profile,int32 Player,const FFiskMarinaIdentity& Identity,const TArray<FFiskMarinaTrophy>& Trophies);
    bool Say(int32 Player,int32 Code,int32 AtMs);
    const TArray<FFiskMarinaResident>& GetResidents() const { return Residents; }
    const TArray<FFiskMarinaMessage>& GetMessages() const { return Messages; }
    static const TCHAR* Phrase(int32 Code);
    static FVector HousePosition(int32 Slot) { return FVector(-3000-1700*FMath::Clamp(Slot,0,3),4500,0); }
private:
    UPROPERTY(ReplicatedUsing=OnRep_Residents) TArray<FFiskMarinaResident> Residents;
    UPROPERTY(Replicated) TArray<FFiskMarinaMessage> Messages;
    UPROPERTY() TObjectPtr<UStaticMesh> Cube;
    UPROPERTY() TObjectPtr<UStaticMesh> Sphere;
    UPROPERTY() TObjectPtr<UStaticMesh> Cylinder;
    UPROPERTY() TObjectPtr<UMaterialInterface> Material;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    UPROPERTY() TArray<TObjectPtr<UTextRenderComponent>> Signs;
    int32 BuildRevision=0,MessageSerial=0;
    UFUNCTION() void OnRep_Residents();
    void Rebuild();
};
