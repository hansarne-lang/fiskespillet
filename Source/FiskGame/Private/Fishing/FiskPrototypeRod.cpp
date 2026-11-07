#include "Fishing/FiskPrototypeRod.h"
#include "FiskFishingGround.h"
#include "DayCycle/FiskDayState.h"
#include "Boats/FiskStarterBoat.h"
#include "Boats/FiskBoardingController.h"
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
    const AFiskBoardingController* RodOwner=::Cast<AFiskBoardingController>(GetOwner());
    if (!GetOwner()->HasAuthority() || !RodOwner || Slot>=RodOwner->GetRodCapacity() || RodOwner->GetRodAt(Slot)!=this) { return false; }
    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    const AFiskBootstrapGameMode* Mode=GetWorld()->GetAuthGameMode<AFiskBootstrapGameMode>();
    const AFiskStarterBoat* Boat=Mode?Mode->GetStarterBoat():nullptr;
    const FFiskFishingGround* Ground=Boat?FiskGrounds::Find(Boat->GetGroundId()):nullptr;
    if (!Day || !Ground || Boat->GetCargo().bMustReturn || !FFiskRigRules::IsValid(Rig.Bait,Rig.Depth,Rig.Program,Ground->BottomDepth)
        || !Cycle.TryStart(ExpectedId, Day->GetElapsedMilliseconds())) { return false; }
    View = FFiskRodView();
    View.CastId = Cycle.GetId();
    CatchId.Invalidate();
    LandedAtMs = -1;
    FirstRunAtMs = 0;
    // CastId is visible to the owner. Catch entropy must never be derived from it.
    const FGuid PrivateEntropy=FGuid::NewGuid();
    CatchData=FiskCatches::Roll(static_cast<int32>(PrivateEntropy.A ^ PrivateEntropy.B ^ PrivateEntropy.C ^ PrivateEntropy.D),Boat->GetGroundId());
#if WITH_DEV_AUTOMATION_TESTS
    // Deterministic catalog fish for native cargo acceptance only. Bite clocks,
    // fight simulation, input, landing and settlement still run normally.
    if(RodOwner->CargoMode==TEXT("tow") || RodOwner->CargoMode==TEXT("late") || RodOwner->CargoMode==TEXT("full") || RodOwner->CargoMode==TEXT("discard"))
    { CatchData.Species=6;CatchData.WeightGrams=RodOwner->CargoMode==TEXT("discard")?250000:150000;const auto& S=*FiskCatches::Find(6);CatchData.LengthMm=FiskCatches::LengthForWeight(S,CatchData.WeightGrams);CatchData.Value=FiskCatches::ValueForWeight(S,CatchData.WeightGrams); }
#endif
    FRandomStream BiteRandom(static_cast<int32>(Cycle.GetId().A ^ Cycle.GetId().B ^ Cycle.GetId().C ^ Cycle.GetId().D));
    NextTransitionAt = Day->GetElapsedRealSeconds() + FFiskRigRules::BiteDelay(Rig, BiteRandom, Ground);
    SetState(EFiskRodState::Fishing);
    SetComponentTickEnabled(true);
    return true;
}

void UFiskPrototypeRod::ResetForDay()
{
    if (!GetOwner()->HasAuthority()) { return; }
    SetComponentTickEnabled(false); Cycle.ResetForDay(); Fight = FFiskFightModel();
    View = FFiskRodView(); View.CastId = Cycle.GetId();
    CatchId.Invalidate(); CatchData = FFiskCatchData(); LandedAtMs = -1;
    NextTransitionAt = LastFightAt = 0; ++Rig.Revision;
#if WITH_DEV_AUTOMATION_TESTS
    TestLandings = 0;
#endif
    OnRep_View(); GetOwner()->ForceNetUpdate();
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
    const AFiskStarterBoat* Boat=Mode->GetStarterBoat();if(!Boat) { return; }
    FFiskCatchContext Context;Context.Ground=Boat->GetGroundId();Context.Bait=Rig.Bait;Context.Depth=Rig.Depth;Context.Program=Rig.Program;
    Context.BoatKind=Boat->GetKind();const AFiskBoardingController* OwnerPC=::Cast<AFiskBoardingController>(GetOwner());Context.Equipment=OwnerPC?OwnerPC->GetServerCareer().Owned&7:0;
    Context.LandedAtMs=Day->GetElapsedMilliseconds();Context.FightMs=FMath::Max(0,Context.LandedAtMs-FirstRunAtMs);
    Context.LandedUtc=FDateTime::UtcNow().ToIso8601();
    Context.bHasSeaPoint=true;const FVector Point=Boat->GetDestination();
    Context.SeaPoint=FVector(FMath::RoundToDouble(Point.X),FMath::RoundToDouble(Point.Y),0);
    if (!Mode->GetSettlement()->RegisterCatch(CatchId, PC->PlayerState->GetPlayerId(),CatchData,Context)) { return; }
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
        FirstRunAtMs=Day->GetElapsedMilliseconds();
        const FFiskFightProfile Profile=FFiskFightProfile::ForSpecies(CatchData.Species);
        InitialRunDuration=3.0f*Profile.RunDuration;InitialRunGain=10.0f*Profile.RunSpeed*InitialRunDuration;
        NextTransitionAt = Now + InitialRunDuration;
        View.Line = Rig.Depth;
        SetState(EFiskRodState::InitialRun);
    }
    else if (View.State == EFiskRodState::InitialRun)
    {
        View.Line = Rig.Depth + FMath::Clamp(FMath::RoundToInt((Now-(NextTransitionAt-InitialRunDuration))*InitialRunGain/InitialRunDuration),0,FMath::RoundToInt(InitialRunGain));
        if (Now >= NextTransitionAt)
        {
            const AFiskBoardingController* OwnerPC=::Cast<AFiskBoardingController>(GetOwner());
            Fight.Reset(FMath::Rand(), Rig.Depth+InitialRunGain,CatchData.Species,OwnerPC?OwnerPC->GetServerCareer().Owned:0);
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
