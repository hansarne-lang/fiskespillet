#include "Fishing/FiskPrototypeRod.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskDayState.h"
#include "Boats/FiskStarterBoat.h"
#include "Bootstrap/FiskBootstrapGameMode.h"
#include "Economy/FiskDaySettlement.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

UFiskPrototypeRod::UFiskPrototypeRod()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}


bool UFiskPrototypeRod::Cast(const FGuid& ExpectedId)
{
    if (!GetOwner()->HasAuthority()) { return false; }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat=Mode?Mode->GetStarterBoat():nullptr;
    const FFiskFishingGround* Ground=Boat?FiskGrounds::Find(Boat->GetGroundId()):nullptr;
    if (!Day || !Ground || !FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,Ground->BottomDepth)
        || !Cycle.TryStart(ExpectedId, Day->GetElapsedMilliseconds())) { return false; }
    View = FFiskRodView();
    View.CastId = Cycle.GetId();
    CatchId.Invalidate();
    LandedAtMs = -1;
    FRandomStream BiteRandom(static_cast<int32>(Cycle.GetId().A ^ Cycle.GetId().B ^ Cycle.GetId().C ^ Cycle.GetId().D));
    NextTransitionAt = Day->GetElapsedRealSeconds() + FFiskRigRules::BiteDelay(Rig, BiteRandom, Ground);
    SetState(EFiskRodState::Fishing);
    SetComponentTickEnabled(true);
    return true;
}

bool UFiskPrototypeRod::CanConfigure() const
{
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    return (View.State == EFiskRodState::Ready || View.State == EFiskRodState::Cancelled
        || View.State == EFiskRodState::Lost || View.State == EFiskRodState::Landed)
        && Day && Day->GetElapsedMilliseconds() >= View.AvailableAtMs;
}
bool UFiskPrototypeRod::Configure(const FGuid& ExpectedId, int32 ExpectedRevision, int32 Bait, int32 Depth, int32 Program)
{
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat=Mode?Mode->GetStarterBoat():nullptr;
    const FFiskFishingGround* Ground=Boat?FiskGrounds::Find(Boat->GetGroundId()):nullptr;
    if (!GetOwner()->HasAuthority() || !Ground || !CanConfigure() || ExpectedId != View.CastId
        || !FFiskRigRules::TryApply(Rig, ExpectedRevision, Bait, Depth, Program, Ground->BottomDepth)) { return false; }
    GetOwner()->ForceNetUpdate();
    return true;
}

void UFiskPrototypeRod::Withdraw()
{
    if (!GetOwner()->HasAuthority() || !Cycle.Withdraw(View.CastId)) { return; }
    const FGuid Id = View.CastId;
    View = FFiskRodView();
    View.CastId = Id;
    CatchId.Invalidate();
    SetState(EFiskRodState::Cancelled);
    SetComponentTickEnabled(false);
}

void UFiskPrototypeRod::Lift()
{
    if (GetOwner()->HasAuthority() && View.State == EFiskRodState::Fighting) { Fight.SetRaised(true); }
}
void UFiskPrototypeRod::Lower()
{
    if (GetOwner()->HasAuthority() && View.State == EFiskRodState::Fighting) { Fight.SetRaised(false); }
}
void UFiskPrototypeRod::Reel()
{
    if (GetOwner()->HasAuthority() && View.State == EFiskRodState::Fighting) { Fight.SetReeling(true); }
}
void UFiskPrototypeRod::StopReel()
{
    if (GetOwner()->HasAuthority() && View.State == EFiskRodState::Fighting) { Fight.SetReeling(false); }
}
void UFiskPrototypeRod::AdjustDrag(int32 Steps)
{
    if (GetOwner()->HasAuthority() && View.State == EFiskRodState::Fighting)
    {
        Fight.AdjustDrag(Steps);
        SyncFight();
        const APlayerController* PC = CastChecked<APlayerController>(GetOwner());
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_DRAG player=%d percent=%d"),
            PC->PlayerState ? PC->PlayerState->GetPlayerId() : INDEX_NONE, View.Drag);
    }
}

void UFiskPrototypeRod::SyncFight()
{
    View.Line = FMath::CeilToInt(Fight.GetLine());
    View.Load = Fight.GetLoadPercent();
    View.Drag = Fight.GetDragPercent();
    View.bSurging = Fight.IsSurging();
    View.bRaised = Fight.IsRaised();
    View.bReeling = Fight.IsReeling();
    OnRep_View();
}

void UFiskPrototypeRod::Land()
{
    if (!GetOwner()->HasAuthority() || View.State != EFiskRodState::ReadyToLand || !Cycle.CanAct(View.CastId)) { return; }
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const APlayerController* PC = CastChecked<APlayerController>(GetOwner());
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Mode || !Mode->GetSettlement() || !PC->PlayerState || !Day) { return; }
    if (!CatchId.IsValid()) { CatchId = FGuid::NewGuid(); }
    if (!Mode->GetSettlement()->RegisterCatch(CatchId, PC->PlayerState->GetPlayerId())) { return; }
    LandedAtMs = Day->GetElapsedMilliseconds();
    Cycle.FinishLanding(View.CastId, LandedAtMs);
    View.AvailableAtMs = Cycle.GetAvailableAtMs();
#if WITH_DEV_AUTOMATION_TESTS
    ++TestLandings;
#endif
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CATCH_RECORDED player=%d catch=%s cast=%s"),
        PC->PlayerState->GetPlayerId(), *CatchId.ToString(EGuidFormats::Digits), *View.CastId.ToString(EGuidFormats::Digits));
    SetState(EFiskRodState::Landed);
    SetComponentTickEnabled(false);
}

void UFiskPrototypeRod::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!GetOwner()->HasAuthority()) { return; }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const AFiskBootstrapGameMode* Mode = GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat = Mode ? Mode->GetStarterBoat() : nullptr;
    if (!Boat || Boat->GetTripProgress() != AFiskStarterBoat::AtFishingSpot
        || !Day || AFiskDayState::GetPhase(Day->GetElapsedSeconds()) != EFiskDayPhase::FishingAndReturn)
    {
        Cycle.Withdraw(View.CastId);
        View = FFiskRodView();
        View.CastId = Cycle.GetId();
        SetState(EFiskRodState::Ready);
        SetComponentTickEnabled(false);
        return;
    }
    const double Now = Day->GetElapsedRealSeconds();
    if (View.State == EFiskRodState::Fishing && Now >= NextTransitionAt)
    {
        NextTransitionAt = Now + 3.0;
        View.Line = Rig.Depth;
        SetState(EFiskRodState::InitialRun);
    }
    else if (View.State == EFiskRodState::InitialRun)
    {
        View.Line = Rig.Depth + FMath::Clamp(FMath::RoundToInt((Now - (NextTransitionAt - 3.0)) * 10), 0, 30);
        if (Now >= NextTransitionAt)
        {
            Fight.Reset(FMath::Rand(), Rig.Depth + 30.0f);
            LastFightAt = Now;
            SyncFight();
            SetState(EFiskRodState::Fighting);
        }
    }
    else if (View.State == EFiskRodState::Fighting)
    {
        Fight.Advance(Now - LastFightAt);
        LastFightAt = Now;
        SyncFight();
        switch (Fight.GetOutcome())
        {
        case EFiskFightOutcome::ReadyToLand:
            View.bSurging = View.bReeling = View.bRaised = false;
            SetState(EFiskRodState::ReadyToLand);
            break;
        case EFiskFightOutcome::LineBreak:
        case EFiskFightOutcome::Slack:
            Cycle.Withdraw(View.CastId);
            View.Loss = Fight.GetOutcome() == EFiskFightOutcome::LineBreak ? EFiskRodLoss::LineBreak : EFiskRodLoss::Slack;
            View.bSurging = View.bReeling = View.bRaised = false;
            SetState(EFiskRodState::Lost);
            SetComponentTickEnabled(false);
            break;
        default: break;
        }
    }
}

void UFiskPrototypeRod::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UFiskPrototypeRod, View, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UFiskPrototypeRod, Rig, COND_OwnerOnly);
}
void UFiskPrototypeRod::SetState(EFiskRodState NewState)
{
    View.State = NewState;
    OnRep_View();
    GetOwner()->ForceNetUpdate();
}
void UFiskPrototypeRod::OnRep_View()
{
    const APlayerController* PC = CastChecked<APlayerController>(GetOwner());
    const int32 Id = PC->PlayerState ? PC->PlayerState->GetPlayerId() : INDEX_NONE;
    if (View.bSurging && !bPreviousSurge) { UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_RUN player=%d"), Id); }
    bPreviousSurge = View.bSurging;
    if (View.State == PreviousState && View.CastId == PreviousCastId) { return; }
    PreviousState = View.State;
    PreviousCastId = View.CastId;
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_CYCLE player=%d cast=%s state=%d"),
        Id, *View.CastId.ToString(EGuidFormats::Digits), static_cast<int32>(View.State));
    switch (View.State)
    {
    case EFiskRodState::Cancelled: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_CANCELLED player=%d"), Id); break;
    case EFiskRodState::Fishing: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_FISHING player=%d"), Id); break;
    case EFiskRodState::InitialRun: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_BITE player=%d"), Id); break;
    case EFiskRodState::Fighting: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FIGHT_STARTED player=%d"), Id); break;
    case EFiskRodState::ReadyToLand: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_READY player=%d"), Id); break;
    case EFiskRodState::Landed: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_LANDED player=%d"), Id); break;
    case EFiskRodState::Lost: UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_LOST player=%d reason=%s"), Id,
        View.Loss == EFiskRodLoss::LineBreak ? TEXT("LineBreak") : TEXT("Slack")); break;
    default: break;
    }
}
