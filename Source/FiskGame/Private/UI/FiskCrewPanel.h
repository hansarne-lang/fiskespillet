#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputKeyEventArgs.h"
#include "FiskCrewPanel.generated.h"
class AHUD;
class UCanvas;
UCLASS()
class UFiskCrewPanel final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskCrewPanel();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    bool IsVisible() const { return bVisible; }
    void SetMessage(const FString& Text) { Message=Text; }
    void Draw(AHUD& HUD,UCanvas& Canvas);
private:
    bool CanOpen() const;
    void Show(bool bShow);
    void Choose();
    bool bVisible=false;
    int32 Row=0;
    FString Message;
    TArray<FBox2D> Buttons;
#if WITH_DEV_AUTOMATION_TESTS
    void Smoke();
    void Key(FKey Value);
    void Capture(const FString& Name);
    void ChooseOther();
    FTimerHandle SmokeTimer;
    int32 Stage=0,InitialRevision=-1;
    double MarkAt=0,TripAt=-1,ReturnAt=-1;
    FString CaptureDir;
#endif
};
