#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "FiskRigPanel.generated.h"
class AHUD;
class UCanvas;
UCLASS()
class UFiskRigPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskRigPanel();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    bool IsVisible() const { return bVisible; }
    void Draw(AHUD& HUD, UCanvas& Canvas);
    void RigReply(bool bAccepted);
    void ResetForDay() { PendingRevision = -1; Message.Reset(); if(bVisible) { Show(false); } }
private:
    void Show(bool bShow);
    void Change(int32 Direction);
    bool bVisible = false;
    int32 Row = 0;
    int32 PresetSlot=0,PendingRevision=-1;
    double PendingAt=0;
    FString Message;
    TArray<FBox2D> Buttons;
#if WITH_DEV_AUTOMATION_TESTS
    void Smoke();
    void Key(FKey Value);
    void Capture(const FString& Name);
    FTimerHandle SmokeTimer;
    int32 Stage = 0;
    double MarkAt = 0;
    FGuid FirstCast;
    FString CaptureDir;
#endif
};
