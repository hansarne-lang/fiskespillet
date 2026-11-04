#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UI/FiskMenuModel.h"
#include "InputKeyEventArgs.h"
#include "FiskFrontEnd.generated.h"
class AHUD;
class UCanvas;
UCLASS()
class UFiskFrontEnd final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskFrontEnd();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    void Draw(AHUD& HUD, UCanvas& Canvas);
    void Click(FVector2D Position);
    bool IsVisible() const { return Model.Page != EFiskMenuPage::Closed; }
private:
    void Dispatch(EFiskMenuAction Action);
    void SyncInput();
    void CancelConnection(const FString& Message);
    FString Label(int32 Row) const;
    FFiskMenuModel Model;
    TArray<FBox2D> Buttons;
    FVector2D LastMouse = FVector2D(-1, -1);
    double ConnectingAt = 0.0;
    bool bControl = false, bShift = false, bReadyReported = false;
#if WITH_DEV_AUTOMATION_TESTS
    void RunMenuSmoke();
    void SmokeKey(FKey Key);
    void SmokeSelect(int32 Row);
    void SmokeAddress(const FString& Address);
    void Capture(const FString& Name);
    FString SmokeCase, ScreenshotDirectory;
    FTimerHandle SmokeTimer;
    double SmokeOpenedAt = 0.0;
#endif
};
