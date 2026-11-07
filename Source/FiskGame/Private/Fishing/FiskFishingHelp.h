#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Fishing/FiskTestPilot.h"
#include "FiskFishingHelp.generated.h"
class AHUD; class UCanvas; struct FInputKeyEventArgs;

UCLASS()
class UFiskFishingHelp final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskFishingHelp();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;
    bool HandleKey(const FInputKeyEventArgs& Args);
    void ToggleAssist();
    void StopAssist(bool Notify=false);
    void RequestReturn();
    void Draw(AHUD& HUD,UCanvas& Canvas) const;
    bool IsAssisting() const { return bAssisting; }
    int32 GetTargetSlot() const { return TargetSlot; }
    bool HasGameView() const;
private:
    bool bAssisting=false,bWaitingSelection=false,bButtonHeld=false,bManagedCursor=false;
    int32 TargetSlot=-1;
    FGuid TargetCast;
    FFiskTestPilot Pilot;
    double NextControlAt=0,SelectionUntil=0;
#if WITH_DEV_AUTOMATION_TESTS
    FString SmokeMode,SmokeDir;FTimerHandle SmokeTimer;
    int32 SmokeStage=0,SmokeSlot=0;double SmokeAt=0,SmokeReturnAt=0;
    int64 SmokeCash=0;bool bSmokeMute=false,bSmokeShot=false;
    void RunSmoke();
#endif
};
