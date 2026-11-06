#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiskMarinaZone.generated.h"

UCLASS()
class AFiskMarinaZone final : public AActor
{
    GENERATED_BODY()
public:
    AFiskMarinaZone();
    virtual void Tick(float DeltaSeconds) override;
};
