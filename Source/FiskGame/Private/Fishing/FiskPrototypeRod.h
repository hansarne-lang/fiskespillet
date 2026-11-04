#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Fishing/FiskFightModel.h"
#include "FiskPrototypeRod.generated.h"

UENUM()
enum class EFiskRodState : uint8 { Ready, Fishing, InitialRun, Fighting, ReadyToLand, Landed, Lost };
UENUM()
enum class EFiskRodLoss : uint8 { None, LineBreak, Slack };

// One owner-only snapshot avoids observing a loss before its reason arrives.
USTRUCT()
struct FFiskRodView
{
    GENERATED_BODY()
    UPROPERTY() EFiskRodState State = EFiskRodState::Ready;
    UPROPERTY() EFiskRodLoss Loss = EFiskRodLoss::None;
    UPROPERTY() int32 Line = 0;
    UPROPERTY() int32 Load = 0;
    UPROPERTY() int32 Drag = 40;
    UPROPERTY() bool bSurging = false;
    UPROPERTY() bool bRaised = false;
    UPROPERTY() bool bReeling = false;
};

UCLASS()
class UFiskPrototypeRod final : public UActorComponent
{
    GENERATED_BODY()
public:
    UFiskPrototypeRod();
    void Cast();
    void Lift();
    void Lower();
    void Reel();
    void StopReel();
    void AdjustDrag(int32 Steps);
    void Land();
    int32 GetLineOut() const { return View.Line; }
    int32 GetLoadPercent() const { return View.Load; }
    int32 GetDragPercent() const { return View.Drag; }
    bool IsSurging() const { return View.bSurging; }
    bool IsRaised() const { return View.bRaised; }
    bool IsReeling() const { return View.bReeling; }
    int32 GetLandedAtMs() const { return LandedAtMs; }
    EFiskRodState GetState() const { return View.State; }
    EFiskRodLoss GetLoss() const { return View.Loss; }
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
    UPROPERTY(ReplicatedUsing = OnRep_View) FFiskRodView View;
    UFUNCTION() void OnRep_View();
    void SetState(EFiskRodState NewState);
    void SyncFight();
    FFiskFightModel Fight;
    EFiskRodState PreviousState = EFiskRodState::Ready;
    bool bPreviousSurge = false;
    double NextTransitionAt = 0.0, LastFightAt = 0.0;
    FGuid CatchId;
    int32 LandedAtMs = -1;
};
