#include "DayCycle/FiskDayState.h"
#include "Net/UnrealNetwork.h"

void AFiskDayState::StartDay()
{
    if (!HasAuthority() || HasStarted())
    {
        return;
    }

    StartedAt = GetServerWorldTimeSeconds();
    OnRep_StartedAt();
    ForceNetUpdate();
}

int32 AFiskDayState::GetElapsedSeconds() const
{
    if (!HasStarted())
    {
        return 0;
    }

    return CalculateElapsedSeconds(GetServerWorldTimeSeconds(), StartedAt);
}

int32 AFiskDayState::CalculateElapsedSeconds(double ServerTime, double StartTime)
{
    return FMath::FloorToInt(FMath::Clamp(ServerTime - StartTime, 0.0, static_cast<double>(DayEnd)));
}

EFiskDayPhase AFiskDayState::GetPhase(int32 ElapsedSeconds)
{
    if (ElapsedSeconds >= DayEnd)
    {
        return EFiskDayPhase::Finished;
    }
    if (ElapsedSeconds >= SettlementStart)
    {
        return EFiskDayPhase::Settlement;
    }
    if (ElapsedSeconds >= PreparationEnd)
    {
        return EFiskDayPhase::FishingAndReturn;
    }
    return EFiskDayPhase::Preparation;
}

void AFiskDayState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDayState, StartedAt);
}

void AFiskDayState::OnRep_StartedAt()
{
    UE_LOG(LogTemp, Display, TEXT("FISKELAGET_DAY_STARTED at %.3f"), StartedAt);
}
