#include "Fishing/FiskPrototypeRod.h"
#include "DayCycle/FiskDayState.h"
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

void UFiskPrototypeRod::Cast()
{
    if (!GetOwner()->HasAuthority() || State != EFiskRodState::Ready)
    {
        return;
    }

    NextTransitionAt = GetWorld()->GetTimeSeconds() + FMath::FRandRange(120.0f, 180.0f);
    SetState(EFiskRodState::Fishing);
    SetComponentTickEnabled(true);
}

void UFiskPrototypeRod::Lift()
{
    if (GetOwner()->HasAuthority() && State == EFiskRodState::Fighting)
    {
        bRodRaised = true;
        bCanReel = false;
    }
}

void UFiskPrototypeRod::Lower()
{
    if (GetOwner()->HasAuthority() && State == EFiskRodState::Fighting && bRodRaised)
    {
        bRodRaised = false;
        bCanReel = true;
    }
}

void UFiskPrototypeRod::Reel()
{
    if (!GetOwner()->HasAuthority() || State != EFiskRodState::Fighting || !bCanReel)
    {
        return;
    }

    bCanReel = false;
    LineOut = FMath::Max(0, LineOut - 10);
    if (LineOut == 0)
    {
        SetState(EFiskRodState::ReadyToLand);
    }
    else
    {
        GetOwner()->ForceNetUpdate();
    }
}

void UFiskPrototypeRod::Land()
{
    if (GetOwner()->HasAuthority() && State == EFiskRodState::ReadyToLand)
    {
        SetState(EFiskRodState::Landed);
        SetComponentTickEnabled(false);
    }
}

void UFiskPrototypeRod::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    const AFiskDayState* Day = GetWorld()->GetGameState<AFiskDayState>();
    if (!Day || AFiskDayState::GetPhase(Day->GetElapsedSeconds()) != EFiskDayPhase::FishingAndReturn)
    {
        SetState(EFiskRodState::Ready);
        SetComponentTickEnabled(false);
    }
    else if (State == EFiskRodState::Fishing && GetWorld()->GetTimeSeconds() >= NextTransitionAt)
    {
        NextTransitionAt = GetWorld()->GetTimeSeconds() + 3.0;
        SetState(EFiskRodState::InitialRun);
    }
    else if (State == EFiskRodState::InitialRun && GetWorld()->GetTimeSeconds() >= NextTransitionAt)
    {
        LineOut = 30;
        SetState(EFiskRodState::Fighting);
    }
}

void UFiskPrototypeRod::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UFiskPrototypeRod, State, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(UFiskPrototypeRod, LineOut, COND_OwnerOnly);
}

void UFiskPrototypeRod::SetState(EFiskRodState NewState)
{
    State = NewState;
    OnRep_State();
    GetOwner()->ForceNetUpdate();
}

void UFiskPrototypeRod::OnRep_State()
{
    const APlayerController* Controller = CastChecked<APlayerController>(GetOwner());
    const int32 PlayerId = Controller->PlayerState ? Controller->PlayerState->GetPlayerId() : INDEX_NONE;
    if (State == EFiskRodState::Fishing)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_FISHING player=%d"), PlayerId);
    }
    else if (State == EFiskRodState::InitialRun)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_BITE player=%d"), PlayerId);
    }
    else if (State == EFiskRodState::Fighting)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FIGHT_STARTED player=%d"), PlayerId);
    }
    else if (State == EFiskRodState::ReadyToLand)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_READY player=%d"), PlayerId);
    }
    else if (State == EFiskRodState::Landed)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_FISH_LANDED player=%d"), PlayerId);
    }
}
