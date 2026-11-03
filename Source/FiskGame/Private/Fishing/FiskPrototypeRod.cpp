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

    BiteAt = GetWorld()->GetTimeSeconds() + FMath::FRandRange(120.0f, 180.0f);
    SetState(EFiskRodState::Fishing);
    SetComponentTickEnabled(true);
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
    else if (State == EFiskRodState::Fishing && GetWorld()->GetTimeSeconds() >= BiteAt)
    {
        SetState(EFiskRodState::Bite);
    }
}

void UFiskPrototypeRod::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UFiskPrototypeRod, State, COND_OwnerOnly);
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
    else if (State == EFiskRodState::Bite)
    {
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_ROD_BITE player=%d"), PlayerId);
    }
}
