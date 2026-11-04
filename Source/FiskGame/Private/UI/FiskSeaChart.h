#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "FiskSeaChart.generated.h"
class AHUD;
class UCanvas;
UCLASS()
class UFiskSeaChart final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskSeaChart();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime,ELevelTick TickType,FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    bool IsVisible() const { return bVisible; }
    void Draw(AHUD& HUD,UCanvas& Canvas);
    void SetMessage(const FString& Text) { Message=Text; }
private:
    bool CanOpen() const;
    void Show(bool bShow);
    void Choose();
    bool bVisible=false;
    int32 Row=0;
    FString Message;
    TArray<FBox2D> Buttons,Points;
#if WITH_DEV_AUTOMATION_TESTS
    void Smoke();
    void Key(FKey Value);
    void Capture(const FString& Name);
    FTimerHandle SmokeTimer;
    int32 Stage=0;
    double MarkAt=0,TravelAt=0,ReturnAt=-1;
    FString CaptureDir;
#endif
};
