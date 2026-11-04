#include "DayCycle/FiskDayState.h"
#include "HAL/PlatformTime.h"
#include "Net/UnrealNetwork.h"

AFiskDayState::AFiskDayState()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void AFiskDayState::StartDay()
{
    if (!HasAuthority() || HasStarted()) { return; }
    StartedMonotonic = FPlatformTime::Seconds();
    ElapsedMilliseconds = 0;
    OnRep_Clock();
    ForceNetUpdate();
}

int32 AFiskDayState::GetElapsedMilliseconds() const
{
    if (!HasStarted()) { return 0; }
    if (!HasAuthority()) { return ElapsedMilliseconds; }
    const double Delta = FMath::Clamp(FPlatformTime::Seconds() - StartedMonotonic, 0.0, static_cast<double>(DayEnd));
    return FMath::FloorToInt(Delta * 1000.0);
}

void AFiskDayState::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HasAuthority() || !HasStarted()) { return; }
    ElapsedMilliseconds = GetElapsedMilliseconds();
    const int32 Minute = ElapsedMilliseconds / 60000;
    if (Minute != LastMinute)
    {
        LastMinute = Minute;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_CLOCK ms=%d"), ElapsedMilliseconds);
    }
}

int32 AFiskDayState::CalculateElapsedSeconds(double ServerTime, double StartTime)
{
    return FMath::FloorToInt(FMath::Clamp(ServerTime - StartTime, 0.0, static_cast<double>(DayEnd)));
}

EFiskDayPhase AFiskDayState::GetPhase(int32 ElapsedSeconds)
{
    if (ElapsedSeconds >= DayEnd) { return EFiskDayPhase::Finished; }
    if (ElapsedSeconds >= SettlementStart) { return EFiskDayPhase::Settlement; }
    if (ElapsedSeconds >= PreparationEnd) { return EFiskDayPhase::FishingAndReturn; }
    return EFiskDayPhase::Preparation;
}

void AFiskDayState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFiskDayState, ElapsedMilliseconds);
}

void AFiskDayState::OnRep_Clock()
{
    if (HasStarted() && !bReportedStart)
    {
        bReportedStart = true;
        UE_LOG(LogTemp, Display, TEXT("FISKELAGET_DAY_STARTED ms=%d"), ElapsedMilliseconds);
    }
}
