#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Fishing/FiskFightModel.h"
#include "Fishing/FiskRodCycle.h"
#include "Fishing/FiskRigSettings.h"
#include "Fishing/FiskCatchCatalog.h"
#include "FiskPrototypeRod.generated.h"

UENUM()
enum class EFiskRodState : uint8 { Ready, Fishing, InitialRun, Fighting, ReadyToLand, Landed, Lost, Cancelled };
UENUM()
enum class EFiskRodLoss : uint8 { None, LineBreak, Slack };

// One owner-only snapshot avoids observing a loss before its reason arrives.
USTRUCT()
struct FFiskRodView
{
    GENERATED_BODY()
    UPROPERTY() EFiskRodState State = EFiskRodState::Ready;
    UPROPERTY() EFiskRodLoss Loss = EFiskRodLoss::None;
    UPROPERTY() FGuid CastId;
    UPROPERTY() int32 AvailableAtMs = 0;
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
    bool Cast(const FGuid& ExpectedId);
    void Withdraw();
    const FFiskRigSettings& GetRig() const { return Rig; }
    bool CanConfigure() const;
    bool Configure(const FGuid& ExpectedId, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program);
    bool IsCurrentCommand(const FGuid& ExpectedId) const { return Cycle.CanAct(ExpectedId); }
    FGuid GetCastId() const { return View.CastId; }
    int32 GetAvailableAtMs() const { return View.AvailableAtMs; }
#if WITH_DEV_AUTOMATION_TESTS
    int32 GetTestLandingCount() const { return TestLandings; }
#endif
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
    UPROPERTY(Replicated) FFiskRigSettings Rig;
    UFUNCTION() void OnRep_View();
    void SetState(EFiskRodState NewState);
    void SyncFight();
    FFiskRodCycle Cycle;
    FFiskFightModel Fight;
    FGuid PreviousCastId;
#if WITH_DEV_AUTOMATION_TESTS
    int32 TestLandings = 0;
#endif
    EFiskRodState PreviousState = EFiskRodState::Ready;
    bool bPreviousSurge = false;
    double NextTransitionAt = 0.0, LastFightAt = 0.0;
    FGuid CatchId;
    // Never replicated or exposed through the owner-only rod view.
    FFiskCatchData CatchData;
    int32 LandedAtMs = -1;
};
